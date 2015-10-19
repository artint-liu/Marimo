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

      Line& set(const float2& dir, float d); // �������;��봴��ֱ��
      Line& set(const float2& p0, const float2& p1); // �Ӿ���ֱ�ߵ������㴴��ֱ��
      Line& midnormal(const float2& p0, const float2& p1); // ����Ϊ��p0ָ��p1�Ĵ�ֱƽ����
      b32 intersect(const Line& line, float2* p) const;

      Line& SetFromDirectionPoint(const float2& dir, const float2& p); // ������+����ֱ�ߵĵ�������ֱ��
      float IsPointIn(const float2& p) const; // ���Ƿ���ֱ�߻��ֵĶ��ֿռ���, С��0�ڿռ��ڣ�����0��ֱ���ϣ�����0�ڿռ���
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

      void AddLines(const Line& line); // ���ֱ��
      void clear();
      b32 calculate(); // ����͹������, ����������

    private:
      b32 CalcSubSegment(Segment& seg, LinesArray::iterator& line_iter, size_t myid);
    };
  } // namespace geometry2d
} // namespace clstd

#endif // #ifndef _CLSTD_GEOMERTY_2D_H_