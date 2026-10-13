#include <cstddef>
#include <exception>
#include <iostream>
#include <new>
#include <random>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

namespace madieva
{

  enum class figure_kind_t
  {
    circle,
    ellipse
  };

  struct figure_t
  {
    figure_kind_t kind = figure_kind_t::circle;
    double a = 0;
    double b = 0;
    double cx = 0;
    double cy = 0;
  };

  struct params_t
  {
    size_t threads = 0;
    size_t tries = 0;
    size_t seed = 0;
  };

  struct frame_t
  {
    double min_x = 0;
    double max_x = 0;
    double min_y = 0;
    double max_y = 0;
  };

  bool parseParams(const int argc, const char * const * argv, params_t & params)
  {
    if (argc != 3 && argc != 4)
    {
      std::cerr << "incorrect number of arguments\n";
      return false;
    }
    try
    {
      if (argv[1][0] == '-')
      {
        std::cerr << "threads must be non-negative\n";
        return false;
      }
      params.threads = static_cast< size_t >(std::stoull(argv[1]));
      if (argv[2][0] == '-')
      {
        std::cerr << "tries must be non-negative\n";
        return false;
      }
      params.tries = static_cast< size_t >(std::stoull(argv[2]));
      if (argc == 4)
      {
        if (argv[3][0] == '-')
        {
          std::cerr << "seed must be non-negative\n";
          return false;
        }
        params.seed = static_cast< size_t >(std::stoull(argv[3]));
      }
    }
    catch (const std::exception & e)
    {
      std::cerr << "cannot parse arguments: " << e.what() << "\n";
      return false;
    }
    if (params.tries == 0)
    {
      std::cerr << "incorrect number of tries\n";
      return false;
    }
    return true;
  }

  bool readFigures(std::vector< figure_t > & figures)
  {
    double a = 0;
    double b = 0;
    double cx = 0;
    double cy = 0;

    while (std::cin >> a >> b >> cx >> cy)
    {
      if (b == 0)
      {
        figures.push_back({figure_kind_t::circle, a, a, cx, cy});
      }
      else
      {
        figures.push_back({figure_kind_t::ellipse, a, b, cx, cy});
      }
    }
    if (!std::cin.eof())
    {
      std::cerr << "error reading figures\n";
      return false;
    }
    if (figures.empty())
    {
      std::cerr << "no figures were provided\n";
      return false;
    }
    return true;
  }

  frame_t computeFrame(const std::vector< figure_t > & figures)
  {
    frame_t frame;
    frame.min_x = figures[0].cx - figures[0].a;
    frame.max_x = figures[0].cx + figures[0].a;
    frame.min_y = figures[0].cy - figures[0].b;
    frame.max_y = figures[0].cy + figures[0].b;
    double min_x = 0;
    double max_x = 0;
    double min_y = 0;
    double max_y = 0;
    for (size_t i = 1; i < figures.size(); ++i)
    {
      min_x = figures[i].cx - figures[i].a;
      max_x = figures[i].cx + figures[i].a;
      min_y = figures[i].cy - figures[i].b;
      max_y = figures[i].cy + figures[i].b;
      if (frame.min_x > min_x)
      {
        frame.min_x = min_x;
      }
      if (frame.max_x < max_x)
      {
        frame.max_x = max_x;
      }
      if (frame.min_y > min_y)
      {
        frame.min_y = min_y;
      }
      if (frame.max_y < max_y)
      {
        frame.max_y = max_y;
      }
    }
    return frame;
  }

  bool isInside(const double x, const double y, const figure_t & figure)
  {
    const double dx = figure.cx - x;
    const double dy = figure.cy - y;
    if (figure.kind == figure_kind_t::circle)
    {
      return (dx * dx + dy * dy) <= (figure.a * figure.a);
    }
    const double a2 = figure.a * figure.a;
    const double b2 = figure.b * figure.b;
    return (b2 * dx * dx + a2 * dy * dy) <= (a2 * b2);
  }

  struct result_t
  {
    double area_all = 0;
    double area_some = 0;
  };

  struct Data
  {
    const std::vector< figure_t > * figures = nullptr;
    frame_t frame;
    size_t tries = 0;
    size_t seed = 0;

    size_t count_all = 0;
    size_t count_some = 0;
  };

  void computeThread(Data * d)
  {
    std::default_random_engine engine(d->seed);
    std::uniform_real_distribution< double > dist_x(d->frame.min_x, d->frame.max_x);
    std::uniform_real_distribution< double > dist_y(d->frame.min_y, d->frame.max_y);
    for (size_t i = 0; i < d->tries; ++i)
    {
      const double x = dist_x(engine);
      const double y = dist_y(engine);

      bool hit_some = false;
      bool hit_all = true;

      for (size_t j = 0; j < d->figures->size(); ++j)
      {
        if (isInside(x, y, (*d->figures)[j]))
        {
          hit_some = true;
        }
        else
        {
          hit_all = false;
        }
      }

      if (hit_some)
      {
        d->count_some++;
      }
      if (hit_all)
      {
        d->count_all++;
      }
    }
  }

  bool computeAreas(
      const std::vector< figure_t > & figures, const params_t & params, const frame_t & frame, result_t & result)
  {
    size_t total_all = 0;
    size_t total_some = 0;

    if (params.threads == 0)
    {
      Data data;
      data.figures = &figures;
      data.frame = frame;
      data.tries = params.tries;
      data.seed = params.seed;
      computeThread(&data);
      total_all = data.count_all;
      total_some = data.count_some;
    }
    else
    {
      std::vector< std::thread > threads;
      std::vector< Data > data;
      try
      {
        threads.reserve(params.threads);
        data.resize(params.threads);
      }
      catch (const std::bad_alloc & e)
      {
        std::cerr << "cannot allocate memory for threads: " << e.what() << "\n";
        return false;
      }
      const size_t tests_per_thread = params.tries / params.threads;
      const size_t remainder = params.tries % params.threads;

      try
      {
        for (size_t i = 0; i < params.threads; ++i)
        {
          data[i].figures = &figures;
          data[i].frame = frame;
          data[i].tries = tests_per_thread;
          if (i == 0)
          {
            data[i].tries += remainder;
          }
          data[i].seed = params.seed + i;
          threads.emplace_back(computeThread, &data[i]);
        }
      }
      catch (const std::system_error & e)
      {
        std::cerr << "cannot create thread: " << e.what() << "\n";
        for (size_t i = 0; i < threads.size(); ++i)
        {
          if (threads[i].joinable())
          {
            threads[i].join();
          }
        }
        return false;
      }
      for (size_t i = 0; i < params.threads; ++i)
      {
        threads[i].join();
        total_all += data[i].count_all;
        total_some += data[i].count_some;
      }
    }

    const double frame_area = (frame.max_x - frame.min_x) * (frame.max_y - frame.min_y);
    const double scale = frame_area / static_cast< double >(params.tries);

    result.area_all = scale * static_cast< double >(total_all);
    result.area_some = scale * static_cast< double >(total_some);
    return true;
  }
}

int main(int argc, char ** argv)
{
  madieva::params_t params;
  if (!madieva::parseParams(argc, argv, params))
  {
    return 1;
  }
  std::vector< madieva::figure_t > figures;
  if (!madieva::readFigures(figures))
  {
    return 1;
  }
  const madieva::frame_t frame = madieva::computeFrame(figures);
  madieva::result_t result{};
  if (!madieva::computeAreas(figures, params, frame, result))
  {
    return 1;
  }
  std::cout << result.area_some << " " << result.area_all << "\n";
  return 0;
}
