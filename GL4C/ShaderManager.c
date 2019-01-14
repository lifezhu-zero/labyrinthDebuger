#pragma once

#include "ShaderManager.h"

#include <stdarg.h>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>

#include "glmath.h"


///////////////////////////////////////////////////////////////////////////////
// Stock Shader Source Code
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Identity Shader (GLT_SHADER_IDENTITY)
// This shader does no transformations at all, and uses the current
// glColor value for fragments.
// It will shade between verticies.
static const char *szIdentityShaderVP = "attribute vec4 vVertex;"
"void main(void) "
"{ gl_Position = vVertex; "
"}";

static const char *szIdentityShaderFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"uniform vec4 vColor;"
"void main(void) "
"{ gl_FragColor = vColor;"
"}";


///////////////////////////////////////////////////////////////////////////////
// Flat Shader (GLT_SHADER_FLAT)
// This shader applies the given model view matrix to the verticies, 
// and uses a uniform color value.
static const char *szFlatShaderVP = "uniform mat4 mvpMatrix;"
"attribute vec4 vVertex;"
"void main(void) "
"{ gl_Position = mvpMatrix * vVertex; "
"}";

static const char *szFlatShaderFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"uniform vec4 vColor;"
"void main(void) "
"{ gl_FragColor = vColor; "
"}";

///////////////////////////////////////////////////////////////////////////////
// GLT_SHADER_SHADED
// Point light, diffuse lighting only
static const char *szShadedVP = "uniform mat4 mvpMatrix;"
"attribute vec4 vColor;"
"attribute vec4 vVertex;"
"varying vec4 vFragColor;"
"void main(void) {"
"vFragColor = vColor; "
" gl_Position = mvpMatrix * vVertex; "
"}";

static const char *szShadedFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec4 vFragColor; "
"void main(void) { "
" gl_FragColor = vFragColor; "
"}";

// GLT_SHADER_DEFAULT_LIGHT
// Simple diffuse, directional, and vertex based light
static const char *szDefaultLightVP = "uniform mat4 mvMatrix;"
"uniform mat4 pMatrix;"
"varying vec4 vFragColor;"
"attribute vec4 vVertex;"
"attribute vec3 vNormal;"
"uniform vec4 vColor;"
"void main(void) { "
" mat3 mNormalMatrix;"
" mNormalMatrix[0] = mvMatrix[0].xyz;"
" mNormalMatrix[1] = mvMatrix[1].xyz;"
" mNormalMatrix[2] = mvMatrix[2].xyz;"
" vec3 vNorm = normalize(mNormalMatrix * vNormal);"
" vec3 vLightDir = vec3(0.0, 0.0, 1.0); "
" float fDot = max(0.0, dot(vNorm, vLightDir)); "
" vFragColor.rgb = vColor.rgb * fDot;"
" vFragColor.a = vColor.a;"
" mat4 mvpMatrix;"
" mvpMatrix = pMatrix * mvMatrix;"
" gl_Position = mvpMatrix * vVertex; "
"}";


static const char *szDefaultLightFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec4 vFragColor; "
"void main(void) { "
" gl_FragColor = vFragColor; "
"}";

//GLT_SHADER_POINT_LIGHT_DIFF
// Point light, diffuse lighting only
static const char *szPointLightDiffVP = "uniform mat4 mvMatrix;"
"uniform mat4 pMatrix;"
"uniform vec3 vLightPos;"
"uniform vec4 vColor;"
"attribute vec4 vVertex;"
"attribute vec3 vNormal;"
"varying vec4 vFragColor;"
"void main(void) { "
" mat3 mNormalMatrix;"
" mNormalMatrix[0] = normalize(mvMatrix[0].xyz);"
" mNormalMatrix[1] = normalize(mvMatrix[1].xyz);"
" mNormalMatrix[2] = normalize(mvMatrix[2].xyz);"
" vec3 vNorm = normalize(mNormalMatrix * vNormal);"
" vec4 ecPosition;"
" vec3 ecPosition3;"
" ecPosition = mvMatrix * vVertex;"
" ecPosition3 = ecPosition.xyz /ecPosition.w;"
" vec3 vLightDir = normalize(vLightPos - ecPosition3);"
" float fDot = max(0.0, dot(vNorm, vLightDir)); "
" vFragColor.rgb = vColor.rgb * fDot;"
" vFragColor.a = vColor.a;"
" mat4 mvpMatrix;"
" mvpMatrix = pMatrix * mvMatrix;"
" gl_Position = mvpMatrix * vVertex; "
"}";


