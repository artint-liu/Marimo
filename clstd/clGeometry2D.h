#ifndef _CLSTD_GEOMERTY_2D_H_
#define _CLSTD_GEOMERTY_2D_H_

namespace clstd
{
  class line2d
  {
  public:
    // ax + by = d
    float2 direction;
    float  distance;

  public:
    line2d(){}
    line2d(const float2& dir, float d)
      : direction(dir), distance(d)
    {
    }

    line2d(const float2& p0, const float2& p1)
    {
      set(p0, p1);
    }

    line2d& set(const float2& dir, float d)
    {
      direction = dir;
      distance = d;
      return *this;
    }

    line2d& SetFromDirectionPoint(const float2& dir, const float2& p) // 从向量+经过直线的点来设置直线
    {
      direction = dir;
      distance = float2::dot(dir, p);
      return *this;
    }

    line2d& set(const float2& p0, const float2& p1)
    {
      float len = direction.normal(p0, p1);
      distance = (p0.x * p1.y - p1.x * p0.y) / len;
      return *this;
    }

    line2d& midnormal(const float2& p0, const float2& p1) // 设置为从p0指向p1的垂直平分线
    {
      float2 dir = p1 - p0;
      dir.normalize();
      return SetFromDirectionPoint(dir, (p0 + p1) * 0.5f);
    }

    b32 intersect(const line2d& line, float2* p) const
    {
      const float a0 = direction.x;
      const float b0 = direction.y;
      const float c0 = -distance;
      const float a1 = line.direction.x;
      const float b1 = line.direction.y;
      const float c1 = -line.distance;

      float D = a0 * b1 - a1 * b0;
      if(D == 0) {
        return FALSE;
      }

      p->x = (b0 * c1 - b1 * c0) / D;
      p->y = (a1 * c0 - a0 * c1) / D;      
      return TRUE;
    }

    float IsPointIn(const float2& p) const // 点是否在直线划分的二分空间内, 小于0在空间内，等于0在直线上，大于0在空间外
    {
      return float2::dot(direction, p) - distance;
    }
  };

  class LineSegment2D
  {
  public:
    float2 point[2];
  public:
    LineSegment2D(){}
    LineSegment2D(const float2& p0, const float2& p1)
    {
      point[0] = p0; point[1] = p1;
    }

    b32 IsValid() const
    {
      return point[0] != point[1];
    }

    LineSegment2D& set(const float2& p0, const float2& p1)
    {
      point[0] = p0;
      point[1] = p1;
      return *this;
    }

    b32 intersect(const line2d& line, float2* p) const
    {
      line2d thisline(point[0], point[1]);
      if( ! thisline.intersect(line, p)) {
        return FALSE;
      }

      return
        ((p->x >= point[0].x && p->x <= point[1].x) || (p->x >= point[1].x && p->x <= point[0].x)) &&
        ((p->y >= point[0].y && p->y <= point[1].y) || (p->y >= point[1].y && p->y <= point[0].y)) ;
    }
  };

  class winding2d
  {
  public:
    //struct LINE {
    //  size_t id;
    //  line2d line;
    //};
    typedef clvector<line2d> LinesArray;
    typedef clvector<float2> VerticesArray;
    LinesArray    lines;
    VerticesArray vertices;

    void AddLines(const line2d& line) // 添加直线
    {
      //for(auto it = lines.begin(); it != lines.end(); ++it)
      //{
      //  if(it->direction.IsClockwise(line.direction) >= 0) {
      //    lines.insert(it, line);
      //    return;
      //  }
      //}

      lines.push_back(line);
    }

    void clear()
    {
      lines.clear();
      vertices.clear();
    }

