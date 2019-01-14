#pragma once

// Bring in OpenGL 
// Windows
#ifdef _WINDOWS
#include <windows.h>		// Must have for Windows platform builds
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <gl\glew.h>			// OpenGL Extension "autoloader"
#include <gl\gl.h>			// Microsoft OpenGL headers (version 1.1 by themselves)
#endif
#include <gl/GL.h>

// Mac OS X
#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE | TARGET_IPHONE_SIMULATOR
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define OPENGL_ES
#else
#include <GL/glew.h>
#include <OpenGL/gl.h>		// Apple OpenGL haders (version depends on OS X SDK version)
#endif
#endif

// Linux
#ifdef linux
#define GLEW_STATIC
#include <glew.h>
#endif

#include "glMath.h"

#ifdef _USRDLL
	#define EXPORTS __declspec(dllexport)
#else
	#ifdef __WINDOWS
		#define EXPORTS __declspec(dllimport)
	#else
		#ifdef _LIB
			#define EXPORTS
		#else
			#define EXPORTS extern
		#endif
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct tagGLBatch
	{
		GLenum primitiveType;

		GLuint uiVertexArray;
		GLuint uiNormalArray;
		GLuint uiColorArray;

		GLuint		*uiTextureCoordArray;
		GLuint		vertexArrayObject;

		GLuint nVertsBuilding;			// Building up vertexes counter (immediate mode emulator)
		GLuint nNumVerts;				// Number of verticies in this batch
		GLuint nNumTextureUnits;		// Number of texture coordinate sets

		int	bBatchDone;				// Batch has been built


		M3DVector3f *pVerts;
		M3DVector3f *pNormals;
		M3DVector4f *pColors;
		M3DVector2f **pTexCoords;

	}GLBatch, *PGLBatch, **PPGLBatch;

	EXPORTS void GLB_Alloc(PPGLBatch _item);
	EXPORTS void GLB_Release(PPGLBatch _item);

	// Start populating the array
	EXPORTS void GLB_Begin(PPGLBatch _item, GLenum primitive, GLuint nVerts, GLuint nTextureUnits);

	// Tell the batch you are done
	EXPORTS void GLB_End(PPGLBatch _item);

	// Block Copy in vertex data
	EXPORTS void GLB_CopyVertexData3f(PPGLBatch _item, M3DVector3f *vVerts);
	EXPORTS void GLB_CopyNormalDataf(PPGLBatch _item, M3DVector3f *vNorms);
	EXPORTS void GLB_CopyColorData4f(PPGLBatch _item, M3DVector4f *vColors);
	EXPORTS void GLB_CopyTexCoordData2f(PPGLBatch _item, M3DVector2f *vTexCoords, GLuint uiTextureLayer);

	// Just to make life easier...
	EXPORTS void GLB_CopyVertexData3f2(PPGLBatch _item, GLfloat *vVerts);
	EXPORTS void GLB_CopyNormalDataf2(PPGLBatch _item, GLfloat *vNorms);
	EXPORTS void GLB_CopyColorData4f2(PPGLBatch _item, GLfloat *vColors);
	EXPORTS void GLB_CopyTexCoordData2f2(PPGLBatch _item, GLfloat *vTex, GLuint uiTextureLayer);

	EXPORTS void GLB_Draw(PPGLBatch _item);

	// Immediate mode emulation
	// Slowest way to build an array on purpose... Use the above if you can instead
	EXPORTS void GLB_Reset(PPGLBatch _item);

	EXPORTS void GLB_Vertex3f(PPGLBatch _item, GLfloat x, GLfloat y, GLfloat z);
	EXPORTS void GLB_Vertex3fv(PPGLBatch _item, M3DVector3f vVertex);

	EXPORTS void GLB_Normal3f(PPGLBatch _item, GLfloat x, GLfloat y, GLfloat z);
	EXPORTS void GLB_Normal3fv(PPGLBatch _item, M3DVector3f vNormal);

	EXPORTS void GLB_Color4f(PPGLBatch _item, GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	EXPORTS void GLB_Color4fv(PPGLBatch _item, M3DVector4f vColor);

	EXPORTS void GLB_MultiTexCoord2f(PPGLBatch _item, GLuint texture, GLclampf s, GLclampf t);
	EXPORTS void GLB_MultiTexCoord2fv(PPGLBatch _item, GLuint texture, M3DVector2f vTexCoord);

#ifdef __cplusplus
}

#undef EXPORTS
#endif