/**
 **	Model.cc - 3D model classes (for OpenGL).
 **
 **	Written: 4/16/02 - JSF
 **/

// GPL, etc....

#include "model.h"

using namespace Exult3d;

/*
 *	Return the (non-unit) normal to this face, with right-handed 
 *	orientation.
 */
Vector3 Face::normal
	(
	vector<Vector3>& vertices	// Vertex list.
	) const
	{
	Vector3 a = vertices[vertex_indices[0]];
	Vector3 b = vertices[vertex_indices[1]];
	Vector3 c = vertices[vertex_indices[2]];
					// Get a->b and a->c.
	Vector3 ab = b - a, ac = c - a;
	return Cross(ab, ac);
	}

/*
 *	Clean up.
 */
Object3d::~Object3d
	(
	)
	{
	}

/*
 *	Compute the normal for each vertex.  This should be called after all
 *	vertices and faces have been added.
 */
void Object3d::compute_normals
	(
	)
	{
	int num_vertices = vertices.size();
	normals.resize(0);
	normals.reserve(num_vertices);	// One for each vertex.
	vector<Vector3> face_normals;	// First compute face normals.
	face_normals.reserve(faces.size());
	for (vector<Face>::const_iterator faceit = faces.begin();
					faceit != faces.end(); ++faceit)
		{
		const Face& face = *faceit;
		face_normals.push_back(face.normal(vertices));
		}
					// Average face-normals for each vertex.
	for (vector<Vector3>::const_iterator vertit = vertices.begin();
					vertit != vertices.end(); ++vertit)
		{
		Vector3 sum(0, 0, 0);
		int vnum = vertit - vertices.begin();
					// Which faces include this vertex?
		for (vector<Face>::const_iterator faceit = faces.begin();
					faceit != faces.end(); ++faceit)
			{
			const Face& face = *faceit;
			if (face.vertex_indices[0] == vnum ||
			    face.vertex_indices[1] == vnum ||
			    face.vertex_indices[2] == vnum)
				sum = sum + face_normals[faceit - faces.begin()];
			}
					// Make result a unit-normal.
		sum = sum/sqrt(Dot(sum, sum));
		normals.push_back(sum);	// Store result.
		}
	}

/*
 *	Clean up model.
 */
Model3d::~Model3d
	(
	)
	{
	for (vector<Material *>::const_iterator it = materials.begin();
				it != materials.end(); ++it)
		delete (*it);
	for (vector<Object3d *>::const_iterator oit = objects.begin();
				oit != objects.end(); ++oit)
		delete (*oit);
	}

/*
 *	Find a material by name.
 *
 *	Output:	->material if found, else 0.
 */

Material *Model3d::find_material
	(
	const char *nm
	)
	{
	for (vector<Material *>::const_iterator it = materials.begin();
				it != materials.end(); ++it)
		if ((*it)->name == nm)
			return *it;
	return 0;			// Not found.
	}

/*
 *	Compute the normal for each vertex.  This should be called after all
 *	vertices and faces have been added.
 */
void Model3d::compute_normals
	(
	)
	{
					// Do it for each object.
	for (vector<Object3d *>::const_iterator it = objects.begin();
				it != objects.end(); ++it)
		(*it)->compute_normals();
	}
