#include "curve2d.h"
#include <cfloat>

Curve2dData::Curve2dData()
{}

Curve2dData::Curve2dData(size_t size)
  : points(size)
{}

Curve2dData::Curve2dData(const std::vector<Point2d>& ps)
  : points(ps)
{}

Curve2dData::Curve2dData(const Curve2dData& other)
  : SharedData(other), points(other.points)
{}

Curve2d::Curve2d()
{
  d = new Curve2dData();
}

Curve2d::Curve2d(size_t size)
{
  d = new Curve2dData(size);
}

Curve2d::Curve2d(const std::vector<Point2d>& ps)
{
  d = new Curve2dData(ps);
}

Curve2d::Curve2d(const Curve2d& other)
  : d(other.d)
{}

//y轴最大最小值
double Curve2d::minY() const
{
  double min = DBL_MAX;
  for(int i = 0; i < size(); i++) {
    auto a = y(i);
    if(a < min)
      min = a;
  }
  return min;
}

double Curve2d::maxY() const
{
  double max = DBL_MIN;
  for(int i = 0; i < size(); i++) {
    auto a = y(i);
    if(a > max)
      max = a;
  }
  return max;
}

Curve2d& Curve2d::append(const Point2d& p) //追加一个点
{
  d->points.push_back(p);
  return *this;
}

Curve2d& Curve2d::append(double x, double y) //追加一个点
{
  append(Point2d(x, y));
  return *this;
}

Curve2d& Curve2d::append(const Curve2d& c) //追加另一个点列
{
  if(c.size() > 0) {
    auto& v1 = d->points;
    auto& v2 = c.d->points;
    int N1 = v1.size();
    int N2 = v2.size();
    v1.resize(N1 + N2);
    memcpy((void*)(&v1[N1]), (void*)(&v2[0]), N2 * sizeof(Point2d));
  }
  return *this;
}

//右侧子列
Curve2d Curve2d::right(int start) const
{
  int N = size();
  if(start >= N)
    return Curve2d();
  int s = (start < 0) ? 0 : start;
  int l = N - s;
  Curve2d re(l);
  auto& v = re.d->points;
  memcpy((void*)(&v[0]), (void*)(&(d->points)[s]), l * sizeof(Point2d));
  return re;
}

Curve2d Curve2d::translated(double dx, double dy) const //平移
{
  int N = size();
  Curve2d re(N);
  auto& v = re.d->points;
  for(int i = 0; i < N; ++i) {
    auto p = (*this)[i];
    v[i] = Point2d(p.x() + dx, p.y() + dy);
  }
  return re;
}

Curve2d Curve2d::scaled(double sx, double sy) const //缩放
{
  int N = size();
  Curve2d re(N);
  auto& v = re.d->points;
  for(int i = 0; i < N; ++i) {
    auto p = (*this)[i];
    v[i] = Point2d(p.x() * sx, p.y() * sy);
  }
  return re;
}

Curve2d Curve2d::subtractedLine(double offset, double slope) const //减去一条直线
{
  int N = size();
  Curve2d re(N);
  auto& v = re.d->points;
  for(int i = 0; i < N; ++i) {
    auto p = (*this)[i];
    v[i] = Point2d(p.x(), p.y() - (offset + slope * p.x()));
  }
  return re;
}
