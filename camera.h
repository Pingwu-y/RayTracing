#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "color.h"
#include "hittable.h"
#include "material.h"
#include <thread>
#include <iostream>


class camera {
  public:
    double aspect_ratio      = 1.0;  
    int    image_width       = 100;  
    int    samples_per_pixel = 10;   
    int    max_depth         = 10;   
    color  background;               

    double vfov     = 90;               // 垂直视角（
    point3 lookfrom = point3(0,0,-1);  // 相机查看的位置
    point3 lookat   = point3(0,0,0);   // 相机正在看的点
    vec3   vup      = vec3(0,1,0);     // 相机相对的“向上”方向

    double defocus_angle = 0;  
    double focus_dist = 10;    

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        
    const int num_threads = std::thread::hardware_concurrency();
    //多线程
    std::vector<std::thread> threads;

    std::vector<color> pixel_colors(image_width * image_height);


    for (int t = 0; t < num_threads; ++t) {
        //创建多线程池
        threads.emplace_back([this, &world, t, num_threads, &pixel_colors]() {
            for (int j = t; j < image_height; j += num_threads) {
               
                 std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
                for (int i = 0; i < image_width; i++) {
                    color pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; sample++) {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    pixel_colors[i + j * image_width] = pixel_color;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            write_color(std::cout, pixel_colors[i + j * image_width], samples_per_pixel);
        }
    }
        std::clog << "\rDone.                 \n";
    }

  private:
    int    image_height;    
    point3 center;          
    point3 pixel00_loc;     
    vec3   pixel_delta_u;   
    vec3   pixel_delta_v;   
    vec3   u, v, w;         // 相机坐标系的基向量
    vec3   defocus_disk_u;  // 失焦盘的水平半径
    vec3   defocus_disk_v;   // 失焦盘的垂直半径

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        center = lookfrom;

        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        
        vec3 viewport_u = viewport_width * u;    
        vec3 viewport_v = viewport_height * -v;  


        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        
        auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j) const {


        auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        auto pixel_sample = pixel_center + pixel_sample_square();

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 pixel_sample_square() const {
       
        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    vec3 pixel_sample_disk(double radius) const {
       
        auto p = radius * random_in_unit_disk();
        return (p[0] * pixel_delta_u) + (p[1] * pixel_delta_v);
    }

    point3 defocus_disk_sample() const {
       
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
       
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;

        if (!world.hit(r, interval(0.001, infinity), rec))
            return background;

        ray scattered;
        color attenuation;
        color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, attenuation, scattered))
            return color_from_emission;

        color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

        return color_from_emission + color_from_scatter;
    }
};


#endif
