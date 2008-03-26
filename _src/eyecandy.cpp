
#include <string.h>
#include <math.h>

#include "global.h"

extern SDL_Rect rectSuperStompLeftSrc[8];
extern SDL_Rect rectSuperStompRightSrc[8];
extern SDL_Rect rectSuperStompLeftDst[8];
extern SDL_Rect rectSuperStompRightDst[8];

EC_StillImage::EC_StillImage(gfxSprite * nspr, short dstx, short dsty, short srcx, short srcy, short w, short h)
{
	spr = nspr;

	iSrcX = srcx;
	iSrcY = srcy;

	ix = dstx;
	iy = dsty;

	iw = w;
	ih = h;

	if(iw == 0)
		iw = (short)spr->getWidth();

	if(ih == 0)
		ih = (short)spr->getHeight();

}

void EC_StillImage::draw()
{
	if(!dead)
		spr->draw(ix, iy, iSrcX, iSrcY, iw, ih);	
}


//------------------------------------------------------------------------------
// base class animated
//------------------------------------------------------------------------------
EC_Animated::EC_Animated(gfxSprite * nspr, short dstx, short dsty, short srcx, short srcy, short w, short h, short speed, short frames)
{
	spr = nspr;

	iAnimationX = srcx;
	iAnimationY = srcy;
	iAnimationW = w;
	iAnimationH = h;

	if(iAnimationW == 0)
		iAnimationW = (short)spr->getWidth();

	if(iAnimationH == 0)
		iAnimationH = (short)spr->getHeight();

	iAnimationSpeed = speed;
	iAnimationFrames = frames * iAnimationW + iAnimationX;

	iAnimationTimer = 0;
	iAnimationFrame = iAnimationX;

	ix = dstx;
	iy = dsty;
}

void EC_Animated::animate()
{
	if(iAnimationSpeed > 0 && ++iAnimationTimer >= iAnimationSpeed)
	{
		iAnimationTimer = 0;
		iAnimationFrame += iAnimationW;

		if(iAnimationFrame >= iAnimationFrames)
		{
			iAnimationFrame = iAnimationX;
		}
	}
}

void EC_Animated::update()
{
	animate();
}

void EC_Animated::draw()
{
	if(!dead)
		spr->draw(ix, iy, iAnimationFrame, iAnimationY, iAnimationW, iAnimationH);
}

//------------------------------------------------------------------------------
// base class OscillatingAnimation
//------------------------------------------------------------------------------

EC_OscillatingAnimation::EC_OscillatingAnimation(gfxSprite * nspr, short dstx, short dsty, short srcx, short srcy, short w, short h, short speed, short frames) :
	EC_Animated(nspr, dstx, dsty, srcx, srcy, w, h, speed, frames)
{
	fForward = true;
}

void EC_OscillatingAnimation::animate()
{
	if(iAnimationSpeed > 0 && ++iAnimationTimer >= iAnimationSpeed)
	{
		iAnimationTimer = 0;

		if(fForward)
		{
			iAnimationFrame += iAnimationW;

			if(iAnimationFrame >= iAnimationFrames - iAnimationW)
				fForward = false;
		}
		else
		{
			iAnimationFrame -= iAnimationW;

			if(iAnimationFrame <= 0)
				fForward = true;
		}
	}
}

//------------------------------------------------------------------------------
// class cloud
//------------------------------------------------------------------------------
EC_Cloud::EC_Cloud(gfxSprite *nspr, float nx, float ny, float nvelx, short srcx, short srcy, short w, short h) :
	EC_StillImage(nspr, (short)nx, (short)ny, srcx, srcy, w, h)
{
	dx = nx;
	dy = ny;
	velx = nvelx;
}

void EC_Cloud::update()
{
	dx += velx;
	
	if(dx > 640.0f)
		dx -= 640.0f;
	else if(dx < 0.0f)
		dx += 640.0f;

	ix = (short)dx;	
}


