//-------------------------------------------------
//-												---
//-			briquet.cpp							---
//-												---
//-------------------------------------------------
//			par Julien		-----------------------
//-------------------------------------------------
//- code du briquet servant de lampe torche		---
//-------------------------------------------------



//----------------------------------------
// inclusions

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#ifndef CLIENT_DLL
extern int gmsgBriquetSwitch;
#endif // !CLIENT_DLL

enum briquet_e
{
	BRIQUET_IDLE = 0,
	BRIQUET_DRAW,
	BRIQUET_ALLUME_ESSAIE,
	BRIQUET_ALLUME,
	BRIQUET_ALLUME_IDLE,
	BRIQUET_ETEINT,
};

#define BRIQUET_IDLE_TIME					19 / 10.0
#define BRIQUET_DRAW_TIME					9 / 20.0
#define BRIQUET_ALLUME_ESSAIE_TIME			39 / 60.0
#define BRIQUET_ALLUME_TIME					9 / 2.0
#define BRIQUET_ALLUME_IDLE_TIME			14 / 30.0

#define BRIQUET_SPRITE						"sprites/briquet.spr"
#define BRIQUET_ETINCELLES_SPRITE			"sprites/richo1.spr"

LINK_ENTITY_TO_CLASS(weapon_briquet, CBriquet);

//----------------------------------------
// spawn / pr�cache

void CBriquet::Spawn()
{
	pev->classname = MAKE_STRING("weapon_briquet");
	Precache();
	m_iId = WEAPON_BRIQUET;
	SET_MODEL(ENT(pev), "models/w_briquet.mdl");
	m_iClip = -1;

	m_bTransition = FALSE;

	FallInit();// get ready to fall down.
}

void CBriquet::Precache(void)
{
	PRECACHE_MODEL("models/v_briquet.mdl");
	PRECACHE_MODEL("models/w_briquet.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_MODEL(BRIQUET_SPRITE);
	PRECACHE_MODEL(BRIQUET_ETINCELLES_SPRITE);

	PRECACHE_SOUND("items/9mmclip1.wav");
}


//----------------------------------------
// add / remove / bazar


int CBriquet::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iFlags = 0;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_BRIQUET;
	p->iWeight = BRIQUET_WEIGHT;

	return 1;
}

int CBriquet::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		//		m_pPlayer->TextAmmo( TA_BRIQUET );

		return TRUE;
	}
	return FALSE;
}

BOOL CBriquet::Deploy()
{
	BOOL bResult = DefaultDeploy("models/v_briquet.mdl", "models/p_crowbar.mdl", BRIQUET_DRAW, "briquet");

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + BRIQUET_DRAW_TIME;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;

	m_bActif = 0;

	return bResult;
}

void CBriquet::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10 + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 5);

#ifndef CLIENT_DLL
	// �teint la flamme
	MESSAGE_BEGIN(MSG_ONE, gmsgBriquetSwitch, NULL, m_pPlayer->pev);
	WRITE_BYTE(0);						// 0 == off, 1 == on
	MESSAGE_END();
#endif // !CLIENT_DLL
}

//----------------------------------------
// attaque



void CBriquet::PrimaryAttack()
{
	// sous l'eau
	if (m_pPlayer->pev->waterlevel >= 2)
	{
		if (m_bActif == 1)
		{
#ifndef CLIENT_DLL
			// �teint la flamme
			MESSAGE_BEGIN(MSG_ONE, gmsgBriquetSwitch, NULL, m_pPlayer->pev);
			WRITE_BYTE(0);						// 0 == off, 1 == on
			MESSAGE_END();
#endif // !CLIENT_DLL

			SendWeaponAnim(BRIQUET_ETEINT);

			m_bActif = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3;
		}
		else
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	// enflamme le gaz

	if (m_pPlayer->IsInGaz() == TRUE)
		m_pPlayer->m_bFireInGaz = TRUE;

	// d�j� allum�

	if (m_bActif == 1)
	{
#ifndef CLIENT_DLL
		// �teint la flamme
		MESSAGE_BEGIN(MSG_ONE, gmsgBriquetSwitch, NULL, m_pPlayer->pev);
		WRITE_BYTE(0);						// 0 == off, 1 == on
		MESSAGE_END();
#endif // ! CLIENT_DLL

		SendWeaponAnim(BRIQUET_ETEINT);

		m_bActif = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3;


		return;
	}

	// 33% de chances de l'allumer

	if (RANDOM_FLOAT(0, 1) < 0.33)
	{
		SendWeaponAnim(BRIQUET_ALLUME);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + BRIQUET_ALLUME_TIME;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;

		m_bActif = 1;

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgBriquetSwitch, NULL, m_pPlayer->pev);
		WRITE_BYTE(1);						// 0 == off, 1 == on
		MESSAGE_END();
#endif // !CLIENT_DLL
	}

	else
	{
		SendWeaponAnim(BRIQUET_ALLUME_ESSAIE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + BRIQUET_ALLUME_ESSAIE_TIME;

#ifndef CLIENT_DLL
		CSprite *pEtincelle = CSprite::SpriteCreate(BRIQUET_ETINCELLES_SPRITE, Vector(0, 0, 0), FALSE);

		pEtincelle->SetAttachment(m_pPlayer->edict(), 1);
		pEtincelle->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
		pEtincelle->SetScale(0.05);
		pEtincelle->SetThink(&CBaseEntity::SUB_Remove);
		pEtincelle->pev->nextthink = gpGlobals->time + 0.05;
#endif

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3;
	}


}


//----------------------------------------
// reload



void CBriquet::Reload(void)
{
	return;
}

//----------------------------------------
// weaponidle


void CBriquet::WeaponIdle(void)
{
	ResetEmptySound();

	// restoration de la flamme � la sauvegarde

	if (m_bTransition == TRUE)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgBriquetSwitch, NULL, m_pPlayer->pev);
		WRITE_BYTE(m_bActif == 1 ? 1 : 0);
		MESSAGE_END();
#endif // !CLIENT_DLL

		m_bTransition = FALSE;
	}

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

#ifndef CLIENT_DLL
	// lumi�re

	if (m_bActif == 1 && gpGlobals->time > m_flNextLight)
	{
		m_flNextLight = UTIL_WeaponTimeBase() + 0.15;

		Vector vecSrc = m_pPlayer->Center();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSrc);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD(vecSrc.x);	// X
		WRITE_COORD(vecSrc.y);	// Y
		WRITE_COORD(vecSrc.z);	// Z
		WRITE_BYTE(15 * RANDOM_FLOAT(0.8, 1.2));		// radius * 0.1
		WRITE_BYTE(255);		// r
		WRITE_BYTE(180);		// g
		WRITE_BYTE(96);		// b
		WRITE_BYTE(3);		// time * 10
		WRITE_BYTE(0);		// decay * 0.1
		MESSAGE_END();

	}
#endif // CLIENT_DLL

	// eau

	if (m_pPlayer->pev->waterlevel >= 2 && m_bActif == 1)
		PrimaryAttack();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;

	if (m_bActif == 1)
		iAnim = BRIQUET_ALLUME_IDLE;

	else
		iAnim = BRIQUET_IDLE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 5, 12);


	SendWeaponAnim(iAnim);
}