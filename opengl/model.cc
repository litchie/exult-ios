/**
 **	Model.cc - 3D model classes (for OpenGL).
 **
 **	Written: 4/16/02 - JSF
 **/

// GPL, etc....

#include "model.h"

using namespace Exult3d;

/*
 *	Clean up.
 */
Object3d::~Object3d
	(
	)
	{
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
 *	Output:	Index if found, else -1.
 */

int Model3d::find_material
	(
	const char *nm
	)
	{
	for (vector<Material *>::const_iterator it = materials.begin();
				it != materials.end(); ++it)
		if ((*it)->name == nm)
			return it - materials.begin();
	return -1;			// Not found.
	}

