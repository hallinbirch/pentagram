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

#include "pent_include.h"
#include "PathfinderProcess.h"

#include "Actor.h"
#include "World.h"
#include "Pathfinder.h"

#include "IDataSource.h"
#include "ODataSource.h"

static const unsigned int PATH_OK = 1;
static const unsigned int PATH_FAILED = 0;

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(PathfinderProcess,Process);

PathfinderProcess::PathfinderProcess() : Process()
{

}

PathfinderProcess::PathfinderProcess(Actor* actor_, ObjId item_)
{
	assert(actor_);
	item_num = actor_->getObjId();
	type = 0x0204; // CONSTANT !


	Item* item = World::get_instance()->getItem(item_);
	if (!item) {
		perr << "PathfinderProcess: non-existent target" << std::endl;
		// can't get there...
		result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	currentstep = 0;
	targetitem = item_;
	assert(targetitem);

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(item);

	bool ok = pf.pathfind(path);

	if (!ok) {
		perr << "PathfinderProcess: failed to find path" << std::endl;
		// can't get there...
		result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor_->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::PathfinderProcess(Actor* actor_,
									 sint32 x, sint32 y, sint32 z)
{
	assert(actor_);
	item_num = actor_->getObjId();

	targetx = x;
	targety = y;
	targetz = z;
	targetitem = 0;

	currentstep = 0;

	Pathfinder pf;
	pf.init(actor_);
	pf.setTarget(targetx, targety, targetz);

	bool ok = pf.pathfind(path);

	if (!ok) {
		perr << "PathfinderProcess: failed to find path" << std::endl;
		// can't get there...
		result = PATH_FAILED;
		terminateDeferred();
		return;
	}

	// TODO: check if flag already set? kill other pathfinders?
	actor_->setActorFlag(Actor::ACT_PATHFINDING);
}

PathfinderProcess::~PathfinderProcess()
{

}

void PathfinderProcess::terminate()
{
	Actor* actor = World::get_instance()->getNPC(item_num);
	if (actor) {
		// TODO: only clear if it was set by us?
		// (slightly more complicated if we kill other pathfinders on startup)
		actor->clearActorFlag(Actor::ACT_PATHFINDING);
	}

	Process::terminate();
}

bool PathfinderProcess::run(const uint32 framenum)
{
	if (currentstep >= path.size()) {
		// done
#if 0
		pout << "PathfinderProcess: done" << std::endl;
#endif
		result = PATH_OK;
		terminate();
		return false;
	}

	// try to take the next step

#if 0
	pout << "PathfinderProcess: trying step" << std::endl;
#endif

	Actor* actor = World::get_instance()->getNPC(item_num);

	// if actor is still animating for whatever reason, wait until he stopped
	// FIXME: this should happen before the pathfinder is actually called,
	// since the running animation may move the actor, which could break
	// the found path.
	if (actor->getActorFlags() & Actor::ACT_ANIMLOCK) {
		perr << "PathfinderProcess: ANIMLOCK, waiting" << std::endl;
		return false;
	}

	bool ok = actor->tryAnim(path[currentstep].action,
							 path[currentstep].direction);

	if (!ok) {
#if 0
		pout << "PathfinderProcess: recalculating path" << std::endl;
#endif

		// need to redetermine path
		ok = true;
		Pathfinder pf;
		pf.init(actor);
		if (targetitem) {
			Item* item = World::get_instance()->getItem(targetitem);
			if (!item)
				ok = false;
			else
				pf.setTarget(item);
		} else {
			pf.setTarget(targetx, targety, targetz);
		}
		if (ok)
			ok = pf.pathfind(path);

		currentstep = 0;
		if (!ok) {
			perr << "PathfinderProcess: failed to find path" << std::endl;
			// can't get there anymore
			result = PATH_FAILED;
			terminate();
			return false;
		}
	}

	if (currentstep >= path.size()) {
#if 0
		pout << "PathfinderProcess: done" << std::endl;
#endif
		// done
		result = PATH_OK;
		terminate();
		return false;
	}

	uint16 animpid = actor->doAnim(path[currentstep].action,
								   path[currentstep].direction);
#if 0
	pout << "PathfinderProcess(" << getPid() << "): taking step "
		 << path[currentstep].action << "," << path[currentstep].direction
		 << " (animpid=" << animpid << ")" << std::endl;
#endif
	currentstep++;

	waitFor(animpid);
	return true;
}

void PathfinderProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	ods->write2(targetitem);
	if (targetitem == 0) {
		ods->write2(static_cast<uint16>(targetx));
		ods->write2(static_cast<uint16>(targety));
		ods->write2(static_cast<uint16>(targetz));
	}
	ods->write2(static_cast<uint16>(currentstep));

	ods->write2(static_cast<uint16>(path.size()));
	for (unsigned int i = 0; i < path.size(); ++i) {
		ods->write2(static_cast<uint16>(path[i].action));
		ods->write2(static_cast<uint16>(path[i].direction));
	}
}

bool PathfinderProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	targetitem = ids->read2();
	if (!targetitem) {
		targetx = ids->read2();
		targety = ids->read2();
		targetz = ids->read2();
	}
	currentstep = ids->read2();

	unsigned int pathsize = ids->read2();
	path.resize(pathsize);
	for (unsigned int i = 0; i < pathsize; ++i) {
		path[i].action = (Animation::Sequence) ids->read2();
		path[i].direction = ids->read2();
	}

	return true;
}