static const char *szPointLightDiffFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec4 vFragColor; "
"void main(void) { "
" gl_FragColor = vFragColor; "
"}";

//GLT_SHADER_TEXTURE_REPLACE
// Just put the texture on the polygons
static const char *szTextureReplaceVP = "uniform mat4 mvpMatrix;"
"attribute vec4 vVertex;"
"attribute vec2 vTexCoord0;"
"varying vec2 vTex;"
"void main(void) "
"{ vTex = vTexCoord0;"
" gl_Position = mvpMatrix * vVertex; "
"}";

static const char *szTextureReplaceFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec2 vTex;"
"uniform sampler2D textureUnit0;"
"void main(void) "
"{ gl_FragColor = texture2D(textureUnit0, vTex); "
"}";


// Just put the texture on the polygons
static const char *szTextureRectReplaceVP = "uniform mat4 mvpMatrix;"
"attribute vec4 vVertex;"
"attribute vec2 vTexCoord0;"
"varying vec2 vTex;"
"void main(void) "
"{ vTex = vTexCoord0;"
" gl_Position = mvpMatrix * vVertex; "
"}";

static const char *szTextureRectReplaceFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec2 vTex;"
"uniform sampler2DRect textureUnit0;"
"void main(void) "
"{ gl_FragColor = texture2DRect(textureUnit0, vTex); "
"}";



//GLT_SHADER_TEXTURE_MODULATE
// Just put the texture on the polygons, but multiply by the color (as a unifomr)
static const char *szTextureModulateVP = "uniform mat4 mvpMatrix;"
"attribute vec4 vVertex;"
"attribute vec2 vTexCoord0;"
"varying vec2 vTex;"
"void main(void) "
"{ vTex = vTexCoord0;"
" gl_Position = mvpMatrix * vVertex; "
"}";

static const char *szTextureModulateFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec2 vTex;"
"uniform sampler2D textureUnit0;"
"uniform vec4 vColor;"
"void main(void) "
"{ gl_FragColor = vColor * texture2D(textureUnit0, vTex); "
"}";



//GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF
// Point light (Diffuse only), with texture (modulated)
static const char *szTexturePointLightDiffVP = "uniform mat4 mvMatrix;"
"uniform mat4 pMatrix;"
"uniform vec3 vLightPos;"
"uniform vec4 vColor;"
"attribute vec4 vVertex;"
"attribute vec3 vNormal;"
"varying vec4 vFragColor;"
"attribute vec2 vTexCoord0;"
"varying vec2 vTex;"
"void main(void) { "
" mat3 mNormalMatrix;"
" mNormalMatrix[0] = normalize(mvMatrix[0].xyz);"
" mNormalMatrix[1] = normalize(mvMatrix[1].xyz);"
" mNormalMatrix[2] = normalize(mvMatrix[2].xyz);"
" vec3 vNorm = normalize(mNormalMatrix * vNormal);"
" vec4 ecPosition;"
" vec3 ecPosition3;"
" ecPosition = mvMatrix * vVertex;"
" ecPosition3 = ecPosition.xyz /ecPosition.w;"
" vec3 vLightDir = normalize(vLightPos - ecPosition3);"
" float fDot = max(0.0, dot(vNorm, vLightDir)); "
" vFragColor.rgb = vColor.rgb * fDot;"
" vFragColor.a = vColor.a;"
" vTex = vTexCoord0;"
" mat4 mvpMatrix;"
" mvpMatrix = pMatrix * mvMatrix;"
" gl_Position = mvpMatrix * vVertex; "
"}";


static const char *szTexturePointLightDiffFP =
#ifdef OPENGL_ES
"precision mediump float;"
#endif
"varying vec4 vFragColor;"
"varying vec2 vTex;"
"uniform sampler2D textureUnit0;"
"void main(void) { "
" gl_FragColor = vFragColor * texture2D(textureUnit0, vTex);"
"}";

// There is a static block allocated for loading shaders to 
// prevent heap fragmentation
#define MAX_SHADER_LENGTH   8192

int gltLoadShaderFile(const char *szFile, GLuint shader);

