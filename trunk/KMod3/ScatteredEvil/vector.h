//**************************************************************************
//**
//**	##   ##    ##    ##   ##   ####     ####   ###     ###
//**	##   ##  ##  ##  ##   ##  ##  ##   ##  ##  ####   ####
//**	 ## ##  ##    ##  ## ##  ##    ## ##    ## ## ## ## ##
//**	 ## ##  ########  ## ##  ##    ## ##    ## ##  ###  ##
//**	  ###   ##    ##   ###    ##  ##   ##  ##  ##       ##
//**	   #    ##    ##    #      ####     ####   ##       ##
//**
//**	$Id$
//**
//**	Copyright (C) 1999-2002 JÆnis Legzdi·ý
//**
//**	This program is free software; you can redistribute it and/or
//**  modify it under the terms of the GNU General Public License
//**  as published by the Free Software Foundation; either version 2
//**  of the License, or (at your option) any later version.
//**
//**	This program is distributed in the hope that it will be useful,
//**  but WITHOUT ANY WARRANTY; without even the implied warranty of
//**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//**  GNU General Public License for more details.
//**
//**************************************************************************

#include <math.h>

class  TVec
{
public:
	float		x;
	float		y;
	float		z;

	TVec(void) { }

	TVec(float Ax, float Ay, float Az)
	{
		x = Ax;
		y = Ay;
		z = Az;
	}

	TVec (const float f[3])
	{
    	x = f[0];
    	y = f[1];
    	z = f[2];
	}

	const float& operator[](int i) const
	{
    	return (&x)[i];
	}

	float& operator[](int i)
	{
	    return (&x)[i];
	}

	TVec &operator += (const TVec &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	TVec &operator -= (const TVec &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	TVec &operator *= (float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	TVec &operator /= (float scale)
	{
		x /= scale;
		y /= scale;
		z /= scale;
		return *this;
	}

	TVec operator + (void) const
	{
		return *this;
	}

	TVec operator - (void) const
	{
		return TVec(-x,	-y, -z);
	}

	float Length(void) const
	{
		return sqrt(x * x + y * y + z * z);
	}
};

inline TVec operator + (const TVec &v1, const TVec &v2)
{
	return TVec(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

inline TVec operator - (const TVec &v1, const TVec &v2)
{
	return TVec(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

inline TVec operator * (const TVec& v, float s)
{
	return TVec(s * v.x, s * v.y, s * v.z);
}

inline TVec operator * (float s, const TVec& v)
{
	return TVec(s * v.x, s * v.y, s * v.z);
}

inline TVec operator / (const TVec& v, float s)
{
	return TVec(v.x / s, v.y / s, v.z / s);
}

inline bool operator == (const TVec& v1, const TVec& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

inline bool operator != (const TVec& v1, const TVec& v2)
{
	return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z;
}

inline float Length(const TVec &v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline TVec Normalize(const TVec& v)
{
   return v / v.Length();
}

inline float DotProduct(const TVec& v1, const TVec& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline TVec CrossProduct(const TVec &v1, const TVec &v2)
{
	return TVec(v1.y * v2.z - v1.z * v2.y,
				v1.z * v2.x - v1.x * v2.z,
				v1.x * v2.y - v1.y * v2.x);
}

#define _DEG2RAD		0.017453292519943296
#define _RAD2DEG		57.2957795130823209

#define DEG2RAD(a)		((a) * _DEG2RAD)
#define RAD2DEG(a)		((a) * _RAD2DEG)

struct TAVec
{
	float pitch;
	float yaw;
	float roll;
};

inline float msin(float angle)
{
	return sin(DEG2RAD(angle));
}

inline float mcos(float angle)
{
	return cos(DEG2RAD(angle));
}

inline float mtan(float angle)
{
	return tan(DEG2RAD(angle));
}

inline float matan(float y, float x)
{
	return RAD2DEG(atan2(y, x));
}

inline void VectorAngles(const TVec &vec, TAVec &angles)
{
	double length = sqrt(vec.x * vec.x + vec.y * vec.y);
	angles.pitch = -matan(vec.z, length);
	angles.yaw = matan(vec.y, vec.x);
	angles.roll = 0;
}

//**************************************************************************
//
//	$Log$
//	Revision 1.1  2004/12/30 10:51:07  tache_man
//	Initial
//
//	Revision 1.1  2002/06/29 16:16:55  dj_jl
//	Started a work on new conversation system.
//	
//**************************************************************************
