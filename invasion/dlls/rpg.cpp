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
#if !defined( OEM_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
#include "soundent.h"
#include "effects.h"
#include "rpg.h"
#include "shake.h"
#endif

#if defined ( HLINVASION_DLL )
extern int gmsgRpgViseur;
extern int gmsgRpgMenu;
extern int gmsgClientDecal;
extern int gEvilImpulse101;

extern short g_sModelIndexBlastCircle;

extern void EnvSmokeCreate(const Vector &center, int m_iScale, float m_fFrameRate, int m_iTime, int m_iEndTime);
#endif // defined ( HLINVASION_DLL )

enum rpg_e {
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD_ROCKET,
	RPG_RELOAD_ELECTRO,
	RPG_RELOAD_NUCLEAR,
	RPG_FIRE,
	RPG_DRAW,
#else
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle
	RPG_FIDGET_UL,	// unloaded fidget
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
};

LINK_ENTITY_TO_CLASS(weapon_rpg, CRpg);

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS(laser_spot, CLaserSpot);

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot(void)
{
	CLaserSpot *pSpot = GetClassPtr((CLaserSpot *)NULL);
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING("laser_spot");

	return pSpot;
}

//=========================================================
//=========================================================
void CLaserSpot::Spawn(void)
{
	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/laserdot.spr");
	UTIL_SetOrigin(pev, pev->origin);
};

//=========================================================
// Suspend- make the laser sight invisible. 
//=========================================================
void CLaserSpot::Suspend(float flSuspendTime)
{
	pev->effects |= EF_NODRAW;

	SetThink(&CLaserSpot::Revive);
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpot::Revive(void)
{
	pev->effects &= ~EF_NODRAW;

	SetThink(NULL);
}

void CLaserSpot::Precache(void)
{
	PRECACHE_MODEL("sprites/laserdot.spr");
};

LINK_ENTITY_TO_CLASS(rpg_rocket, CRpgRocket);

//=========================================================
//=========================================================
CRpgRocket *CRpgRocket::CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher)
{
	CRpgRocket *pRocket = GetClassPtr((CRpgRocket *)NULL);

	UTIL_SetOrigin(pRocket->pev, vecOrigin);
	pRocket->pev->angles = vecAngles;
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	if (pLauncher != NULL)
	{
		pRocket->m_pLauncher = pLauncher;// remember what RPG fired me. 
		pRocket->m_iRocketType = pLauncher->m_iAmmoType;
	}
	else
		pRocket->m_pLauncher = NULL;

	pRocket->Spawn();
	pRocket->SetTouch(&CRpgRocket::RocketTouch);

	if (pOwner != NULL)
		pRocket->pev->owner = pOwner->edict();
	else
		pRocket->pev->owner = NULL;
#else
	CRpgRocket *pRocket = GetClassPtr((CRpgRocket *)NULL);

	UTIL_SetOrigin(pRocket->pev, vecOrigin);
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch(&CRpgRocket::RocketTouch);
	pRocket->m_pLauncher = pLauncher;// remember what RPG fired me. 
	pRocket->m_pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	return pRocket;
}

//=========================================================
//=========================================================
void CRpgRocket::Spawn(void)
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	if (m_iRocketType == AMMO_ELECTRO)
		SET_MODEL(ENT(pev), "models/rpg_electrocket.mdl");
	else if (m_iRocketType == AMMO_NUCLEAR)
		SET_MODEL(ENT(pev), "models/rpg_nuclearrocket.mdl");
	else
		SET_MODEL(ENT(pev), "models/rpgrocket.mdl");
#else
	SET_MODEL(ENT(pev), "models/rpgrocket.mdl");
#endif
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("rpg_rocket");

	SetThink(&CRpgRocket::IgniteThink);
#if !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )
	SetTouch(&CRpgRocket::ExplodeTouch);
#endif

	pev->angles.x -= 30;
	UTIL_MakeVectors(pev->angles);
	pev->angles.x = -(pev->angles.x + 30);

	pev->velocity = gpGlobals->v_forward * 250;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	pev->dmg = gSkillData.plrDmgRPG;
}

