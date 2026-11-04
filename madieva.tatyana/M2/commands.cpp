#include "commands.hpp"
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <poll.h>
#include <iomanip>
#include <vector>
#include <sys/types.h>
#include <utility>
#include "parser.hpp"
#include "file_descriptor.hpp"
#include "message.hpp"
#include "child.hpp"
#include "state.hpp"
#include "circle.hpp"

void madieva::addCircle(madieva::state_t & state, const madieva::Args & args)
{
  constexpr std::size_t circle_args_count = 5;
  if (args.size() != circle_args_count)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  checkName(name);
  const long long r = parseInteger(args[2]);
  const long long x = parseInteger(args[3]);
  const long long y = parseInteger(args[4]);

  const Circle circle(r, x, y);

  auto it = state.circles.find(name);
  if (it != state.circles.end())
  {
    throw std::invalid_argument("the name of the circle exists");
  }
  state.circles.insert({name, circle});
}

void madieva::addSet(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() < 3)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  checkName(name);
  const long long count = parseInteger(args[2]);

  if (count <= 0)
  {
    throw std::invalid_argument("incorrect count");
  }

  if (args.size() != 3 + static_cast< std::size_t >(count))
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const auto it_name = state.sets.find(name);
  if (it_name != state.sets.end())
  {
    throw std::invalid_argument("the name of the set exists");
  }

  std::vector< const Circle * > shapes;
  shapes.reserve(count);

  for (std::size_t i = 3; i < args.size(); ++i)
  {
    const std::string & circle_name = args[i];
    const auto it = state.circles.find(circle_name);
    if (it == state.circles.end())
    {
      throw std::invalid_argument("the circle name does not exist");
    }
    shapes.push_back(&it->second);
  }

  state.sets.insert({name, shapes});
}

void madieva::show(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.circles.find(name);
  if (it == state.circles.end())
  {
    throw std::invalid_argument("the name of the circle does not exist");
  }
  std::cout << it->second << "\n";
}

void madieva::showSet(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.sets.find(name);
  if (it == state.sets.end())
  {
    throw std::invalid_argument("the name of the set does not exist");
  }
  for (std::size_t i = 0; i < it->second.size(); ++i)
  {
    std::cout << *(it->second[i]) << "\n";
  }
}

void madieva::showFrame(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.circles.find(name);
  if (it == state.circles.end())
  {
    throw std::invalid_argument("the name of the circle does not exist");
  }
  std::cout << madieva::computeFrame(it->second) << "\n";
}

void madieva::showFrameSet(const madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }

  const std::string & name = args[1];
  const auto it = state.sets.find(name);
  if (it == state.sets.end())
  {
    throw std::invalid_argument("the name of the set does not exist");
  }
  std::cout << madieva::computeFrame(it->second) << "\n";
}

void madieva::spawn(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 3)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }
  const std::string & process_name = args[1];
  checkName(process_name);
  const long long seed = parseInteger(args[2]);

  if (seed < 0)
  {
    throw std::invalid_argument("seed must be non-negative");
  }
  const auto it = state.processes.find(process_name);
  if (it != state.processes.end())
  {
    throw std::invalid_argument("process with this name already exists");
  }

  int cmd_pps[2] = {};
  int res_pps[2] = {};
  if (pipe(cmd_pps) != 0)
  {
    throw std::runtime_error("pipe failed");
  }
  if (pipe(res_pps) != 0)
  {
    close(cmd_pps[0]);
    close(cmd_pps[1]);
    throw std::runtime_error("pipe failed");
  }

  FileDescriptor cmd_rd(cmd_pps[0]);
  FileDescriptor cmd_wr(cmd_pps[1]);
  FileDescriptor res_rd(res_pps[0]);
  FileDescriptor res_wr(res_pps[1]);

  const pid_t pid = fork();

  if (pid < 0)
  {
    throw std::runtime_error("fork failed");
  }

  if (pid == 0)
  {
    cmd_wr.close();
    res_rd.close();
    madieva::runChild(cmd_rd.get(), res_wr.get(), seed);
    _exit(0);
  }

  cmd_rd.close();
  res_wr.close();

  try
  {
    state.processes.insert({process_name, Process{pid, std::move(cmd_wr), std::move(res_rd), seed}});
  }
  catch (...)
  {
    kill(pid, SIGKILL);
    throw;
  }
}

