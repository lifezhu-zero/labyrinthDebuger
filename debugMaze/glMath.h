#pragma once

#ifndef __GL_MATH_H__
#define __GL_MATH_H__

#ifdef __cplusplus
extern "C" {
#endif
	typedef float M3DMatrix44f[16];		// A 4 X 4 matrix, column major (floats) - OpenGL style
	typedef float	M3DVector4f[4];		// Lesser used... Do we really need these?
	typedef float	M3DVector3f[3];		// Vector of three floats (x, y, z)

	typedef float	M3DVector2f[2];		// 3D points = 3D Vectors, but we need a 
	typedef double	M3DVector2d[2];		// 2D representations sometimes... (x,y) order

#ifdef __cplusplus
}
#endif
#endif