#pragma once

#include "perlin.h"

class texture 
{
public:
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class constant_texture : public texture
{
public:
	constant_texture() {}
	constant_texture(vec3 c) : color(c) {}
	virtual vec3 value(float u, float v, const vec3& p) const { return color; }
	vec3 color;
};

class checker_texture : public texture {
public:
	checker_texture() {}
	checker_texture(texture* t0, texture* t1) : even(t0), odd(t1) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		float sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}

	texture* odd;
	texture* even;
};

class noise_texture : public texture
{
public:
	noise_texture() { scale = 1.f; }
	noise_texture(float sc) : scale(sc) {}
	virtual vec3 value(float u, float v, const vec3& p) const
	{
		//return vec3(1, 1, 1) * 0.5 * (1 + noise.turb(scale*p));
		//return vec3(1, 1, 1) * noise.turb(scale*p);
		return vec3(1, 1, 1) * 0.5 * (1 + sin(scale * p.x() + 10*noise.turb(scale*p)));
	}
	perlin noise;
	float scale;
};

void get_sphere_uv(const vec3& p, float& u, float& v)
{
	float phi = atan2(p.z(), p.x());
	float theta = asin(p.y());
	u = 1 - (phi + M_PI) / (2 * M_PI);
	v = (theta + M_PI/2) / M_PI;
}

class image_texture : public texture
{
public:
	image_texture() {}
	image_texture(unsigned char* pixels, int A, int B) : data(pixels), nx(A), ny(B) {}
	virtual vec3 value(float u, float v, const vec3& p) const;

	unsigned char* data;
	int nx, ny;
};

vec3 image_texture::value(float u, float v, const vec3& p) const
{
	int i = u * nx;
	int j = (1-v) * ny-0.001f;
	if (i < 0) i = 0;
	if (j < 0) j = 0;
	if (i > nx - 1) i = nx - 1;
	if (j > ny - 1) j = ny - 1;
	float r = int(data[3 * i + 3 * nx * j]) / 255.f;
	float g = int(data[3 * i + 3 * nx * j + 1]) / 255.f;
	float b = int(data[3 * i + 3 * nx * j + 2]) / 255.f;
	return vec3(r, g, b);
}