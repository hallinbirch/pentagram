/*
SoftRenderSurface.h : SoftRenderSurface Implementation header

Copyright (C) 2002, 2003 The Pentagram Team

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

#ifndef SOFTRENDERSURFACE_H
#define SOFTRENDERSURFACE_H

#include "BaseSoftRenderSurface.h"

//
// SoftRenderSurface
//
// Desc: The class for software rendering in Pentagram
//
template<class uintX> class SoftRenderSurface : public BaseSoftRenderSurface
{
public:

	// Create from a SDL_Surface
	SoftRenderSurface(SDL_Surface *);

	//
	// Surface Filling
	//

	// Fill buffer (using a palette index)
	virtual void Fill8(uint8 index, sint32 sx, sint32 sy, sint32 w, sint32 h);

	// Fill buffer (using a RGB colour)
	virtual void Fill32(uint32 rgb, sint32 sx, sint32 sy, sint32 w, sint32 h);


	//
	// The rule for painting methods:
	//
	// First arg are the source object to 'draw' with
	// Next args are any other required data to define the 'source'
	// Next args are the destination position
	//

	//
	// Basic Shape Painting
	//

	// Paint a Shape
	// TODO: virtual void Paint(CachedShape* s, uint32 frame, sint32 x, sint32 y);
	virtual void Paint(Shape*s, uint32 frame, sint32 x, sint32 y);

	// Paint an Shape without clipping
	// TODO: virtual void PaintNoClip(CachedShape*s, uint32 frame, sint32 x, sint32 y);
	virtual void PaintNoClip(Shape*s, uint32 frame, sint32 x, sint32 y);

	// Paint a Translucent Shape. 
	// TODO: virtual void PaintTranslucent(CachedShape* s, uint32 frame, sint32 x, sint32 y);
	virtual void PaintTranslucent(Shape* s, uint32 frame, sint32 x, sint32 y);

	// Paint a Mirrored Shape
	// TODO: virtual void PaintMirrored(CachedShape* s, uint32 frame, sint32 x, sint32 y, bool trans = false);
	virtual void PaintMirrored(Shape* s, uint32 frame, sint32 x, sint32 y, bool trans = false);


	//
	// Basic Line Drawing
	//
	
	// Draw a Line (using a palette index)
	// TODO: virtual void DrawLine8(uint8 index, sint32 sx, sint32 sy, sint32 ex, sint32 ey);

	// Draw a RGB Line
	// TODO: virtual void DrawLine32(uint32 rgb, sint32 sx, sint32 sy, sint32 ex, sint32 ey);


	//
	// Basic Font Drawing
	//
	
	// Draw from a Font
	// TODO: virtual void PrintText(Font *, const char *text, int x, int y);

	// Draw a character from a Font
	// TODO: virtual void PrintChar(Font *, char character, int x, int y);

	// Draw fixed width from a Texture buffer (16x16 characters fixed width and height)
	virtual void PrintTextFixed(Texture *, const char *text, int x, int y);

	// Draw a fixed width character from a Texture buffer
	virtual void PrintCharFixed(Texture *, int character, int x, int y);


	//
	// Basic Texture Blitting
	//

	// Blit a region from a Texture (Alpha == 0 -> skipped)
	virtual void Blit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy);

	// Blit a region from a Texture with 3D Alpha Blending Function (Alpha == 0 -> skipped)
	// TODO: virtual void AlphaBlit(Texture *, sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 dx, sint32 dy);

	// Blit a stretched region from a Texture (Alpha == 0 -> skipped)
	// TODO: virtual void StretchBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh);

	// Blit a stretched region from a Texture with 3D Alpha Blending Function (Alpha == 0 -> skipped)
	// TODO: virtual void StretchAlphaBlit(Texture *, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 dx, sint32 dy, sint32 dw, sint32 dh);


	////////////////////////////////////////
	// TODO: Add in Abstract 3d code Here //
	////////////////////////////////////////
};

#endif