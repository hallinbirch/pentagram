/*
Copyright (C) 2003 The Pentagram Team

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

#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include "XFormBlend.h"
#include <vector>

struct Palette;
class IDataSource;
class RenderSurface;

class PaletteManager
{
public:
	explicit PaletteManager(RenderSurface* rs);
	~PaletteManager();

	enum PalIndex {
		Pal_Game = 0
	};

	void load(PalIndex index, IDataSource& ds, const xformBlendFuncType *xff);
	Palette* getPalette(PalIndex index);

private:
	std::vector<Palette*> palettes;
	RenderSurface *rendersurface;
};

#endif