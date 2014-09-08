/*

Sprite class: represents a sprite to be rendered using GDI calls

To handle transparencies two images, the sprite image itself with transparent areas colored in black 
and a mask image having the transparent areas in white.

The class takes care to issue the appropriate GDI calls to bitblt the two images to get transparency:
as usual, sprites overlap whatever is on the backbuffer to begin with, so they have to be drawn from 
farthest away to closest

*/

#pragma once

#include <windows.h>
#include "Resources.h"

class Sprite
{
public:
	//Create from file on disk
	Sprite(LPTSTR ImageFileName, LPTSTR MaskFileName, HDC DestDC, HDC TempDC);
	//Create from resource
	Sprite(int ImageResID, int MaskResID, HDC DestDC, HDC TempDC);
	void Draw(UINT Xpos, UINT Ypos);
	~Sprite(void);

protected:
	//handles to DCs
	HDC m_hDestBuffer, m_hTempBuffer;
	//handles to bitmaps
	HBITMAP m_hImg, m_hMask;
	//bitmap info structures
	BITMAP m_ImgInfo, m_Maskinfo;
};