//=========================================================
//=========================================================
void CRpgRocket::RocketTouch(CBaseEntity *pOther)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	//modif de Julien
	if (m_pTargetMonster != NULL && m_pLauncher && m_pLauncher->m_cActiveRockets == 1)
	{
		if (pOther == m_pTargetMonster)
			m_pLauncher->UpdateCrosshair(RPG_TEXT_TOUCHE);
		else
			m_pLauncher->UpdateCrosshair(RPG_TEXT_MANQUE);

		m_pLauncher->m_cActiveRockets = 0;
	}

	// enflamme le gaz

	if (IsInGaz() == TRUE)
	{
		edict_t *pFind = FIND_ENTITY_BY_CLASSNAME(NULL, "player");
		CBaseEntity *pPlayer = CBaseEntity::Instance(pFind);
		pPlayer->m_bFireInGaz = TRUE;
	}

	switch (m_iRocketType)
	{
	case AMMO_ROCKET:
	{
		pev->origin = pev->origin - pev->velocity * 0.05;		// revient un peu en arriere pour ne pas exploser du sol
		ExplodeTouch(pOther);
		break;
	}

	case AMMO_ELECTRO:
	{
		// truc affreux pour creer une particule a travers ce message pour les decals

		MESSAGE_BEGIN(MSG_ALL, gmsgClientDecal);

		WRITE_COORD(pev->origin.x);			// xyz source
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(0);						// xyz norme
		WRITE_COORD(0);
		WRITE_COORD(0);
		WRITE_CHAR('a');						// type de texture
		WRITE_BYTE(4);						//  4 == electro-rocket

		MESSAGE_END();

		SetThink(&CRpgRocket::ElectroThink);
		// pev->nextthink	= gpGlobals->time + 0.1;
		m_flDiskTime = UTIL_WeaponTimeBase();
		m_flLastRadius = 0.0;						// pour coller avec la dll client

		pev->effects = 0;

		UTIL_ScreenFadeAll(Vector(215, 225, 255), 0.3, 0, 80, FFADE_IN);

		// fait du bruit pour réveiller les voisins

		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/rpg_electrocket.wav", 1, ATTN_NONE);


		break;
	}

	case AMMO_NUCLEAR:
	{
		// explosion

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD(pev->origin.x);	// Send to PAS because of the sound
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 200);
		WRITE_SHORT(m_sNuclearSprite);
		WRITE_BYTE(100); // scale * 10
		WRITE_BYTE(10); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		// cercles

		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 600 * 5); // reach damage radius over .3 seconds
		WRITE_SHORT(g_sModelIndexBlastCircle);
		WRITE_BYTE(0); // startframe
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(3); // life
		WRITE_BYTE(128);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(255);	// rgb
		WRITE_BYTE(255);
		WRITE_BYTE(200);
		WRITE_BYTE(170); //brightness
		WRITE_BYTE(0);		// speed
		MESSAGE_END();
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 32);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z - 600 * 5 / 2); // reach damage radius over .3 seconds
		WRITE_SHORT(g_sModelIndexBlastCircle);
		WRITE_BYTE(0); // startframe
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(4); // life
		WRITE_BYTE(48);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(255);	// rgb
		WRITE_BYTE(255);
		WRITE_BYTE(200);
		WRITE_BYTE(170); //brightness
		WRITE_BYTE(0);		// speed
		MESSAGE_END();


		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/mortarhit.wav", 1, ATTN_NONE);


		UTIL_ScreenFadeAll(Vector(220, 220, 220), 0.2, 0.3, 130, FFADE_IN);
		UTIL_ScreenShakeAll(pev->origin, 200, 100, 3);

		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, BIG_EXPLOSION_VOLUME, 3.0);

		// dommages

		float flRadius = 1000;

		CBaseEntity *pEntity = NULL;
		float flDamage = 400;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage != DAMAGE_NO)
			{
				Vector vecSpot = pEntity->Center();
				TraceResult tr;
				UTIL_TraceLine(pev->origin, vecSpot, dont_ignore_monsters, ENT(pev), &tr);

				if (tr.flFraction != 1.0 && tr.pHit != pEntity->edict())
				{
					flDamage = flDamage * (pev->origin - vecSpot).Length() / 3 * flRadius;
				}

				pEntity->TakeDamage(pev, pev, flDamage, DMG_RADIATION);
			}
		}

		SetThink(&CBaseEntity::SUB_Remove);
		// pev->nextthink = gpGlobals->time + 0.1;


		// détection des triggers nuclear

		CBaseEntity *pTrigger = UTIL_FindEntityByClassname(NULL, "trigger_nuclear");

		while (pTrigger != NULL)
		{

			Vector org = pev->origin, min = pTrigger->pev->absmin, max = pTrigger->pev->absmax;

			if ((org.x > min.x && org.x < max.x) &&
				(org.y > min.y && org.y < max.y) &&
				(org.z > min.z && org.z < max.z))
			{
				FireTargets(STRING(pTrigger->pev->target), this, this, USE_ON, 0);
			}

			pTrigger = UTIL_FindEntityByClassname(pTrigger, "trigger_nuclear");
		}



		break;
	}
	}

	pev->movetype = MOVETYPE_NOCLIP;			// bug de la trainee de fumee rebondissante
	pev->solid = SOLID_NOT;
	pev->velocity = Vector(0, 0, 0);

	STOP_SOUND(edict(), CHAN_VOICE, "weapons/rocket1.wav");
#else
	if (m_pLauncher)
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
	}

	STOP_SOUND(edict(), CHAN_VOICE, "weapons/rocket1.wav");
	ExplodeTouch(pOther);
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}

//=========================================================
//=========================================================
void CRpgRocket::Precache(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	PRECACHE_MODEL("models/rpgrocket.mdl");
	PRECACHE_MODEL("models/rpg_electrocket.mdl");
	PRECACHE_MODEL("models/rpg_nuclearrocket.mdl");

	PRECACHE_SOUND("weapons/rocket1.wav");
	PRECACHE_SOUND("weapons/mortarhit.wav");
	PRECACHE_SOUND("weapons/rpg_electrocket.wav");

	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	m_iDiskTexture = PRECACHE_MODEL("sprites/rpg_disk.spr");
	m_sNuclearSprite = PRECACHE_MODEL("sprites/fexplo.spr");
#else
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
int CRpgRocket::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	return 1;
}

