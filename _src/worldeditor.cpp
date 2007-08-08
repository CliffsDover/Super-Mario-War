/*----------------------------------------------------------+
| Super Mario War World Editor								|
|															|
|															|
| this sourcecode is released under the GPL.				|
|															|
| start:		3.8.2007									|
| last changes:	3.8.2007									|
|															|
|		   � 2007 Florian Hufsky <florian.hufsky@gmail.com>	|
+----------------------------------------------------------*/

#define _SMW_EDITOR
#include "global.h"

#ifdef PNG_SAVE_FORMAT
	#include "savepng.h"
#endif

#include <string.h>
#include <ctype.h>

#ifdef _WIN32
	#include <windows.h>

	#ifndef _XBOX
		#pragma comment(linker, "/NODEFAULTLIB:libc.lib")

		#ifdef PNG_SAVE_FORMAT
			#pragma comment(lib, "libpng.lib")
			#pragma comment(lib, "zlib.lib")
		#endif
    #endif
#endif

#define MAPTITLESTRING "SMW 1.7 World Editor"

enum {EDITOR_EDIT, EDITOR_BACKGROUND, EDITOR_FOREGROUND, EDITOR_VEHICLES, EDITOR_QUIT, SAVE_AS, FIND, CLEAR_WORLD, NEW_WORLD, SAVE, EDITOR_WARP, DISPLAY_HELP};

char * szEditModes[7] = {"Background Mode", "Foreground Mode", "Path Mode", "Stage Mode", "Vehicle Mode", "Warp Mode", "Move Mode"};

SDL_Surface		*screen;
SDL_Surface		*blitdest;
SDL_Surface		*sMapSurface;

SDL_Rect		rectSrcSurface = {0, 0, 768, 608};
SDL_Rect		rectDstSurface = {0, 0, 640, 480};
bool			fNeedBlackBackground = false;
short			iWorldWidth, iWorldHeight;

SDL_Event		event;

gfxFont			font;
gfxFont			fontbig;

gfxSprite		spr_tilesetworld;
gfxSprite		spr_selectedtile;
gfxSprite		spr_worldstarttile;
gfxSprite		spr_dialog;
gfxSprite		menu_shade;

gfxSprite		spr_warps[3];
gfxSprite		spr_backgroundtiles[3];
gfxSprite		spr_foregroundtiles[3];
gfxSprite		spr_vehicles[3];

gfxSprite		spr_worldbackground;
gfxSprite		spr_worldforeground;
gfxSprite		spr_worldvehicle;

int				set_tile = 0;

int				edit_mode = 0;

int				draw_offset_col = 0;  //col and row offset for drawing map to surface
int				draw_offset_row = 0;
int				draw_offset_x = 0;  //x and y offset for drawing maps smaller than screensize
int				draw_offset_y = 0;

int				state;
bool			selectedtiles[MAPWIDTH][MAPHEIGHT];
bool			moveselectedtiles[MAPWIDTH][MAPHEIGHT];
WorldMapTile	copiedtiles[MAPWIDTH][MAPHEIGHT];

//// Global stuff that the map editor doesn't need, but has references to
GraphicsList menugraphicspacklist;
GraphicsList gamegraphicspacklist;
FiltersList filterslist;
gfxSprite		spr_warplock;
short			x_shake = 0;
short			y_shake = 0;
gv				game_values;
void CPlayer::flipsidesifneeded() {}
void CPlayer::KillPlayerMapHazard() {}
void IO_MovingObject::flipsidesifneeded() {}
void IO_MovingObject::KillObjectMapHazard() {}
float CapFallingVelocity(float f) {return 0.0f;}
void removeifprojectile(IO_MovingObject * object, bool playsound, bool forcedead) {}
bool LoadMenuSkin(short playerID, short skinID, short colorID, bool fLoadBothDirections){return false;}

gfxSprite		spr_thumbnail_platformarrows;
gfxSprite		spr_thumbnail_warps[2];
gfxSprite		spr_thumbnail_mapitems[2];
gfxSprite		spr_awardsouls, spr_fireballexplosion;
gfxSprite		spr_maplava, spr_mapwater, spr_mapwaterfall, spr_maplamp;
sfxSound		sfx_boomerang;
SkinList		skinlist;
gfxSprite		**spr_player[4];
CGameMode		*gamemodes[GAMEMODE_LAST];
bool			fResumeMusic;
MapList			maplist;
CMap			g_map;

void DECLSPEC soundfinished(int channel){}
void DECLSPEC musicfinished(){}

sfxSound * g_PlayingSoundChannels[NUM_SOUND_CHANNELS];
CEyecandyContainer eyecandyfront;
gfxSprite		spr_frontmap;
///////

int save_as();
int find();
int clear_world();
bool dialog(char * title, char * instructions, char * input, int inputsize);
int display_help();

