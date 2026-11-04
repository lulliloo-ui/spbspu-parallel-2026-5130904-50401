#include "child.hpp"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <functional>
#include <stdexcept>
#include <exception>
#include "file_descriptor.hpp"
#include "message.hpp"
#include "parser.hpp"
#include "circle.hpp"

namespace
{
  struct ThreadData
  {
    const std::vector< madieva::Circle > *circles = nullptr;
    madieva::frame_t frame;
    std::size_t tries = 0;
    std::size_t seed = 0;
    std::size_t hits = 0;
  };

  bool isInside(const double x, const double y, const madieva::Circle &c)
  {
    const double dx = static_cast< double >(c.getX()) - x;
    const double dy = static_cast< double >(c.getY()) - y;
    const double r = static_cast< double >(c.getRadius());
    return (dx * dx + dy * dy) <= (r * r);
  }

  void computeThread(ThreadData &d)
  {
    std::default_random_engine engine(d.seed);
    const double range = static_cast< double >(engine.max() - engine.min()) + 1.0;
    const double min_x = static_cast< double >(d.frame.min_x);
    const double min_y = static_cast< double >(d.frame.min_y);
    const double width = static_cast< double >(d.frame.max_x - d.frame.min_x);
    const double height = static_cast< double >(d.frame.max_y - d.frame.min_y);

    std::size_t hits = 0;
    for (std::size_t i = 0; i < d.tries; ++i)
    {
      const double x = min_x + width * static_cast< double >(engine() - engine.min()) / range;
      const double y = min_y + height * static_cast< double >(engine() - engine.min()) / range;
      for (std::size_t j = 0; j < d.circles->size(); ++j)
      {
        if (isInside(x, y, (*d.circles)[j]))
        {
          ++hits;
          break;
        }
      }
    }
    d.hits = hits;
  }

  double computeArea(const madieva::AreaRequest &req, long long seed)
  {
    if (req.circles.empty())
    {
      return 0.0;
    }

    madieva::frame_t frame = computeFrame(req.circles[0]);
    for (std::size_t i = 1; i < req.circles.size(); ++i)
    {
      const madieva::Circle &c = req.circles[i];
      frame.min_x = std::min(frame.min_x, c.getX() - c.getRadius());
      frame.min_y = std::min(frame.min_y, c.getY() - c.getRadius());
      frame.max_x = std::max(frame.max_x, c.getX() + c.getRadius());
      frame.max_y = std::max(frame.max_y, c.getY() + c.getRadius());
    }

    const std::size_t total = static_cast< std::size_t >(req.tries);
    const std::size_t nthreads = static_cast< std::size_t >(req.threads);
    const std::size_t per_thread = total / nthreads;
    const std::size_t remainder = total % nthreads;

    std::vector< ThreadData > data(req.threads);
    std::vector< std::thread > threads;
    threads.reserve(req.threads);

    for (std::size_t i = 0; i < nthreads; ++i)
    {
      data[i].circles = &req.circles;
      data[i].frame = frame;
      data[i].tries = per_thread + (i == 0 ? remainder : 0);
      data[i].seed = static_cast< std::size_t >(seed) + i;
      data[i].hits = 0;
      threads.emplace_back(computeThread, std::ref(data[i]));
    }

    std::size_t total_hits = 0;
    for (std::size_t i = 0; i < nthreads; ++i)
    {
      threads[i].join();
      total_hits += data[i].hits;
    }

    const double frame_area = static_cast< double >((frame.max_x - frame.min_x) * (frame.max_y - frame.min_y));
    return frame_area * static_cast< double >(total_hits) / static_cast< double >(total);
  }
}

void madieva::runChild(int cmd_fd, int res_fd, long long seed)
{
  const madieva::FileDescriptor cmd_rd(cmd_fd);
  const madieva::FileDescriptor res_wr(res_fd);

  while (true)
  {
    std::size_t size = 0;
    try
    {
      if (!madieva::recvAll(cmd_rd.get(), reinterpret_cast< char * >(&size), sizeof(size)))
      {
        break;
      }
      std::string msg(size, '\0');
      if (!madieva::recvAll(cmd_rd.get(), &msg[0], msg.size()))
      {
        throw std::runtime_error("unexpected EOF in message body");
      }

      const madieva::Args args = madieva::split(msg);
      const madieva::AreaRequest req = madieva::parseAreaRequest(args);

      const auto start = std::chrono::steady_clock::now();
      const double area = computeArea(req, seed);
      const auto end = std::chrono::steady_clock::now();
      const long long ms = std::chrono::duration_cast< std::chrono::milliseconds >(end - start).count();

      const std::string response = madieva::makeAreaResponse(req.calc_name, ms, area);
      const std::size_t rsize = response.size();
      madieva::sendAll(res_wr.get(), reinterpret_cast< const char * >(&rsize), sizeof(rsize));
      madieva::sendAll(res_wr.get(), response.data(), response.size());
    }
    catch (const std::exception &e)
    {
      std::cerr << "child error: " << e.what() << "\n";
    }
  }
}