void CRpgRocket::Killed(entvars_t *pevAttacker, int iGib)
{	};


void CRpgRocket::ElectroThink(void)
{
	if (m_flLastRadius > ELECTRO_DISK_MAX)
	{
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;

	float flRadius = (gpGlobals->time - m_flDiskTime) * ELECTRO_DISK_SPEED + 1;	// +1 pour coller avec la dll client

	CBaseEntity *pEntity = NULL;
	float flDamage = 100;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			Vector2D vecSpot = (pev->origin - pEntity->Center()).Make2D();

			if (vecSpot.Length() > m_flLastRadius &&
				(
				(pEntity->pev->solid == SOLID_BSP && pEntity->pev->mins.z <= pev->origin.z && pEntity->pev->maxs.z >= pev->origin.z)
				||
				(pEntity->pev->solid != SOLID_BSP && pEntity->pev->absmin.z <= pev->origin.z && pEntity->pev->absmax.z >= pev->origin.z)
				)
				)

			{
				pEntity->TakeDamage(pev, pev, flDamage, DMG_SHOCK);

				if (pEntity->IsPlayer())
				{
					short sens = RANDOM_LONG(0, 1);
					sens = sens == 0 ? -1 : 1;
					pEntity->pev->punchangle.y = 25 * sens;
					pEntity->pev->punchangle.x = 25 * sens;
				}
			}
		}
	}

	m_flLastRadius = flRadius;
}
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )

void CRpgRocket::IgniteThink(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )

	pev->movetype = MOVETYPE_FLY;

	if (m_iRocketType == AMMO_ROCKET)
		pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5);

	int brightness = (m_iRocketType == AMMO_ROCKET) ? 255 : 100;

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(40); // life
	WRITE_BYTE(5);  // width
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(brightness);	// brightness
	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink(&CRpgRocket::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1;
#else
	// pev->movetype = MOVETYPE_TOSS;

	pev->movetype = MOVETYPE_FLY;
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(40); // life
	WRITE_BYTE(5);  // width
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink(&CRpgRocket::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1;
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}


void CRpgRocket::FollowThink(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	Vector angDir;
	TraceResult tr;

	UTIL_MakeAimVectors(pev->angles);

	vecTarget = gpGlobals->v_forward;

	if (m_pTargetMonster != NULL)
	{
		pOther = m_pTargetMonster;
		UTIL_TraceLine(pev->origin, pOther->Center(), dont_ignore_monsters, ENT(pev), &tr);

		if (tr.flFraction >= 0.90)
		{
			vecDir = (pOther->Center() - pev->origin).Normalize();
			angDir = UTIL_VecToAngles(vecDir);


			if ((CBaseEntity::Instance(pev->owner))->IsPlayer())
				vecTarget = vecDir/* * 100*/;

			else
			{
				float flAngDistX = UTIL_AngleDiff(angDir.x, pev->angles.x);
				float flAngDistY = UTIL_AngleDiff(angDir.y, pev->angles.y);

				if (fabs(flAngDistX) < 45 && fabs(flAngDistY) < 45)
				{

					if (fabs(flAngDistX) <= 9 && fabs(flAngDistY) <= 9)
					{
						vecTarget = vecDir;
					}
					else
					{
						angDir = pev->angles;
						int iSens = flAngDistX > 0 ? 1 : -1;
						angDir.x = angDir.x + flAngDistX / 2 * iSens;
						iSens = flAngDistY > 0 ? 1 : -1;
						angDir.y = angDir.y + flAngDistY / 2 * iSens;

						UTIL_MakeVectors(angDir);
						vecTarget = gpGlobals->v_forward;
					}
				}
				else
				{
					m_pTargetMonster = NULL;
				}
			}
		}
	}

	pev->angles = UTIL_VecToAngles(vecTarget);

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 4);
		}
		else
		{
			if (pev->velocity.Length() > 2000)
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav");
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if (pev->waterlevel == 0 && pev->velocity.Length() < 1500)
		{
			Detonate();
		}
	}

	//modif de JUlien

	if (CBaseEntity::Instance(pev->owner)->IsPlayer())
		CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, pev->velocity.Length(), 0.1);

	pev->nextthink = gpGlobals->time + 0.1;
