/**
 **	Import a model from a 3DS file.
 **
 **	Written: 4/16/02 - JSF
 **/

#include <fstream>
#include "model.h"

using namespace Exult3d;

/*
 *	These came from a tutorial by Ben Humphrey:
 */

//>------ Primary Chunk, at the beginning of each file
#define PRIMARY       0x4D4D

//>------ Main Chunks
#define OBJECTINFO    0x3D3D		// This gives the version of the mesh 
					//  and is found right before the 
					//  material and object information
#define VERSION       0x0002		// .3ds file version.
#define EDITKEYFRAME  0xB000		// Hheader for all the key frame info.

//>------ color types
#define COLOR24	      0x0011
#define COLORLIN24    0x0012		// Gamma-corrected.


//>------ sub defines of OBJECTINFO
#define MATERIAL	  0xAFFF	// The stored the texture info.
#define OBJECT		  0x4000	// Faces, vertices, etc...

//>------ sub defines of MATERIAL
#define MATNAME       0xA000		// Material name
#define MATAMBIENT    0xA010		// Ambient color of material.
#define MATDIFFUSE    0xA020		// Color of object/material.
#define MATMAP        0xA200		// Header for a new material
#define MATMAPFILE    0xA300		// Texture filename.

#define OBJECT_MESH   0x4100		// We're reading a new object.

//>------ sub defines of OBJECT_MESH
#define OBJECT_VERTICES     0x4110	// The object's vertices.
#define OBJECT_FACES		0x4120	// The object's faces
#define OBJECT_MATERIAL		0x4130	// Found if the obj. has a material, 
					//   either texture map or color
#define OBJECT_UV		0x4140	// The UV texture coordinates

static int Read_chunk(Model3d *model, istream& in, int top_len);
static int Read_material_chunk(Model3d *model,
				istream& in, int top_len, Material *mat);
static int Read_object_chunk(Model3d *model,
				istream& in, int top_len, Object3d *obj);
static int Read_vertices_chunk(istream& in, int top_len, Object3d *obj);
static int Read_texture_vertices_chunk(istream& in, int top_len,Object3d *obj);
static int Read_faces_chunk(istream& in, int top_len, Object3d *obj);
static int Read_object_material_chunk(Model3d *model,
				istream& in, int top_len, Object3d *obj);

#define CHUNK_HEADER_LENGTH 6

/*
 *	Read 2 or 4 byte quantities, LSB first.
 */
inline unsigned int Read2(istream& in)
	{
	unsigned char v[2];
	in.read(v, sizeof(v));
	return v[0] + ((unsigned int) v[1] << 8);
	}
inline unsigned int Read4(istream& in)
	{
	unsigned char v[4];
	in.read(v, sizeof(v));
	return v[0] + ((unsigned int) v[1] << 8) + 
		((unsigned int) v[2] << 16) + ((unsigned int) v[3] << 24);
	}

/*
 *	Read in a 4-byte float.
 */

inline float Read_float(istream&in)
	{
	unsigned char v[4];
	in.read(v, sizeof(v));
	return *(float *)&v[0];		// +++++++++byte order!!!!!!!!
	}

/*
 *	Get the chunk type and length.
 *
 *	Output:	# bytes read.
 */

inline int Get_chunk_header
	(
	istream& in,
	int& id,
	int& length
	)
	{
	unsigned char header[6];
	in.read(header, sizeof(header));
	id = header[0] + ((unsigned int) header[1] << 8);
	length = header[2] + ((unsigned int) header[3] << 8) +
				((unsigned int) header[4] << 16) +
				((unsigned int) header[5] << 24);
	return sizeof(header);
	}

/*
 *	Read in a null-terminated string.
 *
 *	Output:	# bytes read, including the null.
 */

static int Get_string
	(
	istream& in,
	string& str			// Characters appended here.
	)
	{
	int cnt = 0;
	unsigned char c;
	while (in.good() && (c = in.get()) != 0)
		{
		cnt++;
		str += c;
		}
	if (!c)				// Count null at end.
		cnt++;
	return cnt;
	}

/*
 *	Import a 3DS file into an empty model.
 *
 *	Output:	False if unsuccessful.
 */

