/*
 *  vgafile.h - Handle access to one of the xxx.vga files.
 *
 *  Copyright (C) 1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef VGAFILE_H
#define VGAFILE_H   1


#include <fstream>
#include <iostream>
#include <cassert>
#include "common_types.h"
#include "exult_constants.h"
#include "imagebuf.h"
#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <map>

class IDataSource;
class ODataSource;
class IStreamDataSource;
class Shape;
class Image_buffer8;
class GL_texshape;
class GL_manager;
class Palette;

#ifdef HAVE_OPENGL
#include "glshape.h"

#define GLRENDER    if (glman) glman->paint(this, px, py); else
#define GLTRANSLUCENT   if (glman) glman->paint(this, px, py, xforms, xfcnt); \
	else
#define GLTRANSFORM if (glman) glman->paint_transformed(this, px, py, xform); \
	else
#define GLREMAPPED  if (glman) glman->paint_remapped(this, px, py, trans); else
#define GLOUTLINE   if (glman) glman->paint_outline(this, px, py, color); else
#else
#define GLRENDER
#define GLTRANSLUCENT
#define GLTRANSFORM
#define GLREMAPPED
#define GLOUTLINE
#endif

/*
 *  A shape from "shapes.vga":
 */
class Shape_frame {
	std::unique_ptr<unsigned char[]> data;        // The actual data.
#ifdef HAVE_OPENGL
	GL_texshape *glshape;       // OpenGL texture for painting this.
	GL_texshape *gloutline;     // OpenGL texture for painting outline of this.
#endif
	int datalen;
	short xleft;            // Extent to left of origin.
	short xright;           // Extent to right.
	short yabove;           // Extent above origin.
	short ybelow;           // Extent below origin.
	bool rle;           // Run-length encoded.
	static GL_manager *glman;   // One to rule them all.
	static Image_buffer8 *scrwin;   // Screen window to render to.

	std::unique_ptr<Shape_frame> reflect();     // Create new frame, reflected.
	// Create RLE data & store in frame.
	void create_rle(unsigned char *pixels, int w, int h);
	// Create from RLE entry.
	void get_rle_shape(IDataSource *shapes, long filepos, long len);

public:
	friend class Game_window;
	friend class Shape_manager;
	friend class Shape;
	friend class Shape_file;
	friend class GL_texshape;
	friend class GL_manager;
	Shape_frame()
		:
#ifdef HAVE_OPENGL
		  glshape(0), gloutline(0),
#endif
		  datalen(0)
	{  }
	// Create frame from data.
	Shape_frame(unsigned char *pixels, int w, int h, int xoff, int yoff,
	            bool setrle);
	static void set_to_render(Image_buffer8 *w, GL_manager *gl = 0) {
		scrwin = w;
		glman = gl;
	}
	unsigned char *get_data() {
		return data.get();
	}
	unsigned char get_topleft_pix(unsigned char def = 255) const;
	bool is_rle() const {
		return rle;
	}
	// Convert raw image to RLE.
	static std::unique_ptr<unsigned char[]> encode_rle(unsigned char *pixels, int w, int h,
	                                 int xoff, int yoff, int &datalen);
	// Read in shape/frame.
	unsigned int read(IDataSource *shapes, uint32 shapeoff,
	                  uint32 shapelen, int frnum);
	// Paint into given buffer.
	void paint_rle(Image_buffer8 *win, int px, int py);
	void paint_rle_remapped(Image_buffer8 *win, int px, int py, const unsigned char *trans);
	void paint(Image_buffer8 *win, int px, int py);
	void paint_rle_translucent(Image_buffer8 *win, int px, int py,
	                           const Xform_palette *xforms, int xfcnt);
	void paint_rle_transformed(Image_buffer8 *win, int px, int py,
	                           const Xform_palette &xform);
	void paint_rle_outline(Image_buffer8 *win, int px, int py,
	                       unsigned char color);
	// Paint to screen.
	void paint_rle(int px, int py) {
		GLRENDER  paint_rle(scrwin, px, py);
	}
	void paint_rle_remapped(int px, int py, unsigned char *trans) {
		GLREMAPPED  paint_rle_remapped(scrwin, px, py, trans);
	}
	void paint(int px, int py) {
		GLRENDER  paint(scrwin, px, py);
	}
	// ++++++GL versions of these needed:
	void paint_rle_translucent(int px, int py,
	                           const Xform_palette *xforms, int xfcnt) {
		GLTRANSLUCENT  paint_rle_translucent(
		    scrwin, px, py, xforms, xfcnt);
	}
	void paint_rle_transformed(int px, int py, const Xform_palette &xform) {
		GLTRANSFORM  paint_rle_transformed(scrwin, px, py, xform);
	}
	void paint_rle_outline(int px, int py, unsigned char color) {
		GLOUTLINE  paint_rle_outline(scrwin, px, py, color);
	}