    b32 calculate() // 计算凸包顶点, 纯暴力计算
    {
      vertices.clear();
      if(lines.size() < 3) {
        return FALSE;
      }

      //{
      //  for(auto it = lines.begin(); it != lines.end() - 1; ++it)
      //  {
      //    TRACE("(%f,%f) %f %f\n", it->direction.x, it->direction.y,
      //      it->direction.IsClockwise((it + 1)->direction),
      //      lines.front().direction.IsClockwise(it->direction));
      //  }
      //}

      //std::sort(lines.begin(), lines.end(), [](line2d& a, line2d& b) {
      //  return a.direction.IsClockwise(b.direction) < 0;
      //});
      cllist<LineSegment2D> SegmentList;
      LineSegment2D sLineSeg;
      //SegmentList.resize(lines.size());

      int n = 0;
      for(auto it = lines.begin(); it != lines.end(); ++it, n++)
      {
        if( ! CalcSubSegment(sLineSeg, it, n) || ! sLineSeg.IsValid()) {
          continue;
        }

        /*
        vertices.push_back(ls.point[0]);
        vertices.push_back(ls.point[1]);
        /*/
        if(vertices.empty()) {
          vertices.push_back(sLineSeg.point[0]);
          vertices.push_back(sLineSeg.point[1]);
        }
        else if(vertices.back().IsNearTo(sLineSeg.point[0], 1e-6f))
        {
          vertices.push_back(sLineSeg.point[1]);
        }
        else {
          SegmentList.push_back(sLineSeg);
        }
        //*/
      }

      n = 1;
      while( ! SegmentList.empty() && n != 0)
      {
        n = 0;
        for(auto it = SegmentList.begin(); it != SegmentList.end();)
        {
          if(vertices.back().IsNearTo(it->point[0], 1e-6f)) {
            vertices.push_back(it->point[1]);
            it = SegmentList.erase(it);
            n++;
          }
          else {
            ++it;
          }
        }
      }

      if(vertices.size() < 3) {
        vertices.clear();
      }
      return TRUE;
    }

  private:
#define SEG_TRACE
    b32 CalcSubSegment(LineSegment2D& seg, LinesArray::iterator& line_iter, size_t myid)
    {
      b32 b[2] = {FALSE, FALSE};
      const auto& line = *line_iter;

      int n = 0;
      for(auto it = lines.begin(); it != lines.end(); ++it, n++)
      {
        if(n == myid) {
          SEG_TRACE("(-) --\n");
          continue;
        }

        float2 p;
        if( ! line.intersect(*it, &p)) {
          SEG_TRACE("(-) ---\n");
          continue;
        }

        float cw = line.direction.IsClockwise(it->direction);
        SEG_TRACE(" %f ", cw);
        if(cw < 0.0f)
        {
          SEG_TRACE("(1) %f,%f [%f] ", p.x, p.y, b[1] ? it->IsPointIn(seg.point[1]) : 0);
          if( ! b[1]) {
            b[1] = TRUE;
            seg.point[1] = p;
            SEG_TRACE("*");
          }
          else if(it->IsPointIn(seg.point[1]) > 0) {
            seg.point[1] = p;
            SEG_TRACE("*");
          }
          SEG_TRACE("\n");
        }
        else if(cw > 0.0f)
        {
          SEG_TRACE("(0) %f,%f [%f] ", p.x, p.y, b[0] ? it->IsPointIn(seg.point[0]) : 0);
          if( ! b[0]) {
            b[0] = TRUE;
            seg.point[0] = p;
            SEG_TRACE("*");
          }
          else if(it->IsPointIn(seg.point[0]) > 0) {
            seg.point[0] = p;
            SEG_TRACE("*");
          }
          SEG_TRACE("\n");
        }
        else {
          CLBREAK; // 平行，相交测试时应该已经剔除
        }
      }

      float2 test;
      test.perpendicular(seg.point[0], seg.point[1]);
      float t = float2::dot(test, line.direction);

      SEG_TRACE("%ssel:(%f,%f) (%f,%f)\n\n\n",
        (b[0] && b[1] && t > 0) ? "" : "dis",
        seg.point[0].x, seg.point[0].y, seg.point[1].x, seg.point[1].y);
      return b[0] && b[1] && t > 0;
    }
  };
} // namespace clstd

#endif // #ifndef _CLSTD_GEOMERTY_2D_H_