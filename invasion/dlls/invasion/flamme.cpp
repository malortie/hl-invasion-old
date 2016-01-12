/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"
#include "flamme.h"
#include "decals.h"

extern int gmsgLFlammes;

LINK_ENTITY_TO_CLASS(monster_flamme, CFlamme);

TYPEDESCRIPTION	CFlamme::m_SaveData[] =
{
	DEFINE_FIELD(CFlamme, m_flBirthTime, FIELD_TIME),
	DEFINE_FIELD(CFlamme, m_iMode, FIELD_INTEGER),
	DEFINE_FIELD(CFlamme, m_flPlayerDmg, FIELD_FLOAT),
	DEFINE_FIELD(CFlamme, m_bRestore, FIELD_INTEGER),
	DEFINE_FIELD(CFlamme, m_flMonsterDamage, FIELD_FLOAT),
};
IMPLEMENT_SAVERESTORE(CFlamme, CPointEntity);

//---------------------------------------------------------
// flammes



CFlamme *CFlamme::CreateFlamme(Vector vecOrigin, Vector vecAngles, int imode)
{
	CFlamme *pFlamme = GetClassPtr((CFlamme *)NULL);

	UTIL_SetOrigin(pFlamme->pev, vecOrigin);
	pFlamme->pev->angles = vecAngles;

	pFlamme->m_iMode = imode;
	pFlamme->Spawn();

	CBaseEntity *pPlayer = NULL;
	pPlayer = UTIL_FindEntityByClassname(NULL, "player");

	MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

	WRITE_BYTE(imode);				// mode
	WRITE_COORD(ENTINDEX(pFlamme->edict()));		// idx
	WRITE_COORD(pFlamme->m_flBirthTime);			// age

	if (pFlamme->m_iMode == FLAMME_ATTACHEE)
	{
		// pour les flammes attachées, les coordonnées sont l'offset de l'origine au centre
		WRITE_COORD(vecOrigin.x);			// offset
		WRITE_COORD(vecOrigin.y);			// offset
		WRITE_COORD(vecOrigin.z);			// offset
	}
	else
	{
		WRITE_COORD(0.0f);			// offset
		WRITE_COORD(0.0f);			// offset
		WRITE_COORD(0.0f);			// offset
	}

	MESSAGE_END();

	return pFlamme;
}


void CFlamme::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCEMISSILE;
	//	pev->movetype = MOVETYPE_FLY;
	//	pev->solid = SOLID_SLIDEBOX;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	SET_MODEL(ENT(pev), "models/tank_cam.mdl");				// model
	//	SET_MODEL(ENT(pev), "models/can.mdl" );				// model
	pev->classname = MAKE_STRING("monster_flamme");

	UTIL_MakeVectors(pev->angles);

	pev->velocity = gpGlobals->v_forward.Normalize() * 350;

	m_flBirthTime = gpGlobals->time;
	m_flPlayerDmg = 0;
	m_bRestore = FALSE;

	SetThink(&CFlamme::FlameThink);
	SetTouch(&CFlamme::FlameTouch);
	pev->nextthink = m_flBirthTime + 0.1;


	// son
	EMIT_SOUND_DYN(ENT(pev), CHAN_STREAM, "ambience/burning1.wav", 0.8, ATTN_NORM, 0, 100);

}

void CFlamme::Precache(void)
{
	PRECACHE_MODEL("models/tank_cam.mdl");
	PRECACHE_MODEL("models/can.mdl");
	PRECACHE_MODEL("sprites/lflammes02.spr");
	PRECACHE_MODEL("sprites/tank_smokeball.spr");
	PRECACHE_SOUND("ambience/burning1.wav");
}


BOOL CFlamme::CanCatchMonster(CBaseMonster * pMonster)
{
	if (pMonster == NULL)
		return FALSE;

	if (pMonster->pev->deadflag == DEAD_DEAD || pMonster->pev->deadflag == DEAD_DYING)
		return FALSE;

	if (pMonster->m_MonsterState == MONSTERSTATE_HUNT || pMonster->m_MonsterState == MONSTERSTATE_PLAYDEAD || pMonster->m_MonsterState == MONSTERSTATE_DEAD)
		return FALSE;


	if (!(
		FClassnameIs(pMonster->pev, "monster_human_grunt") ||
		FClassnameIs(pMonster->pev, "monster_sniper") ||
		FClassnameIs(pMonster->pev, "monster_rpg_grunt") ||
		FClassnameIs(pMonster->pev, "monster_human_assassin") ||

		FClassnameIs(pMonster->pev, "monster_scientist") ||
		FClassnameIs(pMonster->pev, "monster_barney") ||

		FClassnameIs(pMonster->pev, "player") ||

		FClassnameIs(pMonster->pev, "monster_headcrab") ||
		FClassnameIs(pMonster->pev, "monster_alien_slave") ||
		FClassnameIs(pMonster->pev, "monster_bullchicken") ||
		FClassnameIs(pMonster->pev, "monster_barnacle") ||
		FClassnameIs(pMonster->pev, "monster_houndeye") ||
		FClassnameIs(pMonster->pev, "monster_flybee") ||
		FClassnameIs(pMonster->pev, "monster_luciole") ||
		FClassnameIs(pMonster->pev, "monster_alien_controller")
		))
		return FALSE;



	return TRUE;
}