void madieva::areaOn(madieva::state_t & state, const madieva::Args & args)
{
  constexpr std::size_t area_on_args_count = 6;
  if (args.size() != area_on_args_count)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }
  const std::string & process_name = args[1];
  const std::string & calc_name = args[2];
  checkName(calc_name);
  const std::string & set_name = args[3];
  const long long threads = parseInteger(args[4]);
  const long long tries = parseInteger(args[5]);

  const auto proc_it = state.processes.find(process_name);
  if (proc_it == state.processes.end())
  {
    throw std::invalid_argument("process with this name does not exist");
  }

  if (state.calc.find(calc_name) != state.calc.end())
  {
    throw std::invalid_argument("calc with this name already exists");
  }

  if (state.sets.find(set_name) == state.sets.end())
  {
    throw std::invalid_argument("set with this name does not exist");
  }

  if (threads <= 0)
  {
    throw std::invalid_argument("the number of threads must be positive");
  }

  if (tries <= 0)
  {
    throw std::invalid_argument("the number of tries must be positive");
  }

  const auto & figures = state.sets.at(set_name);
  const std::string msg = madieva::makeAreaRequest(calc_name, threads, tries, figures);
  const std::size_t size = msg.size();

  madieva::sendAll(proc_it->second.cmd_wr.get(), reinterpret_cast< const char * >(&size), sizeof(size));
  madieva::sendAll(proc_it->second.cmd_wr.get(), msg.data(), msg.size());

  state.calc.insert({calc_name, calc_t{process_name, threads, tries, true, 0.0, 0}});
}

void madieva::status(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }
  const std::string & calc_name = args[1];

  const auto it = state.calc.find(calc_name);
  if (it == state.calc.end())
  {
    throw std::invalid_argument("calc with this name does not exist");
  }

  const std::string & process_name = it->second.process_name;
  const auto proc_it = state.processes.find(process_name);
  if (proc_it == state.processes.end())
  {
    throw std::invalid_argument("process for this calc does not exist");
  }

  const int fd = proc_it->second.res_rd.get();
  while (true)
  {
    struct pollfd pfd = {};
    pfd.fd = fd;
    pfd.events = POLLIN;
    const int r = ::poll(&pfd, 1, 0);
    if (r <= 0 || !(pfd.revents & POLLIN))
    {
      break;
    }
    std::size_t rsize = 0;
    if (!madieva::recvAll(fd, reinterpret_cast< char * >(&rsize), sizeof(rsize)))
    {
      break;
    }
    std::string rmsg(rsize, '\0');
    if (!madieva::recvAll(fd, &rmsg[0], rmsg.size()))
    {
      throw std::runtime_error("unexpected EOF in response body");
    }
    const madieva::Args rargs = madieva::split(rmsg);
    madieva::applyAreaResponse(state, rargs);
  }

  const calc_t & c = it->second;
  if (c.in_progress)
  {
    std::cout << c.process_name << " <IN PROGRESS>\n";
  }
  else
  {
    std::cout << c.process_name << " " << c.ms << " " << std::fixed << std::setprecision(3) << c.area << "\n";
    std::cout << std::defaultfloat;
  }
}

void madieva::wait(madieva::state_t & state, const madieva::Args & args)
{
  if (args.size() != 2)
  {
    throw std::invalid_argument("incorrect number of arguments");
  }
  const std::string & calc_name = args[1];

  const auto it = state.calc.find(calc_name);
  if (it == state.calc.end())
  {
    throw std::invalid_argument("calc with this name does not exist");
  }

  const std::string & process_name = it->second.process_name;
  const auto proc_it = state.processes.find(process_name);
  if (proc_it == state.processes.end())
  {
    throw std::invalid_argument("process for this calc does not exist");
  }

  const calc_t & c = it->second;
  if (!c.in_progress)
  {
    std::cout << c.process_name << " " << c.ms << " " << std::fixed << std::setprecision(3) << c.area << "\n";
    std::cout << std::defaultfloat;
    return;
  }
  while (true)
  {
    std::size_t rsize = 0;
    if (!madieva::recvAll(proc_it->second.res_rd.get(), reinterpret_cast< char * >(&rsize), sizeof(rsize)))
    {
      throw std::runtime_error("child closed pipe before sending response");
    }
    std::string rmsg(rsize, '\0');
    if (!madieva::recvAll(proc_it->second.res_rd.get(), &rmsg[0], rmsg.size()))
    {
      throw std::runtime_error("unexpected EOF in response body");
    }
    const madieva::Args rargs = madieva::split(rmsg);
    madieva::applyAreaResponse(state, rargs);
    if (!c.in_progress)
    {
      std::cout << c.process_name << " " << c.ms << " " << std::fixed << std::setprecision(3) << c.area << "\n";
      std::cout << std::defaultfloat;
      return;
    }
  }
}
