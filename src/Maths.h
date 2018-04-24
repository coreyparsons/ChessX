#ifndef MATHS_H
#define MATHS_H

#define ROOT2 1.4142135623730950488016887f
#define PI 3.14159265359f

#include <math.h>

struct Vec2
{
	float x, y;
};

struct Colour
{	
	unsigned char r, g, b, a;
};

inline Vec2 operator+(Vec2 a, Vec2 b)
{
	Vec2 result = { a.x + b.x, a.y + b.y };
	return result;
}

inline Vec2 & operator+=(Vec2 &a, Vec2 b)
{
	a = (a + b);
	return a;
}


inline Vec2 operator-(Vec2 a, Vec2 b)
{
	Vec2 result = { a.x - b.x, a.y - b.y };
	return result;
}

inline Vec2 & operator-=(Vec2 &a, Vec2 b)
{
	a = (a - b);
	return a;
}


inline Vec2 operator/(Vec2 a, Vec2 b) //don't know if i want this
{
	Vec2 result = { a.x / b.x, a.y / b.y };
	return result;
}


inline Vec2 operator*(Vec2 a, Vec2 b) //don't know if i want this
{
	Vec2 result = { a.x * b.x, a.y * b.y };
	return result;
}


inline Vec2 operator*(Vec2 a, float b)
{
	Vec2 result = { a.x * b, a.y * b };
	return result;
}

inline Vec2 operator*(float b, Vec2 a)
{
	Vec2 result = { a.x * b, a.y * b };
	return result;
}

inline Vec2 & operator*=(Vec2 &a, float b)
{
	a = (a * b);
	return a;
}


inline Vec2 operator/(Vec2 a, float b)
{
	Vec2 result = { a.x / b, a.y / b };
	return result;
}

inline Vec2 operator/(float b, Vec2 a)
{
	Vec2 result = { a.x / b, a.y / b };
	return result;
}

inline Vec2 operator+(float b, Vec2 a)
{
	Vec2 result = { a.x + b, a.y + b };
	return result;
}

inline Vec2 operator+(Vec2 a, float b)
{
	Vec2 result = { a.x + b, a.y + b };
	return result;
}


inline Vec2 operator-(float b, Vec2 a)
{
	Vec2 result = { a.x - b, a.y - b };
	return result;
}

inline Vec2 operator-(Vec2 a, float b)
{
	Vec2 result = { a.x - b, a.y - b };
	return result;
}


inline Vec2 & operator/=(Vec2 &a, float b)
{
	a = (a / b);
	return a;
}

inline bool operator==(Vec2 a, Vec2 b)
{
	bool result = ((a.x == b.x) && (a.y == b.y));
	return result;
}

inline bool operator!=(Vec2 a, Vec2 b)
{
	bool result = !((a.x == b.x) && (a.y == b.y));
	return result;
}

inline float dot(Vec2 a, Vec2 b)
{
	float result = (a.x*b.x + a.y*b.y);
	return result;
}

inline int roundToI(float f)
{
	int result = (int)(f + 0.5f);
	return result;
}

inline Vec2 vectorise(float angle, float magnitude)
{
	Vec2 result = { (float)(magnitude * (cos(angle))), (float)(magnitude * (sin(angle))) };
	return result;
}

#endif