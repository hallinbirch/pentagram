/*
Copyright (C) 2004 The Pentagram team

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

#include "GrantPeaceProcess.h"
#include "World.h"
#include "Actor.h"
#include "CurrentMap.h"
#include "TargetGump.h"
#include "WeaponInfo.h"
#include "PaletteFaderProcess.h"
#include "GUIApp.h"
#include "UCList.h"
#include "LoopScript.h"
#include "Direction.h"
#include "Kernel.h"
#include "GumpNotifyProcess.h"
#include "MainActor.h"
#include "SpriteProcess.h"

#include "IDataSource.h"
#include "ODataSource.h"

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(GrantPeaceProcess,Process);

GrantPeaceProcess::GrantPeaceProcess() : Process()
{

}

GrantPeaceProcess::GrantPeaceProcess(Actor* caster)
{
	assert(caster);
	item_num = caster->getObjId();

	type = 0x21d; // CONSTANT !

	havetarget = false;
}

bool GrantPeaceProcess::run(const uint32 /*framenum*/)
{
	Actor* caster = World::get_instance()->getNPC(item_num);
	if (!caster) {
		terminate();
		return false;
	}

	if (!havetarget) {
		TargetGump* targetgump = new TargetGump(0, 0);
		targetgump->InitGump();
		GUIApp *app = GUIApp::get_instance();
		app->getDesktopGump()->AddChild(targetgump);

		waitFor(targetgump->GetNotifyProcess()->getPid());

		havetarget = true;

		return false;
	}

	// get target result
	ObjId targetid = static_cast<ObjId>(result);
	Actor* target = World::get_instance()->getNPC(targetid);

	if (targetid == 1 || !target ) {
		// targeting the avatar, no target or not an Actor
		terminate();
		return false;
	}

	bool hit = false;

	if (target->getDefenseType() & WeaponInfo::DMG_UNDEAD) {
		// undead

		// first see if we're near Khumash-Gor
		CurrentMap* currentmap = World::get_instance()->getCurrentMap();
		UCList KGlist(2);
		LOOPSCRIPT(script, LS_SHAPE_EQUAL(289));
		currentmap->areaSearch(&KGlist, script, sizeof(script),
							   caster, 2048, false);
		bool khumash = (KGlist.getSize() > 0);

		// then find all the undead in the area
		UCList itemlist(2);
		LOOPSCRIPT(script2, LS_TOKEN_TRUE);
		currentmap->areaSearch(&itemlist, script2, sizeof(script),
							   caster, 768, false);

		for (unsigned int i = 0; i < itemlist.getSize(); ++i) {
			Actor *t = World::get_instance()->getNPC(itemlist.getuint16(i));
			if (!t) continue;

			if (t->getFlags() & Actor::ACT_DEAD) continue;

			// undead?
			if (t->getDefenseType() & WeaponInfo::DMG_UNDEAD) {
				t->receiveHit(item_num, 8, target->getHP(),
							  (WeaponInfo::DMG_MAGIC |
							   WeaponInfo::DMG_PIERCE |
							   WeaponInfo::DMG_FIRE));
				hit = true;

				if (t->getShape() == 411 && khumash) { // CONSTANT!
					sint32 tx,ty,tz;
					t->getLocation(tx,ty,tz);

					// CONSTANT! (shape 480, frame 0-9, repeat 1, delay 1)
					Process *sp = new SpriteProcess(480,0,9,1,1, tx, ty, tz);
					Kernel::get_instance()->addProcess(sp);

					Item* throne = World::get_instance()->getItem(
						KGlist.getuint16(0));
					if (throne) {
						throne->setFrame(1); // CONSTANT!
					}
				}

#if 0
				// FIXME: this seems to screw up the death animation; why?

				int dir = caster->getDirToItemCentre(*t);

				t->hurl(((std::rand() % 5) + 5) * x_fact[dir],
						((std::rand() % 5) + 5) * y_fact[dir],
						((std::rand() % 5) + 5),
						4);
#endif
			}

		}

	} else {
		// not undead

		if (!(target->getActorFlags() & (Actor::ACT_DEAD |
										 Actor::ACT_IMMORTAL |
										 Actor::ACT_INVINCIBLE)))
		{
			if (std::rand() % 10 == 0) {
				target->receiveHit(item_num, 8, target->getHP(),
								   (WeaponInfo::DMG_MAGIC |
									WeaponInfo::DMG_PIERCE |
									WeaponInfo::DMG_FIRE));
				hit = true;
			}
		}

	}


	if (hit) {
		// lightning

		// calling intrinsic...
		PaletteFaderProcess::I_lightningBolt(0, 0);

		// TODO: sfx
	}


	// done
	terminate();

	return true;
}

uint32 GrantPeaceProcess::I_castGrantPeace(const uint8* args,
										   unsigned int /*argsize*/)
{
	MainActor* avatar = World::get_instance()->getMainActor();

	GrantPeaceProcess* gpp = new GrantPeaceProcess(avatar);
	Kernel::get_instance()->addProcess(gpp);

	// start casting
	ProcId anim1 = avatar->doAnim(Animation::cast1, avatar->getDir());

	// cast
	ProcId anim2 = avatar->doAnim(Animation::cast3, avatar->getDir());
	Process* anim2p = Kernel::get_instance()->getProcess(anim2);

	// end casting
	ProcId anim3 = avatar->doAnim(Animation::cast2, avatar->getDir());
	Process* anim3p = Kernel::get_instance()->getProcess(anim3);

	anim2p->waitFor(anim1);
	anim3p->waitFor(anim2);
	gpp->waitFor(anim2);

	return 0;
}

void GrantPeaceProcess::saveData(ODataSource* ods)
{
	ods->write2(1); //version
	Process::saveData(ods);

	uint8 ht = havetarget ? 1 : 0;
	ods->write1(ht);
}

bool GrantPeaceProcess::loadData(IDataSource* ids)
{
	uint16 version = ids->read2();
	if (version != 1) return false;
	if (!Process::loadData(ids)) return false;

	havetarget = (ids->read1() != 0);

	return true;
}