#else
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	float flDist, flMax, flDot;
	TraceResult tr;

	UTIL_MakeAimVectors(pev->angles);

	vecTarget = gpGlobals->v_forward;
	flMax = 4096;

	// Examine all entities within a reasonable radius
	while ((pOther = UTIL_FindEntityByClassname(pOther, "laser_spot")) != NULL)
	{
		UTIL_TraceLine(pev->origin, pOther->pev->origin, dont_ignore_monsters, ENT(pev), &tr);
		// ALERT( at_console, "%f\n", tr.flFraction );
		if (tr.flFraction >= 0.90)
		{
			vecDir = pOther->pev->origin - pev->origin;
			flDist = vecDir.Length();
			vecDir = vecDir.Normalize();
			flDot = DotProduct(gpGlobals->v_forward, vecDir);
			if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
			{
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	pev->angles = UTIL_VecToAngles(vecTarget);

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 4);
		}
		else
		{
			if (pev->velocity.Length() > 2000)
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav");
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if (pev->waterlevel == 0 && pev->velocity.Length() < 1500)
		{
			Detonate();
		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	pev->nextthink = gpGlobals->time + 0.1;
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}
#endif



void CRpg::Reload(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	if (m_cActiveRockets < 0 || m_flTimeWeaponIdle > UTIL_WeaponTimeBase()) // weaponidle () declenche le rechargement
		return;

	if ((m_iAmmoType == AMMO_ROCKET		&& m_iAmmoRocket == 0) ||
		(m_iAmmoType == AMMO_ELECTRO	&& m_iAmmoElectro == 0) ||
		(m_iAmmoType == AMMO_NUCLEAR	&& m_iAmmoNuclear == 0)
		)
		return;

	// bodygroup
	// en cas de changement de munition, le bodygroup n'a pas été effacé par weaponidle()

	pev->body = RPG_WEAPON_EMPTY;

	// animations

	int iAnim;
	switch (m_iAmmoType)
	{
	case AMMO_ROCKET:
		iAnim = RPG_RELOAD_ROCKET;
		m_flReloadTime = gpGlobals->time + 2;
		break;

	case AMMO_ELECTRO:
		iAnim = RPG_RELOAD_ELECTRO;
		m_flReloadTime = gpGlobals->time + 4;
		break;

	case AMMO_NUCLEAR:
		iAnim = RPG_RELOAD_NUCLEAR;
		m_flReloadTime = gpGlobals->time + 6;
		break;
	}


	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usRpgReload, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iAnim, pev->body, 0, 0);

	// validation du rechargement

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10;
	m_flNextPrimaryAttack = GetNextAttackDelay(10);
#else
	int iResult;

	if (m_iClip == 1)
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if (m_pPlayer->ammo_rockets <= 0)
		return;

	// because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
	// weapons code is constantly calling into this function, but is often denied because 
	// a) missiles are in flight, but the LTD is on
	// or
	// b) player is totally out of ammo and has nothing to switch to, and should be allowed to
	//    shine the designator around
	//
	// Set the next attack time into the future so that WeaponIdle will get called more often
	// than reload, allowing the RPG LTD to be updated

	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

	if (m_cActiveRockets && m_fSpotActive)
	{
		// no reloading when there are active missiles tracking the designator.
		// ward off future autoreload attempts by setting next attack time into the future for a bit. 
		return;
	}

#ifndef CLIENT_DLL
	if (m_pSpot && m_fSpotActive)
	{
		m_pSpot->Suspend(2.1);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif

	if (m_iClip == 0)
		iResult = DefaultReload(RPG_MAX_CLIP, RPG_RELOAD, 2);

	if (iResult)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}

void CRpg::Spawn()
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	Precache();
	m_iId = WEAPON_RPG;
	SET_MODEL(ENT(pev), "models/w_rpg.mdl");

	m_iClip = 1;				// empeche le declenchement du rechargement par la classe mere

	m_iAmmoType = AMMO_ROCKET;
	m_bLoaded = TRUE;			//pas de rechargement la premiere fois
	m_flReloadTime = -1;		// -1 = pas en cours de rechargement

	pev->body = RPG_WEAPON_ROCKET;

	m_iMenuState = 0;
	m_iMenuState |= (RPG_MENU_ROCKET_SELECTED | RPG_MENU_ROCKET_EMPTY | RPG_MENU_ELECTRO_EMPTY | RPG_MENU_NUCLEAR_EMPTY);

	m_iDefaultAmmo = RPG_DEFAULT_GIVE;

	m_flLastBip = 0;


	FallInit();// get ready to fall down
#else
	Precache();
	m_iId = WEAPON_RPG;

	SET_MODEL(ENT(pev), "models/w_rpg.mdl");
	m_fSpotActive = 1;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// more default ammo in multiplay. 
		m_iDefaultAmmo = RPG_DEFAULT_GIVE * 2;
	}
	else
	{
		m_iDefaultAmmo = RPG_DEFAULT_GIVE;
	}

	FallInit();// get ready to fall down.
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}


void CRpg::Precache(void)
{
	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	PRECACHE_SOUND("sentences/hev_aim_on.wav");
	PRECACHE_SOUND("sentences/hev_aim_off.wav");
	PRECACHE_SOUND("weapons/rpg_lock03.wav");
#else
	UTIL_PrecacheOther("laser_spot");
	UTIL_PrecacheOther("rpg_rocket");

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound
#endif
	m_usRpg = PRECACHE_EVENT(1, "events/rpg.sc");
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	m_usRpgIdle = PRECACHE_EVENT(1, "events/rpgidle.sc");
	m_usRpgReload = PRECACHE_EVENT(1, "events/rpgreload.sc");
#endif
}