	int has_point(int x, int y) const;    // Is a point within the shape?
	int get_width() const {     // Get dimensions.
		return xleft + xright + 1;
	}
	int get_height() const {
		return yabove + ybelow + 1;
	}
	int get_xleft() const {
		return xleft;
	}
	int get_xright() const {
		return xright;
	}
	int get_yabove() const {
		return yabove;
	}
	int get_ybelow() const {
		return ybelow;
	}
	void set_offset(int new_xright, int new_ybelow);
	int get_size() const {
		return datalen;
	}
	int is_empty() const {
		return !data || (data[0] == 0 && data[1] == 0);
	}
#ifdef HAVE_OPENGL
	virtual ~Shape_frame() {
		if (glshape)
			glshape->disassociate();
		if (gloutline)
			gloutline->disassociate();
	}
#else
	virtual ~Shape_frame() noexcept = default;
#endif
	Shape_frame(const Shape_frame&) = delete;
	Shape_frame& operator=(const Shape_frame&) = delete;
	Shape_frame(Shape_frame&&) noexcept = default;
	Shape_frame& operator=(Shape_frame&&) noexcept = default;
};

/*
 *  A shape from a .vga file consists of one of more frames.
 */
class Shape {
protected:
	std::vector<std::unique_ptr<Shape_frame>> frames;       // List of ->'s to frames.
	unsigned int num_frames;    // # of frames (not counting reflects).
	bool modified;
	bool from_patch;
	// Create reflected frame.
	Shape_frame *reflect(std::vector<std::pair<std::unique_ptr<IDataSource>, bool>> const &shapes, int shnum,
	                     int frnum, std::vector<int> const &counts);
	void create_frames_list(int nframes);
	// Read in shape/frame.
	Shape_frame *read(std::vector<std::pair<std::unique_ptr<IDataSource>, bool>> const &shapes, int shnum,
	                  int frnum, std::vector<int> const &counts, int src = -1);
	// Store shape that was read.
	Shape_frame *store_frame(std::unique_ptr<Shape_frame> frame, int framenum);
public:
	friend class Vga_file;

	Shape() : num_frames(0), modified(false), from_patch(false)	{}
	explicit Shape(std::unique_ptr<Shape_frame> fr);
	explicit Shape(int n);           // Create with given #frames.
	Shape(const Shape&) = delete;
	Shape& operator=(const Shape&) = delete;
	Shape(Shape&&) noexcept = default;
	Shape& operator=(Shape&&) noexcept = default;
	virtual ~Shape() noexcept = default;
	void reset();
	void load(IDataSource *shape_source);
	void write(std::ostream &out);  // Write out.
	void set_modified() {
		modified = true;
	}
	bool get_modified() {
		return modified;
	}
	void set_from_patch() {
		from_patch = true;
	}
	bool get_from_patch() {
		return from_patch;
	}
	Shape_frame *get(std::vector<std::pair<std::unique_ptr<IDataSource>, bool>> const &shapes, int shnum,
	                 int frnum, std::vector<int> const &counts, int src = -1) {
		return (size_t(frnum) < frames.size() && frames[frnum])
		       ? frames[frnum].get()
		       : read(shapes, shnum, frnum, counts, src);
	}
	int get_num_frames() {
		return num_frames;
	}
	Shape_frame *get_frame(int framenum) {
		return 0 <= framenum && size_t(framenum) < frames.size()
		       ? frames[framenum].get()
		       : nullptr;
	}
	void resize(int newsize);   // Modify #frames.
	// Set frame.
	void set_frame(std::unique_ptr<Shape_frame> f, int framenum);
	// Add/insert frame.
	void add_frame(std::unique_ptr<Shape_frame> f, int framenum);
	void del_frame(int framenum);
};

/*
 *  A shape file just has one shape with multiple frames.  They're all
 *  read in during construction.
 */