bool Model3d::import3ds
	(
	const char *fname		// Filename.
	)
	{
	assert(materials.size() == 0 && objects.size() == 0);
	ifstream in(fname);
	if (!in.good())
		{
		cerr << "Failed to open '" << fname << "'" << endl;
		return false;
		}
	int id, length;			// Get main chunk.
	Get_chunk_header(in, id, length);
	if (id != PRIMARY)
		{
		cerr << "'" << fname << "' doesn't look like a 3DS file"
							<< endl;
		return false;
		}
	if (Read_chunk(this, in, length) < 0)
		return false;
	compute_normals();		// Set vertex normals.
	load_textures();		// Load texture files. (Maybe this
					//   should be done above???)
	return true;
	}

/*
 *	Read in a chunk and its subchunks.
 *
 *	Output:	# bytes read, or -1 if error.
 */

static int Read_chunk
	(
	Model3d *model,
	istream& in,			// Header already read.
	int top_len			// Length of this chunk.
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;	// Already read header.
	while (top_read < top_len)	// Go through subchunks.
		{
		int id, len;
		int read = Get_chunk_header(in, id, len);
		switch (id)
			{
		case VERSION:
			{
			assert(len - read == 4);
			int vers = Read4(in);
			read += 4;
			cout << "3DS version is " << vers << endl;
			break;
			}
		case OBJECTINFO:	// Head of MATERIAL, OBJECT chunks.
			{
			int subread = Read_chunk(model, in, len);
			if (subread < 0)	// Error?
				return subread;
			read += subread;
			break;
			}
		case MATERIAL:		// Create a new material.
			{
			Material *mat = new Material();
			int subread = Read_material_chunk(model, in, len, mat);
			if (subread < 0)	// Error?
				{
				delete mat;
				return subread;
				}
			read += subread;
			model->add_material(mat);
			break;
			}
		case OBJECT:		// Create a new object.
			{
			Object3d *obj = new Object3d();
			int subread = Read_object_chunk(model, in, len, obj);
			if (subread < 0)
				{
				delete obj;
				return subread;
				}
			read += subread;
			model->add_object(obj);
			break;
			}
		default:		// Don't care.
			cerr << "3ds id 0x" << hex << id << " skipped" << endl;
			in.seekg(len - read, ios::cur);
			read = len;
			break;
			}
		top_read += read;	// Add to top's total.
		}
	return in.good() ? (top_read - CHUNK_HEADER_LENGTH) : -1;
	}

/*
 *	Read in a new material.
 *
 *	Output:	#bytes read, or -1 if error.
 */


static int Read_material_chunk
	(
	Model3d *model,
	istream& in, 			// Header already read.
	int top_len,			// Total length of this chunk.
	Material *mat
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;	// Already read header.
	while (top_read < top_len)	// Go through subchunks.
		{
		int id, len;
		int read = Get_chunk_header(in, id, len);
		switch (id)
			{
		case MATNAME:		// Material name.
			{
			string name;
			read += Get_string(in, name);
			mat->set_name(name.c_str());
			// ++++++There are more bytes.  For now:
			in.seekg(len - read, ios::cur);	//+++++++
			read = len;	//++++++skip to end.
			break;
			}
		case MATAMBIENT:	//+++++++Unsure.  Fall through.
		case MATDIFFUSE:	// Color.
			{
			int cid, clen;	// Color subchunk.
			read += Get_chunk_header(in, cid, clen);
			unsigned char c[3];
			in.read(c, 3);
			read += 3;
			mat->set_color(&c[0]);
					// Skip others.
			in.seekg(len - read, ios::cur);
			read = len;
			break;
			}
		case MATMAP:		// Parent of texture info.
			{
			int subread = Read_material_chunk(model, in, len, mat);
			if (subread < 0)
				return subread;
			read += subread;
			break;
			}
		case MATMAPFILE:	// Texture filename.
			{
			string name;
			read += Get_string(in, name);
			mat->set_texture_filename(name.c_str());
			assert(read == len);
			break;
			}
		default:		// Don't care.
			cerr << "3ds id 0x" << hex << id << " skipped" << endl;
			in.seekg(len - read, ios::cur);
			read = len;
			break;
			}
		top_read += read;	// Add to top's total.
		}
	return in.good() ? (top_read - CHUNK_HEADER_LENGTH) : -1;
	}

/*
 *	Read in a new object.
 *
 *	Output:	#bytes read, or -1 if error.
 */

