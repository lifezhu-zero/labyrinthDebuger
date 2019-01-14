#pragma once

#ifndef __SHADER_MANAGER_H_
#define __SHADER_MANAGER_H_

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

//#include <vector>
//using namespace std;

// Maximum length of shader name
#define MAX_SHADER_NAME_LENGTH	64

#ifdef __cplusplus
extern "C" {
#endif

	enum GLT_STOCK_SHADER {
		GLT_SHADER_IDENTITY = 0, GLT_SHADER_FLAT, GLT_SHADER_SHADED, GLT_SHADER_DEFAULT_LIGHT, GLT_SHADER_POINT_LIGHT_DIFF,
		GLT_SHADER_TEXTURE_REPLACE, GLT_SHADER_TEXTURE_MODULATE, GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF, GLT_SHADER_TEXTURE_RECT_REPLACE,
		GLT_SHADER_LAST
	};

	enum GLT_SHADER_ATTRIBUTE {
		GLT_ATTRIBUTE_VERTEX = 0, GLT_ATTRIBUTE_COLOR, GLT_ATTRIBUTE_NORMAL,
		GLT_ATTRIBUTE_TEXTURE0, GLT_ATTRIBUTE_TEXTURE1, GLT_ATTRIBUTE_TEXTURE2, GLT_ATTRIBUTE_TEXTURE3,
		GLT_ATTRIBUTE_LAST
	};


	struct SHADERLOOKUPETRY {
		char szVertexShaderName[MAX_SHADER_NAME_LENGTH];
		char szFragShaderName[MAX_SHADER_NAME_LENGTH];
		GLuint	uiShaderID;
	};

	typedef struct tagShaderManager {
		GLuint	uiStockShaders[GLT_SHADER_LAST];
	}ShaderManager, *PShaderManager, **PPShaderManager;

#ifdef _WINDOWS
#ifdef _USRDLL
#define EXPORTS __declspec(dllexport)
#else
#define EXPORTS __declspec(dllimport)
#endif
#else
	#define EXPORTS
#endif

	void EXPORTS PPSM_AllocShaderManager(PPShaderManager _ppsm);
	void EXPORTS PPSM_ReleaseShaderManager(PPShaderManager _ppsm);

	int EXPORTS PPSM_InitializeStockShaders(PPShaderManager _ppsm);

	// Find one of the standard stock shaders and return it's shader handle. 
	GLuint EXPORTS PPSM_GetStockShader(PPShaderManager _ppsm, enum GLT_STOCK_SHADER nShaderID);

	// Use a stock shader, and pass in the parameters needed
	GLint EXPORTS PPSM_UseStockShader(PPShaderManager _ppsm, enum GLT_STOCK_SHADER nShaderID, ...);

	// Load a shader pair from file, return NULL or shader handle. 
	// Vertex program name (minus file extension)
	// is saved in the lookup table
	GLuint EXPORTS PPSM_LoadShaderPair(PPShaderManager _ppsm, const char *szVertexProgFileName, const char *szFragProgFileName);

	// Load shaders from source text.
	GLuint EXPORTS PPSM_LoadShaderPairSrc(PPShaderManager _ppsm, const char *szName, const char *szVertexSrc, const char *szFragSrc);

	// Ditto above, but pop in the attributes
	GLuint EXPORTS PPSM_LoadShaderPairWithAttributes(PPShaderManager _ppsm, const char *szVertexProgFileName, const char *szFragmentProgFileName, ...);
	GLuint EXPORTS PPSM_LoadShaderPairSrcWithAttributes(PPShaderManager _ppsm, const char *szName, const char *szVertexProg, const char *szFragmentProg, ...);

	// Lookup a previously loaded shader
	GLuint EXPORTS PPSM_LookupShader(PPShaderManager _ppsm, const char *szVertexProg, const char *szFragProg);

#undef EXPORTS

#ifdef __cplusplus
}
#endif
#endif