void drawmap(bool fScreenshot, short iBlockSize);

WorldMap g_worldmap;
WorldList worldlist;
void loadcurrentworld();
int savecurrentworld();
int findcurrentstring();
int new_world();

int editor_edit();
int editor_warp();
int	editor_background();
int editor_foreground();
int editor_vehicles();

void updateworldsurface();
void takescreenshot();

bool ignoreclick = false;

char findstring[FILEBUFSIZE] = "";

extern char * g_szMusicCategoryNames[8];
short g_musiccategorydisplaytimer = 0;

//main main main
int main(int argc, char *argv[])
{
    /* This must occur before any data files are loaded */
    Initialize_Paths();

	bool done;

	printf("-------------------------------------------------------------------------------\n");
	printf(" %s\n", MAPTITLESTRING);
	printf("-------------------------------------------------------------------------------\n");
	printf("\n---------------- startup ----------------\n");

	gfx_init(640,480, false);
	blitdest = screen;

	SDL_WM_SetCaption(MAPTITLESTRING, "worldeditor.ico");

	printf("\n---------------- loading graphics ----------------\n");
		
	spr_warps[0].init(convertPath("gfx/leveleditor/leveleditor_warp.png"), 255, 0, 255);
	spr_warps[1].init(convertPath("gfx/leveleditor/leveleditor_warp_preview.png"), 255, 0, 255);
	spr_warps[2].init(convertPath("gfx/leveleditor/leveleditor_warp_thumbnail.png"), 255, 0, 255);

	spr_selectedtile.init(convertPath("gfx/leveleditor/leveleditor_selectedtile.png"), 0, 0, 0, 128);
	spr_worldstarttile.init(convertPath("gfx/leveleditor/leveleditor_platformstarttile.png"), 0, 0, 0, 64);

	spr_dialog.init(convertPath("gfx/leveleditor/leveleditor_dialog.png"), 255, 0, 255, 255);
	menu_shade.init(convertPath("gfx/leveleditor/leveleditor_shade.png"), 255, 0, 255, 128);

	font.init(convertPath("gfx/packs/Classic/fonts/font_small.png"));
	fontbig.init(convertPath("gfx/packs/Classic/fonts/font_large.png"));

	printf("\n---------------- load world ----------------\n");

	spr_backgroundtiles[0].init(convertPath("gfx/packs/Classic/world_background.png"), 255, 0, 255);
	//spr_backgroundtiles[1].init(convertPath("gfx/packs/Classic/world_background_medium.png"), 255, 0, 255);
	//spr_backgroundtiles[2].init(convertPath("gfx/packs/Classic/world_background_small.png"), 255, 0, 255);
	spr_worldbackground.setSurface(spr_backgroundtiles[0].getSurface());

	spr_foregroundtiles[0].init(convertPath("gfx/packs/Classic/world_foreground.png"), 255, 0, 255);
	//spr_foregroundtiles[1].init(convertPath("gfx/packs/Classic/world_foreground_medium.png"), 255, 0, 255);
	//spr_foregroundtiles[2].init(convertPath("gfx/packs/Classic/world_foreground_small.png"), 255, 0, 255);
	spr_worldforeground.setSurface(spr_foregroundtiles[0].getSurface());

	spr_vehicles[0].init(convertPath("gfx/packs/Classic/world_vehicles.png"), 255, 0, 255);
	//vehicles[1].init(convertPath("gfx/packs/Classic/world_vehicles_medium.png"), 255, 0, 255);
	//vehicles[2].init(convertPath("gfx/packs/Classic/world_vehicles_small.png"), 255, 0, 255);
	spr_worldvehicle.setSurface(spr_vehicles[0].getSurface());

	sMapSurface = SDL_CreateRGBSurface(screen->flags, 768, 608, screen->format->BitsPerPixel, 0, 0, 0, 0);

	game_values.worldindex = 0;
	loadcurrentworld();
	
	printf("\n---------------- ready, steady, go! ----------------\n");

	printf("entering world editor loop...\n");
	done = false;
	while(!done)
	{
		switch(state)
		{
			case EDITOR_EDIT:
				state = editor_edit();
			break;

			case EDITOR_BACKGROUND:
				state = editor_background();
			break;

			case EDITOR_FOREGROUND:
				state = editor_foreground();
			break;

			case EDITOR_VEHICLES:
				state = editor_vehicles();
			break;

			case EDITOR_WARP:
				state = editor_warp();
			break;

			case EDITOR_QUIT:
				done = true;
			break;

			case DISPLAY_HELP:
				state = display_help();
			break;

			case SAVE_AS:
				state = save_as();
			break;

			case FIND:
				state = find();
			break;

			case CLEAR_WORLD:
				state = clear_world();
			break;

			case NEW_WORLD:
				state = new_world();
			break;

			case SAVE:
				state = savecurrentworld();
			break;

			default:
				printf(" PANIC: WEIRD GAMESTATE: %d\n", state);
			break;
		}
	}

	SDL_FreeSurface(sMapSurface);

	//These two surfaces share the same surface as the world editor back/foreground gfx array as a memory optimization
	//Clear them here so they don't get freed twice
	spr_worldbackground.clearSurface();
	spr_worldforeground.clearSurface();
	spr_worldvehicle.clearSurface();

	printf("\n---------------- save world ----------------\n");

	g_worldmap.Save(convertPath("worlds/ZZworldeditor.txt").c_str());

	printf("\n---------------- shutdown ----------------\n");
	return 0;
}