//------------------------------------------------------------------------------
// class ghost
//------------------------------------------------------------------------------
EC_Ghost::EC_Ghost(gfxSprite *nspr, float nx, float ny, float nvelx, short ianimationspeed, short inumframes, short srcx, short srcy, short w, short h) :
	EC_Animated(nspr, (short)nx, (short)ny, srcx, srcy, w, h, ianimationspeed, inumframes)
{
	dx = nx;
	dy = ny;
	velx = nvelx;
}

void EC_Ghost::update()
{
	animate();

	dx += velx;

	if(dx >= 640.0f)
		dx -= 640.0f;
	else if(dx < 0.0f)
		dx += 640.0f;

	ix = (short)dx;
	iy = (short)dy;
}

//------------------------------------------------------------------------------
// class leaf
//------------------------------------------------------------------------------
EC_Leaf::EC_Leaf(gfxSprite *nspr, float nx, float ny) :
	EC_OscillatingAnimation(nspr, (short)nx, (short)ny, 0, 0, 16, 16, 16, 4)
{
	dx = nx;
	dy = ny;

	//Create random drag for each leaf to give the effect some variance
	NextLeaf();
}

void EC_Leaf::update()
{
	animate();

	dx += velx + game_values.gamewindx;
	dy += vely + game_values.gamewindy;

	if(dx >= 640.0f)
		dx -= 640.0f;
	else if(dx < 0.0f)
		dx += 640.0f;

	if(vely > 0.0f && iy >= 480)
	{
		dy = -16.0f;
		dx = (float)(rand() % 640);

		NextLeaf();
	}
	else if(vely < 0.0f && iy < -16)
	{
		dy = 480.0f;
		dx = (float)(rand() % 640);

		NextLeaf();
	}

	if(velx > 0.0f && ix >= 640)
	{
		dx = -16.0f;
		dy = (float)(rand() % 480);

		NextLeaf();
	}
	else if(velx < 0.0f && ix < -16)
	{
		dx = 640.0f;
		dy = (float)(rand() % 480);

		NextLeaf();
	}

	ix = (short)dx;
	iy = (short)dy;
}

void EC_Leaf::NextLeaf()
{
	short iRand = rand() % 20;
	if(iRand < 12)
		iAnimationY = 0;
	else if(iRand < 15)
		iAnimationY = 16;
	else if(iRand < 18)
		iAnimationY = 32;
	else
		iAnimationY = 48;

	velx = (float)(rand() % 9) / 4.0f;
	vely = (float)(rand() % 9) / 4.0f + 1.0f;

	fForward = (rand() % 2) == 0;
	iAnimationFrame = ((rand() % 3) + (fForward ? 0 : 1)) * iAnimationW;
	iAnimationTimer = rand() % 16;
}

//------------------------------------------------------------------------------
// class snow
//------------------------------------------------------------------------------
EC_Snow::EC_Snow(gfxSprite *nspr, float nx, float ny) :
	EC_StillImage(nspr, (short)nx, (short)ny, (rand() % 2) << 4, 0, 16, 16)
{
	dx = nx;
	dy = ny;

	velx = (float)(rand() % 9) / 4.0f;
	vely = (float)(rand() % 9) / 4.0f + 1.0f;
}

void EC_Snow::update()
{
	dx += velx + game_values.gamewindx;
	dy += vely + game_values.gamewindy;

	if(dx >= 640.0f)
		dx -= 640.0f;
	else if(dx < 0.0f)
		dx += 640.0f;

	if(vely > 0.0f && iy >= 480)
	{
		dy = -16.0f;
		dx = (float)(rand() % 640);
	}
	else if(vely < 0.0f && iy < -16)
	{
		dy = 480.0f;
		dx = (float)(rand() % 640);
	}

	if(velx > 0.0f && ix >= 640)
	{
		dx = -16.0f;
		dy = (float)(rand() % 480);
	}
	else if(velx < 0.0f && ix < -16)
	{
		dx = 640.0f;
		dy = (float)(rand() % 480);
	}

	ix = (short)dx;
	iy = (short)dy;
}


