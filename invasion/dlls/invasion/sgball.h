//-------------------------------------------------
//-												---
//-			sgball.h							---
//-												---
//-------------------------------------------------
//-------------------------------------------------
//			par Julien		-----------------------
//-------------------------------------------------

#ifndef SGBALL_H
#define SGBALL_H

class CSGBall : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void EXPORT AnimateThink(void);
	void EXPORT ExplodeTouch(CBaseEntity *pOther);

	int	Classify(void);
	static CSGBall *CreateSGBall(Vector vecOrigin, Vector vecAngles, entvars_s *pevOwner);

	int m_iSprite;
};

#endif // SGBALL_H