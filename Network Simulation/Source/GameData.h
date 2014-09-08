/*

Various data structures used throughout the program. As they're rather small, 
they're defined in a single place for convenience.

*/

#pragma once

#include <math.h>
#include <vector>
#include "timer.h"

//A modified version of the Vector class from Henry S. Fortuna's PS2 framework.
//Deals with vectors in two dimensions
class Vector2
{
public:
	// Constructors
	Vector2(void);
	Vector2(const float _x, const float _y);
	Vector2(const Vector2 & rhs);

	~Vector2(void);

	// Operations with other vectors
	Vector2 & operator+=(const Vector2 & rhs);
	Vector2 & operator-=(const Vector2 & rhs);

	// Special arithmetic
	float Dot(const Vector2 & rhs) const;

	Vector2 & operator*=(const float s);
	Vector2 & operator/=(const float s);

	bool operator==(const Vector2 & rhs) const;

	// Miscellaneous
	float Length() const;
	float LengthSqr() const;
	Vector2 Normalise();
	void NormaliseSelf();
	
	// Member data
	float x,y; 
};

inline Vector2 operator + (const Vector2 &v1,
						   const Vector2 &v2)
{
	return Vector2(v1.x + v2.x, v1.y + v2.y);
}

inline Vector2 operator - (const Vector2 &v1,
						   const Vector2 &v2)
{
	return Vector2(v1.x - v2.x, v1.y - v2.y);
}

inline Vector2 operator - (const Vector2 &v1)
{
	return Vector2(-v1.x, -v1.y);
}

inline Vector2 operator * (const Vector2 &v,
						   const float &s)
{
	return Vector2(v.x * s, v.y * s);
}

inline Vector2 operator * (const float & s,
						   const Vector2 &v)
{
	return Vector2(v.x * s, v.y * s);
}

inline Vector2 operator / (const Vector2 &v,
						   const float & s)
{
	return Vector2(v.x / s, v.y / s);
}

inline float DotProduct(const Vector2 &v1,
						 const Vector2 &v2)
{
	return (v1.x * v2.x + v1.y * v2.y);
}

inline Vector2 Normalise (const Vector2 &v)
{
	return v / v.Length();
}





//Internal data structure for entity status. While similar to the one sent over the network, 
//it is kept entirely separate from a coding standpoint.
struct GameEntity
{
	enum eType {PLAYER, AI};
	eType Type;
	unsigned char ID;
	Vector2 Pos, Vel, Acc;
	std::vector<unsigned char> Colliders;
};

//Holds data about one client.
//A server is not limited in the number of clients it accepts by application design.
struct ClientData
{
	sockaddr_in Address;
	Timer ActivityTimer;
};