int editor_edit()
{
	bool done = false;
	g_musiccategorydisplaytimer = 0;

	while (!done)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			Uint8 * keystate = SDL_GetKeyState(NULL);

			switch(event.type)
			{
				case SDL_QUIT:
				{
					done = true;
					break;
				}

				case SDL_KEYDOWN:
				{
					if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						if(g_musiccategorydisplaytimer > 0)
							g_musiccategorydisplaytimer = 0;
						else
							done = true;
					}

					if(event.key.keysym.sym == SDLK_INSERT)
						takescreenshot();

					if(event.key.keysym.sym == SDLK_1)
						return EDITOR_BACKGROUND;

					if(event.key.keysym.sym == SDLK_2)
						return EDITOR_FOREGROUND;

					if(event.key.keysym.sym == SDLK_3)
						return EDITOR_VEHICLES;
					
					/*
					if(event.key.keysym.sym == SDLK_r)
					{
						if(g_musiccategorydisplaytimer > 0 && ++g_map.musicCategoryID >= MAXMUSICCATEGORY)
							g_map.musicCategoryID = 0;
						
						g_musiccategorydisplaytimer = 90;
					}*/
					
					if(event.key.keysym.sym == SDLK_s )
					{
						if(keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT])
							return SAVE_AS;

						return SAVE;
					}

					if(event.key.keysym.sym == SDLK_f )
					{
						if(keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT] || findstring[0] == '\0')
							return FIND;

						findcurrentstring();
					}

					if(event.key.keysym.sym == SDLK_DELETE && (keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]))
					{
						return CLEAR_WORLD;
					}

					if(event.key.keysym.sym == SDLK_n)
						return NEW_WORLD;
										
					if(event.key.keysym.sym == SDLK_w)
						return EDITOR_WARP;

					if(event.key.keysym.sym == SDLK_h || event.key.keysym.sym == SDLK_F1)
						return DISPLAY_HELP;

					if(event.key.keysym.sym == SDLK_UP)
					{
						if(draw_offset_row > 0)
						{
							draw_offset_row--;
							updateworldsurface();
						}
					}
					else if(event.key.keysym.sym == SDLK_DOWN)
					{
						if(draw_offset_row < iWorldHeight - 15)
						{
							draw_offset_row++;
							updateworldsurface();
						}
					}
					else if(event.key.keysym.sym == SDLK_LEFT)
					{
						if(draw_offset_col > 0)
						{
							draw_offset_col--;
							updateworldsurface();
						}
					}
					else if(event.key.keysym.sym == SDLK_RIGHT)
					{
						if(draw_offset_col < iWorldWidth - 20)
						{
							draw_offset_col++;
							updateworldsurface();
						}
					}

					if(event.key.keysym.sym == SDLK_PAGEUP)
					{
						if(--game_values.worldindex < 0)
							game_values.worldindex = worldlist.GetCount() - 1;

						worldlist.prev();

						loadcurrentworld();
					}

					if(event.key.keysym.sym == SDLK_PAGEDOWN)
					{
						if(++game_values.worldindex >= worldlist.GetCount())
							game_values.worldindex = 0;

						worldlist.next();

						loadcurrentworld();
					}

					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					short iButtonX = event.button.x - draw_offset_x;
					short iButtonY = event.button.y - draw_offset_y;
					short iCol = iButtonX / TILESIZE + draw_offset_col;
					short iRow = iButtonY / TILESIZE + draw_offset_row;

					if(iButtonX >= 0 && iButtonY >= 0 && iButtonX < iWorldWidth * TILESIZE && iButtonY < iWorldHeight * TILESIZE)
					{
						if(event.button.button == SDL_BUTTON_LEFT && !ignoreclick)
						{
							if(edit_mode == 0) //selected background
							{
								if(g_worldmap.tiles[iCol][iRow].iBackgroundSprite != set_tile)
								{
									g_worldmap.tiles[iCol][iRow].iBackgroundSprite = set_tile;
									updateworldsurface();
								}
							}
							else if(edit_mode == 1) //selected foreground
							{
								g_worldmap.tiles[iCol][iRow].iForegroundSprite = set_tile;
							}
							else if(edit_mode == 2) //selected connection
							{
								g_worldmap.tiles[iCol][iRow].iConnectionType = set_tile;
							}
							else if(edit_mode == 3) //selected type
							{
								g_worldmap.tiles[iCol][iRow].iType = set_tile;
							}
							/*
							else if(edit_mode == 4) //selected vehicle
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iConnectionType = set_tile;
							}
							else if(edit_mode == 5) //selected warp
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iConnectionType = set_tile;
							}*/
						}
						else if(event.button.button == SDL_BUTTON_RIGHT)
						{
							if(edit_mode == 0)
							{
								if(g_worldmap.tiles[iCol][iRow].iBackgroundSprite != 0)
								{
									g_worldmap.tiles[iCol][iRow].iBackgroundSprite = 0;
									updateworldsurface();
								}
							}
							else if(edit_mode == 1)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iForegroundSprite = 0;
							}
							else if(edit_mode == 2)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iConnectionType = 0;
							}
							else if(edit_mode == 3)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iType = 0;
							}
							/*
							else if(edit_mode == 4)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iType = 0;
							}
							else if(edit_mode == 5)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iType = 0;
							}*/
						}
					}
					
					break;
				}
				//Painting tiles with mouse movement
				case SDL_MOUSEMOTION:
				{
					short iButtonX = event.button.x - draw_offset_x;
					short iButtonY = event.button.y - draw_offset_y;
					short iCol = iButtonX / TILESIZE + draw_offset_col;
					short iRow = iButtonY / TILESIZE + draw_offset_row;

					if(iButtonX >= 0 && iButtonY >= 0 && iButtonX < iWorldWidth * TILESIZE && iButtonY < iWorldHeight * TILESIZE)
					{
						if(event.motion.state == SDL_BUTTON(SDL_BUTTON_LEFT) && !ignoreclick)
						{
							if(edit_mode == 0) //selected background
							{
								if(g_worldmap.tiles[iCol][iRow].iBackgroundSprite != set_tile)
								{
									g_worldmap.tiles[iCol][iRow].iBackgroundSprite = set_tile;
									updateworldsurface();
								}
							}
							else if(edit_mode == 1)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iForegroundSprite = set_tile;
							}
							else if(edit_mode == 2)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iConnectionType = set_tile;
							}
							else if(edit_mode == 3)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iType = set_tile;
							}
						}
						else if(event.motion.state == SDL_BUTTON(SDL_BUTTON_RIGHT))
						{
							if(edit_mode == 0) //selected background
							{
								if(g_worldmap.tiles[iCol][iRow].iBackgroundSprite != 0)
								{
									g_worldmap.tiles[iCol][iRow].iBackgroundSprite = 0;
									updateworldsurface();
								}
							}
							else if(edit_mode == 1)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iForegroundSprite = 0;
							}
							else if(edit_mode == 2)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iConnectionType = 0;
							}
							else if(edit_mode == 3)
							{
								g_worldmap.tiles[event.button.x / TILESIZE][event.button.y / TILESIZE].iType = 0;
							}
						}
					}
				
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						ignoreclick = false;
					}
				
					break;
				}

				default:
					break;
			}
		}

		drawmap(false, TILESIZE);
		
		font.draw(0,0, szEditModes[edit_mode]);

		font.drawRightJustified(640, 0, worldlist.current_name());

		/*
		if(--g_musiccategorydisplaytimer > 0)
		{
			spr_dialog.draw(224, 176);
			font.drawCentered(320, 195, "Music Category");
			fontbig.drawCentered(320, 220, g_szMusicCategoryNames[g_map.musicCategoryID]);

			font.drawCentered(320, 255, "Press 'R' Again");
			font.drawCentered(320, 270, "To Change");
		}*/
		
		SDL_Flip(screen);

		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return EDITOR_QUIT;
}

