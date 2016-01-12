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
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "shake.h"
#include "gamerules.h"
#include "sgball.h"

enum supergun_e {
	SG_IDLE = 0,
	SG_SHOOT1,
	SG_BIGSHOOT,
	SG_DRAW,
	SG_RELOAD
};

LINK_ENTITY_TO_CLASS(weapon_supergun, CSuperGun);

void CSuperGun::Spawn()
{
	Precache();
	m_iId = WEAPON_SUPERGUN;
	SET_MODEL(ENT(pev), "models/w_supergun.mdl");

	m_iDefaultAmmo = SUPERGUN_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CSuperGun::Precache(void)
{
	PRECACHE_MODEL("models/w_supergun.mdl");
	PRECACHE_MODEL("models/v_supergun.mdl");
	PRECACHE_MODEL("models/p_gauss.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("weapons/tu_fire1.wav");
	PRECACHE_SOUND("weapons/gauss2.wav");
	PRECACHE_SOUND("weapons/beamstart14.wav");

	PRECACHE_MODEL("sprites/blueflare1.spr");

	m_usSG = PRECACHE_EVENT(1, "events/supergun.sc");
	m_usSG2 = PRECACHE_EVENT(1, "events/supergun2.sc");
	m_usSG3 = PRECACHE_EVENT(1, "events/supergun3.sc");

	UTIL_PrecacheOther("sg_ball");
}

int CSuperGun::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		m_pPlayer->TextAmmo(TA_SUPERGUN);

		return TRUE;
	}
	return FALSE;
}

int CSuperGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "chewinggum";
	p->iMaxAmmo1 = SUPERGUN_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SUPERGUN_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SUPERGUN;
	p->iFlags = 0;
	p->iWeight = SUPERGUN_WEIGHT;

	return 1;
}

BOOL CSuperGun::Deploy()
{
	BOOL bResult = DefaultDeploy("models/v_supergun.mdl", "models/p_gauss.mdl", SG_DRAW, "supergun");

	if (bResult)
	{
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 2, 5);
	}

	return bResult;
}

void CSuperGun::Holster(int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(SG_DRAW);

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
}

void CSuperGun::Reload(void)
{
	if (m_pPlayer->ammo_chewinggum <= 0)
		return;

	DefaultReload(SUPERGUN_MAX_CLIP, SG_RELOAD, 69 / 25);

	int iskin = 0;
	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usSG3, 1, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iskin, 0, 0, 0);
}



void CSuperGun::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
		return;
	}

	// enflamme le gaz

	if (m_pPlayer->IsInGaz() == TRUE)
		m_pPlayer->m_bFireInGaz = TRUE;

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}


	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// Fire 1
	PLAYBACK_EVENT(0, m_pPlayer->edict(), m_usSG);

#ifndef CLIENT_DLL
	// flash

	CSprite *pMuzzle = CSprite::SpriteCreate ( "sprites/blueflare1.spr"/*"sprites/animglow01.spr"*/, Vector(0,0,0), TRUE );
	pMuzzle->SetAttachment ( m_pPlayer->edict(), 1 );

	pMuzzle->SetScale ( 0.3 );
	pMuzzle->SetTransparency ( kRenderTransAdd, 128, 128, 220, 250, kRenderFxNone );

//	pMuzzle->pev->frame = RANDOM_LONG(3,6);
	pMuzzle->SetThink ( &CBaseEntity::SUB_Remove );
	pMuzzle->pev->nextthink = gpGlobals->time + 0.075;
#endif

	Shoot(0);

	// Viewpunch
	int iskin = (int)((SUPERGUN_MAX_CLIP - m_iClip) * 10 / SUPERGUN_MAX_CLIP);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usSG3, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, iskin, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1, 4);
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);
}

void CSuperGun::SecondaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	// enflamme le gaz

	if (m_pPlayer->IsInGaz() == TRUE)
		m_pPlayer->m_bFireInGaz = TRUE;

	if (m_iClip < 1)
	{
		PlayEmptySound();
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	int iProjectiles = min(m_iClip, 8);	// n'en tire pas plus qu'il n'en a ...
	m_iClip -= iProjectiles;

#ifndef CLIENT_DLL
	// flash

	CSprite *pMuzzle = CSprite::SpriteCreate ( "sprites/blueflare1.spr", Vector(0,0,0), TRUE );
	pMuzzle->SetAttachment ( m_pPlayer->edict(), 1 );
	pMuzzle->SetScale ( 0.3 );
	pMuzzle->SetTransparency ( kRenderTransAdd, 128, 128, 220, 250, kRenderFxNone );
	pMuzzle->SetThink ( &CBaseEntity::SUB_Remove );
	pMuzzle->pev->nextthink = gpGlobals->time + 0.2;
#endif

	// Fire 2 
	PLAYBACK_EVENT(0, m_pPlayer->edict(), m_usSG2);

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector Point[8];

	Point[0] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 8 - gpGlobals->v_up * 8;
	Point[1] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 10 - gpGlobals->v_up * 9;
	Point[2] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 11 - gpGlobals->v_up * 11;
	Point[3] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 10 - gpGlobals->v_up * 13;
	Point[4] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 8 - gpGlobals->v_up * 14;
	Point[5] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 6 - gpGlobals->v_up * 13;
	Point[6] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 5 - gpGlobals->v_up * 11;
	Point[7] = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 6 - gpGlobals->v_up * 9;


	for (int i = 0; i<iProjectiles; i++)
	{

		CSGBall::CreateSGBall(Point[i],
			m_pPlayer->pev->v_angle, m_pPlayer->pev);
	}
#endif // CLIENT_DLL

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 3, 6);
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 31 / 20;
	m_flNextSecondaryAttack = GetNextAttackDelay(31 / 20);
}

void CSuperGun::Shoot(int mode)
{
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	CSGBall::CreateSGBall(
		m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15
		+ gpGlobals->v_right * 8
		- gpGlobals->v_up * 8,
		m_pPlayer->pev->v_angle, m_pPlayer->pev);
#endif
}

void CSuperGun::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(SG_IDLE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}






class CSuperGunAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_supergunammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_supergunammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther)
	{
		if (pOther->GiveAmmo(AMMO_SUPERGUNCLIPGIVE, "chewinggum", SUPERGUN_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_supergun, CSuperGunAmmo);