//------------------------------------------------------------------------------
// class corpse
//------------------------------------------------------------------------------
EC_Corpse::EC_Corpse(gfxSprite *nspr, float nx, float ny, short iSrcOffsetX) :
	EC_StillImage(nspr, (short)nx, (short)ny, iSrcOffsetX, 0, 32, 32)
{
	dx = nx;
	dy = ny;
	vely = GRAVITATION;
	timeleft = CORPSESTAY;
	offsetx = iSrcOffsetX;

	if(ix + PWOFFSET < 0)
		tx = (ix + 640 + PWOFFSET) / TILESIZE;
	else
		tx = (ix + PWOFFSET) / TILESIZE;

	if(ix + PWOFFSET + PW >= 640)
		tx2 = (ix + PWOFFSET + PW - 640) / TILESIZE;
	else
		tx2 = (ix + PWOFFSET + PW) / TILESIZE;
}

void EC_Corpse::update()
{
	if(vely != 0.0f)
	{
		short nexty = (short)(dy + 32.0f + vely);

		if(nexty >= 480)
		{
			dead = true;
			return;
		}

		if(nexty >= 0)
		{
			short ty = nexty / TILESIZE;
			
			if(g_map.map(tx, ty) == tile_solid_on_top || g_map.map(tx2, ty) == tile_solid_on_top)
			{	//on ground on tile solid_on_top
				if((dy + 32.0f - vely) / TILESIZE < ty)
				{	//only if we were above the tile in the previous frame
					dy = (float) (ty * TILESIZE - 32);
					vely = 0.0f;
					iy = (short)dy;

					return;
				}
			}

			IO_Block * leftblock = g_map.block(tx, ty);
			IO_Block * rightblock = g_map.block(tx2, ty);

			if((g_map.map(tx, ty) & 0x13) > 0 || (g_map.map(tx2, ty) & 0x13) > 0 ||
				(leftblock && !leftblock->isTransparent()) || (rightblock && !rightblock->isTransparent()))
			{	//on ground
				dy = (float) (ty * TILESIZE - 32);
				vely = 0.0f;
				iy = (short)dy;

				return;
			}
		}
		
		//falling (in air)
		dy += vely;
		vely = CapFallingVelocity(GRAVITATION + vely);
	}
	else
	{
		if(timeleft > 0)
			timeleft--;
		else
			dead = true;
	}

	iy = (short)dy;
}



//------------------------------------------------------------------------------
// class EC_GravText
//------------------------------------------------------------------------------

EC_GravText::EC_GravText(gfxFont *nfont, short nx, short ny, const char *ntext, float nvely) :
	CEyecandy()
{
	font = nfont;
	x = (float)(nx - (font->getWidth(ntext) / 2));
	y = (float)ny;
	w = (short)font->getWidth(ntext);
	
	text = new char[strlen(ntext)+1];	//someone should test if we got the memory...
	
	if(text)
		strcpy(text, ntext);
	else
		dead = true;

	vely = nvely;
}


void EC_GravText::update()
{
	y += vely;
	vely += GRAVITATION;

	if(y > 480)
		dead = true;
}


void EC_GravText::draw()
{
	font->draw((short)x, (short)y, text);

	if(x < 0)
		font->draw((short)x + 640, (short)y, text);
	else if(x + w > 639)
		font->draw((short)x - 640, (short)y, text);
	
}


EC_GravText::~EC_GravText()
{
	delete [] text;
	text = NULL;
}


//------------------------------------------------------------------------------
// class EC_FallingObject
//------------------------------------------------------------------------------

EC_FallingObject::EC_FallingObject(gfxSprite *nspr, short x, short y, float nvelx, float nvely, short animationframes, short animationspeed, short srcOffsetX, short srcOffsetY, short w, short h) :
	EC_Animated(nspr, x, y, srcOffsetX, srcOffsetY, w, h, animationspeed, animationframes)
{
	fx = (float)x;
	fy = (float)y;

	velx = nvelx;
	vely = nvely;
}

void EC_FallingObject::update()
{
	animate();

	fy += vely;
	fx += velx;
	
	ix = (short)fx;
	iy = (short)fy;

	if(fy >= 480.0f)
	{
		dead = true;
		return;
	}

	vely += GRAVITATION;
}


//------------------------------------------------------------------------------
// class EC_SingleAnimation
//------------------------------------------------------------------------------

