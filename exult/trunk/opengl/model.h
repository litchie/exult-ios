/**
 **	Model.h - 3D model classes (for OpenGL).
 **
 **	Written: 4/16/02 - JSF
 **/

#ifndef INCL_MODEL_H
#define INCL_MODEL_H 1

// GPL, etc....

#include <string>
#include <vector>

namespace Exult3d {

/*
 *	3D vector.
 */
class Vector3
	{
public:
	float x, y, z;
	Vector3() : x(0), y(0), z(0)
		{  }
	Vector3(float xx, float yy, float zz) : x(xx), y(yy), z(zz)
		{  }
	};

/*
 *	2D vector.
 */
class Vector2
	{
public:
	float x, y;
	Vector2() : x(0), y(0)
		{  }
	Vector2(float xx, float yy) : x(xx), y(yy)
		{  }
	};

/*
 *	A material.  Can be a texture or color.
 */
class Material
	{
	string name;			// Name of material.
	string texture_filename;	// For a texture, if non-empty.
	unsigned char r, g, b;		// Color components, 0-255.
public:
	Material() : r(0), g(0), b(0)  {  }
	void set_name(const char *nm)
		{ name = nm; }
	void set_texture_filename(const char *nm)
		{ texture_filename = nm; }
	void set_color(const unsigned char *c)
		{ r = c[0]; g = c[1]; b = c[2]; }
	};

/*
 *	A face is a triangle with texture coordinates.
 */
class Face
	{
public:
	int vertex_indices[3];		// Indices into vertex list.
	int texture_indices[3];		// Texture indices.
	};

/*
 *	A complete object:
 */
class Object3d
	{
	string name;			// Object's name.
	int material;			// Index of material in list, or -1.
	vector<Vector3> vertices;	// All vertices.
	vector<Vector3> normals;	// All normal vectors.
	vector<Vector2> tex_vertices;	// Texture coords.
	vector<Face> faces;		// All faces.
public:
	Object3d() : material(-1)
		{  }
	~Object3d();
	void init_vertices(int cnt);	// Init. to given size.
	Vector3& get_vertex(int i)
		{ return vertices[i]; }
	void init_tex_vertices(int cnt);
	Vector2& get_tex_vertex(int i)
		{ return tex_vertices[i]; }
	void init_faces(int cnt);
	Face& get_face(int i)
		{ return faces[i]; }
	void set_name(const char *nm)
		{ name = nm; }
	void set_material(int mid)
		{ material = mid; }
	};

/*
 *	A 3D model:
 */
class Model3d
	{
	vector<Material *> materials;	// All materials used.
	vector<Object3d *> objects;	// All objects.
public:
	Model3d() {  }
	~Model3d();
					// Import a 3DS model.
	bool import3ds(const char *fname);
	void add_material(Material *m)
		{ materials.push_back(m); }
	void add_object(Object3d *o)
		{ objects.push_back(o); }
					// Find material by name.
	int find_material(const char *nm);
	};

}; // Exult3d namespace.

#endif	/* INCL_MODEL_H */
