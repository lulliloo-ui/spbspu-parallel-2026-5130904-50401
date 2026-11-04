#include <map>
#include <iostream>
#include <string>
#include <stdexcept>
#include <exception>
#include "state.hpp"
#include "commands.hpp"
#include "parser.hpp"

int main()
{
  const std::map< std::string, madieva::Command > commands = {{"circle", madieva::addCircle}, {"set", madieva::addSet},
      {"show", madieva::show}, {"showset", madieva::showSet}, {"frame", madieva::showFrame},
      {"frameset", madieva::showFrameSet}, {"spawn", madieva::spawn}, {"area-on", madieva::areaOn},
      {"status", madieva::status}, {"wait", madieva::wait}};
  madieva::state_t state;
  std::string line;
  while (std::getline(std::cin, line))
  {
    const madieva::Args args = madieva::split(line);
    if (args.empty())
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }

    const auto it = commands.find(args[0]);
    if (it == commands.end())
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }

    try
    {
      it->second(state, args);
    }
    catch (const std::invalid_argument &)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
    catch (const std::exception &e)
    {
      std::cerr << "internal error: " << e.what() << "\n";
      return 2;
    }
  }

  return 0;
}