static int Read_object_chunk
	(
	Model3d *model,
	istream& in, 			// Header already read.
	int top_len,			// Total length of this chunk.
	Object3d *obj			// New object to set up.
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;	// Already read header.
	if (!obj->is_name_set())
		{			// First time.
		string name;		// Read name.
		top_read += Get_string(in, name);
		obj->set_name(name.c_str());
		}
	while (top_read < top_len)	// Go through subchunks.
		{
		int id, len;
		int read = Get_chunk_header(in, id, len);
		switch (id)
			{
		case OBJECT_MESH:	// Indicates start.
			{
			int subread = Read_object_chunk(model, in, len, obj);
			if (subread < 0)
				return subread;
			read += subread;
			break;
			}
		case OBJECT_VERTICES:
			read += Read_vertices_chunk(in, len, obj);
			break;
		case OBJECT_FACES:
			read += Read_faces_chunk(in, len, obj);
			break;
		case OBJECT_MATERIAL:
//+++++This should be part of Read_faces_chunk(), and there is a list of faces
// that the material applies to.
			read += Read_object_material_chunk(model, 
								in, len, obj);
			break;
		case OBJECT_UV:		// UV texture coordinates.
			read += Read_texture_vertices_chunk(in, len, obj);
			break;
		default:		// Don't care.
			cerr << "3ds id 0x" << hex << id << " skipped" << endl;
			in.seekg(len - read, ios::cur);
			read = len;
			break;
			}
		top_read += read;	// Add to top's total.
		}
	return in.good() ? (top_read - CHUNK_HEADER_LENGTH) : -1;
	}

/*
 *	Read in a list of vertices.
 *
 *	Output:	# bytes read.
 */

static int Read_vertices_chunk
	(
	istream& in, 			// Header already read.
	int top_len,			// Total length of this chunk.
	Object3d *obj			// New object to set up.
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;
	int cnt = Read2(in);		// Get # vertices.
	top_read += 2;
	obj->init_vertices(cnt);	// Set vector size.
	for (int i = 0; i < cnt; i++)
		{
		obj->get_vertex(i).x = Read_float(in);
					// 3DS has Y/Z flipped!
		obj->get_vertex(i).z = Read_float(in);
		obj->get_vertex(i).y = Read_float(in);
		top_read += 3*4;
		}
	assert (top_read == top_len);
	return top_read - CHUNK_HEADER_LENGTH;
	}

/*
 *	Read in a list of texture vertices.
 *
 *	Output:	# bytes read.
 */

static int Read_texture_vertices_chunk
	(
	istream& in, 			// Header already read.
	int top_len,			// Total length of this chunk.
	Object3d *obj			// New object to set up.
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;
	int cnt = Read2(in);		// Get # vertices.
	top_read += 2;
	obj->init_tex_vertices(cnt);	// Set vector size.
	for (int i = 0; i < cnt; i++)
		{
		obj->get_tex_vertex(i).x = Read_float(in);
					// 3DS has Y/Z flipped!
		obj->get_tex_vertex(i).y = Read_float(in);
		top_read += 2*4;
		}
	assert (top_read == top_len);
	return top_read - CHUNK_HEADER_LENGTH;
	}

/*
 *	Read in a list of faces.
 *
 *	Output:	# bytes read.
 */

static int Read_faces_chunk
	(
	istream& in, 			// Header already read.
	int top_len,			// Total length of this chunk.
	Object3d *obj			// New object to set up.
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;
	int cnt = Read2(in);		// Get # vertices.
	top_read += 2;
	obj->init_faces(cnt);
	for (int i = 0; i < cnt; i++)
		{
		obj->get_face(i).vertex_indices[0] = Read2(in);
		obj->get_face(i).vertex_indices[1] = Read2(in);
		obj->get_face(i).vertex_indices[2] = Read2(in);
		Read2(in);		// Ignore visibility flag.
		top_read += 4*2;	// Read 4 shorts.
		}
//	assert (top_read == top_len);
	return top_read - CHUNK_HEADER_LENGTH;
	}

/*
 *	Read in an object's material.
 *
 *	Output:	# bytes read.
 */

static int Read_object_material_chunk
	(
	Model3d *model,
	istream& in, 			// Header already read.
	int top_len,			// Total length of this chunk.
	Object3d *obj			// New object to set up.
	)
	{
	int top_read = CHUNK_HEADER_LENGTH;
	string name;			// Read material name.
	top_read += Get_string(in, name);
	if (!obj->get_material())	//+++++FOR NOW.  Material really
//+++++ applies to the list of faces that follow.
		obj->set_material(model->find_material(name.c_str()));
					// Skip the rest.
	in.seekg(top_len - top_read, ios::cur);
	top_read = top_len;
	return top_read - CHUNK_HEADER_LENGTH;
	}

