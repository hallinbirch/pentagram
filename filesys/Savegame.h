/*
Copyright (C) 2003-2004 The Pentagram team

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

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "NamedFlex.h"
#include <string>
#include <vector>
#include <map>

class IDataSource;

class Savegame : public NamedFlex
{
public:
	Savegame() { }
	explicit Savegame(IDataSource* ds);
	virtual ~Savegame();

	//! get the savegame's global version
	uint32 getVersion();

protected:
	virtual uint32 get_size(uint32 index);
	virtual uint32 get_offset(uint32 index);

	std::vector<uint32> offsets;
	std::vector<uint32> sizes;
	std::vector<std::string> names;
};

#endif