EC_SingleAnimation::EC_SingleAnimation(gfxSprite *nspr, short x, short y, short iframes, short irate) :
	EC_Animated(nspr, x, y, 0, 0, (short)nspr->getWidth() / iframes, 0, irate, iframes)
{}

EC_SingleAnimation::EC_SingleAnimation(gfxSprite *nspr, short x, short y, short iframes, short irate, short offsetx, short offsety, short w, short h) :
	EC_Animated(nspr, x, y, offsetx, offsety, w, h, irate, iframes)
{}

void EC_SingleAnimation::update()
{
	animate();

	//This means that the animation wrapped and it is time to kill it
	if(iAnimationTimer == 0 && iAnimationFrame == iAnimationX)
		dead = true;
}

//------------------------------------------------------------------------------
// class EC_LoopingAnimation
//------------------------------------------------------------------------------

EC_LoopingAnimation::EC_LoopingAnimation(gfxSprite *nspr, short x, short y, short iframes, short irate, short loops, short ioffsetx, short ioffsety, short w, short h) :
	EC_Animated(nspr, x, y, ioffsetx, ioffsety, w, h, irate, iframes)
{
	iCountLoops = 0;
	iLoops = loops;
}

void EC_LoopingAnimation::update()
{
	animate();

	if(iAnimationTimer == 0 && iAnimationFrame == iAnimationX)
	{
		if(iLoops > 0 && ++iCountLoops >= iLoops)
			dead = true;
	}
}

//------------------------------------------------------------------------------
// class award
//------------------------------------------------------------------------------
/*
EC_Award::EC_Award(gfxSprite *nspr, short ix, short iy, short destx, short desty, short iframes, short irate) :
	EC_LoopingAnimation(nspr, ix, iy, iframes, irate, 0)
{
	dx = (float)destx;
	dy = (float)desty;

	velx = (dx - x) / 100.0f;
	vely = (dy - y) / 100.0f;
}


void EC_Award::update()
{
	EC_LoopingAnimation::update();

	x += velx;
	y += vely;

	if( fabsf(dx - x) < fabsf(velx) && fabsf(dy - y) < fabsf(vely))
	{
		dead = true;
	}
}
*/


//------------------------------------------------------------------------------
// class EC_ExplodingAward
//------------------------------------------------------------------------------

EC_ExplodingAward::EC_ExplodingAward(gfxSprite *nspr, short nx, short ny, float nvelx, float nvely, short timetolive, short awardID) :
	CEyecandy()
{
	spr = nspr;
	x = (float)nx;
	y = (float)ny;
	vely = nvely;
	velx = nvelx;
	timer = 0;
	ttl = timetolive;
	
	w = 16;
	h = 16;

	id = awardID * w;
}


void EC_ExplodingAward::update()
{
	y += vely;
	x += velx;
	
	if(++timer > ttl)
	{
		dead = true;
		eyecandyfront.add(new EC_SingleAnimation(&spr_fireballexplosion, (short)x + (w >> 1) - 16, (short)y + (h >> 1) - 16, 3, 8));
	}
}


void EC_ExplodingAward::draw()
{
	spr->draw((short)x, (short)y, id, 0, w, h);
}

//------------------------------------------------------------------------------
// class EC_SwirlingAward
//------------------------------------------------------------------------------

EC_SwirlingAward::EC_SwirlingAward(gfxSprite *nspr, short nx, short ny, float nangle, float nradius, float nvel, short timetolive, short srcX, short srcY, short iw, short ih, short animationRate, short animationFrames) :
	CEyecandy()
{
	spr = nspr;
	x = nx;
	y = ny;
	vel = nvel;
	angle = nangle;
	radius = nradius;
	timer = 0;
	ttl = timetolive;

	w = iw;
	h = ih;

	iSrcX = srcX * w;
	iSrcY = srcY * h;

	iAnimationRate = animationRate;
	iAnimationFrames = animationFrames;
	iAnimationTimer = 0;
	iAnimationFrame = iSrcX;
	iAnimationEndFrame = iSrcX + iw * iAnimationFrames;
}


