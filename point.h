#ifndef KEDIT_POINT_H
#define KEDIT_POINT_H

struct point {
  point();
  point(int x, int y);

  int x, y;
};

point operator+ (point lhs, point rhs);
point operator- (point lhs, point rhs);

#endif
