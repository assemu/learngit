#ifndef BTFPCR_MATH_CURVE_2D_H_
#define BTFPCR_MATH_CURVE_2D_H_

#include <vector>
#include "point2d.h"
#include "shared_data.h"

//用于实现隐式共享
class Curve2dData : public SharedData
{
public:
  Curve2dData();
  explicit Curve2dData(size_t size);
  explicit Curve2dData(const std::vector<Point2d>& ps);
  Curve2dData(const Curve2dData& other);
  std::vector<Point2d> points;
};

//二维点列，曲线
class Curve2d
{
public:
  ~Curve2d() = default;

  //空白曲线
  Curve2d();

  //指定曲线大小，元素都是(0,0)
  explicit Curve2d(size_t size);

  //直接使用向量构造曲线
  explicit Curve2d(const std::vector<Point2d>& ps);

  //复制构造
  Curve2d(const Curve2d& other);

  //追加一个点
  Curve2d& append(const Point2d& p);

  //追加一个点
  Curve2d& append(double x, double y);

  //追加另一个点列
  Curve2d& append(const Curve2d& c);

  //右侧子列
  Curve2d right(int start) const;
  Curve2d translated(double dx, double dy) const; //平移
  Curve2d scaled(double sx, double sy) const; //缩放
  Curve2d subtractedLine(double offset, double slope) const; //减去一条直线

  inline bool empty() const //是否为空
  {
    return d->points.empty();
  }

  inline int size() const //包含点的个数
  {
    return d->points.size();
  }

  //y轴最大最小值
  double minY() const;
  double maxY() const;

  //读取横纵坐标值
  inline double x(int i) const
  {
    return (*this)[i](0);
  }

  inline double y(int i) const
  {
    return (*this)[i](1);
  }

  //设置第i个点
  inline void set(int i, const Point2d& p)
  {
    if(i >= 0 && i < size())
      d->points[i] = p;
  }

  inline void set(int i, double x, double y)
  {
    set(i, Point2d(x, y));
  }

  //只读访问第i个点
  inline const Point2d& operator[](int i) const
  {
    return (d->points)[i];
  }

  inline const Point2d& at(int i) const
  {
    return (d->points)[i];
  }

private:
  SharedDataPointer<Curve2dData> d;
};

#endif //BTFPCR_MATH_CURVE_2D_H_
