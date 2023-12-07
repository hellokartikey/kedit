#include "point.h"

point::point() : x(0), y(0) {}

point::point(int x, int y) : x(x), y(y) {}

point operator+ (point lhs, point rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}

point operator- (point lhs, point rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}
