/*
Copyright (C) 2002-2003 The Pentagram team

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

#ifndef USECODE_H
#define USECODE_H

class IDataSource;

class Usecode {
public:
	Usecode() { }
	virtual ~Usecode() { }

	virtual const uint8* get_class(uint32 classid)=0;
	virtual uint32 get_class_size(uint32 classid)=0;

	virtual uint32 get_class_event(uint32 classid, uint32 eventid);
};



#endif