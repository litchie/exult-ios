/**
 **	Modelgl.cc - OpenGL methods for a 3D model class.
 **
 **	Written: 4/17/02 - JSF
 **/

#include "model.h"
#include <GL/gl.h>
#include <GL/glu.h>

using namespace Exult3d;

/*
 *	Render at the current position.
 */
void Object3d::render
	(
	)
	{
	if (material->texture_loaded)	// Texture?
		{
		glEnable(GL_TEXTURE_2D);
					// Bind texture.
		glBindTexture(GL_TEXTURE_2D, material->texture_id);
		}
	else
		glDisable(GL_TEXTURE_2D);
	glColor3ub(255, 255, 255);	// Reset color to white.
	glBegin(GL_TRIANGLES);		// Start drawing.
					// Go through faces.
	for (vector<Face>::const_iterator faceit = faces.begin();
					faceit != faces.end(); ++faceit)
		{
					// Go through triangle's vertices.
		for (int v = 0; v < 3; v++)
			{
			int vindex = (*faceit).vertex_indices[v];
					// Set normal for vertex.
			glNormal3f(normals[vindex].x, normals[vindex].y,
							normals[vindex].z);
			if (material->texture_loaded &&
			    vindex < tex_vertices.size())
					// Set texture coord.
				glTexCoord2f(tex_vertices[vindex].x,
						tex_vertices[vindex].y);
			else
					// Set color.
				glColor3ub(material->r, material->g,
								material->b);
					// Finally, the vertex:
			glVertex3f(vertices[vindex].x, vertices[vindex].y, 
							vertices[vindex].z);
			}
		}
	glEnd();
	}

/*
 *	Render at the current position.
 */
void Model3d::render
	(
	)
	{
					// Do each object.
	for (vector<Object3d *>::const_iterator objit = objects.begin();
					objit != objects.end(); ++objit)
		(*objit)->render();
	}

