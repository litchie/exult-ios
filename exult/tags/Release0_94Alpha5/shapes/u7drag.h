/**
 **	U7drag.h - Common defines for drag-and-drop of U7 shapes.
 **
 **	Written: 12/13/00 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INCL_U7DRAG
#define INCL_U7DRAG 1

//	Target information for dragging a shape:
#define U7_TARGET_SHAPEID_NAME "U7SHAPEID"
#define U7_TARGET_SHAPEID 137

//	Shape files:
#define U7_SHAPE_UNK	-1		/* Unknown.			*/
#define U7_SHAPE_SHAPES	0		/* shapes.vga			*/
#define U7_SHAPE_GUMPS	1		/* gumps.vga			*/
#define U7_SHAPE_FONTS	2		/* fonts.vga			*/
#define U7_SHAPE_FACES	3		/* faces.vga			*/
#define U7_SHAPE_SPRITES 4		/* sprites.vga			*/

//	Store/get shapeid.
int Store_u7_shapeid(unsigned char *data, int file, int shape, int frame);
void Get_u7_shapeid(unsigned char *data, int& file, int& shape, int& frame);

//	Target information for dragging a chunk:
#define U7_TARGET_CHUNKID_NAME "U7CHUNKID"
#define U7_TARGET_CHUNKID 138

//	Store/get chunk #.
int Store_u7_chunkid(unsigned char *data, int cnum);
void Get_u7_chunkid(unsigned char *data, int& cnum);

#endif