void updateworldsurface()
{
	g_worldmap.DrawMapToSurface(true, sMapSurface, draw_offset_col, draw_offset_row, 0);
}

void drawmap(bool fScreenshot, short iBlockSize)
{
	if(fNeedBlackBackground)
		SDL_FillRect(screen, NULL, 0x0);

	SDL_BlitSurface(sMapSurface, &rectSrcSurface, blitdest, &rectDstSurface);

	//g_worldmap.Draw(draw_offset_col, draw_offset_row);

	//Draw warp arrows
	/*
	for(int j = 0; j < MAPHEIGHT; j++)
	{
		for(int i = 0; i < MAPWIDTH; i++)
		{
			Warp * warp = NULL;
			if((move_mode == 1 || move_mode == 3) && i - move_offset_x >= 0 && i - move_offset_x < MAPWIDTH &&
				j - move_offset_y >= 0 && j - move_offset_y < MAPHEIGHT && 
				selectedtiles[i - move_offset_x][j - move_offset_y])
			{
				warp = &copiedtiles[i - move_offset_x][j - move_offset_y].warp;
			}
			else
			{
				warp = &g_map.warpdata[i][j];
			}
			
			if(warp->connection != -1)
			{
				SDL_Rect rSrc = {warp->connection * iBlockSize, warp->direction * iBlockSize, iBlockSize, iBlockSize};
				SDL_Rect rDst = {i * iBlockSize, j * iBlockSize, iBlockSize, iBlockSize};

				SDL_BlitSurface(spr_warps[iBlockSize == TILESIZE ? 0 : iBlockSize == PREVIEWTILESIZE ? 1 : 2].getSurface(), &rSrc, screen, &rDst);
			}
		}
	}
	*/
}

