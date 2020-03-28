#include "pch.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include <limits>
#include <random>
#include <atomic>

#include "utils.h"

#include "sphere.h"
#include "hitable.h"

#include "camera.h"
#include "material.h"
#include "texture.h"

#include "parallel_for.h"

vec3 color(const ray& r, hitable* world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001f, std::numeric_limits<float>::max(), rec))
	{
		ray scattered;
		vec3 attenuation;
		vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		{
			return emitted + attenuation * color(scattered, world, depth + 1);
		}
		else
		{
			return emitted;
		}
	}
	else
	{
		return vec3(0, 0, 0);
	}
}

hitable* random_scene()
{
	int n = 500;
	hitable** list = new hitable*[n + 1];
	texture* checker = new checker_texture(
		new constant_texture(vec3(0.2f, 0.3f, 0.1f)),
		new constant_texture(vec3(0.9f, 0.9f, 0.9f))
	);
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	int i = 1;
	for (int a = -5; a < 5; a++)
	{
		for (int b = -5; b < 5; b++)
		{
			float choose_mat = drand48();
			vec3 center(a + 0.9f*drand48(), 0.2f, b + 0.9f*drand48());
			if ((center - vec3(4, 0.2f, 0.f)).length() > 0.9)
			{
				if (choose_mat < 0.8f)
				{	//diffuse
					list[i++] = new sphere(center, 0.2f, new lambertian(new constant_texture(vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48()))));
					//list[i++] = new moving_sphere(center, center + vec3(0.f, 0.5f * drand48(), 0.f), 0.f, 1.f, 0.2f, new lambertian(new constant_texture(vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48()))));
				}
				else if (choose_mat < 0.95f)
				{	//metal
					list[i++] = new sphere(center, 0.2f, new metal(vec3(0.5f*(1 + drand48()), 0.5f*(1 + drand48()), 0.5f*(1 + drand48())), 0.5f*drand48()));
				}
				else
				{	//glass
					list[i++] = new sphere(center, 0.2f, new dielectric(1.5f));
				}
			}
		}
	}
	list[i++] = new sphere(vec3(0, 1, 0), 1.f, new dielectric(1.5f));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.f, new lambertian(new constant_texture(vec3(0.4f, 0.2f, 0.1f))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.f, new metal(vec3(0.7f, 0.6f, 0.5f), 0.f));

	return new hitable_list(list, i);
}

hitable* two_spheres()
{
	texture* checker = new checker_texture(
		new constant_texture(vec3(0.2f, 0.3f, 0.1f)),
		new constant_texture(vec3(0.9f, 0.9f, 0.9f))
	);
	hitable** list = new hitable * [2];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
	list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));

	return new hitable_list(list, 2);
}

hitable* two_perlin_spheres()
{
	texture* pertext = new noise_texture();
	hitable** list = new hitable * [2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));

	int nx, ny, nn;
	unsigned char* tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);

	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(new image_texture(tex_data, nx, ny)));

	return new hitable_list(list, 2);
}

hitable* simple_light()
{
	texture* pertext = new noise_texture(4);
	hitable** list = new hitable * [4];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	list[2] = new sphere(vec3(0, 7, 0), 2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));
	list[3] = new xy_rect(3,5,1,3, -2, new diffuse_light(new constant_texture(vec3(4, 4, 4))));

	bvh_node* bvh = new bvh_node(list, 4, 0.f, 1.f);

	hitable** final_list = new hitable * [1];
	final_list[0] = bvh;

	return new hitable_list(final_list, 1);
}

