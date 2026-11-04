#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

namespace madieva
{
  using Args = std::vector< std::string >;

  Args split(const std::string &);
  long long parseInteger(const std::string &);
  void checkName(const std::string &);
}

#endif