int editor_warp()
{
	bool done = false;
	
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 640;
	r.h = 480;

	while (!done)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					done = true;
				break;

				case SDL_KEYDOWN:
					edit_mode = 2;  //change to edit mode using warps
					return EDITOR_EDIT;
				break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						if(event.button.x / TILESIZE < 10 && event.button.y / TILESIZE < 4)
						{
							set_tile = event.button.x / TILESIZE;
						}

						edit_mode = 2;  //change to edit mode using warps
						
						//The user must release the mouse button before trying to add a tile
						ignoreclick = true;
						
						return EDITOR_EDIT;
					}
				break;

				default:
					break;
			}
		}

		
		drawmap(false, TILESIZE);
		menu_shade.draw(0, 0);

		SDL_BlitSurface(spr_warps[0].getSurface(), NULL, screen, &r);
		font.drawRightJustified(640, 0, worldlist.current_name());

		SDL_Flip(screen);

		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return EDITOR_QUIT;
}

int editor_background()
{
	bool done = false;
	
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 640;
	r.h = 480;

	while (!done)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					done = true;
				break;

				case SDL_KEYDOWN:
				{	
					if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						return EDITOR_EDIT;
					}

					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						set_tile = 1;
						return EDITOR_EDIT;
					}
					
					break;
				}

				default:
					break;
			}
		}

		SDL_FillRect(screen, NULL, 0x0);

		spr_backgroundtiles[0].draw(0, 0, 0, 0, 640, 480);

		SDL_Flip(screen);

		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return EDITOR_QUIT;
}

int editor_foreground()
{
	bool done = false;
	
	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = 640;
	r.h = 480;

	/*
	short iCurrentTile = 0;
	short iConvertedTile[300];

	for(short iTile = 0; iTile < 300; iTile++)
		iConvertedTile[iTile] = g_iTileConversion[iTile];
	*/

	while (!done)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					done = true;
				break;

				case SDL_KEYDOWN:
				{	
					if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						return EDITOR_EDIT;
					}

					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						set_tile = 1;
						return EDITOR_EDIT;
					}
					
					break;
				}

				default:
					break;
			}
		}

		SDL_FillRect(screen, NULL, 0x0);

		spr_foregroundtiles[0].draw(0, 0, 0, 0, 640, 480);

		SDL_Flip(screen);

		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return EDITOR_QUIT;
}


int editor_vehicles()
{
	bool done = false;
	
	while (!done)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					done = true;
				break;

				case SDL_KEYDOWN:
					edit_mode = 0;
					return EDITOR_EDIT;
				break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						short set_item_x = event.button.x / TILESIZE;
						short set_item_y = event.button.y / TILESIZE;

						//Set the selected block to one of the interaction blocks
						set_tile = set_item_x;

						edit_mode = 0;

						//The user must release the mouse button before trying to add a tile
						ignoreclick = true;
						return EDITOR_EDIT;
					}
				break;

				default:
					break;
			}
		}

		drawmap(false, TILESIZE);
		menu_shade.draw(0, 0);
		
		SDL_Rect rSrc = {0, 960, 224, 32};
		SDL_Rect rDst = {0, 0, 224, 32};

		//spr_mapitems[0].draw(0, 0, 0, 0, 64, 32);

		font.drawRightJustified(640, 0, worldlist.current_name());
				
		SDL_Flip(screen);

		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return EDITOR_QUIT;
}