float CFlamme::FlameDamageMonster(CBaseMonster *pMonster)
{

	if (FClassnameIs(pMonster->pev, "monster_luciole"))
		return 100;

	// temps mis à mourir

	float flAgonie = RANDOM_FLOAT(4, 6);

	// dommages par dixième de seconde

	float flDamage = (pMonster->pev->max_health / flAgonie) * 0.1;

	return flDamage;
}




void EXPORT CFlamme::FlameThink(void)
{

	//------------------------------------------
	// flamme libre
	//------------------------------------------

	CBaseEntity *pPlayer = NULL;
	pPlayer = UTIL_FindEntityByClassname(NULL, "player");

	// restoration après la sauvegarde

	if (m_bRestore == TRUE)
	{
		if (m_iMode == FLAMME_LIBRE)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

			WRITE_BYTE(m_iMode);				// mode
			WRITE_COORD(ENTINDEX(edict()));		// idx
			WRITE_COORD(m_flBirthTime);			// age

			WRITE_COORD(0.0f);			// offset
			WRITE_COORD(0.0f);			// offset
			WRITE_COORD(0.0f);			// offset

			MESSAGE_END();

		}
		else if (m_iMode == FLAMME_ATTACHEE)
		{
			CBaseEntity *pEntityMonster = CBaseEntity::Instance(pev->aiment);

			if (pEntityMonster != NULL)
			{
				CBaseMonster *pMonster = pEntityMonster->MyMonsterPointer();

				Vector vecOrigin = pMonster->Center() - pMonster->pev->origin;

				MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

				WRITE_BYTE(m_iMode);				// mode
				WRITE_COORD(ENTINDEX(edict()));		// idx
				WRITE_COORD(m_flBirthTime);			// age

				WRITE_COORD(vecOrigin.x);			// offset
				WRITE_COORD(vecOrigin.y);			// offset
				WRITE_COORD(vecOrigin.z);			// offset

				MESSAGE_END();
			}
		}
		m_bRestore = FALSE;
	}



	if (m_iMode == FLAMME_LIBRE)
	{

		// vitesse

		if (gpGlobals->time - m_flBirthTime > 1)
			pev->velocity = pev->velocity.Normalize() * min(50, pev->velocity.Length());

		// destruction

		if (gpGlobals->time - m_flBirthTime > 3 || pev->waterlevel > 0)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

			WRITE_BYTE(DETRUIT_FLAMME);			// mode
			WRITE_COORD(ENTINDEX(edict()));		// idx

			MESSAGE_END();

			pev->nextthink = gpGlobals->time + 0.1;
			SetThink(&CBaseEntity::SUB_Remove);

			// son
			STOP_SOUND(ENT(pev), CHAN_STREAM, "ambience/burning1.wav");

		}
	}


	//------------------------------------------
	// détection des ennemis
	//------------------------------------------

	pev->nextthink = gpGlobals->time + 0.1;

	CBaseEntity *pEntity = NULL;
	pEntity = UTIL_FindEntityInSphere(NULL, pev->origin, gpGlobals->time - m_flBirthTime < 1 ? FLAMME_RADIUS_SMALL : FLAMME_RADIUS_BIG);

	while (pEntity != NULL)
	{
		CBaseMonster *pMonster = pEntity->MyMonsterPointer();

		if (CanCatchMonster(pMonster) == FALSE)
		{
			pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, gpGlobals->time - m_flBirthTime < 1 ? FLAMME_RADIUS_SMALL : FLAMME_RADIUS_BIG);
			continue;
		}

		pMonster->pev->renderfx = kRenderFxGlowShell;
		pMonster->pev->rendercolor = Vector(255, 110, 15);

		pMonster->SetState(MONSTERSTATE_HUNT);
		pMonster->SetConditions(
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND |
			bits_COND_ENEMY_DEAD);


		// flamme maîtrese

		CFlamme *pFlamme = CFlamme::CreateFlamme(pMonster->Center() - pMonster->pev->origin, Vector(0, 0, 0), FLAMME_ATTACHEE);
		pFlamme->pev->movetype = MOVETYPE_FOLLOW;
		pFlamme->pev->aiment = pMonster->edict();

		// préparation des dommages

		pFlamme->m_flMonsterDamage = FlameDamageMonster(pMonster);

		// fin de la boucle

		pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, gpGlobals->time - m_flBirthTime < 1 ? FLAMME_RADIUS_SMALL : FLAMME_RADIUS_BIG);
	}

	// lumière

	float ratio = 1;

	if (gpGlobals->time - m_flBirthTime < 0.5)
		ratio = (gpGlobals->time - m_flBirthTime) / 0.5;

	if (gpGlobals->time - m_flBirthTime > 2.5 && m_iMode == FLAMME_LIBRE)
		ratio = min(0, 1 - (gpGlobals->time - m_flBirthTime - 2.5));

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(pev->origin.x);	// X
	WRITE_COORD(pev->origin.y);	// Y
	WRITE_COORD(pev->origin.z);	// Z
	WRITE_BYTE(10 * RANDOM_FLOAT(0.8, 1.2) * ratio);		// radius * 0.1

	WRITE_BYTE(RANDOM_FLOAT(240, 255));		// r
	WRITE_BYTE(RANDOM_FLOAT(180, 213));		// g
	WRITE_BYTE(RANDOM_FLOAT(96, 110));		// b

	WRITE_BYTE(3);		// time * 10
	WRITE_BYTE(0);		// decay * 0.1
	MESSAGE_END();


	// son
	float delta = gpGlobals->time - m_flBirthTime;
	float flVol = delta < 3 ? (delta > 2 ? 0.4 + 0.6 * (1 - (delta - 2)) : 1) : 0;

	EMIT_SOUND_DYN(ENT(pev), CHAN_STREAM, "ambience/burning1.wav", flVol, ATTN_NORM, SND_CHANGE_VOL, 100);


	if (m_iMode == FLAMME_LIBRE)
		return;



	//------------------------------------------
	// routine de contrôle
	//------------------------------------------


	CBaseEntity *pEntityMonster = CBaseEntity::Instance(pev->aiment);

	CBaseMonster *pMonster;

	if (pEntityMonster != NULL)
		pMonster = pEntityMonster->MyMonsterPointer();


	// entité détruite

	if (pEntityMonster == NULL || pMonster == NULL || !UTIL_IsValidEntity(pMonster->edict()) || pMonster->pev->effects & EF_NODRAW)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

		WRITE_BYTE(DETRUIT_FLAMME);			// mode
		WRITE_COORD(ENTINDEX(edict()));		// idx

		MESSAGE_END();

		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CBaseEntity::SUB_Remove);
	}

	// dommages

	else if (pMonster->pev->deadflag != DEAD_DEAD && pMonster->pev->deadflag != DEAD_DYING && !pMonster->IsPlayer())
	{
		pMonster->TakeDamage(pev, pev, m_flMonsterDamage, DMG_BURN | DMG_NEVERGIB);
	}

	// joueur

	else if (pMonster->IsPlayer())
	{
		// fini

		if (gpGlobals->time - m_flBirthTime > PLAYER_BURN_TIME || pMonster->pev->waterlevel >= 2)
		{
			pMonster->SetState(MONSTERSTATE_NONE);

			MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

			WRITE_BYTE(DETRUIT_FLAMME);			// mode
			WRITE_COORD(ENTINDEX(edict()));		// idx

			MESSAGE_END();

			pev->nextthink = gpGlobals->time + 0.1;
			SetThink(&CBaseEntity::SUB_Remove);
			return;
		}

		// dommages

		else
		{
			m_flPlayerDmg += FLAMME_DAMAGE_PLAYER*0.1 / PLAYER_BURN_TIME;

			if ((int)m_flPlayerDmg > 1)
			{
				pMonster->TakeDamage(pev, pev, (int)m_flPlayerDmg, DMG_BURN);
				m_flPlayerDmg -= (int)m_flPlayerDmg;
			}
		}
	}

	// mort

	else if (pMonster->pev->deadflag == DEAD_DEAD)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgLFlammes, NULL, pPlayer->pev);

		WRITE_BYTE(FLAMME_DEAD);			// mode
		WRITE_COORD(ENTINDEX(edict()));		// idx

		MESSAGE_END();

		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CBaseEntity::SUB_Remove);
	}



}

void CFlamme::FlameTouch(CBaseEntity *pOther)
{
	TraceResult trace = UTIL_GetGlobalTrace();

	if (pOther && pOther->IsBSPModel())
	{
		pev->velocity = pev->velocity.Normalize() * min(30, pev->velocity.Length());
		UTIL_DecalTrace(&trace, DECAL_SCORCH1 + RANDOM_LONG(0, 1));
	}
}