void gltLoadShaderSrc(const char *szShaderSrc, GLuint shader)
{
	GLchar *fsStringPtr[1];

	fsStringPtr[0] = (GLchar *)szShaderSrc;
	glShaderSource(shader, 1, (const GLchar **)fsStringPtr, NULL);
}

/////////////////////////////////////////////////////////////////
// Load a pair of shaders, compile, and link together. Specify the complete
// file path for each shader. Note, there is no support for
// just loading say a vertex program... you have to do both.
GLuint gltLoadShaderPairSrc(const char *szVertexSrc, const char *szFragmentSrc)
{
	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;
	GLuint hReturn = 0;
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. 
	gltLoadShaderSrc(szVertexSrc, hVertexShader);
	gltLoadShaderSrc(szFragmentSrc, hFragmentShader);

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Link them - assuming it works...
	hReturn = glCreateProgram();
	glAttachShader(hReturn, hVertexShader);
	glAttachShader(hReturn, hFragmentShader);
	glLinkProgram(hReturn);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);

	// Make sure link worked too
	glGetProgramiv(hReturn, GL_LINK_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteProgram(hReturn);
		return (GLuint)0;
	}

	return hReturn;
}

/////////////////////////////////////////////////////////////////
// Load a pair of shaders, compile, and link together. Specify the complete
// file path for each shader. Note, there is no support for
// just loading say a vertex program... you have to do both.
GLuint gltLoadShaderPair(const char *szVertexProg, const char *szFragmentProg)
{
	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;
	GLuint hReturn = 0;
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. If fail clean up and return null
	if (!gltLoadShaderFile(szVertexProg, hVertexShader))
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	if (!gltLoadShaderFile(szFragmentProg, hFragmentShader))
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Link them - assuming it works...
	hReturn = glCreateProgram();
	glAttachShader(hReturn, hVertexShader);
	glAttachShader(hReturn, hFragmentShader);

	glLinkProgram(hReturn);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);

	// Make sure link worked too
	glGetProgramiv(hReturn, GL_LINK_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteProgram(hReturn);
		return (GLuint)0;
	}

	return hReturn;
}


////////////////////////////////////////////////////////////////
// Load the shader from the specified file. Returns false if the
// shader could not be loaded
int gltLoadShaderFile(const char *szFile, GLuint shader)
{
	GLint shaderLength = 0;
	FILE *fp;
	static GLubyte shaderText[MAX_SHADER_LENGTH];


	// Open the shader file
	fopen_s(&fp, szFile, "r");
	if (fp != NULL)
	{
		// See how long the file is
		while (fgetc(fp) != EOF)
			shaderLength++;

		// Allocate a block of memory to send in the shader
		assert(shaderLength < MAX_SHADER_LENGTH);   // make me bigger!
		if (shaderLength > MAX_SHADER_LENGTH)
		{
			fclose(fp);
			return 0;
		}

		// Go back to beginning of file
		rewind(fp);

		// Read the whole file in
		if (shaderText != NULL)
			fread(shaderText, 1, shaderLength, fp);

		// Make sure it is null terminated and close the file
		shaderText[shaderLength] = '\0';
		fclose(fp);
	}
	else
		return 0;

	// Load the string
	gltLoadShaderSrc((const char *)shaderText, shader);

	return 1;
}


