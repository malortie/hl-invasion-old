//-------------------------------------------------
//-												---
//-			sgball.cpp							---
//-												---
//-------------------------------------------------
//			par Julien		-----------------------
//-------------------------------------------------
//- code des projectiles pour la mitrailleuse laser.
//-------------------------------------------------


//----------------------------------------
// inclusions

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"player.h"
#include	"effects.h"
#include	"weapons.h"
#include	"sgball.h"

extern int gmsgClientDecal;

LINK_ENTITY_TO_CLASS(sg_ball, CSGBall);

CSGBall *CSGBall::CreateSGBall(Vector vecOrigin, Vector vecAngles, entvars_s *pevOwner)
{
	CSGBall *pBall = GetClassPtr((CSGBall *)NULL);

	UTIL_MakeAimVectors(vecAngles);

	float x, y, z;
	do
	{
		x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		z = x*x + y*y;
	} while (z > 1);

	Vector vecDir = gpGlobals->v_forward +
		x * VECTOR_CONE_6DEGREES.x * gpGlobals->v_right +
		y * VECTOR_CONE_6DEGREES.y * gpGlobals->v_up;

	pBall->pev->angles = UTIL_VecToAngles(vecDir.Normalize());

	UTIL_SetOrigin(pBall->pev, vecOrigin);

	pBall->pev->owner = ENT(pevOwner);

	pBall->Spawn();
	pBall->SetTouch(&CSGBall::ExplodeTouch);

	return pBall;
}

void CSGBall::Spawn(void)
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->classname = MAKE_STRING("sg_ball");

	SET_MODEL(ENT(pev), "sprites/cnt1.spr");
	pev->rendermode = kRenderTransAdd;
	pev->rendercolor.x = 255;
	pev->rendercolor.y = 255;
	pev->rendercolor.z = 255;
	pev->renderamt = 190;
	pev->scale = 0.1;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	SetThink( &CSGBall::AnimateThink);
	SetTouch( &CSGBall::ExplodeTouch );

	pev->dmgtime = gpGlobals->time; // keep track of when ball spawned
	pev->nextthink = gpGlobals->time + 0.1;

	UTIL_MakeVectors(pev->angles);
	pev->velocity = gpGlobals->v_forward * 1700;

	m_flFieldOfView = -1;
	m_hEnemy = NULL;
}

int CSGBall::Classify(void)
{
	return CLASS_PLAYER_BIOWEAPON;
}

void CSGBall::Precache(void)
{
	PRECACHE_MODEL("sprites/cnt1.spr");
	PRECACHE_MODEL("sprites/xspark1.spr");
	PRECACHE_MODEL("sprites/xspark4.spr");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");

	m_iSprite = PRECACHE_MODEL("sprites/cnt1.spr");
}


void CSGBall::AnimateThink(void)
{
	pev->nextthink = gpGlobals->time + 0.05;

	// sprite

	pev->frame = ((int)pev->frame + 1) % 5;

	float delta = gpGlobals->time - pev->dmgtime;

	if (delta > 5 || pev->velocity.Length() < 10)
	{
		SetTouch(NULL);
		UTIL_Remove(this);
	}

	// trainée

	if (delta > 0 && delta < 0.9)
	{
		CSprite *pTrail = CSprite::SpriteCreate("sprites/cnt1.spr", pev->origin, TRUE);
		//	pTrail->AnimateAndDie ( 7 );
		pTrail->AnimateAndDie(0.2);
		pTrail->SetScale(0.07);
		pTrail->SetTransparency(kRenderTransAdd, 230, 255, 230, 60, kRenderFxNone);
		pTrail->Expand(pTrail->pev->scale, 120);

	}

	// ennemi

	if (m_hEnemy == NULL)
	{
		Look(600);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy != NULL && FVisible(m_hEnemy))
	{
		Vector vecDirToEnemy = (m_hEnemy->BodyTarget(pev->origin) - pev->origin).Normalize();

		Vector angEnnemy = UTIL_VecToAngles(vecDirToEnemy);
		Vector angBall = UTIL_VecToAngles(pev->velocity);

		float difX = UTIL_AngleDiff(angEnnemy.x, angBall.x);
		float difY = UTIL_AngleDiff(angEnnemy.y, angBall.y);

		if (fabs(difX) < 15 && fabs(difY) < 15)
		{
			pev->velocity = vecDirToEnemy * pev->velocity.Length();
		}
	}
	else
	{
		m_hEnemy = NULL;
	}


}


void CSGBall::ExplodeTouch(CBaseEntity *pOther)
{
	// truc affreux pour creer une particule a travers ce message pour les decals

	TraceResult trace = UTIL_GetGlobalTrace();
	Vector vecNorm = trace.vecPlaneNormal.Normalize();

	MESSAGE_BEGIN(MSG_ALL, gmsgClientDecal);

	WRITE_COORD(trace.vecEndPos.x);			// xyz source
	WRITE_COORD(trace.vecEndPos.y);
	WRITE_COORD(trace.vecEndPos.z);
	WRITE_COORD(vecNorm.x);						// xyz norme
	WRITE_COORD(vecNorm.y);
	WRITE_COORD(vecNorm.z);
	WRITE_CHAR('a');						// type de texture
	WRITE_BYTE(5);						//  4 == electro-rocket

	MESSAGE_END();

	if (pOther->pev->takedamage)
	{
		if (pOther->pev != VARS(pev->owner))
		{
			TraceResult tr = UTIL_GetGlobalTrace();

			ClearMultiDamage();
			pOther->TraceAttack(pev, gSkillData.plrDmgSupergun, pev->velocity.Normalize(), &tr, DMG_ENERGYBEAM);
			ApplyMultiDamage(pev, pev);

			UTIL_EmitAmbientSound(ENT(pev), tr.vecEndPos, "weapons/electro4.wav", 0.3, ATTN_NORM, 0, RANDOM_LONG(90, 99));
		}
	}

	UTIL_Remove(this);
}