void EC_SwirlingAward::update()
{
	angle += vel;
	radius += 3.0f;
	
	if(++timer > ttl)
	{
		short awardx = x + (short)(radius * cos(angle)) + (w >> 1) - 16;
		short awardy = y + (short)(radius * sin(angle)) + (h >> 1) - 16;
		eyecandyfront.add(new EC_SingleAnimation(&spr_fireballexplosion, awardx, awardy, 3, 8));
		
		dead = true;
	}

	if(iAnimationRate > 0 && ++iAnimationTimer > iAnimationRate)
	{
		iAnimationTimer = 0;

		iAnimationFrame += w;
		if(iAnimationFrame >= iAnimationEndFrame)
			iAnimationFrame = iSrcX;
	}
}


void EC_SwirlingAward::draw()
{
	short awardx = x + (short)(radius * cos(angle));
	short awardy = y + (short)(radius * sin(angle));
		
	spr->draw(awardx, awardy, iAnimationFrame, iSrcY, w, h);
}


//------------------------------------------------------------------------------
// class EC_RocketAward
//------------------------------------------------------------------------------

EC_RocketAward::EC_RocketAward(gfxSprite *nspr, short nx, short ny, float nvelx, float nvely, short timetolive, short srcX, short srcY, short iw, short ih, short animationRate, short animationFrames) :
	CEyecandy()
{
	spr = nspr;
	x = (float)nx;
	y = (float)ny;
	velx = nvelx;
	vely = nvely;

	timer = 0;
	ttl = timetolive;
	
	w = iw;
	h = ih;

	iSrcX = srcX * w;
	iSrcY = srcY * h;

	iAnimationRate = animationRate;
	iAnimationFrames = animationFrames;
	iAnimationTimer = 0;
	iAnimationFrame = iSrcX;
	iAnimationEndFrame = iSrcX + iw * iAnimationFrames;
}


void EC_RocketAward::update()
{
	vely += 0.2f;

	y += vely;
	x += velx;

	if(++timer > ttl)
	{
		eyecandyfront.add(new EC_SingleAnimation(&spr_fireballexplosion, (short)x + (w >> 1) - 16, (short)y + (h >> 1) - 16, 3, 8));
		dead = true;
	}

	if(iAnimationRate > 0 && ++iAnimationTimer > iAnimationRate)
	{
		iAnimationTimer = 0;

		iAnimationFrame += w;
		if(iAnimationFrame >= iAnimationEndFrame)
			iAnimationFrame = iSrcX;
	}
}


void EC_RocketAward::draw()
{
	spr->draw((short)x, (short)y, iAnimationFrame, iSrcY, w, h);
}

//------------------------------------------------------------------------------
// class EC_FloatingAward
//------------------------------------------------------------------------------

EC_FloatingObject::EC_FloatingObject(gfxSprite *nspr, short nx, short ny, float nvelx, float nvely, short timetolive, short nsrcx, short nsrcy, short nwidth, short nheight) :
	CEyecandy()
{
	spr = nspr;
	x = (float)nx;
	y = (float)ny;
	velx = nvelx;
	vely = nvely;
	srcx = nsrcx;
	srcy = nsrcy;
	w = nwidth;
	h = nheight;

	timer = 0;
	ttl = timetolive;
}


void EC_FloatingObject::update()
{
	y += vely;
	x += velx;

	if(++timer > ttl)
	{
		eyecandyfront.add(new EC_SingleAnimation(&spr_fireballexplosion, (short)x + (w >> 1) - 16, (short)y + (h >> 1) - 16, 3, 8));
		dead = true;
	}
}


void EC_FloatingObject::draw()
{
	spr->draw((short)x, (short)y, srcx, srcy, w, h);
}

//------------------------------------------------------------------------------
// class EC_SoulsAward
//------------------------------------------------------------------------------

EC_SoulsAward::EC_SoulsAward(gfxSprite *nspr, gfxSprite *nspr2, short nx, short ny, short timetolive, float nSpeed, short nSouls, short * nSoulArray) :
	CEyecandy()
{
	spr = nspr;
	spawnspr = nspr2;

	x = nx;
	y = ny;
	
	ttl = timetolive;
	numSouls = nSouls;

	if(numSouls > MAXAWARDS)
		numSouls = MAXAWARDS;

	id = new short[numSouls];

	for(short k = 0; k < numSouls; k++)
		id[k] = nSoulArray[k];

	timer = 0;
	speed = nSpeed;
	count = 0;

	if(numSouls < 1)
		dead = true;

	animationCount = 0;
	frame = 0;
	endmode = false;

	w = (short)nspr2->getWidth() / 7;
	h = (short)nspr2->getHeight();

}