/////////////////////////////////////////////////////////////////
// Load a pair of shaders, compile, and link together. Specify the complete
// source text for each shader. After the shader names, specify the number
// of attributes, followed by the index and attribute name of each attribute
GLuint gltLoadShaderPairWithAttributes(const char *szVertexProg, const char *szFragmentProg, ...)
{
	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;
	GLuint hReturn = 0;
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. If fail clean up and return null
	// Vertex Program
	if (gltLoadShaderFile(szVertexProg, hVertexShader) == 0)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		fprintf(stderr, "The shader at %s could ot be found.\n", szVertexProg);
		return (GLuint)0;
	}

	// Fragment Program
	if (gltLoadShaderFile(szFragmentProg, hFragmentShader) == 0)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		fprintf(stderr, "The shader at %s  could not be found.\n", szFragmentProg);
		return (GLuint)0;
	}

	// Compile them both
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors in vertex shader
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		char infoLog[1024];
		glGetShaderInfoLog(hVertexShader, 1024, NULL, infoLog);
		fprintf(stderr, "The shader at %s failed to compile with the following error:\n%s\n", szVertexProg, infoLog);
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Check for errors in fragment shader
	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		char infoLog[1024];
		glGetShaderInfoLog(hFragmentShader, 1024, NULL, infoLog);
		fprintf(stderr, "The shader at %s failed to compile with the following error:\n%s\n", szFragmentProg, infoLog);
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Create the final program object, and attach the shaders
	hReturn = glCreateProgram();
	glAttachShader(hReturn, hVertexShader);
	glAttachShader(hReturn, hFragmentShader);


	// Now, we need to bind the attribute names to their specific locations
	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentProg);

	// Iterate over this argument list
	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for (int i = 0; i < iArgCount; i++)
	{
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(hReturn, index, szNextArg);
	}
	va_end(attributeList);

	// Attempt to link    
	glLinkProgram(hReturn);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);

	// Make sure link worked too
	glGetProgramiv(hReturn, GL_LINK_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		char infoLog[1024];
		glGetProgramInfoLog(hReturn, 1024, NULL, infoLog);
		fprintf(stderr, "The programs %s and %s failed to link with the following errors:\n%s\n",
			szVertexProg, szFragmentProg, infoLog);
		glDeleteProgram(hReturn);
		return (GLuint)0;
	}

	// All done, return our ready to use shader program
	return hReturn;
}


GLuint gltLoadShaderPairSrcWithAttributes(const char *szVertexSrc, const char *szFragmentSrc, ...)
{
	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;
	GLuint hReturn = 0;
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. 
	gltLoadShaderSrc(szVertexSrc, hVertexShader);
	gltLoadShaderSrc(szFragmentSrc, hFragmentShader);

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Link them - assuming it works...
	hReturn = glCreateProgram();
	glAttachShader(hReturn, hVertexShader);
	glAttachShader(hReturn, hFragmentShader);

	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentSrc);

	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for (int i = 0; i < iArgCount; i++)
	{
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(hReturn, index, szNextArg);
	}
	va_end(attributeList);


	glLinkProgram(hReturn);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);

	// Make sure link worked too
	glGetProgramiv(hReturn, GL_LINK_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteProgram(hReturn);
		return (GLuint)0;
	}

	return hReturn;
}


void PPSM_AllocShaderManager(PPShaderManager _ppsm)
{
	if (_ppsm == NULL)
		return;
	*_ppsm = (PShaderManager)malloc(sizeof(ShaderManager));
	for (unsigned int i = 0; i < GLT_SHADER_LAST; i++)
		(*_ppsm)->uiStockShaders[i] = 0;
}

void PPSM_ReleaseShaderManager(PPShaderManager _ppsm)
{
	if (_ppsm != NULL && *_ppsm != NULL)
	{
		for (unsigned int i = 0; i < GLT_SHADER_LAST; i++)
			glDeleteProgram((*_ppsm)->uiStockShaders[i]);
		free(*_ppsm);
	}
}

