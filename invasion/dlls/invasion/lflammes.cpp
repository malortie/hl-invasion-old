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
#include "gamerules.h"
#ifndef CLIENT_DLL
#include "flamme.h"
#endif

enum lflammes_e
{
	LFLAMMES_IDLE = 0,
	LFLAMMES_OPEN,
	LFLAMMES_CLOSE,
	LFLAMMES_FIRE,
	LFLAMMES_LONGIDLE,
	LFLAMMES_DEPLOY,
};



LINK_ENTITY_TO_CLASS(weapon_lflammes, CLFlammes);

void CLFlammes::Spawn()
{
	Precache();
	m_iId = WEAPON_LFLAMMES;
	SET_MODEL(ENT(pev), "models/w_lflammes.mdl");

	m_iDefaultAmmo = LFLAMMES_DEFAULT_GIVE;
	m_flAttackReady = 0;
	m_flSoundStartTime = 0;

	FallInit();// get ready to fall down.
}


void CLFlammes::Precache(void)
{
	UTIL_PrecacheOther("monster_flamme");

	PRECACHE_MODEL("models/v_lflammes.mdl");
	PRECACHE_MODEL("models/w_lflammes.mdl");
	PRECACHE_MODEL("models/p_357.mdl");
	PRECACHE_MODEL("models/w_lflammesclip.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("garg/gar_flamerun1.wav");

	m_usLFlammes = PRECACHE_EVENT(1, "events/lflammes.sc");
}

int CLFlammes::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		m_pPlayer->TextAmmo(TA_LFLAMMES);

		return TRUE;
	}
	return FALSE;
}

int CLFlammes::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "oeufs";
	p->iMaxAmmo1 = LFLAMMES_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_LFLAMMES;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_NOAUTORELOAD;
	p->iWeight = LFLAMMES_WEIGHT;

	return 1;
}


BOOL CLFlammes::Deploy()
{
	BOOL bResult = DefaultDeploy("models/v_lflammes.mdl", "models/p_357.mdl", LFLAMMES_DEPLOY, "lflammes");

	if (bResult)
	{
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.9;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}

	return bResult;
}

void CLFlammes::Holster(int skiplocal /* = 0 */)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10 + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 5);
}


void CLFlammes::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
		return;
	}

	// enflamme le gaz

	if (m_pPlayer->IsInGaz() == TRUE)
		m_pPlayer->m_bFireInGaz = TRUE;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif


	if (m_flAttackReady == 0)
	{
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usLFlammes, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 1, 0, 0, 0);

		// SendWeaponAnim(LFLAMMES_OPEN);
		m_flAttackReady = gpGlobals->time + 0.25;
	}

	// lancement de l'anim shoot

	else if (m_flAttackReady < gpGlobals->time && m_flAttackReady != -1)
	{
		m_flAttackReady = -1;
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usLFlammes, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 2, 0, 0, 0);
		// SendWeaponAnim(LFLAMMES_FIRE);

		// son
		m_flSoundStartTime = gpGlobals->time;
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "garg/gar_flamerun1.wav", 0.2, ATTN_NORM, 0, 100);
	}
	// tir

	if (m_flAttackReady == -1)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

#ifndef CLIENT_DLL
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);

		CFlamme *pFlamme = CFlamme::CreateFlamme(
			m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 15 + gpGlobals->v_right * 7 - gpGlobals->v_up * 6,
			m_pPlayer->pev->v_angle
			);

		pFlamme->pev->velocity = pFlamme->pev->velocity + m_pPlayer->pev->velocity;
#endif

		// son
		float delta = gpGlobals->time - m_flSoundStartTime;
		float flVol = delta < 1 ? 0.2 + delta*0.6 : 0.8;

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "garg/gar_flamerun1.wav", flVol, ATTN_NORM, SND_CHANGE_VOL, 100);

	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.02);
}

void CLFlammes::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flAttackReady == -1)
	{
		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usLFlammes, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0);
		// SendWeaponAnim(LFLAMMES_CLOSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		m_flAttackReady = 0;

		// son
		STOP_SOUND(edict(), CHAN_WEAPON, "garg/gar_flamerun1.wav");
	}
	else
	{
		m_flAttackReady = 0;

		int iAnim;
		switch (RANDOM_LONG(0, 1))
		{
		case 1:
		default:
			iAnim = LFLAMMES_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 5, 10);
			break;
		case 0:
			iAnim = LFLAMMES_LONGIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 12);
			break;
		}

		SendWeaponAnim(iAnim);
	}
}

class CLFlammesAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_lflammesclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_lflammesclip.mdl");
		PRECACHE_SOUND("debris/flesh6.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther)
	{
		if (pOther->IsPlayer() == false)
			return FALSE;

		CBasePlayer* pPlayer = (CBasePlayer*)pOther;
		if (!pPlayer || !(pPlayer->pev->weapons & (1 << WEAPON_LFLAMMES)))
			return FALSE;

		if (pOther->GiveAmmo(AMMO_LFLAMMESCLIPGIVE, "oeufs", LFLAMMES_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "debris/flesh6.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_lflammes, CLFlammesAmmo);