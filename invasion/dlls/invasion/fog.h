#ifndef TRIGGER_FOG_H__
#define TRIGGER_FOG_H__

class CTriggerFog : public CPointEntity
{
public:

	void	Spawn		( void );
	void	Activate	( void );

	void	KeyValue	( KeyValueData *pkvd );
	void	Use			( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );


	virtual int	Save	( CSave &save );
	virtual int	Restore	( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_flminDist;
	float	m_flmaxDist;
	float	m_flFadeInTime;
	float	m_flFadeOutTime;

	int		m_bActive;
	short	m_fogdensity;

public:
	BOOL	IsActive() { return m_bActive; }
protected:
	void ApplySteamPipeFogDensityFix();
};

#endif // TRIGGER_FOG_H__