int PPSM_InitializeStockShaders(PPShaderManager _ppsm)
{
	GLuint* uiStockShaders;
	if (_ppsm != NULL && *_ppsm != NULL)
	{
		uiStockShaders = (*_ppsm)->uiStockShaders;

		uiStockShaders[GLT_SHADER_IDENTITY] = gltLoadShaderPairSrcWithAttributes(szIdentityShaderVP, szIdentityShaderFP, 1, GLT_ATTRIBUTE_VERTEX, "vVertex");
		uiStockShaders[GLT_SHADER_FLAT] = gltLoadShaderPairSrcWithAttributes(szFlatShaderVP, szFlatShaderFP, 1, GLT_ATTRIBUTE_VERTEX, "vVertex");
		uiStockShaders[GLT_SHADER_SHADED] = gltLoadShaderPairSrcWithAttributes(szShadedVP, szShadedFP, 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");


		uiStockShaders[GLT_SHADER_DEFAULT_LIGHT] = gltLoadShaderPairSrcWithAttributes(szDefaultLightVP, szDefaultLightFP, 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");

		uiStockShaders[GLT_SHADER_POINT_LIGHT_DIFF] = gltLoadShaderPairSrcWithAttributes(szPointLightDiffVP, szPointLightDiffFP, 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");

		uiStockShaders[GLT_SHADER_TEXTURE_REPLACE] = gltLoadShaderPairSrcWithAttributes(szTextureReplaceVP, szTextureReplaceFP, 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

		uiStockShaders[GLT_SHADER_TEXTURE_MODULATE] = gltLoadShaderPairSrcWithAttributes(szTextureModulateVP, szTextureModulateFP, 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

		uiStockShaders[GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF] = gltLoadShaderPairSrcWithAttributes(szTexturePointLightDiffVP, szTexturePointLightDiffFP, 3,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");


		uiStockShaders[GLT_SHADER_TEXTURE_RECT_REPLACE] = gltLoadShaderPairSrcWithAttributes(szTextureRectReplaceVP, szTextureRectReplaceFP, 2,
			GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_TEXTURE0, "vTexCoord0");

		if (uiStockShaders[0] != 0)
			return 1;
	}
	return 0;
}

// Find one of the standard stock shaders and return it's shader handle. 
GLuint PPSM_GetStockShader(PPShaderManager _ppsm, enum GLT_STOCK_SHADER nShaderID)
{
	GLuint ret = 0;
	if (_ppsm != NULL && *_ppsm != NULL)
	{
		if (nShaderID < GLT_SHADER_LAST)
			ret = (*_ppsm)->uiStockShaders[nShaderID];
	}
	return ret;
}

// Use a stock shader, and pass in the parameters needed
GLint PPSM_UseStockShader(PPShaderManager _ppsm, enum GLT_STOCK_SHADER nShaderID, ...)
{
	if (_ppsm != NULL && *_ppsm != NULL)
	{
		GLuint* uiStockShaders = (*_ppsm)->uiStockShaders;
		// Check for out of bounds
		if (nShaderID >= GLT_SHADER_LAST)
			return -1;

		// List of uniforms
		va_list uniformList;
		va_start(uniformList, nShaderID);

		// Bind to the correct shader
		glUseProgram(uiStockShaders[nShaderID]);

		// Set up the uniforms
		GLint iTransform, iModelMatrix, iProjMatrix, iColor, iLight, iTextureUnit;
		int				iInteger;
		M3DMatrix44f* mvpMatrix;
		M3DMatrix44f*  pMatrix;
		M3DMatrix44f*  mvMatrix;
		M3DVector4f*  vColor;
		M3DVector3f*  vLightPos;

		switch (nShaderID)
		{
		case GLT_SHADER_FLAT:			// Just the modelview projection matrix and the color
			iTransform = glGetUniformLocation(uiStockShaders[nShaderID], "mvpMatrix");
			mvpMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iTransform, 1, GL_FALSE, *mvpMatrix);

			iColor = glGetUniformLocation(uiStockShaders[nShaderID], "vColor");
			vColor = va_arg(uniformList, M3DVector4f*);
			glUniform4fv(iColor, 1, *vColor);
			break;

		case GLT_SHADER_TEXTURE_RECT_REPLACE:
		case GLT_SHADER_TEXTURE_REPLACE:	// Just the texture place
			iTransform = glGetUniformLocation(uiStockShaders[nShaderID], "mvpMatrix");
			mvpMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iTransform, 1, GL_FALSE, *mvpMatrix);

			iTextureUnit = glGetUniformLocation(uiStockShaders[nShaderID], "textureUnit0");
			iInteger = va_arg(uniformList, int);
			glUniform1i(iTextureUnit, iInteger);
			break;

		case GLT_SHADER_TEXTURE_MODULATE: // Multiply the texture by the geometry color
			iTransform = glGetUniformLocation(uiStockShaders[nShaderID], "mvpMatrix");
			mvpMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iTransform, 1, GL_FALSE, *mvpMatrix);

			iColor = glGetUniformLocation(uiStockShaders[nShaderID], "vColor");
			vColor = va_arg(uniformList, M3DVector4f*);
			glUniform4fv(iColor, 1, *vColor);

			iTextureUnit = glGetUniformLocation(uiStockShaders[nShaderID], "textureUnit0");
			iInteger = va_arg(uniformList, int);
			glUniform1i(iTextureUnit, iInteger);
			break;


		case GLT_SHADER_DEFAULT_LIGHT:
			iModelMatrix = glGetUniformLocation(uiStockShaders[nShaderID], "mvMatrix");
			mvMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iModelMatrix, 1, GL_FALSE, *mvMatrix);

			iProjMatrix = glGetUniformLocation(uiStockShaders[nShaderID], "pMatrix");
			pMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iProjMatrix, 1, GL_FALSE, *pMatrix);

			iColor = glGetUniformLocation(uiStockShaders[nShaderID], "vColor");
			vColor = va_arg(uniformList, M3DVector4f*);
			glUniform4fv(iColor, 1, *vColor);
			break;

		case GLT_SHADER_POINT_LIGHT_DIFF:
			iModelMatrix = glGetUniformLocation(uiStockShaders[nShaderID], "mvMatrix");
			mvMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iModelMatrix, 1, GL_FALSE, *mvMatrix);

			iProjMatrix = glGetUniformLocation(uiStockShaders[nShaderID], "pMatrix");
			pMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iProjMatrix, 1, GL_FALSE, *pMatrix);

			iLight = glGetUniformLocation(uiStockShaders[nShaderID], "vLightPos");
			vLightPos = va_arg(uniformList, M3DVector3f*);
			glUniform3fv(iLight, 1, *vLightPos);

			iColor = glGetUniformLocation(uiStockShaders[nShaderID], "vColor");
			vColor = va_arg(uniformList, M3DVector4f*);
			glUniform4fv(iColor, 1, *vColor);
			break;

		case GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF:
			iModelMatrix = glGetUniformLocation(uiStockShaders[nShaderID], "mvMatrix");
			mvMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iModelMatrix, 1, GL_FALSE, *mvMatrix);

			iProjMatrix = glGetUniformLocation(uiStockShaders[nShaderID], "pMatrix");
			pMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iProjMatrix, 1, GL_FALSE, *pMatrix);

			iLight = glGetUniformLocation(uiStockShaders[nShaderID], "vLightPos");
			vLightPos = va_arg(uniformList, M3DVector3f*);
			glUniform3fv(iLight, 1, *vLightPos);

			iColor = glGetUniformLocation(uiStockShaders[nShaderID], "vColor");
			vColor = va_arg(uniformList, M3DVector4f*);
			glUniform4fv(iColor, 1, *vColor);

			iTextureUnit = glGetUniformLocation(uiStockShaders[nShaderID], "textureUnit0");
			iInteger = va_arg(uniformList, int);
			glUniform1i(iTextureUnit, iInteger);
			break;


		case GLT_SHADER_SHADED:		// Just the modelview projection matrix. Color is an attribute
			iTransform = glGetUniformLocation(uiStockShaders[nShaderID], "mvpMatrix");
			pMatrix = va_arg(uniformList, M3DMatrix44f*);
			glUniformMatrix4fv(iTransform, 1, GL_FALSE, *pMatrix);
			break;

		case GLT_SHADER_IDENTITY:	// Just the Color
			iColor = glGetUniformLocation(uiStockShaders[nShaderID], "vColor");
			vColor = va_arg(uniformList, M3DVector4f*);
			glUniform4fv(iColor, 1, *vColor);
		default:
			break;
		}
		va_end(uniformList);

		return uiStockShaders[nShaderID];
	}
	return 0;
}

