#ifndef BTFPCR_MATH_POINT2D_H_
#define BTFPCR_MATH_POINT2D_H_

#include <Eigen/Core>

/**
 * 使用示例
 * Point2d a;
 * Point2d b(1.0,2.0);
 * double x=b.x();
 * double y=b.y();
 * set x: b[0]=10;
 * set y: b[1]=20;
 */

typedef Eigen::Vector2d Point2d;

#endif //BTFPCR_MATH_POINT2D_H_
