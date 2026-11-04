#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <cstddef>
#include "circle.hpp"
#include "parser.hpp"
#include "state.hpp"

namespace madieva
{
  struct AreaRequest
  {
    std::string calc_name;
    long long threads = 0;
    long long tries = 0;
    std::vector< madieva::Circle > circles;
  };

  std::string makeAreaRequest(
      const std::string & calc_name, long long threads, long long tries, const std::vector< const Circle * > & figures);
  AreaRequest parseAreaRequest(const Args & args);

  std::string makeAreaResponse(const std::string & calc_name, long long ms, double area);
  void applyAreaResponse(state_t & state, const Args & args);

  bool sendAll(int fd, const char * data, std::size_t k);
  bool recvAll(int fd, char * data, std::size_t k);
}

#endif
