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
 *	Vector operations:
 */
inline Vector3 Cross(Vector3 a, Vector3 b)
	{ return Vector3(a.y*b.z - a.z*b.y, 
			 a.z*b.x - a.x*b.z,
			 a.x*b.y - a.y*b.x); }
inline float Dot(Vector3 a, Vector3 b)
	{ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vector3 operator-(Vector3 a, Vector3 b)
	{ return Vector3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline Vector3 operator+(Vector3 a, Vector3 b)
	{ return Vector3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline Vector3 operator/(Vector3 a, float b)
	{ return Vector3(a.x/b, a.y/b, a.z/b); }

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

typedef float Color4[4];		// RGBA.

/*
 *	A material.  Can be a texture or color.
 */
class Material
	{
	string name;			// Name of material.
	string texture_filename;	// For a texture, if non-empty.
	unsigned int texture_id;	// OpenGL texture ID.
	bool texture_loaded;		// True if texture_id is valid.
					// Three kinds of colors (rgba), with
					//   index defined below.
	Color4 colors[3];
	void set_color(float *arr, const unsigned char *c)
		{ arr[0] = c[0]/255.0; arr[1] = c[1]/255.0; 
		  arr[2] = c[2]/255.0; arr[3] = 1.0; }
					// Default color:
	static unsigned char def_color[3];
	bool read_texture();		// Read texture & init. for OpenGL.
public:
	friend class Model3d;
	friend class Object3d;
	Material() : texture_id(0), texture_loaded(false)
		{ set_color(ambient, def_color); 
		  set_color(diffuse, def_color);
		  set_color(specular, def_color); }
	void set_name(const char *nm)
		{ name = nm; }
	void set_texture_filename(const char *nm)
		{ texture_filename = nm; }
					// Indices into 'color' above:
	enum Color_index
		{
		diffuse = 0,
		ambient = 1,
		specular = 2
		};
	void set_color(Color_index i, const unsigned char *c)
		{ set_color(colors[i], c); }
	bool load()			// Load texture.
		{ 
		return (texture_loaded || texture_filename.empty()) ? true
							: read_texture();
		}
	};

/*
 *	A face is a triangle with texture coordinates.
 */
class Face
	{
public:
	int vertex_indices[3];		// Indices into vertex list.
	int texture_indices[3];		// Texture indices.
					// Return (non-unit) normal.
	Vector3 normal(vector<Vector3>& vertices) const;
	};

/*
 *	A complete object:
 */
class Object3d
	{
	string name;			// Object's name.
	Material *material;		// Material in model's list.
	vector<Vector3> vertices;	// All vertices.
	vector<Vector3> normals;	// Normal vector to each vertex.
	vector<Vector2> tex_vertices;	// Texture coords.
	vector<Face> faces;		// All faces.
public:
	Object3d() : material(0)
		{  }
	~Object3d();
	void init_vertices(int cnt)	// Init. to given size.
		{ vertices.resize(cnt); }
	int vertices_size() const
		{ return vertices.size(); }
	Vector3& get_vertex(int i)
		{ return vertices[i]; }
	void init_tex_vertices(int cnt)
		{ tex_vertices.resize(cnt); }
	Vector2& get_tex_vertex(int i)
		{ return tex_vertices[i]; }
	void init_faces(int cnt)
		{ faces.resize(cnt); }
	Face& get_face(int i)
		{ return faces[i]; }
	const char *get_name()
		{ return name.c_str(); }
	bool is_name_set() const
		{ return !name.empty(); }
	void set_name(const char *nm)
		{ name = nm; }
	Material *get_material()
		{ return material; }
	void set_material(Material *m)
		{ material = m; }
	void compute_normals();		// Create normals after all vertices 
					//   and faces have been added.
					// OPENGL methods:
	void render();
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
	Material *find_material(const char *nm);
	void compute_normals();		// Create normals after all vertices
					//   and faces have been added.
	void load_textures();		// Load textures for the materials.
					// OPENGL methods:
	void render();
					// Average of all the vertices.
	void find_extents(Vector3& low, Vector3& high) const;
	};

}; // Exult3d namespace.

#endif	/* INCL_MODEL_H */
