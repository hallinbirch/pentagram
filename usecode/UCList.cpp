/*
Copyright (C) 2002 The Pentagram team

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


#include "pent_include.h"

#include "UCList.h"
#include "UCMachine.h"

uint16 UCList::getStringIndex(uint32 index)
{
	return elements[index*2] + (elements[index*2+1]<<8);
}

std::string& UCList::getString(uint32 index)
{
	uint16 sindex = getStringIndex(index);
	return UCMachine::get_instance()->getString(sindex);
}

void UCList::freeStrings()
{
	UCMachine *ucm = UCMachine::get_instance();
	for (unsigned int i = 0; i < size; i++) {
		ucm->freeString(getStringIndex(i));
	}
	free();
}

void UCList::copyStringList(UCList& l)
{
	UCMachine *ucm = UCMachine::get_instance();
	freeStrings();
	for (unsigned int i = 0; i < l.size; i++) {
		uint16 s = ucm->duplicateString(l.getStringIndex(i));
		uint8 tmp[2]; // ugly...
		tmp[0] = s & 0xFF;
		tmp[1] = s >> 8;
		append(tmp);
	}
}

void UCList::unionStringList(UCList& l)
{
	UCMachine *ucm = UCMachine::get_instance();
	// take the union of two stringlists
	// i.e., append the second to this one, removing any duplicates
	for (unsigned int i = 0; i < l.size; i++) {
		if (!stringInList(l.getStringIndex(i))) {
			append(l[i]);
		} else {
			// free it if we're not keeping it
			ucm->freeString(l.getStringIndex(i));
		}
	}
	l.free(); // NB: do _not_ free the strings in l, since they're in this one
}

void UCList::substractStringList(UCList& l)
{
	for (unsigned int i = 0; i < l.size; i++)
		removeString(l.getStringIndex(i));
}

bool UCList::stringInList(uint16 s)
{
	std::string str = UCMachine::get_instance()->getString(s);
	for (unsigned int i = 0; i < size; i++)
		if (getString(i) == str)
			return true;

	return false;
}

void UCList::assignString(uint32 index, uint16 str)
{
	// assign string str to element index
	// free old contents of element index; take ownership of str(?)

	UCMachine::get_instance()->freeString(getStringIndex(index));
	elements[index*elementsize] = str & 0xFF;
	elements[index*elementsize+1] = str >> 8;
}

void UCList::removeString(uint16 s)
{
	// do we need to erase all occurences of str or just the first one?
	// (deleting all, currently)
	std::string str = UCMachine::get_instance()->getString(s);
	for (unsigned int i = 0; i < size; i++) {
		if (getString(i) == str) {
			elements.erase(elements.begin()+i*elementsize,
						   elements.begin()+(i+1)*elementsize);
			size--;
			i--; // back up a bit
		}
	}
}