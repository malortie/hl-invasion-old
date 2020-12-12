/****************************************************************
*																*
*				hudzoom.cpp										*
*																*
*				par Julien										*
*																*
****************************************************************/

// code du viseur du fusil de snipe


#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"

void CHudSniper::DrawBlackOverlay(int x, int y, int w, int h, 
	int spriteWidth, int spriteHeight)
{
	// Do not draw whenever we have a null dimension.
	if (w == 0 || h == 0)
		return;

	SPR_Set(m_sprBlack, 255, 255, 255);

	m_wrcNoir.left = 0;
	m_wrcNoir.top = 0;
	m_wrcNoir.right = spriteWidth;

	// Assume there is enough room for the desired sprite height.
	m_wrcNoir.bottom = spriteHeight;

	int yoffset = 0;

	while (h > 0)
	{
		// Clamp the height of the area if the remaining height
		// is too small.
		if (h < m_wrcNoir.bottom)
			m_wrcNoir.bottom = h;

		// Assume there is enough room for the desired sprite width.
		m_wrcNoir.right = spriteWidth;

		int xoffset = 0;

		int w2 = w;
		while (w2 > 0)
		{
			// Clamp the width of the area if the remaining width
			// is too small.
			if (w2 < m_wrcNoir.right)
				m_wrcNoir.right = w2;

			// Draw the sprite.
			SPR_DrawHoles(0, x + xoffset, y + yoffset, &m_wrcNoir);

			// Subtract remaining width by the width of the sprite we draw.
			w2 -= m_wrcNoir.right;
			// Advance by the width of the sprite we draw.
			xoffset += m_wrcNoir.right;
		}

		// Subtract remaining height by the height of the sprite we draw.
		h -= m_wrcNoir.bottom;
		// Advance by the height of the sprite we draw.
		yoffset += m_wrcNoir.bottom;
	}
}

int CHudSniper :: Draw	( float flTime )
{
	if ( gHUD.m_iFOV == 90 || gHUD.m_iFOV == 0 )
		return 1;

	static const int CenterX = ScreenWidth / 2;
	static const int CenterY = ScreenHeight / 2;

	if ( ScreenWidth >= 1024 )
	{
		// ligne du haut

		SPR_Set( m_sprHG, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 128 - 256, CenterY -128 - 256, &m_wrc1024);	

		SPR_Set( m_sprH, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 128, CenterY - 128 - 256, &m_wrc1024);

		SPR_Set( m_sprHD, 255, 255, 255);
		SPR_DrawHoles(0, CenterX + 128, CenterY -128 - 256, &m_wrc1024);

		// ligne du milieu

		SPR_Set( m_sprG, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 128 - 256, CenterY -128, &m_wrc1024);	

		SPR_Set( m_sprViseur, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 128, CenterY -128, &m_wrc1024);

		SPR_Set( m_sprD, 255, 255, 255);
		SPR_DrawHoles(0, CenterX + 128, CenterY -128, &m_wrc1024);

		// ligne du bas 

		SPR_Set( m_sprBG, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 128 - 256, CenterY + 128, &m_wrc1024);	

		SPR_Set( m_sprB, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 128, CenterY + 128, &m_wrc1024);

		SPR_Set( m_sprBD, 255, 255, 255);
		SPR_DrawHoles(0, CenterX + 128, CenterY + 128, &m_wrc1024);

		int overlayWidth = CenterX - 128 - 256;
		int overlayHeight = ScreenHeight;

		// noir à gauche et à droite

		DrawBlackOverlay(0, 0, overlayWidth, overlayHeight, 256, 256);
		DrawBlackOverlay(CenterX + 128 + 256, 0, overlayWidth, overlayHeight, 256, 256);

		// noir en haut et en bas

		if ( CenterY - 128 - 256 > 0 )
		{
			overlayWidth = 3 * 256;
			overlayHeight = CenterY - 128 - 256;

			DrawBlackOverlay(CenterX - 128 - 256, 0, overlayWidth, overlayHeight, 256, 256);
			DrawBlackOverlay(CenterX - 128 - 256, CenterY + 128 + 256, overlayWidth, overlayHeight, 256, 256);
		}
	}


	else
	{

		// ligne du haut

		SPR_Set( m_sprHG, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 240, CenterY -240, &m_wrc640);	

		SPR_Set( m_sprHD, 255, 255, 255);
		SPR_DrawHoles(0, CenterX, CenterY -240, &m_wrc640);

		// ligne du bas 

		SPR_Set( m_sprBG, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 240, CenterY, &m_wrc640);	

		SPR_Set( m_sprBD, 255, 255, 255);
		SPR_DrawHoles(0, CenterX, CenterY, &m_wrc640);
		
		// viseur

		SPR_Set( m_sprViseur, 255, 255, 255);
		SPR_DrawHoles(0, CenterX - 16, CenterY - 16, &m_wrc640Viseur);


		// noir à gauche et à droite

		if ( CenterX - 240 > 0 )
		{
			int overlayWidth = CenterX - 240;
			int overlayHeight = ScreenHeight;

			DrawBlackOverlay(0, 0, overlayWidth, overlayHeight, 256, 256);
			DrawBlackOverlay(CenterX + 240, 0, overlayWidth, overlayHeight, 256, 256);
		}

		// noir en haut et en bas

		if ( CenterY - 240 > 0 )
		{
			int overlayWidth = 2 * 240;
			int overlayHeight = CenterY - 240;

			DrawBlackOverlay(CenterX - 240, 0, overlayWidth, overlayHeight, 256, 256);
			DrawBlackOverlay(CenterX - 240, CenterY + 240, overlayWidth, overlayHeight, 256, 256);
		}
	}


	return 1;
}




