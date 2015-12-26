#ifndef _CLSTD_GEOMERTY_2D_H_
#define _CLSTD_GEOMERTY_2D_H_

namespace clstd
{
  namespace geometry2d
  {
    class Line
    {
    public:
      // ax + by = d
      float2 direction;
      float  distance;

    public:
      Line(){}
      Line(const float2& dir, float d);
      Line(const float2& p0, const float2& p1);

      Line& set(const float2& dir, float d); // 从向量和距离创建直线
      Line& set(const float2& p0, const float2& p1); // 从经过直线的两个点创建直线
      Line& midnormal(const float2& p0, const float2& p1); // 设置为从p0指向p1的垂直平分线
      b32 intersect(const Line& line, float2* p) const;

      Line& SetFromDirectionPoint(const float2& dir, const float2& p); // 从向量+经过直线的点来设置直线
      float IsPointIn(const float2& p) const; // 点是否在直线划分的二分空间内, 小于0在空间内，等于0在直线上，大于0在空间外
    };

    class Segment
    {
    public:
      float2 point[2];

    public:
      Segment(){}
      Segment(const float2& p0, const float2& p1);

      Segment& set(const float2& p0, const float2& p1);

      b32 IsValid   () const;
      b32 intersect (const Line& line, float2* p) const;
    };

    class Winding
    {
    public:
      typedef clvector<Line> LinesArray;
      typedef clvector<float2> VerticesArray;

    public:
      LinesArray    lines;
      VerticesArray vertices;

      void AddLines(const Line& line); // 添加直线
      void clear();
      b32 calculate(); // 计算凸包顶点, 纯暴力计算

    private:
      b32 CalcSubSegment(Segment& seg, LinesArray::iterator& line_iter, size_t myid);
    };
  } // namespace geometry2d
} // namespace clstd

#endif // #ifndef _CLSTD_GEOMERTY_2D_H_