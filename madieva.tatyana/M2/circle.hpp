#ifndef CIRCLE_HPP
#define CIRCLE_HPP

#include <iosfwd>
#include <vector>

namespace madieva
{
  class Circle
  {
  public:
    Circle(long long r, long long cx, long long cy);
    long long getRadius() const;
    long long getX() const;
    long long getY() const;

  private:
    long long r_ = 0;
    long long x_ = 0;
    long long y_ = 0;
  };

  std::ostream &operator<<(std::ostream &out, const Circle &circle);

  struct frame_t
  {
    long long min_x = 0;
    long long min_y = 0;
    long long max_x = 0;
    long long max_y = 0;
  };

  frame_t computeFrame(const Circle &);
  frame_t computeFrame(const std::vector< const Circle * > &);

  std::ostream &operator<<(std::ostream &out, const frame_t &frame);
}

#endif