// Load a shader pair from file, return NULL or shader handle. 
// Vertex program name (minus file extension)
// is saved in the lookup table
GLuint PPSM_LoadShaderPair(PPShaderManager _ppsm, const char *szVertexProgFileName, const char *szFragProgFileName)
{
	struct SHADERLOOKUPETRY shaderEntry;

	// Make sure it's not already loaded
	GLuint uiReturn = PPSM_LookupShader(_ppsm, szVertexProgFileName, szFragProgFileName);
	if (uiReturn != 0)
		return uiReturn;

	// Load shader and test for fail
	shaderEntry.uiShaderID = gltLoadShaderPair(szVertexProgFileName, szFragProgFileName);
	if (shaderEntry.uiShaderID == 0)
		return 0;

	// Add to the table
	strncpy_s(shaderEntry.szVertexShaderName, MAX_SHADER_NAME_LENGTH, szVertexProgFileName, MAX_SHADER_NAME_LENGTH);
	strncpy_s(shaderEntry.szFragShaderName, MAX_SHADER_NAME_LENGTH, szFragProgFileName, MAX_SHADER_NAME_LENGTH);
	//	shaderTable.push_back(shaderEntry);	
	return shaderEntry.uiShaderID;
}

// Load shaders from source text.
GLuint PPSM_LoadShaderPairSrc(PPShaderManager _ppsm, const char *szName, const char *szVertexSrc, const char *szFragSrc)
{
	// Just make it and return
	if (szName == NULL)
		return gltLoadShaderPairSrc(szVertexSrc, szFragSrc);

	// It has a name, check for duplicate
	GLuint uiShader = PPSM_LookupShader(_ppsm, szName, szName);
	if (uiShader != 0)
		return uiShader;

	// Ok, make it and add to table
	struct SHADERLOOKUPETRY shaderEntry;
	shaderEntry.uiShaderID = gltLoadShaderPairSrc(szVertexSrc, szFragSrc);
	if (shaderEntry.uiShaderID == 0)
		return 0;	// Game over, won't compile

	// Add it...
	strncpy_s(shaderEntry.szVertexShaderName, MAX_SHADER_NAME_LENGTH, szName, MAX_SHADER_NAME_LENGTH);
	strncpy_s(shaderEntry.szFragShaderName, MAX_SHADER_NAME_LENGTH, szName, MAX_SHADER_NAME_LENGTH);
	//	shaderTable.push_back(shaderEntry);	
	return shaderEntry.uiShaderID;
}

