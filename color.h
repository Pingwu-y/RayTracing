#ifndef COLOR_H
#define COLOR_H
#include "vec3.h"

#include <iostream>

using color = vec3;

inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0)
        return sqrt(linear_component);

    return 0;
}

void write_color(std::ostream& out, const color& pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

 
    static const interval intensity(0.000, 0.999);
    out << int(256 * intensity.clamp(r)) << ' '
        << int(256 * intensity.clamp(g)) << ' '
        << int(256 * intensity.clamp(b)) << '\n';
}


#endif