class Shape_file : public Shape {
public:
	explicit Shape_file(const char *nm);
	explicit Shape_file(std::unique_ptr<Shape_frame> fr): Shape(std::move(fr)) {}
	explicit Shape_file(IDataSource *shape_source);
	Shape_file();
	Shape_file(Shape_file&& other) noexcept = default;
	Shape_file& operator=(Shape_file&& other) noexcept = default;
	~Shape_file() noexcept override final = default;
	void load(const char *nm);
	void load(IDataSource *shape_source) {
		Shape::load(shape_source);
	}
	int get_size();
	void save(ODataSource *shape_source);
};

/*
 *  A class for accessing any .vga file:
 */
class Vga_file {
protected:
	struct imported_map {
		int realshape;
		int pointer_offset;
		int source_offset;
	};
	std::vector<std::pair<std::unique_ptr<IDataSource>, bool>> shape_sources;
	std::vector<std::pair<std::unique_ptr<IDataSource>, bool>> imported_sources;
	std::map<int, imported_map> imported_shape_table;
	int u7drag_type;        // # from u7drag.h, or -1.
	std::vector<int> shape_cnts;    // Total # of shapes in each file.
	std::vector<int> imported_cnts; // Total # of shapes in each file.
	std::vector<Shape> shapes;              // List of ->'s to shapes' lists
	std::vector<Shape> imported_shapes;     // List of ->'s to shapes' lists
	bool flex;          // This is the normal case (all .vga
	//   files).  If false, file is a
	//   single shape, like 'pointers.shp'.
	// In this case, all frames are pre-
	//   loaded.
public:
	explicit Vga_file(const char *nm, int u7drag = -1, const char *nm2 = 0);
	explicit Vga_file(std::vector<std::pair<std::string, int>> const &sources, int u7drag = -1);
	Vga_file();
	int get_u7drag_type() const {
		return u7drag_type;
	}
	IDataSource *U7load(
	    std::pair<std::string, int> const &resource,
	    std::vector<std::pair<std::unique_ptr<IDataSource>, bool>> &shps);
	bool load(const char *nm, const char *nm2 = 0, bool resetimports = false);
	bool load(std::vector<std::pair<std::string, int>> const &sources, bool resetimports = false);
	bool import_shapes(std::pair<std::string, int> const &source,
	                   std::vector<std::pair<int, int>> const &imports);
	bool is_shape_imported(int shnum);
	bool get_imported_shape_data(int shnum, imported_map &data);
	void reset();
	void reset_imports();
	virtual ~Vga_file() noexcept;
	Vga_file(const Vga_file&) = delete;
	Vga_file& operator=(const Vga_file&) = delete;
	Vga_file(Vga_file&&) noexcept = default;
	Vga_file& operator=(Vga_file&&) noexcept = default;
	int get_num_shapes() const {
		return shapes.size();
	}
	int is_good() const {
		return !shapes.empty();
	}
	bool is_flex() const {
		return flex;
	}
	// Get shape.
	Shape_frame *get_shape(int shapenum, int framenum = 0) {
		Shape_frame *r;
		imported_map data;
		if (get_imported_shape_data(shapenum, data)) {
			// The shape is imported from another file.
			// Import table was set but the source could not be opened.
			if (data.pointer_offset < 0)
				return 0;
			r = (imported_shapes[data.pointer_offset].get(imported_sources,
			        data.realshape, framenum, imported_cnts, data.source_offset));
		} else {
			assert(!shapes.empty()); // Because if shapes is NULL
			// here, we won't die on the deref
			// but we will return rubbish.
			// I've put this assert in _before_ you know...
			// So this isn't the first time we've had trouble here
			r = (shapes[shapenum].get(shape_sources, shapenum,
			                          framenum, shape_cnts, -1));
		}
		return r;
	}

	Shape *extract_shape(int shapenum) {
		imported_map data;
		bool is_imported = get_imported_shape_data(shapenum, data);
		if (is_imported)
			assert(!imported_shapes.empty()); // For safety.
		else
			assert(!shapes.empty());
		// Load all frames into memory
		int count = get_num_frames(shapenum);
		for (int i = 1; i < count; i++)
			get_shape(shapenum, i);
		if (is_imported)
			return &imported_shapes[data.pointer_offset];
		else
			return &shapes[shapenum];
	}
	// Get # frames for a shape.
	int get_num_frames(int shapenum) {
		get_shape(shapenum, 0); // Force it into memory.
		imported_map data;
		if (get_imported_shape_data(shapenum, data))
			return imported_shapes[data.pointer_offset].num_frames;
		else
			return shapes[shapenum].num_frames;
	}
	// Create new shape (or del old).
	virtual Shape *new_shape(int shapenum);
};

#endif  /* VGAFILE_H */
