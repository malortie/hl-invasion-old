/**************************************************************
*																*
*																*
*			= = == Rpg.h == = =									*
*																*
****************************************************************/

//==================
// le fichier rpg.h est necessaire pour pouvoir l inclure 
// dans le rpggrunt.cpp
//

#ifndef RPG_H
#define RPG_H


// viseur

enum
{
	RPG_CROSSHAIR_NORMAL = 0,
	RPG_CROSSHAIR_EMPTY,
	RPG_CROSSHAIR_PROCESS,
	RPG_CROSSHAIR_LOCKED,
};

enum
{
	RPG_TEXT_TOUCHE = 4,
	RPG_TEXT_MANQUE,
};

// menu

#define RPG_MENU_ROCKET_SELECTED		( 1 << 0 )
#define RPG_MENU_ROCKET_EMPTY			( 1 << 1 )
#define RPG_MENU_ELECTRO_SELECTED		( 1 << 2 )
#define RPG_MENU_ELECTRO_EMPTY			( 1 << 3 )
#define RPG_MENU_NUCLEAR_SELECTED		( 1 << 4 )
#define RPG_MENU_NUCLEAR_EMPTY			( 1 << 5 )
#define RPG_MENU_ACTIVE					( 1 << 6 )
#define RPG_CLOSE						( 1 << 7 )
#define RPG_NEUTRE						( 1 << 8 )

// munitions

enum
{
	AMMO_ROCKET = 0,
	AMMO_ELECTRO,
	AMMO_NUCLEAR,
};

#define RPG_MAX_AMMO					5


// submodels

#define RPG_WEAPON_EMPTY				0
#define RPG_WEAPON_ROCKET				1
#define RPG_WEAPON_ELECTRO				2
#define RPG_WEAPON_NUCLEAR				3

#if 1
#define RPG_BODYGROUP_NADE				1
#endif

#endif		// RPG_H
