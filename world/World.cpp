/*
Copyright (C) 2003 The Pentagram team

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

#include "World.h"
#include "Map.h"
#include "CurrentMap.h"
#include "IDataSource.h"
#include "Flex.h"
#include "ItemFactory.h"
#include "Actor.h"

#include "ItemSorter.h"	// TODO: Remove this

//#define DUMP_ITEMS

World* World::world = 0;

World::World()
	: currentmap(0), fixed(0), fixedds(0)
{
	assert(world == 0);
	world = this;

	objects.resize(65536);
	lastobjid = 256;
}


World::~World()
{
	clear();

	world = 0;
}


void World::clear()
{
	unsigned int i;

	for (i = 0; i < maps.size(); ++i) {
		delete maps[i];
	}
	maps.clear();

	for (i = 0; i < npcs.size(); ++i) {
		delete npcs[i];
	}
	npcs.clear();

	if (currentmap)
		delete currentmap;

	if (fixed)
		delete fixed;
	if (fixedds)
		delete fixedds;

	//! Need to check the object delete policy
	// If everything works out, there shouldn't be any objects left
	// (maybe NPCs?)
	for (i = 0; i < objects.size(); ++i) {
		objects[i] = 0;
	}
	lastobjid = 256;
}

void World::initMaps()
{
	// Q: How do we determine which Maps to create? Only create those
	// with non-zero size in fixed.dat?

	maps.resize(256);
	for (unsigned int i = 0; i < 256; ++i) {
		maps[i] = new Map(i);
	}

	currentmap = new CurrentMap();
}

void World::initNPCs()
{
	// Q: How do we determine which NPCs to create?

	// automatically initialized to 0
	npcs.resize(256);
}

bool World::switchMap(uint32 newmap)
{
	assert(currentmap);

	if (currentmap->getNum() == newmap)
		return true;

	if (newmap >= maps.size() || maps[newmap] == 0)
		return false; // no such map

	// Map switching procedure:
	// write back CurrentMap to the old map, which
	//   un-expands GlobEggs (delete EXT_INGLOB items, reset contents fields)
	// clear all objIDs
	// swap out fixed items in old map?
	// make sure fixed items in the new map are loaded
	// load new map into CurrentMap, which also
	//   expands GlobEggs
	//   assigns objIDs to fixed items
	//   assigns objIDs to nonfixed items

	// NB: not only World has to perform actions on a map switch
	// other things that should/could also happen:
	// - usecode processes have to be terminated (forcefully?)
	// - autosave?
	// - ...?

	uint32 oldmap = currentmap->getNum();

	if (oldmap != 0) {
		perr << "Unloading map " << oldmap << std::endl;

		assert(oldmap < maps.size() && maps[oldmap] != 0);

		currentmap->writeback();

		perr << "Unloading Fixed items from map " << oldmap << std::endl;

		maps[oldmap]->unloadFixed();

		//! constant
		for (unsigned int i = 256; i < objects.size(); ++i) {
			if (objects[i] != 0)
				objects[i]->clearObjId();
		}
	}

	//! constant
	lastobjid = 256; // reset objid counter

	pout << "Loading Fixed items in map " << newmap << std::endl;
	pout << "-----------------------------------------" << std::endl;
	IDataSource *items = fixed->get_datasource(newmap);
	maps[newmap]->loadFixed(items);
	delete items;
	pout << "-----------------------------------------" << std::endl;

	currentmap->loadMap(maps[newmap]);

	return true;
}

void World::loadNonFixed(IDataSource* ds)
{
	Flex* f = new Flex(ds);

	for (unsigned int i = 0; i < f->get_count(); ++i) {

		// items in this map?
		if (f->get_size(i) > 0) {
			pout << "Loading NonFixed items in map " << i << std::endl;
			pout << "-----------------------------------------" << std::endl;

			assert(maps.size() > i);
			assert(maps[i] != 0);

			IDataSource *items = f->get_datasource(i);

			maps[i]->loadNonFixed(items);

			delete items;

		}
	}

	delete f;
}


void World::loadFixed(IDataSource* ds)
{
	fixed = new Flex(ds);
	fixedds = ds;
}

void World::loadItemCachNPCData(IDataSource* itemcach, IDataSource* npcdata)
{
	Flex* itemcachflex = new Flex(itemcach);
	Flex* npcdataflex = new Flex(npcdata);

	IDataSource* itemds = itemcachflex->get_datasource(0);
	IDataSource* npcds = npcdataflex->get_datasource(0);

	pout << "Loading NPCs" << std::endl;
	pout << "----------------" << std::endl;

	for (uint32 i = 1; i < 256; ++i) // Get rid of constants?
	{
		itemds->seek(0x00000+i*2);
		sint32 x = static_cast<sint32>(itemds->readXS(2));
		itemds->seek(0x04800+i*2);
		sint32 y = static_cast<sint32>(itemds->readXS(2));
		itemds->seek(0x09000+i*1);
		sint32 z = static_cast<sint32>(itemds->readXS(1));

		itemds->seek(0x0B400+i*2);
		uint32 shape = itemds->read2();
		itemds->seek(0x0FC00+i*1);
		uint32 frame = itemds->read1();
		itemds->seek(0x12000+i*2);
		uint32 flags = itemds->read2();
		itemds->seek(0x16800+i*2);
		uint16 quality = itemds->read2();
		itemds->seek(0x1B000+i*1);
		uint32 npcnum = itemds->read1();
		itemds->seek(0x1D400+i*1);
		uint32 mapnum = itemds->read1();
		itemds->seek(0x1F800+i*2);
		uint32 next = itemds->read2();

		// half the frame number is stored in npcdata.dat
		npcds->seek(7 + i * 0x31);
		frame += npcds->read1() << 8;

		// TODO: (decode and) read rest of npcdata.dat...
		// TODO: locate inventory

		if (shape == 0) {
			// U8's itemcach has a lot of garbage in it.
			// Ignore it.
			x = 0; y = 0; z = 0;
			frame = 0;
			flags = 0; quality = 0;
			npcnum = 0; mapnum = 0;
			next = 0;
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Actor *actor = ItemFactory::createActor(shape,frame,quality,flags,
												npcnum,mapnum,
												Item::EXT_NOTINMAP);
		if (!actor) {
#ifdef DUMP_ITEMS
			// this 'error' message is supposed to occur rather a lot
			pout << "Couldn't create actor" << std::endl;
#endif
			continue;
		}
		actor->setLocation(x,y,z);

		npcs[i] = actor;
		objects[i] = actor;
	}

	delete itemcachflex;
	delete npcdataflex;
	delete itemds;
	delete npcds;
}


uint16 World::assignObjId(Object* obj)
{
	// !constant

	// I'm not exactly sure if this is the most efficient way to do this

	//! infinite loop when too many objects

	// find unassigned element
	do {
		lastobjid++;
		if (lastobjid > 0xFFFE) lastobjid = 256;
	} while (objects[lastobjid] != 0);

	objects[lastobjid] = obj;

	return lastobjid;
}

void World::clearObjId(uint16 objid)
{
	objects[objid] = 0;
}

void World::worldStats()
{
	unsigned int i, npccount = 0, objcount = 0, mapcount = 0;

	//!constants
	for (i = 1; i < 256; i++) {
		if (objects[i] != 0)
			npccount++;
	}
	for (i = 256; i < objects.size(); i++) {
		if (objects[i] != 0)
			objcount++;
	}

	for (i = 0; i < maps.size(); i++) {
		if (maps[i] != 0)
			mapcount++;
	}

	pout << "World memory stats:" << std::endl;
	pout << "NPCs    : " << npccount << "/255" << std::endl;
	pout << "Objects : " << objcount << "/65279" << std::endl;
	pout << "Maps    : " << mapcount << "/256" << std::endl;
}

Object* World::getObject(uint16 objid) const
{
	return objects[objid];
}

Actor* World::getNPC(uint16 npcid) const
{
	if (npcid >= npcs.size()) return 0;

	return npcs[npcid];
}

typedef std::vector<Object*> object_vector;

void World::SetupDisplayList(ItemSorter *display_list)
{
	// This is a big hack for now
	sint32 x,y,z;
	int map_num = currentmap->getNum();
	Item* av;
	av = p_dynamic_cast<Item*>(objects[1]);
	
	if (!av || av->getMapNum() != map_num) 
	{
		x = 8192;
		y = 8192;
		z = 64;
	}
	else 
		av->getLocation(x,y,z);

	object_vector::iterator it = objects.begin();
	object_vector::iterator end = objects.end();
	for (; it != end; ++it)
	{
		if (!(*it)) continue;

		Item *item = p_dynamic_cast<Item*>(*it);

		if (!item) continue;

		if (item->getObjId() < 256 && item->getMapNum() != map_num) continue;

		item->setupLerp(x,y,z);
//		item->doLerp(256);
		display_list->AddItem(item);
	}
}


typedef std::vector<Actor*> actor_vector;

void World::SetupDisplayListNPCs(ItemSorter *display_list)
{
	// This is a big hack for now
	sint32 x,y,z;
	int map_num = currentmap->getNum();
	Item* av;
	av = p_dynamic_cast<Item*>(objects[1]);
	
	if (!av || av->getMapNum() != map_num) 
	{
		x = 8192;
		y = 8192;
		z = 64;
	}
	else 
		av->getLocation(x,y,z);

	actor_vector::iterator it = npcs.begin();
	actor_vector::iterator end = npcs.end();
	for (; it != end; ++it)
	{
		if (!(*it)) continue;

		if ((*it)->getMapNum() != map_num) continue;

		(*it)->setupLerp(x,y,z);
		(*it)->doLerp(256);
		display_list->AddItem(*it);
	}
}

