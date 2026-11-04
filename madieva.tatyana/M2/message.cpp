#include "message.hpp"
#include <string>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <cerrno>
#include "parser.hpp"
#include "circle.hpp"
#include "state.hpp"

std::string madieva::makeAreaRequest(
    const std::string & calc_name, long long threads, long long tries, const std::vector< const Circle * > & figures)
{
  std::string message;
  message += calc_name;
  message += ' ';
  message += std::to_string(threads);
  message += ' ';
  message += std::to_string(tries);
  message += ' ';
  message += std::to_string(figures.size());
  for (std::size_t i = 0; i < figures.size(); ++i)
  {
    const Circle * const circle = figures[i];
    message += ' ';
    message += std::to_string(circle->getRadius());
    message += ' ';
    message += std::to_string(circle->getX());
    message += ' ';
    message += std::to_string(circle->getY());
  }
  return message;
}

madieva::AreaRequest madieva::parseAreaRequest(const madieva::Args & args)
{
  if (args.size() < 4)
  {
    throw std::invalid_argument("area request: too few fields");
  }

  AreaRequest req;
  req.calc_name = args[0];
  checkName(req.calc_name);
  req.threads = parseInteger(args[1]);
  req.tries = parseInteger(args[2]);
  const long long count = parseInteger(args[3]);

  if (req.threads <= 0 || req.tries <= 0 || count < 0)
  {
    throw std::invalid_argument("area request: invalid parameters");
  }

  const std::size_t expected = 4 + 3 * static_cast< std::size_t >(count);
  if (args.size() != expected)
  {
    throw std::invalid_argument("area request: wrong number of circle fields");
  }

  req.circles.reserve(static_cast< std::size_t >(count));
  for (std::size_t i = 0; i < static_cast< std::size_t >(count); ++i)
  {
    const long long r = parseInteger(args[4 + 3 * i + 0]);
    const long long x = parseInteger(args[4 + 3 * i + 1]);
    const long long y = parseInteger(args[4 + 3 * i + 2]);
    req.circles.emplace_back(r, x, y);
  }
  return req;
}

std::string madieva::makeAreaResponse(const std::string & calc_name, long long ms, double area)
{
  std::string message;
  message += calc_name;
  message += ' ';
  message += std::to_string(ms);
  message += ' ';
  message += std::to_string(area);
  return message;
}

void madieva::applyAreaResponse(state_t & state, const Args & args)
{
  if (args.size() < 3)
  {
    throw std::runtime_error("bad area response");
  }
  const std::string & calc_name = args[0];
  const auto it = state.calc.find(calc_name);
  if (it == state.calc.end())
  {
    return;
  }
  calc_t & c = it->second;
  c.ms = parseInteger(args[1]);
  c.area = std::stod(args[2]);
  c.in_progress = false;
}

bool madieva::sendAll(int fd, const char * data, std::size_t k)
{
  std::size_t sent = 0;
  while (sent < k)
  {
    const int r = ::write(fd, data + sent, k - sent);
    if (r < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
    }
    sent += static_cast< std::size_t >(r);
  }
  return true;
}

bool madieva::recvAll(int fd, char * data, std::size_t k)
{
  std::size_t received = 0;
  while (received < k)
  {
    const int r = ::read(fd, data + received, k - received);
    if (r < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      throw std::runtime_error(std::string("read failed: ") + std::strerror(errno));
    }
    if (r == 0)
    {
      return false;
    }
    received += static_cast< std::size_t >(r);
  }
  return true;
}
