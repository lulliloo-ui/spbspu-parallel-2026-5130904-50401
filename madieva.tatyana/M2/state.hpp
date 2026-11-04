#ifndef STATE_HPP
#define STATE_HPP

#include <map>
#include <vector>
#include <string>
#include <sys/types.h>
#include "circle.hpp"
#include "file_descriptor.hpp"

namespace madieva
{

  struct calc_t
  {
    std::string process_name;
    long long threads = 0;
    long long tries = 0;
    bool in_progress = true;
    double area = 0.0;
    long long ms = 0;
  };

  struct Process
  {
    pid_t pid = -1;
    FileDescriptor cmd_wr;
    FileDescriptor res_rd;
    long long seed = 0;
  };

  struct state_t
  {
    std::map< std::string, Circle > circles;
    std::map< std::string, std::vector< const Circle * > > sets;
    std::map< std::string, Process > processes;
    std::map< std::string, calc_t > calc;
  };
}

#endif