hitable* cornell_box()
{
	hitable** list = new hitable* [8];
	int i = 0;
	material* red = new lambertian(new constant_texture(vec3(0.65f, 0.05f, 0.05f)));
	material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
	material* green = new lambertian(new constant_texture(vec3(0.12f, 0.45f, 0.15f)));
	material* light = new diffuse_light(new constant_texture(vec3(15, 15, 15)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(213, 343, 227, 332, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

	bvh_node* bvh = new bvh_node(list, i, 0.f, 1.f);

	hitable** final_list = new hitable * [1];
	final_list[0] = bvh;

	return new hitable_list(final_list, 1);
}

hitable* cornell_smoke()
{
	hitable** list = new hitable * [8];
	int i = 0;
	material* red = new lambertian(new constant_texture(vec3(0.65f, 0.05f, 0.05f)));
	material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
	material* green = new lambertian(new constant_texture(vec3(0.12f, 0.45f, 0.15f)));
	material* light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(113, 443, 127, 432, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	hitable* b1 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
	hitable* b2 = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

	list[i++] = new constant_medium(b1, 0.01f, new isotropic(new constant_texture(vec3(1.f, 1.f, 1.f))));
	list[i++] = new constant_medium(b2, 0.01f, new isotropic(new constant_texture(vec3(0.f, 0.f, 0.f))));

	bvh_node * bvh = new bvh_node(list, i, 0.f, 1.f);

	hitable * *final_list = new hitable * [1];
	final_list[0] = bvh;

	return new hitable_list(final_list, 1);
}

hitable* cornell_reflections()
{
	hitable** list = new hitable * [7];
	int i = 0;
	material* red = new lambertian(new constant_texture(vec3(0.65f, 0.05f, 0.05f)));
	material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
	material* green = new lambertian(new constant_texture(vec3(0.12f, 0.45f, 0.15f)));
	material* light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));
	list[i++] = new flip_normals(new yz_rect(0, 555, 0, 555, 555, green));
	list[i++] = new yz_rect(0, 555, 0, 555, 0, red);
	list[i++] = new xz_rect(113, 443, 127, 432, 554, light);
	list[i++] = new flip_normals(new xz_rect(0, 555, 0, 555, 555, white));
	list[i++] = new xz_rect(0, 555, 0, 555, 0, white);
	list[i++] = new flip_normals(new xy_rect(0, 555, 0, 555, 555, white));
	list[i++] = new sphere(vec3(278, 278, 278), 50.f, new metal(vec3(0.7f, 0.6f, 0.5f), 0.f));

	bvh_node * bvh = new bvh_node(list, i, 0.f, 1.f);

	hitable * *final_list = new hitable * [1];
	final_list[0] = bvh;

	return new hitable_list(final_list, 1);
}

hitable* final()
{
	int nb = 20;
	hitable** list = new hitable * [30];
	hitable** boxlist = new hitable * [10000];
	hitable** boxlist2 = new hitable * [10000];
	material* white = new lambertian(new constant_texture(vec3(0.73f, 0.73f, 0.73f)));
	material* ground = new lambertian(new constant_texture(vec3(0.48f, 0.83f, 0.53f)));
	int b = 0;
	for (int i = 0; i < nb ; ++i)
	{
		for (int j = 0; j < nb; ++j)
		{
			float w = 100;
			float x0 = -1000 + i * w;
			float z0 = -1000 + j * w;
			float y0 = 0;
			float x1 = x0 + w;
			float z1 = z0 + w;
			float y1 = 100 * (drand48() + 0.01f);
			boxlist[b++] = new box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
		}
	}
	int l = 0;
	list[l++] = new bvh_node(boxlist, b, 0, 1);
	material* light = new diffuse_light(new constant_texture(vec3(7, 7, 7)));
	list[l++] = new xz_rect(123, 423, 147, 412, 554, light);
	vec3 center(400, 400, 200);
	list[l++] = new moving_sphere(center, center + vec3(30, 0, 0), 0, 1, 50, new lambertian(new constant_texture(vec3(0.7f, 0.3f, 0.1f))));
	list[l++] = new sphere(vec3(260, 150, 45), 50, new dielectric(1.5f));
	list[l++] = new sphere(vec3(0, 150, 145), 50, new metal(vec3(0.8f, 0.8f, 0.9f), 10.f));
	hitable* boundary = new sphere(vec3(360, 150, 145), 70, new dielectric(1.5f));
	list[l++] = boundary;
	list[l++] = new constant_medium(boundary, 0.2f, new isotropic(new constant_texture(vec3(0.2f, 0.4f, 0.9f))));
	int nx, ny, nn;
	unsigned char* tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
	material* emat = new lambertian(new image_texture(tex_data, nx, ny));
	list[l++] = new sphere(vec3(400, 200, 400), 100, emat);
	texture* pertext = new noise_texture(0.1f);
	list[l++] = new sphere(vec3(220, 280, 300), 80, new lambertian(pertext));
	int ns = 1000;
	for (int j = 0; j < ns; ++j)
	{
		boxlist2[j] = new sphere(vec3(165 * drand48(), 165 * drand48(), 165 * drand48()), 10, white);
	}
	list[l++] = new translate(new rotate_y(new bvh_node(boxlist2, ns, 0.f, 1.f), 15), vec3(-100, 270, 395));
	return new hitable_list(list, l);
}

int main()
{
	int nx = 1024;
	int ny = 1024;
	int ns = 100;
	std::cout << "P3" << std::endl << nx << " " << ny << std::endl << "255" << std::endl;

	//vec3 lookfrom(278, 278, -1);
	vec3 lookfrom(278, 278, -800);	//final
	//vec3 lookfrom(278, 278, -100);	
	vec3 lookat(278, 278, 0);			//final
	//vec3 lookfrom(500, 280, -700);
	//vec3 lookat(220, 280, 300);
	//float dist_to_focus = 20.f;
	//float vfov = 40.f;
	//vec3 lookfrom(10, 3, 2);
	//vec3 lookat(0, 1, 0);
	//vec3 lookfrom(13, 2, 3);			//two_spheres
	//vec3 lookat(0, 0, 0);				//two_spheres
	float dist_to_focus = 10.f;
	float vfov = 40.f;	//final
	//float vfov = 20.f;					//two_spheres
	float aperture = 0.0f;
	float t0 = 0.f;
	float t1 = 1.f;

	hitable* world = final();

	camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus, t0, t1);
	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			std::atomic<vec3> col(vec3(0,0,0));
			parallel_for(ns, [&](int start, int end) {
				for (int s = start; s < end; s++)
				{
					float u = float(i + drand48()) / float(nx);
					float v = float(j + drand48()) / float(ny);
					ray r = cam.get_ray(u, v);
					vec3 tmp_color = color(r, world, 0);
					col = col+tmp_color;
				}
			}, true);

			col = col/float(ns);

			vec3 col_val = col.load();

			col_val = vec3(sqrt(col_val[0]), sqrt(col_val[1]), sqrt(col_val[2]));

			if (col_val[0] > 1)
				col_val[0] = 1;
			if (col_val[1] > 1)
				col_val[1] = 1;
			if (col_val[2] > 1)
				col_val[2] = 1;
			int ir = int(255.99f * col_val.r());
			int ig = int(255.99f * col_val.g());
			int ib = int(255.99f * col_val.b());
			std::cout << ir << " " << ig << " " << ib << std::endl;
		}
	}
}