int CRpg::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	p->pszAmmo1 = NULL;
#else
	p->pszAmmo1 = "rockets";
#endif
	p->iMaxAmmo1 = ROCKET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RPG_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_RPG;
	p->iFlags = 0;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

int CRpg::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
		m_pPlayer->TextAmmo(TA_RPG);

		// ajoute la première munition

		AddAmmo(this, AMMO_ROCKET, 1);
#endif
		return TRUE;
	}
	return FALSE;
}

BOOL CRpg::Deploy()
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	// démarrage du hud rpg
	UpdateMenu();

	PlayStateSound();

	if (m_iAmmoType == AMMO_ROCKET)
		UpdateCrosshair(RPG_CROSSHAIR_EMPTY);
	else
		UpdateCrosshair(RPG_CROSSHAIR_NORMAL);

	m_flReloadTime = -1;		// -1 = pas en cours de rechargement

	// animation

	BOOL bResult = DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW, "rpg");

	m_flNextPrimaryAttack = GetNextAttackDelay(8 / 5.0);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8 / 5.0;

	return bResult;
#else
	if (m_iClip == 0)
	{
		return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW_UL, "rpg");
	}

	return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW1, "rpg");
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}


BOOL CRpg::CanHolster(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	if (m_cActiveRockets)
		return FALSE;
#else
	if (m_fSpotActive && m_cActiveRockets)
	{
		// can't put away while guiding a missile.
		return FALSE;
	}
#endif

	return TRUE;
}

void CRpg::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);

	// fermeture du hud rpg

	m_iMenuState |= RPG_CLOSE;
	UpdateMenu();
	m_iMenuState &= ~RPG_CLOSE;

#else
	SendWeaponAnim(RPG_HOLSTER1);

#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed(NULL, GIB_NEVER);
		m_pSpot = NULL;
	}
#endif
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}



void CRpg::PrimaryAttack()
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	// confirmation de la selection

	if (m_iMenuState & RPG_MENU_ACTIVE)
	{
		m_iMenuState &= ~RPG_MENU_ACTIVE;
		UpdateMenu();

		if (m_iMenuState & RPG_MENU_ROCKET_SELECTED)
		{
			if (m_iAmmoType != AMMO_ROCKET)
			{
				m_bLoaded = FALSE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() - 0.1;
			}
			m_iAmmoType = AMMO_ROCKET;
		}
		else if (m_iMenuState & RPG_MENU_ELECTRO_SELECTED)
		{
			if (m_iAmmoType != AMMO_ELECTRO)
			{
				m_bLoaded = FALSE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() - 0.1;
			}
			m_iAmmoType = AMMO_ELECTRO;
		}
		else if (m_iMenuState & RPG_MENU_NUCLEAR_SELECTED)
		{
			if (m_iAmmoType != AMMO_NUCLEAR)
			{
				m_bLoaded = FALSE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() - 0.1;
			}
			m_iAmmoType = AMMO_NUCLEAR;
		}

		// bon viseur !

		if (m_iAmmoType == AMMO_ROCKET)
			UpdateCrosshair(RPG_CROSSHAIR_EMPTY);
		else
			UpdateCrosshair(RPG_CROSSHAIR_NORMAL);


		m_flNextPrimaryAttack = GetNextAttackDelay(0.3);
	}

	// tir

	else if (m_bLoaded == TRUE &&
		((m_iAmmoType == AMMO_ROCKET		&& m_iAmmoRocket != 0) ||
		(m_iAmmoType == AMMO_ELECTRO	&& m_iAmmoElectro != 0) ||
		(m_iAmmoType == AMMO_NUCLEAR	&& m_iAmmoNuclear != 0))
		)
	{

		// flash et son

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		// bodygroup
		pev->body = RPG_WEAPON_EMPTY;

		// animations
		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usRpg);

		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL
		// roquette

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

		pRocket = CRpgRocket::CreateRpgRocket(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this);

		if ((m_iAmmoType == AMMO_ROCKET) && (m_pEntityLocked != NULL))
		{
			pRocket->m_pTargetMonster = m_pEntityLocked;
			m_cActiveRockets = 1;
		}
		else
		{
			pRocket->m_pTargetMonster = NULL;
			m_cActiveRockets = 0;			// m_cActiveRockets empeche le rechargement
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);// RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct(m_pPlayer->pev->velocity, gpGlobals->v_forward);
#endif // !CLIENT_DLL

		// munitions

		m_bLoaded = FALSE;

		if (m_iAmmoType == AMMO_ROCKET)		// A FAIRE  : continuer de remplacer les masks par iammotype
		{
			m_iAmmoRocket--;

			if (m_iAmmoRocket == 0)
				m_iMenuState |= RPG_MENU_ROCKET_EMPTY;
		}
		else if (m_iAmmoType == AMMO_ELECTRO)
		{
			m_iAmmoElectro--;

			if (m_iAmmoElectro == 0)
				m_iMenuState |= RPG_MENU_ELECTRO_EMPTY;
		}
		else if (m_iAmmoType == AMMO_NUCLEAR)
		{
			m_iAmmoNuclear--;

			if (m_iAmmoNuclear == 0)
				m_iMenuState |= RPG_MENU_NUCLEAR_EMPTY;
		}

		// mise a jour du hud

		UpdateMenu();

		m_pEntityTarget = m_pEntityLocked = NULL;
		m_flLockTime = 0;


		// prochaine attaque

		m_flNextPrimaryAttack = GetNextAttackDelay(1.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;		// weaponidle () declenche le rechargement
	}
	else
	{
		PlayEmptySound();
	}