EC_SoulsAward::~EC_SoulsAward()
{
	delete [] id;
}

void EC_SoulsAward::update()
{
	if(!endmode && ++timer > speed)
	{
		timer = 0;

		float addangle = QUARTER_PI / 20.0f;
		float startangle = -HALF_PI;
	
		float angle = (float)(rand()%21 - 10) * addangle + startangle;
		float velx = speed * cos(angle);
		float vely = speed * sin(angle);

		eyecandyfront.add(new EC_RocketAward(&spr_awardsouls, x - 8, y - 8, velx, vely, ttl, id[count], 0, 16, 16));
		
		if(++count >= numSouls)
		{
			endmode = true;
			frame = 4;
		}
	}

	if(++animationCount > 6)
	{
		animationCount = 0;

		frame++;

		if(!endmode && frame >= 4)
			frame = 0;
		else if(endmode && frame >= 7)
			dead = true; 
	}
}


void EC_SoulsAward::draw()
{
	spawnspr->draw(x - 16, y - 16, frame * w, 0, w, h);
}

//------------------------------------------------------------------------------
// class EC_DoorFront
//------------------------------------------------------------------------------

EC_Door::EC_Door(gfxSprite *nspr, gfxSprite *nmariospr, short nx, short ny, short irate, short xOffset, short iColor) :
	CEyecandy()
{
	spr = nspr;
	mariospr = nmariospr;

	x = nx;
	y = ny;
	timer = 0;
	rate = irate;

	iw = 32;
	ih = 32;

	offsety = ih;
	state = 0;
	frame = 0;

	offsetx = xOffset;
	colorOffset = iColor << 5; //select which color door to use
}


void EC_Door::update()
{
	if(++timer > rate)
	{
		timer = 0;

		if(state == 0)
		{
			offsety -= 3;

			if(offsety <= 0)
			{
				state = 1;
				offsety = 0;
			}

			frame = 1;	
		}
		else if(state == 1)
		{
			if(++frame >= 9)
			{
				state = 2;
				frame = 9;
			}
		}
		else if(state == 2)
		{
			if(--frame <= 1)
			{
				state = 3;
				frame = 1;
			}
		}
		else if(state == 3)
		{
			offsety += 3;

			if(offsety >= 32)
			{
				dead = true;
				offsety = 32;
			}

			frame = 1;	
		}
	}
}

void EC_Door::draw()
{
	spr->draw(x, y + offsety, 0, colorOffset, iw, ih - offsety);

	if(state == 1)
		mariospr->draw(x + 16 - HALFPW - PWOFFSET, y + 16 - HALFPH - PHOFFSET + offsety, offsetx, 0, iw, ih - offsety);

	spr->draw(x, y + offsety, frame * iw, colorOffset, iw, ih - offsety);
}

//------------------------------------------------------------------------------
// class EC_BossPeeker
//------------------------------------------------------------------------------
/*
EC_BossPeeker::EC_BossPeeker(gfxSprite *nspr, short speed, short bossType) :
	CEyecandy()
{
	spr = nspr;
	iSpeed = speed;
	iBossColorOffsetY = bossType * 64;

	timer = 0;
	state = 0;
	ix = 592;
	iy = 480;

	game_values.bosspeeking = bossType;
}


void EC_BossPeeker::update()
{
	if(state == 0)
	{
		iy -= 2;
		if(iy <= 432)
		{
			iy = 432;
			state = 1;
		}
	}
	else if(state == 1)
	{
		if(++timer == iSpeed)
			state = 2;
	}
	else if(state == 2)
	{
		iy += 2;
		if(iy >= 480)
		{
			dead = true;
			game_values.bosspeeking = -1;
		}
	}
}


void EC_BossPeeker::draw()
{
	spr->draw(ix, iy, 192, iBossColorOffsetY, 48, 64);
}
*/

