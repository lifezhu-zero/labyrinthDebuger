/*
GLBatch.cpp

Copyright (c) 2009, Richard S. Wright Jr.
GLTools Open Source Library
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other
materials provided with the distribution.

Neither the name of Richard S. Wright Jr. nor the names of other contributors may be used
to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __GLT_BATCH
#define __GLT_BATCH

#include "Batch.h"
#include "ShaderManager.h"

#include <malloc.h>


//////////////////////// TEMPORARY TEMPORARY TEMPORARY - On SnowLeopard this is suppored, but GLEW doens't hook up properly
//////////////////////// Fixed probably in 10.6.3
#ifdef __APPLE__
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glDeleteVertexArrays  glDeleteVertexArraysAPPLE
#define glBindVertexArray	glBindVertexArrayAPPLE
#endif

/////////////////////// OpenGL ES support on iPhone/iPad
#ifdef OPENGL_ES
#define GL_WRITE_ONLY   GL_WRITE_ONLY_OES
#define glMapBuffer     glMapBufferOES
#define glUnmapBuffer   glUnmapBufferOES
#endif

void GLB_Alloc(PPGLBatch	_item)
{
	if (_item != NULL)
	{
		PGLBatch item = (PGLBatch)malloc(sizeof(GLBatch));
		item->nNumTextureUnits = 0;
		item->nNumVerts = 0;
		item->pVerts = NULL;
		item->pNormals = NULL;
		item->pColors = NULL;
		item->pTexCoords = NULL;
		item->uiVertexArray = 0;
		item->uiNormalArray = 0;
		item->uiColorArray = 0;
		item->vertexArrayObject = 0;
		item->bBatchDone = 0;
		item->nVertsBuilding = 0;
		item->uiTextureCoordArray = NULL;
		*_item = item;
	}
}

void GLB_Release(PPGLBatch _item)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// Vertex buffer objects
		if (item->uiVertexArray != 0)
			glDeleteBuffers(1, &item->uiVertexArray);

		if (item->uiNormalArray != 0)
			glDeleteBuffers(1, &item->uiNormalArray);

		if (item->uiColorArray != 0)
			glDeleteBuffers(1, &item->uiColorArray);

		for (unsigned int i = 0; i < item->nNumTextureUnits; i++)
			glDeleteBuffers(1, &item->uiTextureCoordArray[i]);

#ifndef OPENGL_ES
		glDeleteVertexArrays(1, &item->vertexArrayObject);
#endif

		free(item->uiTextureCoordArray);
		free(item->pTexCoords);
	}
}


// Start the primitive batch.
void GLB_Begin(PPGLBatch _item, GLenum primitive, GLuint nVerts, GLuint nTextureUnits)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		item->primitiveType = primitive;
		item->nNumVerts = nVerts;

		if (nTextureUnits > 4)   // Limit to four texture units
			nTextureUnits = 4;

		item->nNumTextureUnits = nTextureUnits;

		if (item->nNumTextureUnits != 0) {
			item->uiTextureCoordArray = (GLuint*)malloc(sizeof(GLuint)*item->nNumTextureUnits);

			// An array of pointers to texture coordinate arrays
			item->pTexCoords = (M3DVector2f**)malloc(sizeof(M3DVector2f*)*item->nNumTextureUnits);
			for (unsigned int i = 0; i < item->nNumTextureUnits; i++) {
				item->uiTextureCoordArray[i] = 0;
				item->pTexCoords[i] = NULL;
			}
		}

		// Vertex Array object for this Array
#ifndef OPENGL_ES
		glGenVertexArrays(1, &item->vertexArrayObject);
		glBindVertexArray(item->vertexArrayObject);
#endif
	}
}


// Block Copy in vertex data
void GLB_CopyVertexData3f(PPGLBatch _item, M3DVector3f *vVerts)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First time, create the buffer object, allocate the space
		if (item->uiVertexArray == 0) {
			glGenBuffers(1, &item->uiVertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * item->nNumVerts, vVerts, GL_DYNAMIC_DRAW);
		}
		else { // Just bind to existing object
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);

			// Copy the data in
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * item->nNumVerts, vVerts);
			item->pVerts = NULL;
		}
	}
}

// Block copy in normal data
void GLB_CopyNormalDataf(PPGLBatch _item, M3DVector3f *vNorms)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First time, create the buffer object, allocate the space
		if (item->uiNormalArray == 0) {
			glGenBuffers(1, &item->uiNormalArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * item->nNumVerts, vNorms, GL_DYNAMIC_DRAW);
		}
		else {	// Just bind to existing object
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);

			// Copy the data in
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 3 * item->nNumVerts, vNorms);
			item->pNormals = NULL;
		}
	}
}

void GLB_CopyColorData4f(PPGLBatch _item, M3DVector4f *vColors)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First time, create the buffer object, allocate the space
		if (item->uiColorArray == 0) {
			glGenBuffers(1, &item->uiColorArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * item->nNumVerts, vColors, GL_DYNAMIC_DRAW);
		}
		else {	// Just bind to existing object
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);

			// Copy the data in
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 4 * item->nNumVerts, vColors);
			item->pColors = NULL;
		}
	}
}

void GLB_CopyTexCoordData2f(PPGLBatch _item, M3DVector2f *vTexCoords, GLuint uiTextureLayer)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First time, create the buffer object, allocate the space
		if (item->uiTextureCoordArray[uiTextureLayer] == 0) {
			glGenBuffers(1, &item->uiTextureCoordArray[uiTextureLayer]);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[uiTextureLayer]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * item->nNumVerts, vTexCoords, GL_DYNAMIC_DRAW);
		}
		else {	// Just bind to existing object
			glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[uiTextureLayer]);

			// Copy the data in
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2 * item->nNumVerts, vTexCoords);
			item->pTexCoords[uiTextureLayer] = NULL;
		}
	}
}

// Bind everything up in a little package
void GLB_End(PPGLBatch _item)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
#ifndef OPENGL_ES
		// Check to see if items have been added one at a time
		if (item->pVerts != NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			item->pVerts = NULL;
		}

		if (item->pColors != NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			item->pColors = NULL;
		}

		if (item->pNormals != NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			glUnmapBuffer(GL_ARRAY_BUFFER);
			item->pNormals = NULL;
		}

		for (unsigned int i = 0; i < item->nNumTextureUnits; i++)
			if (item->pTexCoords[i] != NULL) {
				glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[i]);
				glUnmapBuffer(GL_ARRAY_BUFFER);
				item->pTexCoords[i] = NULL;
			}

		// Set up the vertex array object
		glBindVertexArray(item->vertexArrayObject);
#endif

		if (item->uiVertexArray != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (item->uiColorArray != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (item->uiNormalArray != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		// How many texture units
		for (unsigned int i = 0; i < item->nNumTextureUnits; i++)
			if (item->uiTextureCoordArray[i] != 0) {
				glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i),
					glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[i]);
				glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0 + i, 2, GL_FLOAT, GL_FALSE, 0, 0);
			}

		item->bBatchDone = 1;
#ifndef OPENGL_ES
		glBindVertexArray(0);
#endif
	}
}


// Just start over. No reallocations, etc.
void GLB_Reset(PPGLBatch _item)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		item->bBatchDone = 0;
		item->nVertsBuilding = 0;
	}
}


// Add a single vertex to the end of the array
void GLB_Vertex3f(PPGLBatch _item, GLfloat x, GLfloat y, GLfloat z)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiVertexArray == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiVertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pVerts == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			item->pVerts = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		item->pVerts[item->nVertsBuilding][0] = x;
		item->pVerts[item->nVertsBuilding][1] = y;
		item->pVerts[item->nVertsBuilding][2] = z;
		item->nVertsBuilding++;
	}
}

void GLB_Vertex3fv(PPGLBatch _item, M3DVector3f vVertex)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;

		// First see if the vertex array buffer has been created...
		if (item->uiVertexArray == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiVertexArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pVerts == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			item->pVerts = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		memcpy(item->pVerts[item->nVertsBuilding], vVertex, sizeof(M3DVector3f));
		item->nVertsBuilding++;
	}
}

// Unlike normal OpenGL immediate mode, you must specify a normal per vertex
// or you will get junk...
void GLB_Normal3f(PPGLBatch _item, GLfloat x, GLfloat y, GLfloat z)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiNormalArray == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiNormalArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pNormals == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			item->pNormals = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		item->pNormals[item->nVertsBuilding][0] = x;
		item->pNormals[item->nVertsBuilding][1] = y;
		item->pNormals[item->nVertsBuilding][2] = z;
	}
}

// Ditto above
void GLB_Normal3fv(PPGLBatch _item, M3DVector3f vNormal)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiNormalArray == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiNormalArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pNormals == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			item->pNormals = (M3DVector3f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		memcpy(item->pNormals[item->nVertsBuilding], vNormal, sizeof(M3DVector3f));
	}
}


void GLB_Color4f(PPGLBatch _item, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiColorArray == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiColorArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pColors == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			item->pColors = (M3DVector4f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		item->pColors[item->nVertsBuilding][0] = r;
		item->pColors[item->nVertsBuilding][1] = g;
		item->pColors[item->nVertsBuilding][2] = b;
		item->pColors[item->nVertsBuilding][3] = a;
	}
}

void GLB_Color4fv(PPGLBatch _item, M3DVector4f vColor)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiColorArray == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiColorArray);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pColors == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			item->pColors = (M3DVector4f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		memcpy(item->pColors[item->nVertsBuilding], vColor, sizeof(M3DVector4f));
	}
}

// Unlike normal OpenGL immediate mode, you must specify a texture coord
// per vertex or you will get junk...
void GLB_MultiTexCoord2f(PPGLBatch _item, GLuint texture, GLclampf s, GLclampf t)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiTextureCoordArray[texture] == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiTextureCoordArray[texture]);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[texture]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pTexCoords[texture] == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[texture]);
			item->pTexCoords[texture] = (M3DVector2f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		item->pTexCoords[texture][item->nVertsBuilding][0] = s;
		item->pTexCoords[texture][item->nVertsBuilding][1] = t;
	}
}

// Ditto above  
void GLB_MultiTexCoord2fv(PPGLBatch _item, GLuint texture, M3DVector2f vTexCoord)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		// First see if the vertex array buffer has been created...
		if (item->uiTextureCoordArray[texture] == 0) {	// Nope, we need to create it
			glGenBuffers(1, &item->uiTextureCoordArray[texture]);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[texture]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * item->nNumVerts, NULL, GL_DYNAMIC_DRAW);
		}

		// Now see if it's already mapped, if not, map it
		if (item->pTexCoords[texture] == NULL) {
			glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[texture]);
			item->pTexCoords[texture] = (M3DVector2f *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		}

		// Ignore if we go past the end, keeps things from blowing up
		if (item->nVertsBuilding >= item->nNumVerts)
			return;

		// Copy it in...
		memcpy(item->pTexCoords[texture], vTexCoord, sizeof(M3DVector2f));
	}
}


void GLB_Draw(PPGLBatch _item)
{
	if (_item != NULL && *_item != NULL)
	{
		PGLBatch item = *_item;
		if (!item->bBatchDone)
			return;

#ifndef OPENGL_ES
		// Set up the vertex array object
		glBindVertexArray(item->vertexArrayObject);
#else
		if (item->uiVertexArray != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiVertexArray);
			glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (item->uiColorArray != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiColorArray);
			glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (item->uiNormalArray != 0) {
			glEnableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
			glBindBuffer(GL_ARRAY_BUFFER, item->uiNormalArray);
			glVertexAttribPointer(GLT_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}

		// How many texture units
		for (unsigned int i = 0; i < item->nNumTextureUnits; i++)
			if (item->uiTextureCoordArray[i] != 0) {
				glEnableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i),
					glBindBuffer(GL_ARRAY_BUFFER, item->uiTextureCoordArray[i]);
				glVertexAttribPointer(GLT_ATTRIBUTE_TEXTURE0 + i, 2, GL_FLOAT, GL_FALSE, 0, 0);
			}
#endif


		glDrawArrays(item->primitiveType, 0, item->nNumVerts);

#ifndef OPENGL_ES
		glBindVertexArray(0);
#else
		glDisableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_NORMAL);
		glDisableVertexAttribArray(GLT_ATTRIBUTE_COLOR);

		for (unsigned int i = 0; i < item->nNumTextureUnits; i++)
			if (item->uiTextureCoordArray[i] != 0)
				glDisableVertexAttribArray(GLT_ATTRIBUTE_TEXTURE0 + i);

#endif
	}
}

void GLB_CopyVertexData3f2(PPGLBatch _item, GLfloat *vVerts) { GLB_CopyVertexData3f(_item, (M3DVector3f *)(vVerts)); }
void GLB_CopyNormalDataf2(PPGLBatch _item, GLfloat *vNorms) { GLB_CopyNormalDataf(_item, (M3DVector3f *)(vNorms)); }
void GLB_CopyColorData4f2(PPGLBatch _item, GLfloat *vColors) { GLB_CopyColorData4f(_item, (M3DVector4f *)(vColors)); }
void GLB_CopyTexCoordData2f2(PPGLBatch _item, GLfloat *vTex, GLuint uiTextureLayer) { GLB_CopyTexCoordData2f(_item, (M3DVector2f *)(vTex), uiTextureLayer); }

#endif