#else
	if (m_iClip)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

		CRpgRocket *pRocket = CRpgRocket::CreateRpgRocket(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);// RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct(m_pPlayer->pev->velocity, gpGlobals->v_forward);
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usRpg);

		m_iClip--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		PlayEmptySound();
	}
	UpdateSpot();
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}


void CRpg::SecondaryAttack()
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	// menu

	if (!(m_iMenuState & RPG_MENU_ACTIVE))
		m_iMenuState |= RPG_MENU_ACTIVE;

	else if (m_iMenuState & RPG_MENU_ROCKET_SELECTED)
	{
		m_iMenuState &= ~RPG_MENU_ROCKET_SELECTED;

		if (!(m_iMenuState & RPG_MENU_ELECTRO_EMPTY))
			m_iMenuState |= RPG_MENU_ELECTRO_SELECTED;

		else if (!(m_iMenuState & RPG_MENU_NUCLEAR_EMPTY))
			m_iMenuState |= RPG_MENU_NUCLEAR_SELECTED;

		else
			m_iMenuState |= RPG_MENU_ROCKET_SELECTED;
	}

	else if (m_iMenuState & RPG_MENU_ELECTRO_SELECTED)
	{
		m_iMenuState &= ~RPG_MENU_ELECTRO_SELECTED;

		if (!(m_iMenuState & RPG_MENU_NUCLEAR_EMPTY))
			m_iMenuState |= RPG_MENU_NUCLEAR_SELECTED;

		else if (!(m_iMenuState & RPG_MENU_ROCKET_EMPTY))
			m_iMenuState |= RPG_MENU_ROCKET_SELECTED;

		else
			m_iMenuState |= RPG_MENU_ELECTRO_SELECTED;
	}

	else if (m_iMenuState & RPG_MENU_NUCLEAR_SELECTED)
	{
		m_iMenuState &= ~RPG_MENU_NUCLEAR_SELECTED;

		if (!(m_iMenuState & RPG_MENU_ROCKET_EMPTY))
			m_iMenuState |= RPG_MENU_ROCKET_SELECTED;

		else if (!(m_iMenuState & RPG_MENU_ELECTRO_EMPTY))
			m_iMenuState |= RPG_MENU_ELECTRO_SELECTED;

		else
			m_iMenuState |= RPG_MENU_NUCLEAR_SELECTED;
	}

	UpdateMenu();

	PlayStateSound();

#else
	m_fSpotActive = !m_fSpotActive;

#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
		m_pSpot->Killed(NULL, GIB_NORMAL);
		m_pSpot = NULL;
	}
#endif
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	m_flNextSecondaryAttack = GetNextAttackDelay(0.2);
}


void CRpg::WeaponIdle(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	// rafraichissement des donnees client

	if (m_bRpgUpdate == 1)
	{
		UpdateMenu();

		if (m_iAmmoType == AMMO_ROCKET)
			UpdateCrosshair(RPG_CROSSHAIR_EMPTY);
		else
			UpdateCrosshair(RPG_CROSSHAIR_NORMAL);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() - 0.1;		// pour charger une anim et changer le bodygroup
		m_bRpgUpdate = 0;		// weaponidle, ca rafraichit, et c'est déjà pas mal
	}

	// viseur

	if ((m_iAmmoType == AMMO_ROCKET) && (m_cActiveRockets == 0))
		UpdateEntityTarget();

	ResetEmptySound();

	// rechargement

	if (m_flTimeWeaponIdle <  UTIL_WeaponTimeBase() && m_bLoaded == FALSE && m_flReloadTime == -1)	// -1 = pas en cours de rechargement
	{
		Reload();
		return;			// pour ne pas lancer d anim idle
	}

	else if (m_bLoaded == FALSE && m_flReloadTime != -1 && m_flReloadTime < gpGlobals->time)
	{
		m_bLoaded = TRUE;
		m_flReloadTime = -1;
		m_flNextPrimaryAttack = GetNextAttackDelay(0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
	}

	// ajustement du bodygroup
	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		int ibody;
		switch (m_iAmmoType)
		{
		case AMMO_ROCKET:
			ibody = RPG_WEAPON_ROCKET; break;
		case AMMO_ELECTRO:
			ibody = RPG_WEAPON_ELECTRO; break;
		case AMMO_NUCLEAR:
			ibody = RPG_WEAPON_NUCLEAR; break;
		}
		if (m_bLoaded == FALSE)
			ibody = RPG_WEAPON_EMPTY;

		pev->body = ibody;
	}

	// animations idle

	if (m_flTimeWeaponIdle >  UTIL_WeaponTimeBase())
		return;

	int iAnim;
	switch (RANDOM_LONG(0, 6))
	{
	case 0:
		iAnim = RPG_FIDGET;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 15 / 4.0;
		break;

	default:
		iAnim = RPG_IDLE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 15 / 3.0;
		break;
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usRpgIdle, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iAnim, pev->body, 0, 0);

#else
	UpdateSpot();

	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
		if (flRand <= 0.75 || m_fSpotActive)
		{
			if (m_iClip == 0)
				iAnim = RPG_IDLE_UL;
			else
				iAnim = RPG_IDLE;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
		}
		else
		{
			if (m_iClip == 0)
				iAnim = RPG_FIDGET_UL;
			else
				iAnim = RPG_FIDGET;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}

		SendWeaponAnim(iAnim);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )

