/**
 ** Modelgl.cc - OpenGL methods for a 3D model class.
 **
 ** Written: 4/17/02 - JSF
 **/

#include "model.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL_image.h"
//#define HAVE_PNG_H
//#include "pngio.h"

using namespace Exult3d;

/*
 *  Read texture file for OpenGL.
 *
 *  Output: False if error.
 */
bool Material::read_texture(
) {
	const char *fname = texture_filename.c_str();
	SDL_Surface *image = IMG_Load(fname);
	if (!image)
		return false;       // Failed.
	glGenTextures(1, &texture_id);  // Generate (empty) texture.
	glBindTexture(GL_TEXTURE_2D, texture_id);
	// Stick in the data.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h,
	             0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
	SDL_FreeSurface(image);
#if 0
	// Find file extension.
	const char *ext = strrchr(fname, '.');
	if (!ext)
		return false;       // Can't handle it.
	// Read in .png.
	if (strcasecmp(ext, ".png") == 0) {
		int width, height, rowbytes, xoff, yoff;
		unsigned char *pixels;  // Want last row, first.
		if (!Import_png32(fname, width, height, rowbytes,
		                  xoff, yoff, pixels, true))
			return false;
		glGenTextures(1, &texture_id);  // Generate (empty) texture.
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		             GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		delete pixels;
	} else if (strcasecmp(ext, ".bmp") == 0) {
		// Load from a .bmp.
		SDL_Surface *image = SDL_LoadBMP(fname);
		if (!image)
			return false;   // Failed.
		glGenTextures(1, &texture_id);  // Generate (empty) texture.
		glBindTexture(GL_TEXTURE_2D, texture_id);
		// Stick in the data.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h,
		             0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
		SDL_FreeSurface(image);
	} else
		return false;       // We don't handle this yet.
#endif
	// Linear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	texture_loaded = true;
	return true;
}

/*
 *  Render at the current position.
 */
void Object3d::render(
) {
	Material *prev_mat = (Material *) - 1;
	glColor3ub(255, 255, 255);  // Reset color to white.
	glBegin(GL_TRIANGLES);      // Start drawing.
	// Go through faces.
	for (vector<Face>::const_iterator faceit = faces.begin();
	        faceit != faces.end(); ++faceit) {
		Material *mat = (*faceit).get_material();
		if (mat != prev_mat) {  // Different than prev.?
			prev_mat = mat;
			if (mat && mat->texture_loaded) { // Texture?
				glEnable(GL_TEXTURE_2D);// Bind texture.
				glBindTexture(GL_TEXTURE_2D, mat->texture_id);
			} else
				glDisable(GL_TEXTURE_2D);
			if (mat) {          // Set colors.
				glMaterialfv(GL_FRONT, GL_AMBIENT,
				             mat->colors[Material::ambient]);
				glMaterialfv(GL_FRONT, GL_SPECULAR,
				             mat->colors[Material::specular]);
				glMaterialfv(GL_FRONT, GL_DIFFUSE,
				             mat->colors[Material::diffuse]);
			} else {
				static float a[] = {.2, .2, .2, 1.0};
				static float d[] = {.8, .8, .8, 1.0};
				static float s[] = {0, 0, 0, 1.0};
				glMaterialfv(GL_FRONT, GL_AMBIENT, a);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
				glMaterialfv(GL_FRONT, GL_SPECULAR, s);
			}
		}
		// Go through triangle's vertices.
		for (int v = 0; v < 3; v++) {
			unsigned int vindex = (*faceit).vertex_indices[v];
			// Set normal for vertex.
			Vector3 &norm = normals[vindex];
			glNormal3f(norm.x, norm.y, norm.z);
			if (mat && mat->texture_loaded &&
			        vindex < tex_vertices.size()) {
				// Set texture coord.
				Vector2 &tv = tex_vertices[vindex];
				glTexCoord2f(tv.x, tv.y);
			}
			// Finally, the vertex:
			Vector3 &vert = vertices[vindex];
			glVertex3f(vert.x, vert.y, vert.z);
		}
	}
	glEnd();
}

/*
 *  Load textures for the materials.
 */
void Model3d::load_textures(
) {
	for (vector<Material *>::const_iterator it = materials.begin();
	        it != materials.end(); ++it) {
		Material *mat = *it;
		if (!mat->load())
			cerr << "Error loading material '" <<
			     mat->name.c_str() << "'" << endl;
	}
}

/*
 *  Render at the current position.
 */
void Model3d::render(
) {
	// Do each object.
	for (vector<Object3d *>::const_iterator objit = objects.begin();
	        objit != objects.end(); ++objit)
		(*objit)->render();
}