int CHudSniper :: Init( void )
{

	if ( ScreenWidth >= 1024 )
	{
		
		m_sprHG	= SPR_Load("sprites/fsniper/fsniper_1024_up_l.spr");
		m_sprH	= SPR_Load("sprites/fsniper/fsniper_1024_up.spr");
		m_sprHD = SPR_Load("sprites/fsniper/fsniper_1024_up_r.spr");

		m_sprBD = SPR_Load("sprites/fsniper/fsniper_1024_down_r.spr");
		m_sprB	= SPR_Load("sprites/fsniper/fsniper_1024_down.spr");
		m_sprBG = SPR_Load("sprites/fsniper/fsniper_1024_down_l.spr");

		m_sprG	= SPR_Load("sprites/fsniper/fsniper_1024_left.spr");
		m_sprD	= SPR_Load("sprites/fsniper/fsniper_1024_right.spr");

		m_sprViseur	= SPR_Load("sprites/fsniper/fsniper_1024_cross.spr");
		m_sprBlack	= SPR_Load("sprites/fsniper/fsniper_black.spr");


		m_wrc1024.left = 0;
		m_wrc1024.top = 0;
		m_wrc1024.right = 256;
		m_wrc1024.bottom = 256;

	}

	else /* if ( ScreenWidth <= 640 )*/
	{

		m_sprHG	= SPR_Load("sprites/fsniper/fsniper_640_up_l.spr");
		m_sprHD = SPR_Load("sprites/fsniper/fsniper_640_up_r.spr");
		m_sprBD = SPR_Load("sprites/fsniper/fsniper_640_down_r.spr");
		m_sprBG = SPR_Load("sprites/fsniper/fsniper_640_down_l.spr");

		m_sprViseur	= SPR_Load("sprites/fsniper/fsniper_640_cross.spr");
		m_sprBlack	= SPR_Load("sprites/fsniper/fsniper_black.spr");



		m_wrc640.left	= 0;
		m_wrc640.top	= 0;
		m_wrc640.right	= 240;
		m_wrc640.bottom = 240;

		m_wrc640Viseur.left		= 0;
		m_wrc640Viseur.top		= 0;
		m_wrc640Viseur.right	= 32;
		m_wrc640Viseur.bottom	= 32;
	}


	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
}



int CHudSniper :: VidInit( void )
{

	if ( ScreenWidth >= 1024 )
	{
		
		m_sprHG	= SPR_Load("sprites/fsniper/fsniper_1024_up_l.spr");
		m_sprH	= SPR_Load("sprites/fsniper/fsniper_1024_up.spr");
		m_sprHD = SPR_Load("sprites/fsniper/fsniper_1024_up_r.spr");

		m_sprBD = SPR_Load("sprites/fsniper/fsniper_1024_down_r.spr");
		m_sprB	= SPR_Load("sprites/fsniper/fsniper_1024_down.spr");
		m_sprBG = SPR_Load("sprites/fsniper/fsniper_1024_down_l.spr");

		m_sprG	= SPR_Load("sprites/fsniper/fsniper_1024_left.spr");
		m_sprD	= SPR_Load("sprites/fsniper/fsniper_1024_right.spr");

		m_sprViseur	= SPR_Load("sprites/fsniper/fsniper_1024_cross.spr");
		m_sprBlack	= SPR_Load("sprites/fsniper/fsniper_black.spr");


		m_wrc1024.left = 0;
		m_wrc1024.top = 0;
		m_wrc1024.right = 256;
		m_wrc1024.bottom = 256;

	}

	else /*if ( ScreenWidth <= 640 )*/
	{

		m_sprHG	= SPR_Load("sprites/fsniper/fsniper_640_up_l.spr");
		m_sprHD = SPR_Load("sprites/fsniper/fsniper_640_up_r.spr");
		m_sprBD = SPR_Load("sprites/fsniper/fsniper_640_down_r.spr");
		m_sprBG = SPR_Load("sprites/fsniper/fsniper_640_down_l.spr");

		m_sprViseur	= SPR_Load("sprites/fsniper/fsniper_640_cross.spr");
		m_sprBlack	= SPR_Load("sprites/fsniper/fsniper_black.spr");



		m_wrc640.left	= 0;
		m_wrc640.top	= 0;
		m_wrc640.right	= 240;
		m_wrc640.bottom = 240;

		m_wrc640Viseur.left		= 0;
		m_wrc640Viseur.top		= 0;
		m_wrc640Viseur.right	= 32;
		m_wrc640Viseur.bottom	= 32;
	}

	m_iFlags |= HUD_ACTIVE;

	return 1;
}