void CRpg::UpdateEntityTarget(void)
{
	if (m_iMenuState & RPG_MENU_ACTIVE)
		return;			// le texte s'affiche et disparait selon la selection

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);		//tracage du vecteur


	if (tr.pHit && (CBaseEntity::Instance(tr.pHit))->MyMonsterPointer() != NULL)
	{

		// si une entite est touchee et 
		//que l'entité touchée est un monstre


		if (m_pEntityTarget == NULL || (m_pEntityTarget != NULL && (m_pEntityTarget != (CBaseEntity::Instance(tr.pHit)))))
		{

			// si il n'y avait pas de cible, on en met une . On met en memoire l heure de reperage de la cible
			// une cible, mais pas la meme qu avant, ...

			m_pEntityTarget = CBaseEntity::Instance(tr.pHit);
			m_pEntityLocked = NULL;
			m_flLockTime = gpGlobals->time;
			UpdateCrosshair(RPG_CROSSHAIR_PROCESS);


			// son
			EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/rpg_lock03.wav", 0.5, ATTN_NORM);

		}

		else if ((m_pEntityTarget != NULL) && (m_pEntityTarget == (CBaseEntity::Instance(tr.pHit))) && (gpGlobals->time - m_flLockTime > 1))
		{
			// la meme cible pendant 1 sec , on verrouille

			m_pEntityLocked = m_pEntityTarget;
			UpdateCrosshair(RPG_CROSSHAIR_LOCKED);

			// son
			if (m_flLastBip <= 0 || gpGlobals->time - m_flLastBip >= 0.1)
			{
				EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/rpg_lock03.wav", 0.5, ATTN_NORM);
				m_flLastBip = gpGlobals->time;
			}
		}

	}

	else
	{
		//si rien n est touche

		UpdateCrosshair(RPG_CROSSHAIR_EMPTY);
		m_pEntityTarget = m_pEntityLocked = NULL;
		m_flLockTime = 0;
	}

}


void CRpg::PlayStateSound(void)
{
	/*
	if ( m_iAmmoType == AMMO_ROCKET )
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "sentences/hev_aim_on.wav", 0.9, ATTN_NORM );
	else
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "sentences/hev_aim_off.wav", 0.9, ATTN_NORM );
	*/
}

//------------------------------------------
//
// messages au client
//
//------------------------------------------

void CRpg::UpdateCrosshair(int crosshair)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgRpgViseur, NULL, m_pPlayer->pev);
	WRITE_BYTE(crosshair);
	MESSAGE_END();
#endif
}


void CRpg::UpdateMenu(void)
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgRpgMenu, NULL, m_pPlayer->pev);
	WRITE_BYTE(m_iMenuState);
	WRITE_BYTE(m_iAmmoRocket);
	WRITE_BYTE(m_iAmmoElectro);
	WRITE_BYTE(m_iAmmoNuclear);
	MESSAGE_END();
#endif
}


//------------------------------------------
//
// munitions
//
//------------------------------------------


void CRpg::ItemTouch(CBaseEntity *pOther)
{
	if (pOther->IsPlayer() == FALSE)
	{
		return;
	}

	CBasePlayer *pPlayer = (CBasePlayer*)pOther;
	CBasePlayerItem *pItem;
	CRpg *pRpg = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pItem = pPlayer->m_rgpPlayerItems[i];

		while (pItem)
		{
			if (!strcmp("weapon_rpg", STRING(pItem->pev->classname)))
			{
				pRpg = (CRpg*)pItem->GetWeaponPtr();
				break;
			}
			pItem = pItem->m_pNext;
		}
	}

	if (pRpg == NULL)
		return;

	if (pRpg->AddAmmo((CBasePlayerWeapon *)pRpg, AMMO_ROCKET, 1))
	{
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		UTIL_Remove(this);
	}
}


int CRpg::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	//	AddAmmo ( pWeapon, m_iAmmoType, 1 );
	return 1;
}

int CRpg::ExtractClipAmmo(CBasePlayerWeapon *pWeapon)
{
	//	AddAmmo ( pWeapon, m_iAmmoType, 1 );
	return 1;
}

