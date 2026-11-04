#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <functional>
#include "state.hpp"
#include "parser.hpp"

namespace madieva
{
  using Command = std::function< void(state_t &, const Args &) >;

  void addCircle(state_t &, const Args &);
  void addSet(state_t &, const Args &);
  void show(const state_t &, const Args &);
  void showSet(const state_t &, const Args &);
  void showFrame(const state_t &, const Args &);
  void showFrameSet(const state_t &, const Args &);
  void spawn(state_t &, const Args &);
  void areaOn(state_t &, const Args &);
  void status(state_t &, const Args &);
  void wait(state_t &, const Args &);
}

#endif
