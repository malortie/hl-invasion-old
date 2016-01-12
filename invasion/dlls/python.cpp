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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"


enum python_e {
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	/*	PYTHON_IDLE1 = 0,	//modif de Julien
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
	*/
	PYTHON_DRAW = 0,
	PYTHON_IDLE1,
	PYTHON_IDLE2,
	PYTHON_IDLE3,
	PYTHON_FIRE1,
	PYTHON_FIRE_EMPTY,
	PYTHON_RELOAD_EMPTY,
	PYTHON_RELOAD
#else
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
};

LINK_ENTITY_TO_CLASS(weapon_python, CPython);
LINK_ENTITY_TO_CLASS(weapon_357, CPython);

int CPython::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = _357_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = PYTHON_MAX_CLIP;
	p->iFlags = 0;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_PYTHON;
	p->iWeight = PYTHON_WEIGHT;

	return 1;
}

int CPython::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
		m_pPlayer->TextAmmo(TA_BERETTA);
#endif

		return TRUE;
	}
	return FALSE;
}

void CPython::Spawn()
{
	pev->classname = MAKE_STRING("weapon_357"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_PYTHON;
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	SET_MODEL(ENT(pev), "models/w_beretta.mdl");
#else
	SET_MODEL(ENT(pev), "models/w_357.mdl");
#endif

	m_iDefaultAmmo = PYTHON_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CPython::Precache(void)
{
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	PRECACHE_MODEL("models/v_beretta.mdl");
	PRECACHE_MODEL("models/w_beretta.mdl");
#else
	PRECACHE_MODEL("models/v_357.mdl");
	PRECACHE_MODEL("models/w_357.mdl");
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	PRECACHE_MODEL("models/p_357.mdl");

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	PRECACHE_MODEL("models/w_berettaclip.mdl");
#else
	PRECACHE_MODEL("models/w_357ammobox.mdl");
#endif
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/357_reload1.wav");
	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_SOUND("weapons/357_shot1.wav");
	PRECACHE_SOUND("weapons/357_shot2.wav");

	m_usFirePython = PRECACHE_EVENT(1, "events/python.sc");

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	m_iShell = PRECACHE_MODEL("models/beretta_shell.mdl");// brass shell
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}

BOOL CPython::Deploy()
{
#if !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// enable laser sight geometry.
		pev->body = 1;
	}
	else
	{
		pev->body = 0;
	}
#endif // !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	return DefaultDeploy("models/v_beretta.mdl", "models/p_357.mdl", PYTHON_DRAW, "python", UseDecrement(), pev->body);
#else
	return DefaultDeploy("models/v_357.mdl", "models/p_357.mdl", PYTHON_DRAW, "python", UseDecrement(), pev->body);
#endif
}


void CPython::Holster(int skiplocal /* = 0 */)
{
	m_fInReload = FALSE;// cancel any reload in progress.

#if !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )
	if (m_fInZoom)
	{
		SecondaryAttack();
	}
#endif // !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 5);
#else
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
	SendWeaponAnim(PYTHON_HOLSTER);
#endif
}

#if !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )
void CPython::SecondaryAttack(void)
{
#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
		return;
	}

	if (m_pPlayer->pev->fov != 0)
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}
	else if (m_pPlayer->pev->fov != 40)
	{
		m_fInZoom = TRUE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 40;
	}

	m_flNextSecondaryAttack = 0.5;
}
#endif // !defined ( HLINVASION_DLL ) && !defined ( HLINVASION_CLIENT_DLL )

void CPython::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (!m_fFireOnEmpty)
			Reload();
		else
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);


	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFirePython, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	m_flNextPrimaryAttack = 0.5;
#else
	m_flNextPrimaryAttack = 0.75;
#endif
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}


void CPython::Reload(void)
{
	if (m_pPlayer->ammo_357 <= 0)
		return;

	if (m_pPlayer->pev->fov != 0)
	{
		m_fInZoom = FALSE;
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;  // 0 means reset to default fov
	}

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	DefaultReload(6, PYTHON_RELOAD, 2.0, bUseScope);
}


void CPython::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
#if defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
	if (flRand <= 0.5)
	{
		iAnim = PYTHON_IDLE1;
		m_flTimeWeaponIdle = (21.0 / 3.0);
	}
	else if (flRand <= 0.7)
	{
		iAnim = PYTHON_IDLE2;
		m_flTimeWeaponIdle = (23.0 / 7.0);
	}
	else
	{
		iAnim = PYTHON_IDLE3;
		m_flTimeWeaponIdle = (21.0 / 2.0);
	}

	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0);
#else
	if (flRand <= 0.5)
	{
		iAnim = PYTHON_IDLE1;
		m_flTimeWeaponIdle = (70.0 / 30.0);
	}
	else if (flRand <= 0.7)
	{
		iAnim = PYTHON_IDLE2;
		m_flTimeWeaponIdle = (60.0 / 30.0);
	}
	else if (flRand <= 0.9)
	{
		iAnim = PYTHON_IDLE3;
		m_flTimeWeaponIdle = (88.0 / 30.0);
	}
	else
	{
		iAnim = PYTHON_FIDGET;
		m_flTimeWeaponIdle = (170.0 / 30.0);
	}

	int bUseScope = FALSE;
#ifdef CLIENT_DLL
	bUseScope = bIsMultiplayer();
#else
	bUseScope = g_pGameRules->IsMultiplayer();
#endif

	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0, bUseScope);
#endif // defined ( HLINVASION_DLL ) || defined ( HLINVASION_CLIENT_DLL )
}


class CPythonAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity *pOther)
	{
		if (pOther->GiveAmmo(AMMO_357BOX_GIVE, "357", _357_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS(ammo_357, CPythonAmmo);


#endif