// Ditto above, but pop in the attributes
GLuint PPSM_LoadShaderPairWithAttributes(PPShaderManager _ppsm, const char *szVertexProgFileName, const char *szFragmentProgFileName, ...)
{
	// Check for duplicate
	GLuint uiShader = PPSM_LookupShader(_ppsm,szVertexProgFileName, szFragmentProgFileName);
	if (uiShader != 0)
		return uiShader;

	struct SHADERLOOKUPETRY shaderEntry;

	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. If fail clean up and return null
	if (!gltLoadShaderFile(szVertexProgFileName, hVertexShader))
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	if (!gltLoadShaderFile(szFragmentProgFileName, hFragmentShader))
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return 0;
	}

	// Link them - assuming it works...
	shaderEntry.uiShaderID = glCreateProgram();
	glAttachShader(shaderEntry.uiShaderID, hVertexShader);
	glAttachShader(shaderEntry.uiShaderID, hFragmentShader);


	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentProgFileName);

	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for (int i = 0; i < iArgCount; i++)
	{
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(shaderEntry.uiShaderID, index, szNextArg);
	}

	va_end(attributeList);

	glLinkProgram(shaderEntry.uiShaderID);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);

	// Make sure link worked too
	glGetProgramiv(shaderEntry.uiShaderID, GL_LINK_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteProgram(shaderEntry.uiShaderID);
		return 0;
	}


	// Add it...
	strncpy_s(shaderEntry.szVertexShaderName, MAX_SHADER_NAME_LENGTH, szVertexProgFileName, MAX_SHADER_NAME_LENGTH);
	strncpy_s(shaderEntry.szFragShaderName, MAX_SHADER_NAME_LENGTH, szFragmentProgFileName, MAX_SHADER_NAME_LENGTH);
	//	shaderTable.push_back(shaderEntry);	
	return shaderEntry.uiShaderID;
}

GLuint PPSM_LoadShaderPairSrcWithAttributes(PPShaderManager _ppsm, const char *szName, const char *szVertexSrc, const char *szFragmentSrc, ...)
{
	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader;
	GLuint hReturn = 0;
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. 
	gltLoadShaderSrc(szVertexSrc, hVertexShader);
	gltLoadShaderSrc(szFragmentSrc, hFragmentShader);

	// Compile them
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)0;
	}

	// Link them - assuming it works...
	hReturn = glCreateProgram();
	glAttachShader(hReturn, hVertexShader);
	glAttachShader(hReturn, hFragmentShader);

	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentSrc);

	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for (int i = 0; i < iArgCount; i++)
	{
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(hReturn, index, szNextArg);
	}
	va_end(attributeList);


	glLinkProgram(hReturn);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);

	// Make sure link worked too
	glGetProgramiv(hReturn, GL_LINK_STATUS, &testVal);
	if (testVal == GL_FALSE)
	{
		glDeleteProgram(hReturn);
		return (GLuint)0;
	}

	return hReturn;
}

// Lookup a previously loaded shader
GLuint PPSM_LookupShader(PPShaderManager _ppsm, const char *szVertexProg, const char *szFragProg)
{
	GLuint ret = 0;
	return ret;
}