BOOL CRpg::AddAmmo(CBasePlayerWeapon *pWeapon, int iAmmotype, int iNombre)
{
	int *pType;
	CRpg *pRpg = (CRpg*)pWeapon;

	switch (iAmmotype)
	{
	default:
	case AMMO_ROCKET:
		pType = &pRpg->m_iAmmoRocket;
		pRpg->m_iMenuState &= ~RPG_MENU_ROCKET_EMPTY;
		break;

	case AMMO_ELECTRO:
		pType = &pRpg->m_iAmmoElectro;
		pRpg->m_iMenuState &= ~RPG_MENU_ELECTRO_EMPTY;
		break;

	case AMMO_NUCLEAR:
		pType = &pRpg->m_iAmmoNuclear;
		pRpg->m_iMenuState &= ~RPG_MENU_NUCLEAR_EMPTY;
		break;

	}

	if (pType[0] == RPG_MAX_AMMO)
		return FALSE;

	pType[0] += iNombre;

	if (m_pPlayer->m_pActiveItem == pWeapon)
	{
		pRpg->UpdateMenu();
	}

	return TRUE;
}

int CRpg::GiveAmmo(int iAmount, char *szName, int iMax)
{
	//	AddAmmo ( this, m_iAmmoType, 1 );
	return 1;
}
#else
void CRpg::UpdateSpot(void)
{

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		UTIL_SetOrigin(m_pSpot->pev, tr.vecEndPos);
	}
#endif

}
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )  

class CRpgAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
		if (FClassnameIs(pev, "ammo_rpgclip"))
			SET_MODEL(ENT(pev), "models/w_rpgclip.mdl");
		else if (FClassnameIs(pev, "ammo_rpgelectroclip"))
			SET_MODEL(ENT(pev), "models/w_rpgelectroclip.mdl");
		else if (FClassnameIs(pev, "ammo_rpgnuclearclip"))
			SET_MODEL(ENT(pev), "models/w_rpgnuclearclip.mdl");
#else
		SET_MODEL(ENT(pev), "models/w_rpgammo.mdl");
#endif
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
		PRECACHE_MODEL("models/w_rpgclip.mdl");
		PRECACHE_MODEL("models/w_rpgelectroclip.mdl");
		PRECACHE_MODEL("models/w_rpgnuclearclip.mdl");
#else
		PRECACHE_MODEL("models/w_rpgammo.mdl");
#endif
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther)
	{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
		if (pOther->IsPlayer() == 0)
			return FALSE;

		CBasePlayer *pPlayer = (CBasePlayer *)pOther;


		CBasePlayerItem *pItem;
		CRpg *pRpg = NULL;
		int i;
		int fin = 0;

		for (i = 0; i < MAX_ITEM_TYPES; i++)
		{
			pItem = pPlayer->m_rgpPlayerItems[i];

			while (pItem)
			{
				if (!strcmp("weapon_rpg"/*&pszItemName*/, STRING(pItem->pev->classname)))
				{
					fin = 1;
					pRpg = (CRpg*)pItem->GetWeaponPtr();
				}
				pItem = pItem->m_pNext;

				if (fin)
					break;
			}

			if (fin)
				break;
		}

		if (pRpg == NULL)
			return FALSE;

		int iAmmotype = 0;

		if (FClassnameIs(pev, "ammo_rpgclip"))
			iAmmotype = AMMO_ROCKET;
		else if (FClassnameIs(pev, "ammo_rpgelectroclip"))
			iAmmotype = AMMO_ELECTRO;
		else if (FClassnameIs(pev, "ammo_rpgnuclearclip"))
			iAmmotype = AMMO_NUCLEAR;
		else
			return FALSE;

		if (pRpg->AddAmmo((CBasePlayerWeapon *)pRpg, iAmmotype, 1) == TRUE)
		{
			// accepté, voit si on peut afficher le texte

			switch (iAmmotype)
			{
			default:
			case AMMO_ROCKET:
				break;
			case AMMO_ELECTRO:
				pRpg->m_pPlayer->TextAmmo(TA_ELECTROROCKET); break;
			case AMMO_NUCLEAR:
				pRpg->m_pPlayer->TextAmmo(TA_NUCLEARROCKET); break;
			}

			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}

		else
			return FALSE;
#else
		int iGive;

#ifdef CLIENT_DLL
		if (bIsMultiplayer())
#else
		if (g_pGameRules->IsMultiplayer())
#endif
		{
			// hand out more ammo per rocket in multiplayer.
			iGive = AMMO_RPGCLIP_GIVE * 2;
		}
		else
		{
			iGive = AMMO_RPGCLIP_GIVE;
		}

		if (pOther->GiveAmmo(iGive, "rockets", ROCKET_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	}
};
LINK_ENTITY_TO_CLASS(ammo_rpgclip, CRpgAmmo);
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
LINK_ENTITY_TO_CLASS(ammo_rpgelectroclip, CRpgAmmo);
LINK_ENTITY_TO_CLASS(ammo_rpgnuclearclip, CRpgAmmo);
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )

#endif