int display_help()
{
	//unsigned int currentChar = 0;

	drawmap(false, TILESIZE);
	menu_shade.draw(0, 0);
	fontbig.drawCentered(320, 15, "Help");
	
	int offsety = 55;
	int offsetx = 50;
	font.draw(offsetx, offsety, "Modes:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[t] - Tile Mode");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[i] - Block Mode");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[w] - Warp Mode");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[m] - Move Mode");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[l] - Tile Type Mode");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[p] - Platform Mode");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[x] - No Player Spawn Area");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[z] - No Item Spawn Area");
	offsety += font.getHeight() + 20;

	font.draw(offsetx, offsety, "Layers:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[v] - Hide Blocks");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[y] - Select Active Tile Layer");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[u] - Hide Inactive Tile Layers");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[o] - Optimize Layers");
	offsety += font.getHeight() + 20;

	font.draw(offsetx, offsety, "File:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[n] - New Map");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[s] - Save Map");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[shift] + [s] - Save As");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[f] - Find Map");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[shift] + [f] - New Search");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[pageup] - Go To Previous Map");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[pagedown] - Go To Next Map");

	offsetx = 320;
	offsety = 55;

	font.draw(offsetx, offsety, "Tile, Warp and Block Modes:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[Left Mouse Button] - Place Item");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[Right Mouse Button] - Remove Item");
	offsety += font.getHeight() + 20;

	font.draw(offsetx, offsety, "Move Mode:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[Right Mouse Button] - Select Area");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[Left Mouse Button] - Unselect Area");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "Select And Drag - Move Selections");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "Hold [shift] - Multiple Selections");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "Hold [ctrl] - Freehand Selections");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[delete] - Delete Selection");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[c] - Copy Selection");
	offsety += font.getHeight() + 20;

	font.draw(offsetx, offsety, "Platforms:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[p] - Path");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[+/-] - Change Speed");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[delete] - Delete");
	offsety += font.getHeight() + 20;

	font.draw(offsetx, offsety, "Miscellaneous:");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[b] - Background Thumbnails");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[g] - Change Backgrounds");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[r] - Change Music Category");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[e] - Change Floating Eyecandy");
	offsety += font.getHeight() + 2;
	font.draw(offsetx, offsety, "[ctrl] + [delete] - Clear All");
	offsety += font.getHeight() + 2;


	SDL_Flip(screen);

    while (true)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					return 0;
				break;

				case SDL_KEYDOWN:
					return 0;
				break;

				default:
					break;
			}
		}
		
		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return 0;
}

int save_as()
{	
	char fileName[FILEBUFSIZE] = "";
	char mapLocation[FILEBUFSIZE] = "maps/";

	if(dialog("Save As", "Enter name:", fileName, 64))
	{
		worldlist.add(strcat(fileName, ".map"));
		worldlist.find(fileName);
		savecurrentworld();
		loadcurrentworld();
	}

	return 0;
}

bool dialog(char * title, char * instructions, char * input, int inputsize)
{	
	unsigned int currentChar = 0;

	drawmap(false, TILESIZE);
	menu_shade.draw(0, 0);
	spr_dialog.draw(224, 176);
	fontbig.drawCentered(320, 200, title);
	font.draw(240, 235, instructions);
	font.drawRightJustified(640, 0, worldlist.current_name());
	SDL_Flip(screen);

    while (true)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					return false;
				break;

				case SDL_KEYDOWN:

					if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN)
					{
						return true;
					}
					else if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						return false;
					}
					else if(event.key.keysym.sym == SDLK_BACKSPACE)
					{
						if(currentChar > 0)
						{
							input[currentChar-1] = '\0';
							
							drawmap(false, TILESIZE);
							menu_shade.draw(0, 0);
							spr_dialog.draw(224, 176);
							fontbig.drawCentered(320, 200, title);
							font.draw(240, 235, instructions);
							font.draw(240, 255, input);
							font.drawRightJustified(640, 0, worldlist.current_name());
							SDL_Flip(screen);
							
							currentChar--;
						}
					}
					else
					{
						
						/* I realize the if statement below is long and can be substituted with 
						the function isalnum(event.key.keysym.sym) but I did it this way because
						isalnum acts funny (ie wrong) when the number pad is pressed. */
						if((isdigit(event.key.keysym.sym) || event.key.keysym.sym == 45 || event.key.keysym.sym == 32 || event.key.keysym.sym == 61 || (event.key.keysym.sym >= 95 && event.key.keysym.sym <= 122)) && currentChar < (unsigned)inputsize - 1)
						{
							//insert character into fileName and onScreenText and increment current char
							Uint8 key = event.key.keysym.sym;

							Uint8 * keystate = SDL_GetKeyState(NULL);
							if (keystate[SDLK_LSHIFT] || keystate[SDLK_RSHIFT]) 
							{
								if(event.key.keysym.sym == 45)
									key = 95;
								else if(event.key.keysym.sym >= 95 && event.key.keysym.sym <= 122)
									key-= 32;  //Capitalize
								else if(event.key.keysym.sym == 48)
									key = 41;
								else if(event.key.keysym.sym == 49)
									key = 33;
								else if(event.key.keysym.sym == 50)
									key = 64;
								else if(event.key.keysym.sym == 51)
									key = 35;
								else if(event.key.keysym.sym == 52)
									key = 36;
								else if(event.key.keysym.sym == 53)
									key = 37;
								else if(event.key.keysym.sym == 54)
									key = 94;
								else if(event.key.keysym.sym == 55)
									key = 38;
								else if(event.key.keysym.sym == 57)
									key = 40;
								else if(event.key.keysym.sym == 61)
									key = 43;
							}

							input[currentChar] = key;
							currentChar++;
							input[currentChar] = '\0';

							drawmap(false, TILESIZE);
							menu_shade.draw(0, 0);
							spr_dialog.draw(224, 176);
							fontbig.drawCentered(320, 200, title);
							font.draw(240, 235, instructions);
							font.draw(240, 255, input);
							font.drawRightJustified(640, 0, worldlist.current_name());
							SDL_Flip(screen);
						}
					}	
				break;

				default:
					break;
			}
		}
		
		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	return false;
}

