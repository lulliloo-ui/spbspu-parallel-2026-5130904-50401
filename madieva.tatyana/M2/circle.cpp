#include "circle.hpp"
#include <stdexcept>
#include <vector>
#include <ostream>
#include <cstddef>
#include <algorithm>

madieva::Circle::Circle(long long r, long long x, long long y):
  r_(r),
  x_(x),
  y_(y)
{
  if (r <= 0)
  {
    throw std::invalid_argument("a radius must be positive");
  }
}

long long madieva::Circle::getRadius() const
{
  return r_;
}

long long madieva::Circle::getX() const
{
  return x_;
}

long long madieva::Circle::getY() const
{
  return y_;
}

std::ostream & madieva::operator<<(std::ostream & out, const madieva::Circle & circle)
{
  out << circle.getRadius() << " (" << circle.getX() << " " << circle.getY() << ")";
  return out;
}

madieva::frame_t madieva::computeFrame(const madieva::Circle & circle)
{
  const long long min_x = circle.getX() - circle.getRadius();
  const long long min_y = circle.getY() - circle.getRadius();
  const long long max_x = circle.getX() + circle.getRadius();
  const long long max_y = circle.getY() + circle.getRadius();
  return frame_t{min_x, min_y, max_x, max_y};
}

madieva::frame_t madieva::computeFrame(const std::vector< const madieva::Circle * > & figures)
{
  frame_t frame = computeFrame(*figures[0]);
  for (std::size_t i = 1; i < figures.size(); ++i)
  {
    const frame_t current = computeFrame(*figures[i]);
    frame.min_x = std::min(frame.min_x, current.min_x);
    frame.min_y = std::min(frame.min_y, current.min_y);
    frame.max_x = std::max(frame.max_x, current.max_x);
    frame.max_y = std::max(frame.max_y, current.max_y);
  }
  return frame;
}

std::ostream & madieva::operator<<(std::ostream & out, const madieva::frame_t & frame)
{
  out << "(" << frame.min_x << " " << frame.min_y << ") (" << frame.max_x << " " << frame.max_y << ")";
  return out;
}