//------------------------------------------------------------------------------
// class EC_SuperStompExplosion
//------------------------------------------------------------------------------
EC_SuperStompExplosion::EC_SuperStompExplosion(gfxSprite *nspr, short x, short y, short irate)
{
	spr = nspr;
	iRate = irate;
	ix = x;
	iy = y;
	
	iAnimationFrame = 0;
	iAnimationTimer = 0;
}

void EC_SuperStompExplosion::update()
{
	if(++iAnimationTimer >= iRate)
	{
		iAnimationTimer = 0;

		if(++iAnimationFrame > 7)
		{
			dead = true;
		}
	}
}

void EC_SuperStompExplosion::draw()
{
	if(dead)
		return;

	static const SDL_Rect rectSuperStompLeftSrc[8] = {{0,0,48,40},{0,40,54,42},{0,82,48,58},{0,140,42,70},{0,210,44,78},{108,0,44,82},{96,82,42,76},{88,158,34,74}};
	static const SDL_Rect rectSuperStompRightSrc[8] = {{48,0,48,40},{54,40,54,42},{48,82,48,58},{42,140,42,70},{44,210,44,78},{152,0,44,82},{138,82,42,76},{122,158,34,74}};

	static const SDL_Rect rectSuperStompLeftDst[8] = {{-48,-40,48,40},{-56,-42,54,42},{-66,-58,48,58},{-68,-70,42,70},{-72,-78,44,78},{-74,-82,44,82},{-74,-76,42,76},{-70,-74,34,74}};
	static const SDL_Rect rectSuperStompRightDst[8] = {{0,-40,48,40},{2,-42,54,42},{18,-58,48,58},{26,-70,42,70},{28,-78,44,78},{30,-82,44,82},{32,-76,42,76},{36,-74,34,74}};

	spr->draw(rectSuperStompLeftDst[iAnimationFrame].x + ix, rectSuperStompLeftDst[iAnimationFrame].y + iy, rectSuperStompLeftSrc[iAnimationFrame].x, rectSuperStompLeftSrc[iAnimationFrame].y, rectSuperStompLeftSrc[iAnimationFrame].w, rectSuperStompLeftSrc[iAnimationFrame].h);
	spr->draw(rectSuperStompRightDst[iAnimationFrame].x + ix, rectSuperStompRightDst[iAnimationFrame].y + iy, rectSuperStompRightSrc[iAnimationFrame].x, rectSuperStompRightSrc[iAnimationFrame].y, rectSuperStompRightSrc[iAnimationFrame].w, rectSuperStompRightSrc[iAnimationFrame].h);
}

//------------------------------------------------------------------------------
// class eyecandy_container
//------------------------------------------------------------------------------
CEyecandyContainer::CEyecandyContainer()
{
	for(short i = 0; i < MAXEYECANDY; i++)
		list[i] = NULL;

	list_end = 0;
}


CEyecandyContainer::~CEyecandyContainer()
{
	clean();
}


void CEyecandyContainer::clean()
{
	for(short i = 0; i < list_end; i++)
	{
		delete list[i];
		list[i] = NULL;
	}
	list_end = 0;
}


short CEyecandyContainer::add(CEyecandy *ec)
{
	if(list_end < MAXEYECANDY)
	{
		list[list_end] = ec;
		ec->dead = false;
		list_end++;

		return list_end - 1;
	}
	else
	{
		delete ec;	//otherwise memory leak!
		//printf("eyecandy list full!\n");
	}

	return -1;
}


void CEyecandyContainer::remove(short i)
{	
	delete list[i];
	list_end--;

	if(i != list_end)
	{		//if we didn't remove the last element we move the last elemnt in the new free place in the eyecandy list
		list[i] = list[list_end];
	}
}

void CEyecandyContainer::cleandeadobjects()
{
	for(short i = 0; i < list_end; i++)
	{
		if(list[i]->dead)
		{
			delete list[i];
			list_end--;

			if(i != list_end)
			{
				list[i] = list[list_end];
			}

			i--;
		}
	}
}