int find()
{
	char fileName[FILEBUFSIZE] = "";
	//char mapLocation[FILEBUFSIZE] = "maps/";

	if(dialog("Find Map", "Enter name:", fileName, 64))
	{
		strcpy(findstring, fileName);

		if(worldlist.find(findstring))
		{
			game_values.worldindex = worldlist.GetCurrentIndex();
			loadcurrentworld();
		}
	}

	return 0;
}

int clear_world()
{ 
	g_worldmap.Clear();
	updateworldsurface();

	printf("World Cleared\n");
	return 0;
}

void loadcurrentworld()
{
	g_worldmap.Load();

	draw_offset_col = 0;
	draw_offset_row = 0;

	rectSrcSurface.x = 0;
	rectSrcSurface.y = 0;
	fNeedBlackBackground = false;

	g_worldmap.GetWorldSize(&iWorldWidth, &iWorldHeight);

	if(iWorldWidth >= 20)
	{
		rectSrcSurface.w = 640;
		draw_offset_x = 0;
	}
	else
	{
		rectSrcSurface.w = iWorldWidth * TILESIZE;
		draw_offset_x = (640 - iWorldWidth * TILESIZE) >> 1;
		fNeedBlackBackground = true;
	}

	if(iWorldHeight >= 15)
	{
		rectSrcSurface.h = 480;
		draw_offset_y = 0;
	}
	else
	{
		rectSrcSurface.h = iWorldHeight * TILESIZE;
		draw_offset_y = (480 - iWorldHeight * TILESIZE) >> 1;
		fNeedBlackBackground = true;
	}

	rectDstSurface.x = draw_offset_x;
	rectDstSurface.y = draw_offset_y;

	updateworldsurface();
}

int savecurrentworld()
{
	g_worldmap.Save();
	return 0;
}

int findcurrentstring()
{
	if(findstring[0] != '\0')
	{
		if(worldlist.find(findstring))
		{
			game_values.worldindex = worldlist.GetCurrentIndex();
			loadcurrentworld();
		}
	}

	return 0;
}

int new_world()
{
	char fileName[FILEBUFSIZE] = "";
	char worldLocation[FILEBUFSIZE] = "worlds/";
	char szWidth[5], szHeight[5];

	if(dialog("New World", "Enter name:", fileName, 64) && 
		dialog("New World", "Width:", szWidth, 4) && 
		dialog("New World", "Height:", szHeight, 4))
	{
		short iWidth = atoi(szWidth);
		short iHeight = atoi(szHeight);

		if(iWidth < 1)
			iWidth = 1;

		if(iHeight < 1)
			iHeight = 1;

		g_worldmap.New(iWidth, iHeight);
		worldlist.add(strcat(worldLocation, strcat(fileName, ".txt")));
		worldlist.find(fileName);
		game_values.worldindex = worldlist.GetCurrentIndex();
		savecurrentworld();
		loadcurrentworld();
	}

	return 0;
}


