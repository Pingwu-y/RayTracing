#ifndef AABB_H
#define AABB_H
#include "rtweekend.h"


class aabb {
  public:
    interval x, y, z;

    aabb() {} // 默认的AABB是空的，因为区间默认是空的

    aabb(const interval& ix, const interval& iy, const interval& iz)
      : x(ix), y(iy), z(iz)
    {
        pad_to_minimums();
    }

    aabb(const point3& a, const point3& b) {
    // 将两个点a和b视为包围盒的极值，这样我们不需要特定的最小/最大坐标顺序。
        x = interval(fmin(a[0],b[0]), fmax(a[0],b[0]));
        y = interval(fmin(a[1],b[1]), fmax(a[1],b[1]));
        z = interval(fmin(a[2],b[2]), fmax(a[2],b[2]));

        pad_to_minimums();
    }

    aabb(const aabb& box0, const aabb& box1) {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
    }

    const interval& axis(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool hit(const ray& r, interval ray_t) const {
        for (int a = 0; a < 3; a++) {
            auto invD = 1 / r.direction()[a];
            auto orig = r.origin()[a];

            auto t0 = (axis(a).min - orig) * invD;
            auto t1 = (axis(a).max - orig) * invD;

            if (invD < 0)
                std::swap(t0, t1);

            if (t0 > ray_t.min) ray_t.min = t0;
            if (t1 < ray_t.max) ray_t.max = t1;

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

    int longest_axis() const {
        // 返回边界框的最长轴的索引。

        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const aabb empty, universe;

  private:

    void pad_to_minimums() {
         // 调整AABB，使得没有一边比某个delta更窄，如果需要的话进行填充。


        double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

const aabb aabb::empty    = aabb(interval::empty,    interval::empty,    interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+(const aabb& bbox, const vec3& offset) {
    return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

aabb operator+(const vec3& offset, const aabb& bbox) {
    return bbox + offset;
}


#endif