//take screenshots in full and thumbnail sizes
void takescreenshot()
{
	/*
	short iTileSizes[3] = {32, 16, 8};
	SDL_Surface * old_screen = screen;

	for(short iScreenshotSize = 0; iScreenshotSize < 3; iScreenshotSize++)
	{
		short iTileSize = iTileSizes[iScreenshotSize];

		SDL_Surface * screenshot = SDL_CreateRGBSurface(old_screen->flags, iTileSize * 20, iTileSize * 15, old_screen->format->BitsPerPixel, 0, 0, 0, 0);
		blitdest = screenshot;
		screen = screenshot;
		drawmap(true, iTileSize);

		//Draw platforms to screenshot
		SDL_Rect rSrc = {0, 0, iTileSize, iTileSize};
		SDL_Rect rDst = {0, 0, iTileSize, iTileSize};

		for(short iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
		{
			for(short iPlatformX = 0; iPlatformX < g_map.platforms[iPlatform]->iTileWidth; iPlatformX++)
			{
				for(short iPlatformY = 0; iPlatformY < g_map.platforms[iPlatform]->iTileHeight; iPlatformY++)
				{
					short iTile = g_Platforms[iPlatform].tiles[iPlatformX][iPlatformY];

					if(iTile != TILESETSIZE)
					{
						rSrc.x = iTile % TILESETWIDTH * iTileSize;
						rSrc.y = iTile / TILESETWIDTH * iTileSize;

						rDst.x = (g_Platforms[iPlatform].iStartX + iPlatformX) * iTileSize;
						rDst.y = (g_Platforms[iPlatform].iStartY + iPlatformY) * iTileSize;

						SDL_BlitSurface(g_map.tilesetsurface[iScreenshotSize], &rSrc, blitdest, &rDst);
					}
				}
			}
		}
	
		//And add platform paths
		for(short iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
		{
			if(g_Platforms[iPlatform].iStartX != g_Platforms[iPlatform].iEndX)
			{
				short iCenterOffsetY = (g_map.platforms[iPlatform]->iHeight >> 1) - 16;
				iCenterOffsetY >>= iScreenshotSize; //Resize for preview and thumbnails

				bool fMoveToRight = g_Platforms[iPlatform].iStartX < g_Platforms[iPlatform].iEndX;

				short iSpotLeft = (fMoveToRight ? g_Platforms[iPlatform].iStartX : g_Platforms[iPlatform].iEndX) + 1;
				short iSpotRight = (fMoveToRight ? g_Platforms[iPlatform].iEndX : g_Platforms[iPlatform].iStartX) - 2 + g_map.platforms[iPlatform]->iTileWidth;

				for(short iSpot = iSpotLeft; iSpot <= iSpotRight; iSpot++)
					spr_platformarrows[iScreenshotSize].draw(iSpot * iTileSize, g_Platforms[iPlatform].iStartY * iTileSize + iCenterOffsetY, iTileSize * 5, 0, iTileSize, iTileSize);

				spr_platformarrows[iScreenshotSize].draw((iSpotLeft - 1) * iTileSize, g_Platforms[iPlatform].iStartY * iTileSize + iCenterOffsetY, iTileSize * 2, 0, iTileSize, iTileSize);
				spr_platformarrows[iScreenshotSize].draw((iSpotRight + 1) * iTileSize, g_Platforms[iPlatform].iStartY * iTileSize + iCenterOffsetY, iTileSize * 3, 0, iTileSize, iTileSize);
			}
			else
			{
				short iCenterOffsetX = (g_map.platforms[iPlatform]->iWidth >> 1) - 16;
				iCenterOffsetX >>= iScreenshotSize; //Resize for preview and thumbnails

				bool fMoveUp = g_Platforms[iPlatform].iStartY < g_Platforms[iPlatform].iEndY;

				short iSpotTop = (fMoveUp ? g_Platforms[iPlatform].iStartY : g_Platforms[iPlatform].iEndY) + 1;
				short iSpotBottom = (fMoveUp ? g_Platforms[iPlatform].iEndY : g_Platforms[iPlatform].iStartY) - 2 + g_map.platforms[iPlatform]->iTileHeight;

				for(short iSpot = iSpotTop; iSpot <= iSpotBottom; iSpot++)
					spr_platformarrows[iScreenshotSize].draw(g_Platforms[iPlatform].iStartX * iTileSize + iCenterOffsetX, iSpot * iTileSize, iTileSize * 4, 0, iTileSize, iTileSize);

				spr_platformarrows[iScreenshotSize].draw(g_Platforms[iPlatform].iStartX * iTileSize + iCenterOffsetX, (iSpotTop - 1) * iTileSize, 0, 0, iTileSize, iTileSize);
				spr_platformarrows[iScreenshotSize].draw(g_Platforms[iPlatform].iStartX * iTileSize + iCenterOffsetX, (iSpotBottom + 1) * iTileSize, iTileSize, 0, iTileSize, iTileSize);
			}
		}

		//Save the screenshot with the same name as the map file
		char szSaveFile[256];
		strcpy(szSaveFile, "maps/screenshots/");
		char * pszSaveFile = szSaveFile + strlen(szSaveFile);
		GetNameFromFileName(pszSaveFile, worldlist.current_name());
		
		if(iTileSize == PREVIEWTILESIZE)
			strcat(szSaveFile, "_preview");
		else if(iTileSize == THUMBTILESIZE)
			strcat(szSaveFile, "_thumb");

#ifdef PNG_SAVE_FORMAT
		strcat(szSaveFile, ".png");
		IMG_SavePNG(screenshot, szSaveFile);
#else
		strcat(szSaveFile, ".bmp");
		SDL_SaveBMP(screenshot, szSaveFile);
#endif

		SDL_FreeSurface(screenshot);
	}

	screen = old_screen;
	blitdest = screen;
	*/
}