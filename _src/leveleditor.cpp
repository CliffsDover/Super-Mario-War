/*----------------------------------------------------------+
| Super Mario War Leveleditor								|
|															|
| based on the jnrdev #1/#2 level editor					|
|															|
| ...a big bunch of crappy code ;)							|
| ...with some improvements									|
|															|
|															|
| this sourcecode is released under the GPL.				|
|															|
| start:		18.12.2003									|
| last changes:	20.02.2004									|
| save/load added 03.7.2004 by Armen - thanks!				|
| blocks/maplist/2-layers/extended tile set added 15.07.2005|
|                                                  by Two52 |
|															|
|		� 2004 Florian Hufsky <florian.hufsky@gmail.com>	|
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

#define MAPTITLESTRING "SMW 1.8 Leveleditor"

enum {EDITOR_EDIT, EDITOR_TILES, EDITOR_QUIT, SAVE_AS, FIND, CLEAR_MAP, EDITOR_BLOCKS, NEW_MAP, SAVE, EDITOR_WARP, EDITOR_EYECANDY, DISPLAY_HELP, EDITOR_PLATFORM, EDITOR_TILETYPE, EDITOR_BACKGROUNDS, EDITOR_MAPITEMS, EDITOR_ANIMATION};

#define MAX_PLATFORMS 8
#define MAX_PLATFORM_VELOCITY 16

class MapTile
{
	public:
		TilesetTile	tile[MAPLAYERS];
		int			block;
		Warp		warp;
		bool		nospawn[NUMSPAWNAREATYPES];
		TileType	tiletype;
		int			item;
};

class MapPlatform
{
	public:
		TilesetTile tiles[MAPWIDTH][MAPHEIGHT];
		TileType types[MAPWIDTH][MAPHEIGHT];
		SDL_Rect rIcon[2];
		short iVelocity;
		short iStartX;
		short iStartY;
		short iEndX;
		short iEndY;
};

SDL_Surface		*screen;
SDL_Surface		*blitdest;
SDL_Event		event;

gfxFont			menu_font_small;
gfxFont			menu_font_large;

gfxSprite		spr_background;
gfxSprite		spr_frontmap[2];
gfxSprite		spr_tiletypes[6];
gfxSprite		spr_transparenttiles;

gfxSprite		spr_backgroundlevel;
gfxSprite		spr_tilesetlevel;
gfxSprite		spr_selectedtile;
gfxSprite		spr_nospawntile;
gfxSprite		spr_noitemspawntile;
gfxSprite		spr_platformstarttile;
gfxSprite		spr_platformendtile;
gfxSprite		spr_dialog;
gfxSprite		menu_shade;
gfxSprite		spr_mapitems[3];
gfxSprite		spr_tileanimation[3];

gfxSprite		spr_platformarrows[3];
gfxSprite		spr_warps[3];

gfxSprite		spr_blocks[3];
gfxSprite		spr_unknowntile[3];

TileType		set_type = tile_solid;
int				set_tile_rows = 0;
int				set_tile_cols = 0;
int				set_tile_tileset = 0;
int				set_tile_start_x = 0;
int				set_tile_start_y = 0;
int				set_tile_end_x = 0;
int				set_tile_end_y = 0;
bool			set_tile_drag = false;

int				view_tileset_x = 0;
int				view_tileset_y = 0;

int				set_block = 0;
TileType		set_tiletype = tile_nonsolid;
int				set_mapitem = 0;

int				set_direction = 0;
int				set_connection = 0;

int				edit_mode = 1;
int				selected_layer = 0;

int				move_mode = 0;
int				move_start_x = 0;
int				move_start_y = 0;
int				move_offset_x = 0;
int				move_offset_y = 0;
bool			move_nodrag = false;
bool			move_replace = true;

int				move_drag_start_x = 0;
int				move_drag_start_y = 0;
int				move_drag_offset_x = 0;
int				move_drag_offset_y = 0;

int				set_animation = 0;

CMap			g_map;
CTilesetManager g_tilesetmanager;
int				state;
bool			selectedtiles[MAPWIDTH][MAPHEIGHT];
bool			moveselectedtiles[MAPWIDTH][MAPHEIGHT];
MapTile			copiedtiles[MAPWIDTH][MAPHEIGHT];
int				copiedlayer;

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
gfxSprite		spr_thumbnail_platformarrows;
gfxSprite		spr_thumbnail_warps[2];
gfxSprite		spr_thumbnail_mapitems[2];
gfxSprite		spr_awardsouls, spr_fireballexplosion;

gfxSprite		spr_backmap[2];
CEyecandyContainer eyecandyfront;
CGameMode		*gamemodes[GAMEMODE_LAST];
CPlayer			*list_players[4];
short			list_players_cnt = 0;

bool			g_fLoadMessages = true;

bool			fResumeMusic;
void DECLSPEC soundfinished(int channel){}
void DECLSPEC musicfinished(){}
sfxSound * g_PlayingSoundChannels[NUM_SOUND_CHANNELS];
short			g_iCurrentDrawIndex = 0;
///////

SDL_Surface * s_eyecandy;
SDL_Surface * s_eyecandyindicator;
SDL_Surface * s_platform;

int save_as();
int find();
int clearMap();
bool dialog(char * title, char * instructions, char * input, int inputsize);
int display_help();

#ifdef _DEBUG
	void convertAll();
#endif

void drawmap(bool fScreenshot, short iBlockSize);
void draw_platform(short iPlatform, bool fDrawTileTypes);
void SetPlatformToDefaults(short iPlatform);

MapList maplist;
void loadcurrentmap();
int savecurrentmap();
int findcurrentstring();
int newmap();
void save_map(const std::string &file);
void insert_platforms_into_map();
void CalculatePlatformDims(short iPlatform, short * ix, short * iy, short * iw, short * ih);
void LoadBackgroundPage(SDL_Surface ** sBackgrounds, short iPage);

int editor_edit();
int editor_warp();
int editor_eyecandy();
int	editor_tiles();
int editor_blocks();
int editor_mapitems();
int editor_platforms();
int editor_tiletype();
int editor_backgrounds();
int editor_animation();

void resetselectedtiles();
void copymoveselection();
void pastemoveselection(int movex, int movey);
bool copyselectedtiles();
void clearselectedmaptiles();
void pasteselectedtiles(int movex, int movey);
void getcenterselection(int * x, int * y);
void takescreenshot();

bool viewblocks = true;
bool view_only_layer = false;
bool viewwarps = true;
bool ignoreclick = false;

char findstring[FILEBUFSIZE] = "";

short g_iNumPlatforms = 0;
MapPlatform g_Platforms[MAX_PLATFORMS];

BackgroundList backgroundlist;
extern char * g_szMusicCategoryNames[MAXMUSICCATEGORY];
extern char * g_szBackgroundConversion[26];
extern short g_iMusicCategoryConversion[26];
short g_musiccategorydisplaytimer = 0;

void CopyTilesetTile(TilesetTile * to, TilesetTile * from)
{
	to->iID = from->iID;
	to->iCol = from->iCol;
	to->iRow = from->iRow;
}

void SetTilesetTile(TilesetTile * tile, short iTileset, short iCol, short iRow)
{
	tile->iID = iTileset;
	tile->iCol = iCol;
	tile->iRow = iRow;
}

void ClearTilesetTile(TilesetTile * tile)
{
	SetTilesetTile(tile, TILESETNONE, 0, 0);
}

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
	g_tilesetmanager.Init();

	SDL_WM_SetCaption(MAPTITLESTRING, "leveleditor.ico");

	printf("\n---------------- loading graphics ----------------\n");

	spr_tiletypes[0].init(convertPath("gfx/leveleditor/leveleditor_solid.png"));
	spr_tiletypes[1].init(convertPath("gfx/leveleditor/leveleditor_solid_on_top.png"));
	spr_tiletypes[2].init(convertPath("gfx/leveleditor/leveleditor_ice.png"));
	spr_tiletypes[3].init(convertPath("gfx/leveleditor/leveleditor_deadly.png"));
	spr_tiletypes[4].init(convertPath("gfx/leveleditor/leveleditor_death_on_top.png"));
	spr_tiletypes[5].init(convertPath("gfx/leveleditor/leveleditor_death_on_bottom.png"));
	spr_transparenttiles.init(convertPath("gfx/leveleditor/leveleditor_transparent_tiles.png"), 255, 0, 255, 160);
	
	spr_backgroundlevel.init(convertPath("gfx/leveleditor/leveleditor_background_levels.png"), 255, 0, 255);
	spr_tilesetlevel.init(convertPath("gfx/leveleditor/leveleditor_tileset_levels.png"), 255, 0, 255);
	
	s_eyecandy = IMG_Load(convertPathC("gfx/leveleditor/leveleditor_eyecandy.png"));
	s_eyecandyindicator = IMG_Load(convertPathC("gfx/leveleditor/leveleditor_eyecandyindicator.png"));

	s_platform = IMG_Load(convertPathC("gfx/leveleditor/leveleditor_platform.png"));

	spr_warps[0].init(convertPath("gfx/leveleditor/leveleditor_warp.png"), 255, 0, 255);
	spr_warps[1].init(convertPath("gfx/leveleditor/leveleditor_warp_preview.png"), 255, 0, 255);
	spr_warps[2].init(convertPath("gfx/leveleditor/leveleditor_warp_thumbnail.png"), 255, 0, 255);

	spr_platformarrows[0].init(convertPath("gfx/leveleditor/leveleditor_platform_arrows.png"), 255, 0, 255, 128);
	spr_platformarrows[1].init(convertPath("gfx/leveleditor/leveleditor_platform_arrows_preview.png"), 255, 0, 255, 128);
	spr_platformarrows[2].init(convertPath("gfx/leveleditor/leveleditor_platform_arrows_thumbnail.png"), 255, 0, 255, 128);

	spr_selectedtile.init(convertPath("gfx/leveleditor/leveleditor_selectedtile.png"), 0, 0, 0, 128);
	spr_nospawntile.init(convertPath("gfx/leveleditor/leveleditor_nospawntile.png"), 0, 0, 0, 128);
	spr_noitemspawntile.init(convertPath("gfx/leveleditor/leveleditor_noitemspawntile.png"), 0, 0, 0, 128);
	spr_platformstarttile.init(convertPath("gfx/leveleditor/leveleditor_platformstarttile.png"), 0, 0, 0, 64);
	spr_platformendtile.init(convertPath("gfx/leveleditor/leveleditor_selectedtile.png"), 0, 0, 0, 64);

	spr_mapitems[0].init(convertPath("gfx/leveleditor/leveleditor_mapitems.png"), 255, 0, 255);
	spr_mapitems[1].init(convertPath("gfx/leveleditor/leveleditor_mapitems_preview.png"), 255, 0, 255);
	spr_mapitems[2].init(convertPath("gfx/leveleditor/leveleditor_mapitems_thumbnail.png"), 255, 0, 255);

	spr_dialog.init(convertPath("gfx/leveleditor/leveleditor_dialog.png"), 255, 0, 255, 255);
	menu_shade.init(convertPath("gfx/leveleditor/leveleditor_shade.png"), 255, 0, 255, 128);

	spr_tileanimation[0].init(convertPath("gfx/packs/Classic/tilesets/tile_animation.png"), 255, 0, 255);
	spr_tileanimation[1].init(convertPath("gfx/packs/Classic/tilesets/tile_animation_preview.png"), 255, 0, 255);
	spr_tileanimation[2].init(convertPath("gfx/packs/Classic/tilesets/tile_animation_thumbnail.png"), 255, 0, 255);

	spr_blocks[0].init(convertPath("gfx/packs/Classic/tilesets/blocks.png"), 255, 0, 255);
	spr_blocks[1].init(convertPath("gfx/packs/Classic/tilesets/blocks_preview.png"), 255, 0, 255);
	spr_blocks[2].init(convertPath("gfx/packs/Classic/tilesets/blocks_thumbnail.png"), 255, 0, 255);

	spr_unknowntile[0].init(convertPath("gfx/packs/Classic/tilesets/unknown_tile.png"), 255, 0, 255);
	spr_unknowntile[1].init(convertPath("gfx/packs/Classic/tilesets/unknown_tile_preview.png"), 255, 0, 255);
	spr_unknowntile[2].init(convertPath("gfx/packs/Classic/tilesets/unknown_tile_thumbnail.png"), 255, 0, 255);

	if( SDL_SetColorKey(s_eyecandy, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(s_eyecandy->format, 255, 0, 255)) < 0)
	{
		printf("\n ERROR: Couldn't set ColorKey + RLE: %s\n", SDL_GetError());
	}

	if( SDL_SetColorKey(s_eyecandyindicator, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(s_eyecandyindicator->format, 255, 0, 255)) < 0)
	{
		printf("\n ERROR: Couldn't set ColorKey + RLE: %s\n", SDL_GetError());
	}

	if( SDL_SetColorKey(s_platform, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(s_platform->format, 255, 0, 255)) < 0)
	{
		printf("\n ERROR: Couldn't set ColorKey + RLE: %s\n", SDL_GetError());
	}

	menu_font_small.init(convertPath("gfx/packs/Classic/fonts/font_small.png"));
	menu_font_large.init(convertPath("gfx/packs/Classic/fonts/font_large.png"));

	printf("\n---------------- load map ----------------\n");

	//Setup Platforms
	for(short iPlatform = 0; iPlatform < MAX_PLATFORMS; iPlatform++)
	{
		g_Platforms[iPlatform].rIcon[0].x = (iPlatform % 6) * 32;
		g_Platforms[iPlatform].rIcon[0].y = (iPlatform / 6) * 32 + 224;
		g_Platforms[iPlatform].rIcon[0].w = 32;
		g_Platforms[iPlatform].rIcon[0].h = 32;

		g_Platforms[iPlatform].rIcon[1].x = (iPlatform % 4) * 42 + 240;
		g_Platforms[iPlatform].rIcon[1].y = (iPlatform / 4) * 42 + 174;
		g_Platforms[iPlatform].rIcon[1].w = 32;
		g_Platforms[iPlatform].rIcon[1].h = 32;

		for(short iCol = 0; iCol < MAPWIDTH; iCol++)
		{
			for(short iRow = 0; iRow < MAPHEIGHT; iRow++)
			{
				ClearTilesetTile(&g_Platforms[iPlatform].tiles[iCol][iRow]);
				g_Platforms[iPlatform].types[iCol][iRow] = tile_nonsolid;
			}
		}

		g_Platforms[iPlatform].iVelocity = 4;
	}

	loadcurrentmap();
	
	printf("\n---------------- ready, steady, go! ----------------\n");

	resetselectedtiles();

	printf("entering level editor loop...\n");
	done = false;
	while(!done)
	{
		switch(state)
		{
			case EDITOR_EDIT:
				move_nodrag = false;
				state = editor_edit();
			break;

			case EDITOR_TILES:
				state = editor_tiles();
			break;

			case EDITOR_BLOCKS:
				state = editor_blocks();
			break;

			case EDITOR_MAPITEMS:
				state = editor_mapitems();
			break;

			case EDITOR_WARP:
				state = editor_warp();
			break;

			case EDITOR_EYECANDY:
				state = editor_eyecandy();
			break;

			case EDITOR_PLATFORM:
				state = editor_platforms();
			break;

			case EDITOR_TILETYPE:
				state = editor_tiletype();
			break;

			case EDITOR_BACKGROUNDS:
				state = editor_backgrounds();
			break;

			case EDITOR_ANIMATION:
				state = editor_animation();
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

			case CLEAR_MAP:
				state = clearMap();
			break;

			case NEW_MAP:
				state = newmap();
			break;

			case SAVE:
				state = savecurrentmap();
			break;

			default:
				printf(" PANIC: WEIRD GAMESTATE: %d\n", state);
			break;
		}
	}



	printf("\n---------------- save map ----------------\n");

	save_map(convertPath("maps/ZZleveleditor.map"));
	//g_map.saveTileSet(convertPath("maps/tileset/tileset.tls"));
	g_tilesetmanager.SaveTilesets();

	printf("\n---------------- shutdown ----------------\n");
	return 0;
}

void UpdateTileType(short x, short y)
{
	TileType type = tile_nonsolid;
	for(short k = MAPLAYERS - 1; k >= 0; k--)
	{
		TilesetTile * tile = &g_map.mapdata[x][y][k];

		TileType iTileType = tile_nonsolid;
		if(tile->iID >= 0)
			iTileType = g_tilesetmanager.GetTileset(tile->iID)->GetTileType(tile->iCol, tile->iRow);

		if(iTileType != tile_nonsolid)
		{
			type = iTileType;
			break;
		}
	}

	g_map.mapdatatop[x][y] = type;
}


void AdjustMapItems(short iClickX, short iClickY)
{
	for(short j = 0; j < g_map.iNumMapItems; j++)
	{
		if(g_map.mapitems[j].ix == iClickX && g_map.mapitems[j].iy == iClickY)
		{
			if(g_map.mapdatatop[iClickX][iClickY] != tile_nonsolid && g_map.mapdatatop[iClickX][iClickY] != tile_solid_on_top ||
				g_map.objectdata[iClickX][iClickY] != BLOCKSETSIZE)
			{
				g_map.iNumMapItems--;

				for(short k = j; k < g_map.iNumMapItems - 1; k++)
				{
					g_map.mapitems[k].itype = g_map.mapitems[k + 1].itype;
					g_map.mapitems[k].ix = g_map.mapitems[k + 1].ix;
					g_map.mapitems[k].iy = g_map.mapitems[k + 1].iy;
				}
			}

			break;
		}
	}
}

void RemoveMapItemAt(short x, short y)
{
	for(short j = 0; j < g_map.iNumMapItems; j++)
	{
		if(g_map.mapitems[j].ix == x && g_map.mapitems[j].iy == y)
		{
			g_map.iNumMapItems--;

			for(short k = j; k < g_map.iNumMapItems; k++)
			{
				g_map.mapitems[k].itype = g_map.mapitems[k + 1].itype;
				g_map.mapitems[k].ix = g_map.mapitems[k + 1].ix;
				g_map.mapitems[k].iy = g_map.mapitems[k + 1].iy;
			}

			break;
		}
	}
}

int editor_edit()
{
	bool done = false;
	g_musiccategorydisplaytimer = 0;

	//int iTickStart = 0, iTicks = 0;
	//int iMax = 0, iMin = 100000000, iTotal = 0;
	//int iLoops = 0;
	//bool fTestSwitch = false;

	//maplist.next(false);
	//maplist.next(false);
	//maplist.next(false);
	//maplist.next(false);

	while (!done)
	{
		int framestart = SDL_GetTicks();

		/*
		if(iLoops < 100)
		{
			iTickStart = SDL_GetTicks();
			
			if(fTestSwitch)
				maplist.next(false);
			else
				maplist.prev(false);
				
			fTestSwitch = !fTestSwitch;

			loadcurrentmap();
			
			iTicks = SDL_GetTicks() - iTickStart;

			if(iTicks > iMax)
				iMax = iTicks;

			if(iTicks < iMin)
				iMin = iTicks;

			iTotal += iTicks;

			printf("%s: %d\n", maplist.currentShortmapname(), iTicks);

			if(++iLoops >= 100)
			{
				printf("Map Load Timer - Avg: %d  Min: %d  Max: %d", iTotal / 100, iMin, iMax);
			}
		}
		*/

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
						else if(edit_mode != 1)
							edit_mode = 1;
						else
							done = true;
					}

					if(event.key.keysym.sym == SDLK_INSERT)
						takescreenshot();

					if(event.key.keysym.sym == SDLK_t)
						return EDITOR_TILES;

					if(event.key.keysym.sym == SDLK_i)
						return EDITOR_BLOCKS;

					if(event.key.keysym.sym == SDLK_a)
						return EDITOR_ANIMATION;

					if(event.key.keysym.sym == SDLK_o)
						return EDITOR_MAPITEMS;

					//if 'B' is pressed, rotate backgrounds
					if(event.key.keysym.sym == SDLK_b)
						return EDITOR_BACKGROUNDS;
					
					if(event.key.keysym.sym == SDLK_g)
					{
						backgroundlist.next();

						spr_background.init(convertPath(backgroundlist.current_name()));
						strcpy(g_map.szBackgroundFile, getFileFromPath(backgroundlist.current_name()).c_str());

						if(!keystate[SDLK_LSHIFT] && !keystate[SDLK_RSHIFT])
						{
							//Set music to background default
							for(short iCategory = 0; iCategory < MAXMUSICCATEGORY; iCategory++)
							{
								if(!strncmp(g_szMusicCategoryNames[iCategory], g_map.szBackgroundFile, strlen(g_szMusicCategoryNames[iCategory])))
								{
									g_map.musicCategoryID = iCategory;
									break;
								}
							}
						}
					}
					
					if(event.key.keysym.sym == SDLK_r)
					{
						if(g_musiccategorydisplaytimer > 0 && ++g_map.musicCategoryID >= MAXMUSICCATEGORY)
							g_map.musicCategoryID = 0;
						
						g_musiccategorydisplaytimer = 90;
					}
					
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
						return CLEAR_MAP;
					}

					if(event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE)
					{
						clearselectedmaptiles();
						resetselectedtiles();
					}

					if(event.key.keysym.sym == SDLK_n)
						return NEW_MAP;

					if(event.key.keysym.sym == SDLK_v)
						viewblocks = !viewblocks;
					
					if(event.key.keysym.sym == SDLK_e)
						return EDITOR_EYECANDY;
					
					if(event.key.keysym.sym == SDLK_w)
						return EDITOR_WARP;

					if(event.key.keysym.sym == SDLK_l)
						return EDITOR_TILETYPE;

					if(event.key.keysym.sym == SDLK_h || event.key.keysym.sym == SDLK_F1)
						return DISPLAY_HELP;

					if(event.key.keysym.sym == SDLK_PAGEUP)
					{
						do
						{
							maplist.prev(false);
						}
						while(!maplist.GetValid());

						loadcurrentmap();
					}

					if(event.key.keysym.sym == SDLK_PAGEDOWN)
					{
						do
						{
							maplist.next(false);
						}
						while(!maplist.GetValid());

						loadcurrentmap();
					}

					if(event.key.keysym.sym == SDLK_y)
					{
						if(++selected_layer >= MAPLAYERS)
							selected_layer = 0;
					}

					if(event.key.keysym.sym == SDLK_u)
					{
						view_only_layer = !view_only_layer;
					}

#ifdef _DEBUG
					if(event.key.keysym.sym == SDLK_HOME)
						convertAll();

					//Move map out of maps dir
					if(event.key.keysym.sym == SDLK_F12)
					{
						//printf("Path: %s\n", maplist.currentFilename());
						
						const char * szCurrentPath = maplist.currentFilename();
						char szNewPath[PATH_MAX];
						strcpy(szNewPath, "maps/moved/");
						
						const char * psz = strrchr(szCurrentPath, '/');

						if(!psz)
							psz = szCurrentPath;
						else
							psz++;

						strcat(szNewPath, psz);

						//printf("NewPath: %s\n", szNewPath);

						if(CopyFile(szCurrentPath, szNewPath, false))
						{
							unlink(szCurrentPath);
							maplist.SetValid(false);
						}
					}
#endif

					if(event.key.keysym.sym == SDLK_END)
						g_map.optimize();

					if(event.key.keysym.sym == SDLK_m)
					{
						if(edit_mode == 3)
							move_replace = !move_replace;

						edit_mode = 3;
					}

					if(event.key.keysym.sym == SDLK_x)
						edit_mode = 4;

					if(event.key.keysym.sym == SDLK_z)
						edit_mode = 5;

					if(event.key.keysym.sym == SDLK_LCTRL)
						move_nodrag = true;

					if(event.key.keysym.sym == SDLK_p)
						return EDITOR_PLATFORM;

					if(event.key.keysym.sym == SDLK_c)
					{
						if(edit_mode == 3)
						{
							if(copyselectedtiles())
							{
								move_mode = 3;
								getcenterselection(&move_start_x, &move_start_y);
							}
						}
					}

					break;
				}

				case SDL_KEYUP:
				{
					if(event.key.keysym.sym == SDLK_LCTRL)
						move_nodrag = false;
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					short iClickX = event.button.x / TILESIZE;
					short iClickY = event.button.y / TILESIZE;

					if(event.button.button == SDL_BUTTON_LEFT && !ignoreclick)
					{
						if(edit_mode == 0) //selected blocks
						{
							g_map.objectdata[iClickX][iClickY] = set_block;
							AdjustMapItems(iClickX, iClickY);
						}
						else if(edit_mode == 1) //selected tile(s)
						{
							for(short i = 0; i < set_tile_cols; i++)
							{
								short iLocalX = iClickX + i;

								for(short j = 0; j < set_tile_rows; j++)
								{
									short iLocalY = iClickY + j;

									if(iLocalX >= 0 && iLocalX < MAPWIDTH && iLocalY >= 0 && iLocalY < MAPHEIGHT)
									{
										SetTilesetTile(&g_map.mapdata[iLocalX][iLocalY][selected_layer], set_tile_tileset, set_tile_start_x + i, set_tile_start_y + j);
										
										UpdateTileType(iLocalX, iLocalY);
										AdjustMapItems(iLocalX, iLocalY);
									}
								}
							}
						}
						else if(edit_mode == 2)
						{
							g_map.warpdata[iClickX][iClickY].direction = set_direction;
							g_map.warpdata[iClickX][iClickY].connection = set_connection;
						}
						else if(edit_mode == 3)
						{
							if(move_mode == 3)
							{
								move_mode = 0;

								pasteselectedtiles(move_offset_x, move_offset_y);

								copymoveselection();
								resetselectedtiles();
								pastemoveselection(move_offset_x, move_offset_y);

								move_offset_x = 0;
								move_offset_y = 0;
							}
							else
							{
								if(selectedtiles[iClickX][iClickY])
								{	
									if(copyselectedtiles())
									{
										move_mode = 1;
										clearselectedmaptiles();
									}
								}
								else
								{
									if(!keystate[SDLK_LSHIFT] && !keystate[SDLK_RSHIFT] && !keystate[SDLK_LCTRL])
										resetselectedtiles();

									if(move_nodrag)
									{
										selectedtiles[iClickX][iClickY] = true;
									}
									else
									{
										move_mode = 2;
										move_drag_start_x = iClickX;
										move_drag_start_y = iClickY;
									}
								}

								move_start_x = iClickX;
								move_start_y = iClickY;
							}
						}
						else if(edit_mode == 4)
						{
							g_map.nospawn[0][iClickX][iClickY] = true;
						}
						else if(edit_mode == 5)
						{
							g_map.nospawn[1][iClickX][iClickY] = true;
						}
						else if(edit_mode == 6)
						{
							g_map.mapdatatop[iClickX][iClickY] = set_tiletype;
							AdjustMapItems(iClickX, iClickY);
						}
						else if(edit_mode == 7)
						{
							if(g_map.iNumMapItems < MAXMAPITEMS)
							{
								bool fTileNotAvailable = false;
								for(short j = 0; j < g_map.iNumMapItems; j++)
								{
									if(g_map.mapitems[j].ix == iClickX && g_map.mapitems[j].iy == iClickY)
									{
										fTileNotAvailable = true;
										break;
									}
								}

								if(g_map.mapdatatop[iClickX][iClickY] != tile_nonsolid && g_map.mapdatatop[iClickX][iClickY] != tile_solid_on_top)
									fTileNotAvailable = true;

								if(!fTileNotAvailable)
								{
									MapItem * mapitem = &g_map.mapitems[g_map.iNumMapItems];
									mapitem->itype = set_mapitem;
									mapitem->ix = iClickX;
									mapitem->iy = iClickY;

									g_map.iNumMapItems++;
								}
							}
						}
						else if(edit_mode == 8)
						{
							SetTilesetTile(&g_map.mapdata[iClickX][iClickY][selected_layer], TILESETANIMATED, 0, set_animation);
						}
					}
					else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						if(edit_mode == 0)
							g_map.objectdata[iClickX][iClickY] = BLOCKSETSIZE;
						else if(edit_mode == 1 || edit_mode == 8)
						{
							g_map.mapdata[iClickX][iClickY][selected_layer].iID = TILESETNONE;
							UpdateTileType(iClickX, iClickY);
						}
						else if(edit_mode == 2)
						{
							g_map.warpdata[iClickX][iClickY].direction = -1;
							g_map.warpdata[iClickX][iClickY].connection = -1;
						}
						else if(edit_mode == 3)
						{
							if(move_mode == 3)
							{
								move_mode = 0;
								resetselectedtiles();
								move_offset_x = 0;
								move_offset_y = 0;
							}
							else
							{
								if(selectedtiles[iClickX][iClickY])
								{
									selectedtiles[iClickX][iClickY] = false;
								}
								else
								{
									resetselectedtiles();
								}
							}
						}
						else if(edit_mode == 4)
						{
							g_map.nospawn[0][iClickX][iClickY] = false;
						}
						else if(edit_mode == 5)
						{
							g_map.nospawn[1][iClickX][iClickY] = false;
						}
						else if(edit_mode == 6)
						{
							g_map.mapdatatop[iClickX][iClickY] = tile_nonsolid;
						}
						else if(edit_mode == 7)
						{
							RemoveMapItemAt(iClickX, iClickY);
						}
					}
					
					break;
				}

				case SDL_MOUSEMOTION:
				{
					short iClickX = event.button.x / TILESIZE;
					short iClickY = event.button.y / TILESIZE;

					if(event.motion.state == SDL_BUTTON(SDL_BUTTON_LEFT) && !ignoreclick)
					{
						if(edit_mode == 0)
						{
							g_map.objectdata[iClickX][iClickY] = set_block;
							AdjustMapItems(iClickX, iClickY);
						}
						else if(edit_mode == 1)
						{
							for(short i = 0; i < set_tile_cols; i++)
							{
								short iLocalX = iClickX + i;

								for(short j = 0; j < set_tile_rows; j++)
								{
									short iLocalY = iClickY + j;

									if(iLocalX >= 0 && iLocalX < MAPWIDTH && iLocalY >= 0 && iLocalY < MAPHEIGHT)
									{
										SetTilesetTile(&g_map.mapdata[iLocalX][iLocalY][selected_layer], set_tile_tileset, set_tile_start_x + i, set_tile_start_y + j);

										UpdateTileType(iLocalX, iLocalY);
										AdjustMapItems(iLocalY, iLocalY);
									}
								}
							}
						}
						else if(edit_mode == 2)
						{
							g_map.warpdata[iClickX][iClickY].direction = set_direction;
							g_map.warpdata[iClickX][iClickY].connection = set_connection;
						}
						else if(edit_mode == 3)
						{
							if(move_mode == 0)
							{
								selectedtiles[iClickX][iClickY] = true;
							}
						}
						else if(edit_mode == 4)
						{
							g_map.nospawn[0][iClickX][iClickY] = true;
						}
						else if(edit_mode == 5)
						{
							g_map.nospawn[1][iClickX][iClickY] = true;
						}
						else if(edit_mode == 6)
						{
							g_map.mapdatatop[iClickX][iClickY] = set_tiletype;
							AdjustMapItems(iClickX, iClickY);
						}
						else if(edit_mode == 8)
						{
							SetTilesetTile(&g_map.mapdata[iClickX][iClickY][selected_layer], TILESETANIMATED, 0, set_animation);
						}
					}
					else if(event.motion.state == SDL_BUTTON(SDL_BUTTON_RIGHT))
					{
						if(edit_mode == 0)
							g_map.objectdata[iClickX][iClickY] = BLOCKSETSIZE;
						else if(edit_mode == 1 || edit_mode == 8)
						{
							g_map.mapdata[iClickX][iClickY][selected_layer].iID = TILESETNONE;
							UpdateTileType(iClickX, iClickY);
						}
						else if(edit_mode == 2)
						{
							g_map.warpdata[iClickX][iClickY].direction = -1;
							g_map.warpdata[iClickX][iClickY].connection = -1;
						}
						else if(edit_mode == 3)
						{
							if(move_mode == 0)
								selectedtiles[iClickX][iClickY] = false;
						}
						else if(edit_mode == 4)
						{
							g_map.nospawn[0][iClickX][iClickY] = false;
						}
						else if(edit_mode == 5)
						{
							g_map.nospawn[1][iClickX][iClickY] = false;
						}
						else if(edit_mode == 6)
						{
							g_map.mapdatatop[iClickX][iClickY] = tile_nonsolid;
						}
					}
				
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					short iClickX = event.button.x / TILESIZE;
					short iClickY = event.button.y / TILESIZE;

					if(event.button.button == SDL_BUTTON_LEFT)
					{
						ignoreclick = false;

						if(move_mode == 1)
						{
							pasteselectedtiles(move_offset_x, move_offset_y);

							copymoveselection();
							resetselectedtiles();
							pastemoveselection(move_offset_x, move_offset_y);

							move_offset_x = 0;
							move_offset_y = 0;
						}
						else if(move_mode == 2)
						{
							int left = move_drag_start_x < iClickX ? move_drag_start_x : iClickX;
							int top = move_drag_start_y < iClickY ? move_drag_start_y : iClickY;
							int right = move_drag_start_x < iClickX ? iClickX : move_drag_start_x;
							int bottom = move_drag_start_y < iClickY ? iClickY : move_drag_start_y;

							for(int k = top; k <= bottom; k++)
							{
								for(int j = left; j <= right; j++)
								{
									selectedtiles[j][k] = true;
								}
							}
						}

						move_mode = 0;
					}
				
					break;
				}

				default:
					break;
			}
		}

		if(move_mode == 1 || move_mode == 3)
		{
			int mousex, mousey;
			SDL_GetMouseState(&mousex, &mousey);
			move_offset_x = (mousex / TILESIZE) - move_start_x;
			move_offset_y = (mousey / TILESIZE) - move_start_y;
		}
		else if(move_mode == 2)
		{
			int mousex, mousey;
			SDL_GetMouseState(&mousex, &mousey);
			move_drag_offset_x = (mousex / TILESIZE);
			move_drag_offset_y = (mousey / TILESIZE);
		}

		if(maplist.GetValid())
		{
			drawmap(false, TILESIZE);
		}
		else
		{
			SDL_FillRect(screen, NULL, 0x0);
			menu_font_large.drawCentered(320, 200, "Map has been deleted.");
		}

		if(edit_mode == 0)
		{
			menu_font_small.draw(0,0, "Block Mode");
		}
		else if(edit_mode == 1 || edit_mode == 8)
		{
			char modestring[128] = "";

			if(edit_mode == 1)
				strcpy(modestring, "Tile Mode - ");
			else
				strcpy(modestring, "Animated Tile Mode - ");
			
			if(selected_layer == 0)
				strcat(modestring, "Bottom Background");
			else if(selected_layer == 1)
				strcat(modestring, "Top Background");
			else if(selected_layer == 2)
				strcat(modestring, "Bottom Foreground");
			else if(selected_layer == 3)
				strcat(modestring, "Top Foreground");

			if(view_only_layer)
				strcat(modestring, " Only");

			menu_font_small.draw(0,0, modestring);

			if(view_only_layer)
				spr_backgroundlevel.draw(2, 18 + (3 - selected_layer) * 18, selected_layer * 16, (3 - selected_layer) * 18, 16, 16);
			else
				spr_backgroundlevel.draw(2, 18, selected_layer * 16, 0, 16, 70);
		}
		else if(edit_mode == 2)
		{
			menu_font_small.draw(0,0, "Warp Mode");
		}
		else if(edit_mode == 3)
		{
			for(int k = 0; k < MAPHEIGHT; k++)
			{
				for(int j = 0; j < MAPWIDTH; j++)
				{
					if(selectedtiles[j][k])
						spr_selectedtile.draw((j + move_offset_x) * TILESIZE, (k + move_offset_y) * TILESIZE);
				}
			}

			//Draw dragging selection
			if(move_mode == 2)
			{
				int left = move_drag_start_x < move_drag_offset_x ? move_drag_start_x : move_drag_offset_x;
				int top = move_drag_start_y < move_drag_offset_y ? move_drag_start_y : move_drag_offset_y;
				int right = move_drag_start_x < move_drag_offset_x ? move_drag_offset_x : move_drag_start_x;
				int bottom = move_drag_start_y < move_drag_offset_y ? move_drag_offset_y : move_drag_start_y;

				for(int k = top; k <= bottom; k++)
				{
					for(int j = left; j <= right; j++)
					{
						if(!selectedtiles[j][k])
							spr_selectedtile.draw(j * TILESIZE, k * TILESIZE);
					}
				}
			}

			if(move_replace)
				menu_font_small.draw(0,0, "Move Mode - Replace");
			else
				menu_font_small.draw(0,0, "Move Mode - Merge");

			if(view_only_layer)
				spr_backgroundlevel.draw(2, 18 + (3 - selected_layer) * 18, selected_layer * 16, (3 - selected_layer) * 18, 16, 16);
			else
				spr_backgroundlevel.draw(2, 18, selected_layer * 16, 0, 16, 70);
		}
		else if(edit_mode == 4)
		{
			for(int k = 0; k < MAPHEIGHT; k++)
			{
				for(int j = 0; j < MAPWIDTH; j++)
				{
					if(g_map.nospawn[0][j][k])
						spr_nospawntile.draw(j * TILESIZE, k * TILESIZE);
				}
			}

			menu_font_small.draw(0, 0, "No Player Spawn Mode");
		}
		else if(edit_mode == 5)
		{
			for(int k = 0; k < MAPHEIGHT; k++)
			{
				for(int j = 0; j < MAPWIDTH; j++)
				{
					if(g_map.nospawn[1][j][k])
						spr_noitemspawntile.draw(j * TILESIZE, k  * TILESIZE);
				}
			}

			menu_font_small.draw(0, 0, "No Item Spawn Mode");
		}
		else if(edit_mode == 6)
		{
			for(int k = 0; k < MAPHEIGHT; k++)
			{
				for(int j = 0; j < MAPWIDTH; j++)
				{
					if(g_map.mapdatatop[j][k] > 0)
						spr_transparenttiles.draw(j * TILESIZE, k * TILESIZE, (g_map.mapdatatop[j][k] - 1) * TILESIZE, 0, TILESIZE, TILESIZE);
				}
			}

			menu_font_small.draw(0, 0, "Tile Type Mode");
		}
		else if(edit_mode == 7)
		{
			menu_font_small.draw(0, 0, "Map Item Mode");
		}

		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());

		if(--g_musiccategorydisplaytimer > 0)
		{
			spr_dialog.draw(224, 176);
			menu_font_small.drawCentered(320, 195, "Music Category");
			menu_font_large.drawCentered(320, 220, g_szMusicCategoryNames[g_map.musicCategoryID]);

			menu_font_small.drawCentered(320, 255, "Press 'R' Again");
			menu_font_small.drawCentered(320, 270, "To Change");
		}
		
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

void drawlayer(int layer, bool fUseCopied, short iBlockSize)
{	
	short iTilesetIndex = iBlockSize == TILESIZE ? 0 : iBlockSize == PREVIEWTILESIZE ? 1 : 2;

	//draw left to right full vertical
	for(short i = 0; i < MAPWIDTH; i++)
	{
		for(short j = 0; j < MAPHEIGHT; j++)
		{
			TilesetTile * tile = NULL;
			if((move_mode == 1 || move_mode == 3) && i - move_offset_x >= 0 && i - move_offset_x < MAPWIDTH &&
				j - move_offset_y >= 0 && j - move_offset_y < MAPHEIGHT && 
				selectedtiles[i - move_offset_x][j - move_offset_y])
			{
				if(fUseCopied)
					tile = &copiedtiles[i - move_offset_x][j - move_offset_y].tile[layer];
			}
			else
			{
				if(!fUseCopied)
					tile = &g_map.mapdata[i][j][layer];
			}

			if(!tile || tile->iID == TILESETNONE)
				continue;
			
			if(tile->iID >= 0)
			{
				g_tilesetmanager.Draw(screen, tile->iID, iTilesetIndex, tile->iCol, tile->iRow, i, j);
				//SDL_BlitSurface(g_tilesetmanager.GetTileset(tile->iID)->GetSurface(iTilesetIndex), &g_tilesetmanager.rRects[iTilesetIndex][tile->iCol][tile->iRow], screen, &bltrect);
			}
			else if(tile->iID == TILESETANIMATED)
			{
				SDL_BlitSurface(spr_tileanimation[iTilesetIndex].getSurface(), &g_tilesetmanager.rRects[iTilesetIndex][tile->iCol << 2][tile->iRow], screen, &g_tilesetmanager.rRects[iTilesetIndex][i][j]);
			}
			else if(tile->iID == TILESETUNKNOWN)
			{
				SDL_BlitSurface(spr_unknowntile[iTilesetIndex].getSurface(), &g_tilesetmanager.rRects[iTilesetIndex][0][0], screen, &g_tilesetmanager.rRects[iTilesetIndex][i][j]);
			}
		}
	}
}

void drawmap(bool fScreenshot, short iBlockSize)
{
	if(iBlockSize != TILESIZE)
	{
		SDL_Rect srcrect;
		srcrect.x = 0;
		srcrect.y = 0;
		srcrect.w = 640;
		srcrect.h = 480;

		SDL_Rect dstrect;
		dstrect.x = 0;
		dstrect.y = 0;
		dstrect.w = iBlockSize * 20;
		dstrect.h = iBlockSize * 15;

		if(SDL_SoftStretch(spr_background.getSurface(), &srcrect, blitdest, &dstrect) < 0)
		{
			fprintf(stderr, "SDL_SoftStretch error: %s\n", SDL_GetError());
			return;
		}
	}
	else
	{
		spr_background.draw(0,0);
	}

	if((view_only_layer && selected_layer == 0) || !view_only_layer)
		drawlayer(0, false, iBlockSize);

	if(!fScreenshot)
	{
		if((view_only_layer && copiedlayer == 0) || !view_only_layer)
			drawlayer(0, true, iBlockSize);
	}
	
	if((view_only_layer && selected_layer == 1) || !view_only_layer)
		drawlayer(1, false, iBlockSize);

	if(!fScreenshot)
	{
		if((view_only_layer && copiedlayer == 1) || !view_only_layer)
			drawlayer(1, true, iBlockSize);
	}

	if((viewblocks && !view_only_layer) || fScreenshot)
	{
		short iTilesizeIndex = iBlockSize == 32 ? 0 : iBlockSize == 16 ? 1 : 2;

		SDL_Rect rSrc = {0, 0, iBlockSize, iBlockSize};

		for(int j = 0; j < MAPHEIGHT; j++)
		{
			for(int i = 0; i < MAPWIDTH; i++)
			{
				int displayblock = BLOCKSETSIZE;
				if((move_mode == 1 || move_mode == 3) && i - move_offset_x >= 0 && i - move_offset_x < MAPWIDTH &&
					j - move_offset_y >= 0 && j - move_offset_y < MAPHEIGHT && 
					selectedtiles[i - move_offset_x][j - move_offset_y])
				{
					displayblock = copiedtiles[i - move_offset_x][j - move_offset_y].block;
				}
				else
				{
					displayblock = g_map.objectdata[i][j];
				}

				if(displayblock < BLOCKSETSIZE)
				{
					if(displayblock < 7)
					{
						rSrc.x = displayblock * iBlockSize;
						rSrc.y = 0;
					}
					if(displayblock >= 7 && displayblock <= 14)
					{
						rSrc.x = displayblock * iBlockSize;
						rSrc.y = iBlockSize * g_map.iSwitches[(displayblock - 7) % 4];
					}
					else if(displayblock >= 15 && displayblock <= 18)
					{
						rSrc.x = (displayblock - 15) * iBlockSize;
						rSrc.y = iBlockSize;
					}
					
					SDL_BlitSurface(spr_blocks[iTilesizeIndex].getSurface(), &rSrc, screen, &g_tilesetmanager.rRects[iTilesizeIndex][i][j]);
				}
			}
		}
	}
	
	if(!view_only_layer || fScreenshot)
	{
		for(short j = 0; j < g_map.iNumMapItems; j++)
		{
			spr_mapitems[iBlockSize == TILESIZE ? 0 : iBlockSize == PREVIEWTILESIZE ? 1 : 2].draw(g_map.mapitems[j].ix * iBlockSize, g_map.mapitems[j].iy * iBlockSize, g_map.mapitems[j].itype * iBlockSize, 0, iBlockSize, iBlockSize);
		}

		if(!fScreenshot)
		{
			for(int j = 0; j < MAPHEIGHT; j++)
			{
				for(int i = 0; i < MAPWIDTH; i++)
				{
					short iNewX = i - move_offset_x;
					short iNewY = j - move_offset_y;

					if((move_mode == 1 || move_mode == 3) && iNewX >= 0 && iNewX < MAPWIDTH &&
						iNewY >= 0 && iNewY < MAPHEIGHT && 
						selectedtiles[iNewX][iNewY])
					{
						if(copiedtiles[iNewX][iNewY].item >= 0)
							spr_mapitems[0].draw(i << 5, j << 5, copiedtiles[iNewX][iNewY].item << 5, 0, TILESIZE, TILESIZE);
					}
				}
			}
		}
	}

	if((view_only_layer && selected_layer == 2) || !view_only_layer)
		drawlayer(2, false, iBlockSize);

	if(!fScreenshot)
	{
		if((view_only_layer && copiedlayer == 2) || !view_only_layer)
			drawlayer(2, true, iBlockSize);
	}
	
	if((view_only_layer && selected_layer == 3) || !view_only_layer)
		drawlayer(3, false, iBlockSize);

	if(!fScreenshot)
	{
		if((view_only_layer && copiedlayer == 3) || !view_only_layer)
			drawlayer(3, true, iBlockSize);
	}

	if((viewwarps && !view_only_layer) || fScreenshot)
	{
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
	}

	/*
	for(int k = 0; k < g_map.numwarpexits; k++)
	{
		SDL_Rect rSrc = {g_map.warpexits[k].connection * TILESIZE, g_map.warpexits[k].direction * TILESIZE, TILESIZE, TILESIZE};
		SDL_Rect rDst = {g_map.warpexits[k].x, g_map.warpexits[k].y, TILESIZE, TILESIZE};

		SDL_BlitSurface(spr_warps[0].getSurface(), &rSrc, screen, &rDst);
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
							set_direction = event.button.y / TILESIZE;
							set_connection = event.button.x / TILESIZE;
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
		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());

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

int editor_eyecandy()
{
	bool done = false;
	
	SDL_Rect r;
	r.x = 224;
	r.y = 128;
	r.w = 192;
	r.h = 224;

	SDL_Rect ri;
	ri.x = r.x + 20;
	ri.y = r.y + 50 + g_map.eyecandyID * 55;
	ri.w = 152;
	ri.h = 24;

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
						return EDITOR_EDIT;
				break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						for(int k = 0; k < 3; k++)
						{
							if(event.button.x >= r.x && event.button.x < r.x + r.w &&
								event.button.y >= r.y + 30 + k * 60 && event.button.y < r.y + 30 + (k + 1) * 60)
							{
								g_map.eyecandyID = k;
								ri.y = r.y + 50 + g_map.eyecandyID * 55;
								
								//The user must release the mouse button before trying to add a tile
								ignoreclick = true;
								return EDITOR_EDIT;
							}
						}
					}
				break;

				default:
					break;
			}
		}

		
		drawmap(false, TILESIZE);
		menu_shade.draw(0, 0);

		SDL_BlitSurface(s_eyecandy, NULL, screen, &r);
		SDL_BlitSurface(s_eyecandyindicator, NULL, screen, &ri);

		menu_font_small.draw(0,480-menu_font_small.getHeight(), "eyecandy mode: [e] edit mode, [LMB] choose eyecandy");
		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());

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

int editor_platforms()
{
	bool done = false;
	
	SDL_Rect r;
	r.x = 224;
	r.y = 128;
	r.w = 192;
	r.h = 224;

	SDL_Rect ri;
	ri.x = r.x + 20;
	ri.y = r.y + 50 + g_map.eyecandyID * 55;
	ri.w = 152;
	ri.h = 24;

	SDL_Rect rNewButton[2];
	rNewButton[0].x = 0;
	rNewButton[0].y = 288;
	rNewButton[0].w = 76;
	rNewButton[0].h = 32;

	rNewButton[1].x = r.x + (r.w >> 1) - (rNewButton[0].w >> 1);
	rNewButton[1].y = r.y + r.h - 64;
	rNewButton[1].w = 76;
	rNewButton[1].h = 32;

	SDL_Rect rVelocity[4];
	rVelocity[0].x = 12;
	rVelocity[0].y = 320;
	rVelocity[0].w = 172;
	rVelocity[0].h = 13;

	rVelocity[1].x = 418;
	rVelocity[1].y = 10;
	rVelocity[1].w = 172;
	rVelocity[1].h = 13;

	rVelocity[2].x = 184;
	rVelocity[2].y = 320;
	rVelocity[2].w = 8;
	rVelocity[2].h = 16;

	rVelocity[3].x = 404;
	rVelocity[3].y = 8;
	rVelocity[3].w = 8;
	rVelocity[3].h = 16;

	SDL_Rect rPath[6];
	rPath[0].x = 0;
	rPath[0].y = 336;
	rPath[0].w = 32;
	rPath[0].h = 32;

	rPath[1].x = 0;
	rPath[1].y = 0;
	rPath[1].w = 32;
	rPath[1].h = 32;

	rPath[2].x = 32;
	rPath[2].y = 336;
	rPath[2].w = 32;
	rPath[2].h = 32;

	rPath[3].x = 0;
	rPath[3].y = 0;
	rPath[3].w = 32;
	rPath[3].h = 32;

	rPath[4].x = 64;
	rPath[4].y = 336;
	rPath[4].w = 32;
	rPath[4].h = 32;

	rPath[5].x = 0;
	rPath[5].y = 0;
	rPath[5].w = 32;
	rPath[5].h = 32;

	enum {PLATFORM_EDIT_STATE_SELECT, PLATFORM_EDIT_STATE_EDIT, PLATFORM_EDIT_STATE_PATH, PLATFORM_EDIT_STATE_TEST, PLATFORM_EDIT_STATE_TILETYPE};

	short iPlatformEditState = PLATFORM_EDIT_STATE_SELECT;
	short iEditPlatform = 0;
	short iPlatformTop, iPlatformLeft, iPlatformWidth, iPlatformHeight;
								
	while (!done)
	{
		int framestart = SDL_GetTicks();

		//handle messages
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
				{
					done = true;
					break;
				}
				case SDL_KEYDOWN:
				{
					if(event.key.keysym.sym == SDLK_t)
					{
						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							editor_tiles();
							iPlatformEditState = PLATFORM_EDIT_STATE_EDIT;
						}
					}
					else if(event.key.keysym.sym == SDLK_l)
					{
						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							editor_tiletype();
							iPlatformEditState = PLATFORM_EDIT_STATE_TILETYPE;
						}
					}
					else if(event.key.keysym.sym == SDLK_c)
					{
						if(PLATFORM_EDIT_STATE_SELECT == iPlatformEditState)
						{
							iPlatformEditState = PLATFORM_EDIT_STATE_TEST;
							insert_platforms_into_map();
							g_map.resetPlatforms();
						}
					}
					else if(event.key.keysym.sym == SDLK_DELETE)
					{
						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState)
						{
							//Copy platforms into empty spot
							for(short iPlatform = iEditPlatform; iPlatform < g_iNumPlatforms - 1; iPlatform++)
							{
								for(short iCol = 0; iCol < MAPWIDTH; iCol++)
								{
									for(short iRow = 0; iRow < MAPHEIGHT; iRow++)
									{
										CopyTilesetTile(&g_Platforms[iPlatform].tiles[iCol][iRow], &g_Platforms[iPlatform + 1].tiles[iCol][iRow]);
										g_Platforms[iPlatform].types[iCol][iRow] = g_Platforms[iPlatform + 1].types[iCol][iRow];
									}
								}

								g_Platforms[iPlatform].iVelocity = g_Platforms[iPlatform + 1].iVelocity;
								g_Platforms[iPlatform].iStartX = g_Platforms[iPlatform + 1].iStartX;
								g_Platforms[iPlatform].iStartY = g_Platforms[iPlatform + 1].iStartY;
								g_Platforms[iPlatform].iEndX = g_Platforms[iPlatform + 1].iEndX;
								g_Platforms[iPlatform].iEndY = g_Platforms[iPlatform + 1].iEndY;
							}
							
							g_iNumPlatforms--;
							iPlatformEditState = PLATFORM_EDIT_STATE_SELECT;
						}
					}
					else if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						if(PLATFORM_EDIT_STATE_SELECT == iPlatformEditState)
						{
							return EDITOR_EDIT;
						}
						else if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState)
						{
							iPlatformEditState = PLATFORM_EDIT_STATE_SELECT;
						}
						else if(PLATFORM_EDIT_STATE_PATH == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							iPlatformEditState = PLATFORM_EDIT_STATE_EDIT;
						}
						else if(PLATFORM_EDIT_STATE_TEST == iPlatformEditState)
						{
							iPlatformEditState = PLATFORM_EDIT_STATE_SELECT;
						}
					}
					else if(event.key.keysym.sym == SDLK_KP_MINUS || event.key.keysym.sym == SDLK_MINUS)
					{
						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							if(g_Platforms[iEditPlatform].iVelocity > 2)
								g_Platforms[iEditPlatform].iVelocity--;
						}
					}
					else if(event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_EQUALS)
					{
						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							if(g_Platforms[iEditPlatform].iVelocity < MAX_PLATFORM_VELOCITY)
								g_Platforms[iEditPlatform].iVelocity++;
						}
					}
					else if(event.key.keysym.sym == SDLK_p)
					{
						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							iPlatformEditState = PLATFORM_EDIT_STATE_PATH;
							CalculatePlatformDims(iEditPlatform, &iPlatformLeft, &iPlatformTop, &iPlatformWidth, &iPlatformHeight);
						}
					}

					break;
				}
				case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT && !ignoreclick)
					{
						if(PLATFORM_EDIT_STATE_SELECT == iPlatformEditState)
						{
							//check clicks on existing platforms
							for(int iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
							{
								if(event.button.x >= g_Platforms[iPlatform].rIcon[1].x && event.button.x < g_Platforms[iPlatform].rIcon[1].x + g_Platforms[iPlatform].rIcon[1].w &&
								   event.button.y >= g_Platforms[iPlatform].rIcon[1].y && event.button.y < g_Platforms[iPlatform].rIcon[1].y + g_Platforms[iPlatform].rIcon[1].h)
								{
									iEditPlatform = iPlatform;
									iPlatformEditState = PLATFORM_EDIT_STATE_EDIT;
									ignoreclick = true;
								}
							}

							if(g_iNumPlatforms < MAX_PLATFORMS && event.button.x >= rNewButton[1].x && event.button.x < rNewButton[1].x + rNewButton[1].w &&
							   event.button.y >= rNewButton[1].y && event.button.y < rNewButton[1].y + rNewButton[1].h)
							{
								//Create a new platform then edit it
								
								iEditPlatform = g_iNumPlatforms;
								g_iNumPlatforms++;
								iPlatformEditState = PLATFORM_EDIT_STATE_EDIT;
								ignoreclick = true;

								SetPlatformToDefaults(iEditPlatform);
							}
						}
						else if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState)
						{
							if(!ignoreclick)
							{
								short ix = event.button.x / TILESIZE;
								short iy = event.button.y / TILESIZE;

								for(short i = 0; i < set_tile_cols; i++)
								{
									for(short j = 0; j < set_tile_rows; j++)
									{
										if(ix + i >= 0 && ix + i < MAPWIDTH && iy + j >= 0 && iy + j < MAPHEIGHT)
										{
											TilesetTile * tile = &g_Platforms[iEditPlatform].tiles[ix + i][iy + j];
											SetTilesetTile(tile, set_tile_tileset, set_tile_start_x + i, set_tile_start_y + j);
											g_Platforms[iEditPlatform].types[ix + i][iy + j] = g_tilesetmanager.GetTileset(tile->iID)->GetTileType(tile->iCol, tile->iRow);
										}
									}
								}
							}
						}
						else if(PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							if(!ignoreclick)
							{
								short ix = event.button.x / TILESIZE;
								short iy = event.button.y / TILESIZE;

								g_Platforms[iEditPlatform].types[ix][iy] = set_tiletype;
							}
						}
						else if(PLATFORM_EDIT_STATE_PATH == iPlatformEditState)
						{
							if(!ignoreclick)
							{
								short iTempStartX = event.button.x / TILESIZE;
								short iTempStartY = event.button.y / TILESIZE;
								short iTempEndX = g_Platforms[iEditPlatform].iEndX;
								short iTempEndY = g_Platforms[iEditPlatform].iEndY;

								if(iTempStartX >= 0 && iTempStartX <= MAPWIDTH - iPlatformWidth &&
									iTempStartY >= 0 && iTempStartY <= MAPHEIGHT - iPlatformHeight)
								{
									//If the start isn't in the same row or column, then move the end point so that it is
									if(iTempStartX != iTempEndX && iTempStartY != iTempEndY)
									{
										short iXDiff = abs(iTempStartX - iTempEndX);
										short iYDiff = abs(iTempStartY - iTempEndY);

										//If the x difference is smaller, move it so it lines up
										if(iYDiff > iXDiff)
											iTempEndX = iTempStartX;
										else
											iTempEndY = iTempStartY;
									}

									//If the start and end are the same point, then don't allow this change
									if(iTempStartX != iTempEndX ||
										iTempStartY != iTempEndY)
									{
										g_Platforms[iEditPlatform].iStartX = iTempStartX;
										g_Platforms[iEditPlatform].iStartY = iTempStartY;
										g_Platforms[iEditPlatform].iEndX = iTempEndX;
										g_Platforms[iEditPlatform].iEndY = iTempEndY;
									}
								}
							}
						}
						break;
					}
					else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						short ix = event.button.x / TILESIZE;
						short iy = event.button.y / TILESIZE;

						if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState)
						{
							ClearTilesetTile(&g_Platforms[iEditPlatform].tiles[ix][iy]);
							g_Platforms[iEditPlatform].types[ix][iy] = tile_nonsolid;
						}
						else if(PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
						{
							g_Platforms[iEditPlatform].types[ix][iy] = tile_nonsolid;
						}
						else if(PLATFORM_EDIT_STATE_PATH == iPlatformEditState)
						{
							if(!ignoreclick)
							{
								short iTempStartX = g_Platforms[iEditPlatform].iStartX;
								short iTempStartY = g_Platforms[iEditPlatform].iStartY;
							
								if(ix >= 0 && ix <= MAPWIDTH - iPlatformWidth &&
									iy >= 0 && iy <= MAPHEIGHT - iPlatformHeight)
								{

									//If the start isn't in the same row or column, then move the end point so that it is
									if(iTempStartX != ix &&
										iTempStartY != iy)
									{
										short iXDiff = abs(iTempStartX - ix);
										short iYDiff = abs(iTempStartY - iy);

										//If the x difference is smaller, move it so it lines up
										if(iYDiff > iXDiff)
											iTempStartX = ix;
										else
											iTempStartY = iy;
									}

									//If the start and end are the same point, then don't allow this change
									if(iTempStartX != ix ||
										iTempStartY != iy)
									{
										g_Platforms[iEditPlatform].iStartX = iTempStartX;
										g_Platforms[iEditPlatform].iStartY = iTempStartY;
										g_Platforms[iEditPlatform].iEndX = ix;
										g_Platforms[iEditPlatform].iEndY = iy;
									}
								}
							}
						}
					}
				}
				case SDL_MOUSEMOTION:
				{
					short ix = event.button.x / TILESIZE;
					short iy = event.button.y / TILESIZE;

					if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState)
					{
						if(event.motion.state == SDL_BUTTON(SDL_BUTTON_LEFT) && !ignoreclick)
						{
							for(short i = 0; i < set_tile_cols; i++)
							{
								for(short j = 0; j < set_tile_rows; j++)
								{
									if(ix + i >= 0 && ix + i < MAPWIDTH && iy + j >= 0 && iy + j < MAPHEIGHT)
									{
										TilesetTile * tile = &g_Platforms[iEditPlatform].tiles[ix + i][iy + j];
										SetTilesetTile(tile, set_tile_tileset, set_tile_start_x + i, set_tile_start_y + j);
										g_Platforms[iEditPlatform].types[ix + i][iy + j] = g_tilesetmanager.GetTileset(tile->iID)->GetTileType(tile->iCol, tile->iRow);
									}
								}
							}
						}
						else if(event.motion.state == SDL_BUTTON(SDL_BUTTON_RIGHT))
						{
							ClearTilesetTile(&g_Platforms[iEditPlatform].tiles[ix][iy]);
							g_Platforms[iEditPlatform].types[ix][iy] = tile_nonsolid;
						}
					}
					else if(PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
					{
						if(event.motion.state == SDL_BUTTON(SDL_BUTTON_LEFT) && !ignoreclick)
							g_Platforms[iEditPlatform].types[ix][iy] = set_tiletype;
						else if(event.motion.state == SDL_BUTTON(SDL_BUTTON_RIGHT))
							g_Platforms[iEditPlatform].types[ix][iy] = tile_nonsolid;
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

		//Draw platform editing
		drawmap(false, TILESIZE);

		if(PLATFORM_EDIT_STATE_TEST != iPlatformEditState)
			menu_shade.draw(0, 0);

		if(PLATFORM_EDIT_STATE_SELECT == iPlatformEditState)
		{
			SDL_Rect rp;
			rp.x = 0;
			rp.y = 0;
			rp.w = 192;
			rp.h = 224;

			SDL_BlitSurface(s_platform, &rp, screen, &r);

			menu_font_small.draw(0, 480 - menu_font_small.getHeight(), "Platform Mode: [esc] Exit  [c] Check Paths");
			menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());

			for(int iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
				SDL_BlitSurface(s_platform, &g_Platforms[iPlatform].rIcon[0], screen, &g_Platforms[iPlatform].rIcon[1]);

			if(g_iNumPlatforms < MAX_PLATFORMS)
				SDL_BlitSurface(s_platform, &rNewButton[0], screen, &rNewButton[1]);

		}
		else if(PLATFORM_EDIT_STATE_EDIT == iPlatformEditState || PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState)
		{
			menu_font_small.draw(0, 480 - menu_font_small.getHeight(), "Edit Platform: [esc] Exit  [t] Tiles  [l] Types [del] Delete  [p] Path  [+/-] Velocity");
			draw_platform(iEditPlatform, PLATFORM_EDIT_STATE_TILETYPE == iPlatformEditState);

			rVelocity[3].x = 404 + (g_Platforms[iEditPlatform].iVelocity - 1) * 12;

			SDL_BlitSurface(s_platform, &rVelocity[0], screen, &rVelocity[1]);
			SDL_BlitSurface(s_platform, &rVelocity[2], screen, &rVelocity[3]);

			menu_font_small.drawRightJustified(413, 10, "Slow");
			menu_font_small.draw(594, 10, "Fast");
		}
		else if(PLATFORM_EDIT_STATE_PATH == iPlatformEditState)
		{
			for(short iCol = g_Platforms[iEditPlatform].iStartX; iCol < g_Platforms[iEditPlatform].iStartX + iPlatformWidth; iCol++)
			{
				for(short iRow = g_Platforms[iEditPlatform].iStartY; iRow < g_Platforms[iEditPlatform].iStartY + iPlatformHeight; iRow++)
				{
					spr_platformstarttile.draw(iCol * TILESIZE, iRow * TILESIZE);
				}
			}

			for(short iCol = g_Platforms[iEditPlatform].iEndX; iCol < g_Platforms[iEditPlatform].iEndX + iPlatformWidth; iCol++)
			{
				for(short iRow = g_Platforms[iEditPlatform].iEndY; iRow < g_Platforms[iEditPlatform].iEndY + iPlatformHeight; iRow++)
				{
					spr_platformendtile.draw(iCol * TILESIZE, iRow * TILESIZE);
				}
			}

			menu_font_small.draw(0, 480 - menu_font_small.getHeight(), "Edit Path: [esc] Exit  [LMB] Set Start Point  [RMB] Set End Point");

			rPath[1].x = g_Platforms[iEditPlatform].iStartX * 32;
			rPath[1].y = g_Platforms[iEditPlatform].iStartY * 32;

			rPath[3].x = g_Platforms[iEditPlatform].iEndX * 32;
			rPath[3].y = g_Platforms[iEditPlatform].iEndY * 32;

			SDL_BlitSurface(s_platform, &rPath[0], screen, &rPath[1]);
			SDL_BlitSurface(s_platform, &rPath[2], screen, &rPath[3]);

			if(g_Platforms[iEditPlatform].iStartX != g_Platforms[iEditPlatform].iEndX)
			{
				short iSpotLeft = (g_Platforms[iEditPlatform].iStartX < g_Platforms[iEditPlatform].iEndX ? g_Platforms[iEditPlatform].iStartX + 1 : g_Platforms[iEditPlatform].iEndX + 1);
				short iSpotRight = (g_Platforms[iEditPlatform].iStartX < g_Platforms[iEditPlatform].iEndX ? g_Platforms[iEditPlatform].iEndX - 1 : g_Platforms[iEditPlatform].iStartX - 1);

				for(short iSpot = iSpotLeft; iSpot <= iSpotRight; iSpot++)
				{
					rPath[5].x = iSpot * TILESIZE;
					rPath[5].y = g_Platforms[iEditPlatform].iStartY * TILESIZE;
					SDL_BlitSurface(s_platform, &rPath[4], screen, &rPath[5]);
				}
			}
			else
			{
				short iSpotTop = (g_Platforms[iEditPlatform].iStartY < g_Platforms[iEditPlatform].iEndY ? g_Platforms[iEditPlatform].iStartY + 1 : g_Platforms[iEditPlatform].iEndY + 1);
				short iSpotBottom = (g_Platforms[iEditPlatform].iStartY < g_Platforms[iEditPlatform].iEndY ? g_Platforms[iEditPlatform].iEndY - 1 : g_Platforms[iEditPlatform].iStartY - 1);

				for(short iSpot = iSpotTop; iSpot <= iSpotBottom; iSpot++)
				{
					rPath[5].x = g_Platforms[iEditPlatform].iStartX * TILESIZE;
					rPath[5].y = iSpot * TILESIZE;
					SDL_BlitSurface(s_platform, &rPath[4], screen, &rPath[5]);
				}
			}

			
			/*
			SDL_Rect rFillRect[2];
			rFillRect[0].x = 640 - (iPlatformWidth - 1) * TILESIZE;
			rFillRect[0].y = 0;
			rFillRect[0].w = (iPlatformWidth - 1) * TILESIZE;
			rFillRect[0].h = 480;

			rFillRect[1].x = 0;
			rFillRect[1].y = 480 - (iPlatformHeight - 1) * TILESIZE;
			rFillRect[1].w = 640;
			rFillRect[1].h = (iPlatformHeight - 1) * TILESIZE;

			SDL_FillRect(screen, &rFillRect[0], 0x0);
			SDL_FillRect(screen, &rFillRect[1], 0x0);
			*/
		}
		else if(PLATFORM_EDIT_STATE_TEST == iPlatformEditState)
		{
			menu_font_small.draw(0, 480 - menu_font_small.getHeight(), "Check Paths: [esc] Exit");
			
			g_map.updatePlatforms();
			g_map.drawPlatforms();
		}

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

void draw_platform(short iPlatform, bool fDrawTileTypes)
{	
	for(short iCol = 0; iCol < MAPWIDTH; iCol++)
	{
		for(short iRow = 0; iRow < MAPHEIGHT; iRow++)
		{
			TilesetTile * tile = &g_Platforms[iPlatform].tiles[iCol][iRow];
			
			if(tile->iID >= 0)
			{
				g_tilesetmanager.Draw(screen, tile->iID, 0, tile->iCol, tile->iRow, iCol, iRow);
				//SDL_BlitSurface(g_tilesetmanager.GetTileset(tile->iID)->GetSurface(0), &g_tilesetmanager.rRects[0][tile->iCol][tile->iRow], screen, &bltrect);
			}
			else if(tile->iID == TILESETUNKNOWN)
			{
				SDL_BlitSurface(spr_unknowntile[0].getSurface(), &g_tilesetmanager.rRects[0][0][0], screen, &g_tilesetmanager.rRects[0][iCol][iRow]);
			}

			if(fDrawTileTypes)
			{
				TileType type = g_Platforms[iPlatform].types[iCol][iRow];
				spr_transparenttiles.draw(iCol * TILESIZE, iRow * TILESIZE, (type - 1) * TILESIZE, 0, TILESIZE, TILESIZE);
			}
		}
	}
}

int editor_tiles()
{
	int i, j;
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

	set_tile_drag = false;

	CTileset * tileset = g_tilesetmanager.GetTileset(set_tile_tileset);

	short view_tileset_repeat_direction = -1;
	short view_tileset_repeat_timer = 0;

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
					if(!set_tile_drag)
					{
						if(event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9 && event.key.keysym.sym < SDLK_1 + g_tilesetmanager.GetCount())
						{
							set_tile_tileset = event.key.keysym.sym - SDLK_1;
						}
						else if(event.key.keysym.sym == SDLK_PAGEUP)
						{
							if(set_tile_tileset > 0)
							{
								set_tile_tileset--;
								tileset = g_tilesetmanager.GetTileset(set_tile_tileset);
								view_tileset_x = 0;
								view_tileset_y = 0;
							}
						}
						else if(event.key.keysym.sym == SDLK_PAGEDOWN)
						{
							if(set_tile_tileset < g_tilesetmanager.GetCount() - 1)
							{
								set_tile_tileset++;
								tileset = g_tilesetmanager.GetTileset(set_tile_tileset);
								view_tileset_x = 0;
								view_tileset_y = 0;
							}
						}
						else if(event.key.keysym.sym == SDLK_UP)
						{
							if(view_tileset_y > 0)
							{
								view_tileset_y--;
								view_tileset_repeat_direction = 0;
								view_tileset_repeat_timer = 30;
							}
						}
						else if(event.key.keysym.sym == SDLK_DOWN)
						{
							if(view_tileset_y < g_tilesetmanager.GetTileset(set_tile_tileset)->GetHeight() - 15)
							{
								view_tileset_y++;
								view_tileset_repeat_direction = 1;
								view_tileset_repeat_timer = 30;
							}
						}
						else if(event.key.keysym.sym == SDLK_LEFT)
						{
							if(view_tileset_x > 0)
							{
								view_tileset_x--;
								view_tileset_repeat_direction = 2;
								view_tileset_repeat_timer = 30;
							}
						}
						else if(event.key.keysym.sym == SDLK_RIGHT)
						{
							if(view_tileset_x < g_tilesetmanager.GetTileset(set_tile_tileset)->GetWidth() - 20)
							{
								view_tileset_x++;
								view_tileset_repeat_direction = 3;
								view_tileset_repeat_timer = 30;
							}
						}
						else
						{
							edit_mode = 1;  //change to edit mode using tiles
							set_tile_drag = false;
							return EDITOR_EDIT;
						}
					}
				
					break;
				}

				case SDL_KEYUP:
				{
					if(event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN ||
						event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT)
					{
						view_tileset_repeat_direction = -1;
						view_tileset_repeat_timer = 0;
					}
					
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					short iCol = event.button.x / TILESIZE + view_tileset_x;
					short iRow = event.button.y / TILESIZE + view_tileset_y;

					if(event.button.button == SDL_BUTTON_LEFT)
					{
						CTileset * tileset = g_tilesetmanager.GetTileset(set_tile_tileset);

						if(iCol < tileset->GetWidth() && iRow < tileset->GetHeight())
						{
							set_tile_start_x = iCol;
							set_tile_start_y = iRow;
							set_tile_end_x = set_tile_start_x;
							set_tile_end_y = set_tile_start_y;

							set_tile_drag = true;
						}
					}
					else if(event.button.button == SDL_BUTTON_RIGHT)
					{
						set_type = tileset->IncrementTileType(iCol, iRow);
					}
					
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					short iCol = event.button.x / TILESIZE + view_tileset_x;
					short iRow = event.button.y / TILESIZE + view_tileset_y;

					if(event.button.button == SDL_BUTTON_LEFT)
					{
						if(iCol < tileset->GetWidth() && iRow < tileset->GetHeight())
						{
							set_tile_cols = set_tile_end_x - set_tile_start_x + 1;
							set_tile_rows = set_tile_end_y - set_tile_start_y + 1;

							set_tile_drag = false;
							edit_mode = 1;  //change to edit mode using tiles
							return EDITOR_EDIT;
							
						}
					}
									
					break;
				}

				case SDL_MOUSEMOTION:
				{
					short iCol = event.button.x / TILESIZE + view_tileset_x;
					short iRow = event.button.y / TILESIZE + view_tileset_y;

					if(iCol < tileset->GetWidth() && iRow < tileset->GetHeight())
					{
						if(event.motion.state == SDL_BUTTON(SDL_BUTTON_LEFT))
						{
							if(iCol < set_tile_start_x)
								set_tile_start_x = iCol;
							
							if(iCol > set_tile_end_x)
								set_tile_end_x = iCol;

							if(iRow < set_tile_start_y)
								set_tile_start_y = iRow;
							
							if(iRow > set_tile_end_y)
								set_tile_end_y = iRow;
						}
						else if(event.motion.state == SDL_BUTTON(SDL_BUTTON_RIGHT))
						{
							tileset->SetTileType(iCol, iRow, set_type);
						}
					}
					
					break;
				}
				

				default:
					break;
			}
		}

		//Allow auto-scrolling of tilesets when the arrow keys are held down
		if(view_tileset_repeat_direction >= 0 && view_tileset_repeat_timer > 0)
		{
			if(--view_tileset_repeat_timer <= 0)
			{
				view_tileset_repeat_timer = 5;

				if(view_tileset_repeat_direction == 0 && view_tileset_y > 0)
				{
					view_tileset_y--;
				}
				else if(view_tileset_repeat_direction == 1 && view_tileset_y < g_tilesetmanager.GetTileset(set_tile_tileset)->GetHeight() - 15)
				{
					view_tileset_y++;
				}
				else if(view_tileset_repeat_direction == 2 && view_tileset_x > 0)
				{
					view_tileset_x--;
				}
				else if(view_tileset_repeat_direction == 3 && view_tileset_x < g_tilesetmanager.GetTileset(set_tile_tileset)->GetWidth() - 20)
				{
					view_tileset_x++;
				}
			}
		}

		
		//drawmap(false, TILESIZE);
		//menu_shade.draw(0, 0);

		SDL_FillRect(screen, NULL, 0x0);

		SDL_Rect rectSrc;
		rectSrc.x = view_tileset_x << 5;
		rectSrc.y = view_tileset_y << 5;
		rectSrc.w = tileset->GetWidth() > 20 ? 640 : tileset->GetWidth() << 5;
		rectSrc.h = tileset->GetHeight() > 15 ? 480 : tileset->GetHeight() << 5;

		SDL_BlitSurface(g_tilesetmanager.GetTileset(set_tile_tileset)->GetSurface(0), &rectSrc, screen, &r);
		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
		
		for(i = view_tileset_x; i < view_tileset_x + 20 && i < tileset->GetWidth(); i++)
		{
			for(j = view_tileset_y; j < view_tileset_y + 15 && j < tileset->GetHeight(); j++)
			{
				TileType t = tileset->GetTileType(i, j);
				if(t != tile_nonsolid)
					spr_tiletypes[t-1].draw((i - view_tileset_x) << 5, (j - view_tileset_y) << 5);
			}
		}

		if(set_tile_drag)
		{
			for(i = set_tile_start_x; i <= set_tile_end_x; i++)
			{
				for(j = set_tile_start_y; j <= set_tile_end_y; j++)
				{
					spr_selectedtile.draw((i - view_tileset_x) << 5, (j - view_tileset_y) << 5);
				}
			}
		}
		/*
		
		//Test code to help convert old tilesets into new tilesets

		menu_font_small.drawRightJustified(640, 0, "%d", iCurrentTile);
		spr_OldTileSet.draw(576, 224, iCurrentTile % 20 * 32, iCurrentTile / 20 * 32, 32, 32);
		

		short iTileCol = iConvertedTile[iCurrentTile] % 32;
		short iTileRow = iConvertedTile[iCurrentTile] / 32;

		if(selected_tileset == 0 && iTileCol < 16 && iTileRow < 15)
			spr_selectedtile.draw(iTileCol * 32, iTileRow * 32, 0, 0, 32, 32);
		else if(selected_tileset == 1 && iTileCol >= 16 && iTileRow < 15)
			spr_selectedtile.draw((iTileCol - 16) * 32, iTileRow * 32, 0, 0, 32, 32);
		else if(selected_tileset == 2 && iTileCol < 16 && iTileRow >= 15)
			spr_selectedtile.draw(iTileCol * 32, (iTileRow - 15) * 32, 0, 0, 32, 32);
		else if(selected_tileset == 3 && iTileCol >= 16 && iTileRow >= 15)
			spr_selectedtile.draw((iTileCol - 16) * 32, (iTileRow - 15) * 32, 0, 0, 32, 32);


		*/


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

int editor_blocks()
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
						short set_block_x = event.button.x / TILESIZE;
						short set_block_y = event.button.y / TILESIZE;

						//Set the selected block to one of the interaction blocks
						if(set_block_y == 0 && set_block_x >= 0 && set_block_x <= 6)
							set_block = set_block_x;
						else if(set_block_y == 0 && set_block_x >= 7 && set_block_x <= 10)
							set_block = set_block_x + 8;
						else if(set_block_y >= 1 && set_block_y <= 2 && set_block_x >= 0 && set_block_x <= 3)
						{  //set the selected block to an on/off switch block
							
							set_block = set_block_x + 7;
							g_map.iSwitches[set_block_x] = set_block_y - 1;
						}
						else if(set_block_y == 3 && set_block_x >= 0 && set_block_x <= 3)
						{  //set the selected block to a switch block
							
							set_block = set_block_x + 11;
						}

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
		
		SDL_Rect rSrc = {0, 0, 224, 32};
		SDL_Rect rDst = {0, 0, 224, 32};

		SDL_BlitSurface(spr_blocks[0].getSurface(), &rSrc, screen, &rDst);

		SDL_Rect rOnOffSrc = {224, 0, 128, 64};
		SDL_Rect rOnOffDst = {0, 32, 128, 64};

		SDL_BlitSurface(spr_blocks[0].getSurface(), &rOnOffSrc, screen, &rOnOffDst);

		SDL_Rect rOnOffBlockSrc = {352, 0, 128, 32};
		SDL_Rect rOnOffBlockDst = {0, 96, 128, 32};

		SDL_BlitSurface(spr_blocks[0].getSurface(), &rOnOffBlockSrc, screen, &rOnOffBlockDst);

		SDL_Rect rBlocksRow2Src = {0, 32, 128, 32};
		SDL_Rect rBlocksRow2Dst = {224, 0, 128, 32};

		SDL_BlitSurface(spr_blocks[0].getSurface(), &rBlocksRow2Src, screen, &rBlocksRow2Dst);

		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
				
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


int editor_mapitems()
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
					edit_mode = 7;
					return EDITOR_EDIT;
				break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						short set_item_x = event.button.x / TILESIZE;
						short set_item_y = event.button.y / TILESIZE;

						//Set the selected block to one of the interaction blocks
						if(set_item_y == 0 && set_item_x >= 0 && set_item_x <= 2)
							set_mapitem = set_item_x;

						edit_mode = 7;

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
		
		spr_mapitems[0].draw(0, 0, 0, 0, 96, 32);

		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
				
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


int editor_tiletype()
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
				{	
					edit_mode = 6;
					return EDITOR_EDIT;
				
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						if(event.button.x < 192 && event.button.y < 32)
						{
							set_tiletype = (TileType)((event.button.x / TILESIZE) + 1);

							edit_mode = 6;
							ignoreclick = true;
							return EDITOR_EDIT;
						}
					}
				
					break;
				}
				
				default:
					break;
			}
		}

		
		drawmap(false, TILESIZE);
		menu_shade.draw(0, 0);
		
		spr_transparenttiles.draw(0, 0);

		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
				
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

int editor_backgrounds()
{
	bool done = false;
	short iPage = backgroundlist.GetCurrentIndex() / 16;

	SDL_Surface * sBackgrounds[16];
	SDL_Rect rSrc = {0, 0, 160, 120};

	SDL_Rect rDst[16];

	for(short iRectY = 0; iRectY < 4; iRectY++)
	{
		for(short iRectX = 0; iRectX < 4; iRectX++)
		{
			rDst[iRectY * 4 + iRectX].x = iRectX * 160;
			rDst[iRectY * 4 + iRectX].y = iRectY * 120;
			rDst[iRectY * 4 + iRectX].w = 160;
			rDst[iRectY * 4 + iRectX].h = 120;
		}
	}

	for(short iSurface = 0; iSurface < 16; iSurface++)
		sBackgrounds[iSurface] = SDL_CreateRGBSurface(screen->flags, 160, 120, 16, 0, 0, 0, 0);

	LoadBackgroundPage(sBackgrounds, iPage);

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
					if(event.key.keysym.sym == SDLK_ESCAPE)
					{
						done = true;
					}
					else if(event.key.keysym.sym == SDLK_PAGEDOWN || event.key.keysym.sym == SDLK_DOWN)
					{
						if((iPage + 1) * 16 < backgroundlist.GetCount())
						{
							iPage++;
							LoadBackgroundPage(sBackgrounds, iPage);
						}
					}
					else if(event.key.keysym.sym == SDLK_PAGEUP || event.key.keysym.sym == SDLK_UP)
					{
						if((iPage - 1) * 16 >= 0)
						{
							iPage--;
							LoadBackgroundPage(sBackgrounds, iPage);
						}
					}
					
				break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT)
					{
						for(short iBackground = 0; iBackground < 16; iBackground++)
						{
							if(iPage * 16 + iBackground >= backgroundlist.GetCount())
								break;

							if(event.button.x >= rDst[iBackground].x && event.button.x < rDst[iBackground].x + rDst[iBackground].w && 
								event.button.y >= rDst[iBackground].y && event.button.y < rDst[iBackground].y + rDst[iBackground].h)
							{
								done = true;
								backgroundlist.SetCurrent(iPage * 16 + iBackground);

								spr_background.init(convertPath(backgroundlist.current_name()));
								strcpy(g_map.szBackgroundFile, getFileFromPath(backgroundlist.current_name()).c_str());

								if(event.button.button == SDL_BUTTON_LEFT)
								{
									//Set music to background default
									for(short iCategory = 0; iCategory < MAXMUSICCATEGORY; iCategory++)
									{
										if(!strncmp(g_szMusicCategoryNames[iCategory], g_map.szBackgroundFile, strlen(g_szMusicCategoryNames[iCategory])))
										{
											g_map.musicCategoryID = iCategory;
											break;
										}
									}
								}
							}
						}
					}
				break;

				default:
					break;
			}
		}

		SDL_Rect rect = {0, 0, 640, 480};
		SDL_FillRect(screen, &rect, 0x0);

		for(short iBackground = 0; iBackground < 16; iBackground++)
		{
			if(iPage * 16 + iBackground >= backgroundlist.GetCount())
				break;

			SDL_BlitSurface(sBackgrounds[iBackground], &rSrc, screen, &rDst[iBackground]);
		}

		menu_font_small.draw(0,480-menu_font_small.getHeight() * 2, "[Page Up] next page, [Page Down] previous page");
		menu_font_small.draw(0,480-menu_font_small.getHeight(), "[LMB] choose background with music category, [RMB] choose just background");

		int x, y;

		SDL_GetMouseState(&x, &y);

		int iID = x / 160 + y / 120 * 4 + iPage * 16;

		if(iID < backgroundlist.GetCount())
			menu_font_small.draw(0, 0, backgroundlist.GetIndex(iID));

		SDL_Flip(screen);

		int delay = WAITTIME - (SDL_GetTicks() - framestart);
		if(delay < 0)
			delay = 0;
		else if(delay > WAITTIME)
			delay = WAITTIME;
		
		SDL_Delay(delay);
	}

	for(short iSurface = 0; iSurface < 16; iSurface++)
		SDL_FreeSurface(sBackgrounds[iSurface]);

	return EDITOR_EDIT;
}

int editor_animation()
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
					edit_mode = 8;
					return EDITOR_EDIT;
				break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						short set_animation_x = event.button.x / TILESIZE;
						short set_animation_y = event.button.y / TILESIZE;

						if(set_animation_y == 0 && set_animation_x >= 0 && set_animation_x < TILEANIMATIONSIZE)
							set_animation = set_animation_x;

						edit_mode = 8;

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

		for(short iTile = 0; iTile < TILEANIMATIONSIZE; iTile++)
		{
			short iDestX = (iTile % 16) << 5;
			short iDestY = (iTile / 16) << 5;
			short iSrcX = (iTile / 16) << 7;
			short iSrcY = (iTile % 16) << 5;

			spr_tileanimation[0].draw(iDestX, iDestY, iSrcX, iSrcY, TILESIZE, TILESIZE);
		}

		menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
				
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

void LoadBackgroundPage(SDL_Surface ** sBackgrounds, short iPage)
{
	SDL_Rect srcRectBackground = {0, 0, 640, 480};
	SDL_Rect dstRectBackground = {0, 0, 160, 120};

	for(short iIndex = 0; iIndex < 16; iIndex++)
	{
		const char * szFileName = backgroundlist.GetIndex(iPage * 16 + iIndex);

		if(!szFileName)
			return;

		SDL_Surface * temp = IMG_Load(szFileName);

		SDL_Surface * sBackground = SDL_DisplayFormat(temp);
		if(!sBackground)
		{
			printf("ERROR: Couldn't convert thumbnail background to diplay pixel format: %s\n", SDL_GetError());
			return;
		}

		SDL_FreeSurface(temp);

		if(SDL_SoftStretch(sBackground, &srcRectBackground, sBackgrounds[iIndex], &dstRectBackground) < 0)
		{
			fprintf(stderr, "SDL_SoftStretch error: %s\n", SDL_GetError());
			return;
		}

		SDL_FreeSurface(sBackground);
	}
}

int display_help()
{
	//unsigned int currentChar = 0;

	drawmap(false, TILESIZE);
	menu_shade.draw(0, 0);
	//menu_font_large.drawCentered(320, 15, "Help");
	
	int offsety = 30;
	int offsetx = 20;
	menu_font_small.draw(offsetx, offsety, "Modes:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[t] - Tile Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[i] - Block Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[o] - Map Item Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[w] - Warp Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[m] - Move Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[l] - Tile Type Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[p] - Platform Mode");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[x] - No Player Spawn Area");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[z] - No Item Spawn Area");
	offsety += menu_font_small.getHeight() + 20;

	menu_font_small.draw(offsetx, offsety, "Layers:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[v] - Hide Blocks");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[y] - Select Active Tile Layer");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[u] - Hide Inactive Tile Layers");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[end] - Optimize Layers");
	offsety += menu_font_small.getHeight() + 20;

	menu_font_small.draw(offsetx, offsety, "File:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[n] - New Map");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[s] - Save Map");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[shift] + [s] - Save As");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[f] - Find Map");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[shift] + [f] - New Search");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[pageup] - Go To Previous Map");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[pagedown] - Go To Next Map");

	offsetx = 305;
	offsety = 30;

	menu_font_small.draw(offsetx, offsety, "Tile, Warp and Block Modes:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[Left Mouse Button] - Place Item");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[Right Mouse Button] - Remove Item");
	offsety += menu_font_small.getHeight() + 20;

	menu_font_small.draw(offsetx, offsety, "Move Mode:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[Left Mouse Button] - Select Area");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[Right Mouse Button] - Unselect Area");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "Select And Drag - Move Selections");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "Hold [shift] - Multiple Selections");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "Hold [ctrl] - Freehand Selections");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[delete] - Delete Selection");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[c] - Copy Selection");
	offsety += menu_font_small.getHeight() + 20;

	menu_font_small.draw(offsetx, offsety, "Platforms:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[p] - Path");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[+/-] - Change Speed");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[delete] - Delete");
	offsety += menu_font_small.getHeight() + 20;

	menu_font_small.draw(offsetx, offsety, "Miscellaneous:");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[b] - Background Thumbnails");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[g] - Change Backgrounds");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[r] - Change Music Category");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[e] - Change Floating Eyecandy");
	offsety += menu_font_small.getHeight() + 2;
	menu_font_small.draw(offsetx, offsety, "[ctrl] + [delete] - Clear All");
	offsety += menu_font_small.getHeight() + 2;


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
		save_map(convertPath(strcat(strcat(mapLocation, fileName),".map")));
		maplist.add(strcat(fileName, ".map"));
		maplist.find(fileName);
		loadcurrentmap();
	}

	return 0;
}

bool dialog(char * title, char * instructions, char * input, int inputsize)
{	
	unsigned int currentChar = 0;

	drawmap(false, TILESIZE);
	menu_shade.draw(0, 0);
	spr_dialog.draw(224, 176);
	menu_font_large.drawCentered(320, 200, title);
	menu_font_small.draw(240, 235, instructions);
	menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
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
							menu_font_large.drawCentered(320, 200, title);
							menu_font_small.draw(240, 235, instructions);
							menu_font_small.draw(240, 255, input);
							menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
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
							menu_font_large.drawCentered(320, 200, title);
							menu_font_small.draw(240, 235, instructions);
							menu_font_small.draw(240, 255, input);
							menu_font_small.drawRightJustified(640, 0, maplist.currentFilename());
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

		if(maplist.find(findstring))
		{
			loadcurrentmap();
		}
	}

	return 0;
}

int clearMap()
{ 
	g_map.clearMap();
	g_iNumPlatforms = 0;


	printf("Map Cleared\n");
	return 0;
}

#ifdef _DEBUG
void convertAll()
{
	for(int k = 0; k < maplist.GetCount(); k++)
	{
		//g_map.convertMap();
		g_map.saveMap(maplist.currentFilename());
		maplist.next(false);
		g_map.loadMap(maplist.currentFilename(), read_type_full);
	}
}
#endif


void loadcurrentmap()
{
	g_map.loadMap(maplist.currentFilename(), read_type_full);

	char filename[128];
	sprintf(filename, "gfx/packs/Classic/backgrounds/%s", g_map.szBackgroundFile);
	std::string path = convertPath(filename);
	backgroundlist.SetCurrentName(filename);
	
	if(!File_Exists(path))
	{
		path = convertPath("gfx/packs/Classic/backgrounds/Land_Classic.png");
		backgroundlist.SetCurrentName("gfx/packs/Classic/backgrounds/Land_Classic.png");
	}
	
	spr_background.init(path);

	g_iNumPlatforms = g_map.iNumPlatforms;

	for(short iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
	{
		for(short iCol = 0; iCol < MAPWIDTH; iCol++)
		{
			for(short iRow = 0; iRow < MAPHEIGHT; iRow++)
			{
				if(iCol < g_map.platforms[iPlatform]->iTileWidth && iRow < g_map.platforms[iPlatform]->iTileHeight)
				{
					CopyTilesetTile(&g_Platforms[iPlatform].tiles[iCol][iRow], &g_map.platforms[iPlatform]->iTileData[iCol][iRow]);
					g_Platforms[iPlatform].types[iCol][iRow] = g_map.platforms[iPlatform]->iTileType[iCol][iRow];
				}
				else
				{
					ClearTilesetTile(&g_Platforms[iPlatform].tiles[iCol][iRow]);
					g_Platforms[iPlatform].types[iCol][iRow] = tile_nonsolid;
				}
			}
		}

		g_Platforms[iPlatform].iVelocity = (int)(g_map.platforms[iPlatform]->pPath->fVelocity * 4.0f);
		g_Platforms[iPlatform].iStartX = (int)(g_map.platforms[iPlatform]->pPath->fStartX - g_map.platforms[iPlatform]->iHalfWidth + 1) / TILESIZE;
		g_Platforms[iPlatform].iStartY = (int)(g_map.platforms[iPlatform]->pPath->fStartY - g_map.platforms[iPlatform]->iHalfHeight + 1) / TILESIZE;
		g_Platforms[iPlatform].iEndX = (int)(g_map.platforms[iPlatform]->pPath->fEndX - g_map.platforms[iPlatform]->iHalfWidth + 1) / TILESIZE;
		g_Platforms[iPlatform].iEndY = (int)(g_map.platforms[iPlatform]->pPath->fEndY - g_map.platforms[iPlatform]->iHalfHeight + 1) / TILESIZE;
	}
}

void SetPlatformToDefaults(short iPlatform)
{
	for(short iCol = 0; iCol < MAPWIDTH; iCol++)
	{
		for(short iRow = 0; iRow < MAPHEIGHT; iRow++)
		{
			ClearTilesetTile(&g_Platforms[iPlatform].tiles[iCol][iRow]);
			g_Platforms[iPlatform].types[iCol][iRow] = tile_nonsolid;
		}
	}

	g_Platforms[iPlatform].iVelocity = 4;
	g_Platforms[iPlatform].iStartX = 0;
	g_Platforms[iPlatform].iStartY = 0;
	g_Platforms[iPlatform].iEndX = 1;
	g_Platforms[iPlatform].iEndY = 0;
}

int savecurrentmap()
{
	save_map(maplist.currentFilename());
	return 0;
}

void insert_platforms_into_map()
{
	//First take the created platforms and move them into the actual map
	g_map.clearPlatforms();

	g_map.iNumPlatforms = g_iNumPlatforms;
	g_map.platforms = new MovingPlatform*[g_iNumPlatforms];

	for(short iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
	{
		short iTop, iLeft, iWidth, iHeight;
		CalculatePlatformDims(iPlatform, &iLeft, &iTop, &iWidth, &iHeight);

		TilesetTile ** tiles = new TilesetTile*[iWidth];
		TileType ** types = new TileType*[iWidth];

		for(short iCol = 0; iCol < iWidth; iCol++)
		{
			tiles[iCol] = new TilesetTile[iHeight];
			types[iCol] = new TileType[iHeight];

			for(short iRow = 0; iRow < iHeight; iRow++)
			{
				CopyTilesetTile(&tiles[iCol][iRow], &g_Platforms[iPlatform].tiles[iCol + iLeft][iRow + iTop]);
				types[iCol][iRow] = g_Platforms[iPlatform].types[iCol + iLeft][iRow + iTop];
			}
		}
	
		float fVelocity = (float)g_Platforms[iPlatform].iVelocity * 0.26f;

		float fStartX = (float)(g_Platforms[iPlatform].iStartX * TILESIZE) + (float)(iWidth * TILESIZE) / 2.0f;
		float fStartY = (float)(g_Platforms[iPlatform].iStartY * TILESIZE) + (float)(iHeight * TILESIZE) / 2.0f;
		float fEndX = (float)(g_Platforms[iPlatform].iEndX * TILESIZE) + (float)(iWidth * TILESIZE) / 2.0f;
		float fEndY = (float)(g_Platforms[iPlatform].iEndY * TILESIZE) + (float)(iHeight * TILESIZE) / 2.0f;
				
		MovingPlatformPath * path = new MovingPlatformPath(fVelocity, fStartX, fStartY, fEndX, fEndY, false);

		g_map.platforms[iPlatform] = new MovingPlatform(tiles, types, iWidth, iHeight, path, true, 0, false);
	}
}

void save_map(const std::string &file)
{
	insert_platforms_into_map();

	//Then save the rest of the map
	g_map.saveMap(file);
}

void CalculatePlatformDims(short iPlatform, short * ix, short * iy, short * iw, short * ih)
{
	short iTop = MAPHEIGHT, iRight = -1, iBottom = -1, iLeft = MAPWIDTH;
	//Calculate the height and width of the platform
	for(short iCol = 0; iCol < MAPWIDTH; iCol++)
	{
		for(short iRow = 0; iRow < MAPHEIGHT; iRow++)
		{
			if(g_Platforms[iPlatform].tiles[iCol][iRow].iID != TILESETNONE)
			{
				if(iTop > iRow)
					iTop = iRow;

				if(iLeft > iCol)
					iLeft = iCol;

				if(iBottom < iRow)
					iBottom = iRow;

				if(iRight < iCol)
					iRight = iCol;
			}
		}
	}

	short iWidth = iRight - iLeft + 1;
	short iHeight = iBottom - iTop + 1;

	if(iRight == -1 || iBottom == -1)
	{
		iWidth = 1;
		iHeight = 1;
		iLeft = 0;
		iTop = 0;
	}

	*ix = iLeft;
	*iy = iTop;

	*iw = iWidth;
	*ih = iHeight;

	//Also adjust path if it is outside the allowed area
	short iTempStartX = g_Platforms[iPlatform].iStartX;
	short iTempStartY = g_Platforms[iPlatform].iStartY;
	short iTempEndX = g_Platforms[iPlatform].iEndX;
	short iTempEndY = g_Platforms[iPlatform].iEndY;

	if(iTempStartX > MAPWIDTH - iWidth)
		iTempStartX = MAPWIDTH - iWidth;

	if(iTempStartY > MAPHEIGHT - iHeight)
		iTempStartY = MAPHEIGHT - iHeight;

	if(iTempEndX > MAPWIDTH - iWidth)
		iTempEndX = MAPWIDTH - iWidth;

	if(iTempEndY > MAPHEIGHT - iHeight)
		iTempEndY = MAPHEIGHT - iHeight;

	if(iTempStartX == iTempEndX && iTempStartY == iTempEndY)
	{
		iTempStartX = 0;
		iTempStartY = 0;

		if(MAPWIDTH - iWidth > MAPHEIGHT - iHeight)
		{
			iTempEndX = MAPWIDTH - iWidth;
			iTempEndY = 0;
		}
		else
		{
			iTempEndX = 0;
			iTempEndY = MAPHEIGHT - iHeight;
		}
	}

	g_Platforms[iPlatform].iStartX = iTempStartX;
	g_Platforms[iPlatform].iStartY = iTempStartY;
	g_Platforms[iPlatform].iEndX = iTempEndX;
	g_Platforms[iPlatform].iEndY = iTempEndY;
}

int findcurrentstring()
{
	if(findstring[0] != '\0')
	{
		if(maplist.find(findstring))
		{
			loadcurrentmap();
		}
	}

	return 0;
}

int newmap()
{
	char fileName[FILEBUFSIZE] = "";
	char mapLocation[FILEBUFSIZE] = "maps/";

	if(dialog("New Map", "Enter name:", fileName, 64))
	{
		g_map.clearMap();
		g_map.clearPlatforms();
		g_map.saveMap(convertPath(strcat(strcat(mapLocation, fileName),".map")));
		maplist.add(strcat(fileName, ".map"));
		maplist.find(fileName);
		loadcurrentmap();
	}

	return 0;
}

void resetselectedtiles()
{
	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			selectedtiles[j][k] = false;
		}
	}
}

void copymoveselection()
{
	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			moveselectedtiles[j][k] = selectedtiles[j][k];
		}
	}
	
	copiedlayer = selected_layer;
}

void pastemoveselection(int movex, int movey)
{
	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			if(moveselectedtiles[j][k])
			{
				if(j + movex >= 0 && j + movex < MAPWIDTH && 
					k + movey >= 0 && k + movey < MAPHEIGHT)
				{
					selectedtiles[j + movex][k + movey] = moveselectedtiles[j][k];
				}
			}
		}
	}
}

bool copyselectedtiles()
{
	//Copy the selected tiles and remove tiles from map
	bool ret = false;
	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			if(selectedtiles[j][k])
			{
				ret = true;
				for(short iLayer = 0; iLayer < MAPLAYERS; iLayer++)
				{
					CopyTilesetTile(&copiedtiles[j][k].tile[iLayer], &g_map.mapdata[j][k][iLayer]);
				}

				copiedtiles[j][k].block = g_map.objectdata[j][k];

				copiedtiles[j][k].warp.connection = g_map.warpdata[j][k].connection;
				copiedtiles[j][k].warp.direction = g_map.warpdata[j][k].direction;
				copiedtiles[j][k].warp.id = g_map.warpdata[j][k].id;

				copiedtiles[j][k].tiletype = g_map.mapdatatop[j][k];
				
				for(short iType = 0; iType < NUMSPAWNAREATYPES; iType++)
					copiedtiles[j][k].nospawn[iType] = g_map.nospawn[iType][j][k];

				copiedtiles[j][k].item = -1;
				for(short iMapItem = 0; iMapItem < g_map.iNumMapItems; iMapItem++)
				{
					if(g_map.mapitems[iMapItem].ix == j && g_map.mapitems[iMapItem].iy == k)
					{
						copiedtiles[j][k].item = g_map.mapitems[iMapItem].itype;
						break;
					}
				}
			}
		}
	}

	copiedlayer = selected_layer;

	return ret;
}

void clearselectedmaptiles()
{
	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			if(selectedtiles[j][k])
			{
				if(view_only_layer)
				{
					g_map.mapdata[j][k][selected_layer].iID = TILESETNONE;
				}
				else
				{
					for(short iLayer = 0; iLayer < MAPLAYERS; iLayer++)
						g_map.mapdata[j][k][iLayer].iID = TILESETNONE;
					
					g_map.objectdata[j][k] = BLOCKSETSIZE;

					g_map.warpdata[j][k].connection = -1;
					g_map.warpdata[j][k].direction = -1;
					g_map.warpdata[j][k].id = -1;

					for(short iType = 0; iType < NUMSPAWNAREATYPES; iType++)
						g_map.nospawn[iType][j][k] = false;

					RemoveMapItemAt(j, k);
				}

				UpdateTileType(j, k);
			}
		}
	}
}

void replacetile(short * iDstTile, short iSrcTile, short iType)
{
	if(move_replace)
	{
		*iDstTile = iSrcTile;
	}
	else
	{
		if(iType == 1 && iSrcTile != BLOCKSETSIZE)
			*iDstTile = iSrcTile;
		else if(iType == 2)
			*iDstTile = iSrcTile;
	}
}

void copytilesettile(TilesetTile * dst, TilesetTile * src)
{
	dst->iID = src->iID;
	dst->iCol = src->iCol;
	dst->iRow = src->iRow;
}

void replacetile(TilesetTile * dstTile, TilesetTile * srcTile)
{
	if(move_replace)
	{
		copytilesettile(dstTile, srcTile);
	}
	else
	{
		if(srcTile->iID != TILESETNONE)
			copytilesettile(dstTile, srcTile);
	}
}

void pasteselectedtiles(int movex, int movey)
{
	//Paste the tiles into their new location
	//Removing tiles that hang over the edges of the map
	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			if(selectedtiles[j][k])
			{
				if(j + movex >= 0 && j + movex < MAPWIDTH && 
					k + movey >= 0 && k + movey < MAPHEIGHT)
				{
					short iNewX = j + movex;
					short iNewY = k + movey;

					if(view_only_layer)
					{
						replacetile(&g_map.mapdata[iNewX][iNewY][selected_layer], &copiedtiles[j][k].tile[copiedlayer]);
					}
					else
					{
						for(short iLayer = 0; iLayer < MAPLAYERS; iLayer++)
							replacetile(&g_map.mapdata[iNewX][iNewY][iLayer], &copiedtiles[j][k].tile[iLayer]);

						replacetile(&g_map.objectdata[iNewX][iNewY], copiedtiles[j][k].block, 1);

						replacetile(&g_map.warpdata[iNewX][iNewY].connection, copiedtiles[j][k].warp.connection, copiedtiles[j][k].warp.connection == -1 ? -1 : 2);
						replacetile(&g_map.warpdata[iNewX][iNewY].direction, copiedtiles[j][k].warp.direction, copiedtiles[j][k].warp.connection == -1 ? -1 : 2);
						replacetile(&g_map.warpdata[iNewX][iNewY].id, copiedtiles[j][k].warp.id, copiedtiles[j][k].warp.connection == -1 ? -1 : 2);

						if(move_replace)
							g_map.mapdatatop[iNewX][iNewY] = copiedtiles[j][k].tiletype;
						else
							UpdateTileType(j, k);
						
						for(short iType = 0; iType < NUMSPAWNAREATYPES; iType++)
							g_map.nospawn[iType][iNewX][iNewY] = copiedtiles[j][k].nospawn[iType];

						if(move_replace)
							RemoveMapItemAt(j, k);

						if(g_map.iNumMapItems < MAXMAPITEMS && copiedtiles[j][k].item >= 0)
						{
							if(!move_replace)
								RemoveMapItemAt(j, k);

							MapItem * mapitem = &g_map.mapitems[g_map.iNumMapItems];
							mapitem->itype = copiedtiles[j][k].item;
							mapitem->ix = iNewX;
							mapitem->iy = iNewY;

							g_map.iNumMapItems++;
						}
					}
				}
			}
		}
	}
}

void getcenterselection(int * x, int * y)
{
	int left = -1;
	int top = -1;
	int right = -1;
	int bottom = -1;

	for(int k = 0; k < MAPHEIGHT; k++)
	{
		for(int j = 0; j < MAPWIDTH; j++)
		{
			if(selectedtiles[j][k])
			{
				if(left == -1)
					left = j;

				if(top == -1)
					top = k;

				right = j;
				bottom = k;
			}
		}
	}

	if(right == -1)
	{
		*x = 0;
		*y = 0;
		return;
	}

	*x = ((right - left) >> 1) + left;
	*y = ((bottom - top) >> 1) + top;
}

//take screenshots in full and thumbnail sizes
void takescreenshot()
{
	short iTileSizes[3] = {TILESIZE, PREVIEWTILESIZE, THUMBTILESIZE};
	SDL_Surface * old_screen = screen;

	for(short iScreenshotSize = 0; iScreenshotSize < 3; iScreenshotSize++)
	{
		short iTileSize = iTileSizes[iScreenshotSize];

		SDL_Surface * screenshot = SDL_CreateRGBSurface(old_screen->flags, iTileSize * 20, iTileSize * 15, old_screen->format->BitsPerPixel, 0, 0, 0, 0);
		blitdest = screenshot;
		screen = screenshot;
		drawmap(true, iTileSize);

		//Draw platforms to screenshot
		for(short iPlatform = 0; iPlatform < g_iNumPlatforms; iPlatform++)
		{
			for(short iPlatformX = 0; iPlatformX < g_map.platforms[iPlatform]->iTileWidth; iPlatformX++)
			{
				for(short iPlatformY = 0; iPlatformY < g_map.platforms[iPlatform]->iTileHeight; iPlatformY++)
				{
					TilesetTile * tile = &g_Platforms[iPlatform].tiles[iPlatformX][iPlatformY];

					if(tile->iID >= 0)
					{
						g_tilesetmanager.Draw(blitdest, tile->iID, iScreenshotSize, tile->iCol, tile->iRow, g_Platforms[iPlatform].iStartX + iPlatformX, g_Platforms[iPlatform].iStartY + iPlatformY);
						//SDL_BlitSurface(spr_maptiles[iScreenshotSize].getSurface(), &g_tilesetmanager.rRects[iScreenshotSize][tile->iCol][tile->iRow], blitdest, &g_tilesetmanager.rRects[iScreenshotSize][g_Platforms[iPlatform].iStartX + iPlatformX][g_Platforms[iPlatform].iStartY + iPlatformY]);
					}
					else if(tile->iID == TILESETUNKNOWN)
					{
						//Draw unknown tile
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
		GetNameFromFileName(pszSaveFile, maplist.currentFilename());
		
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
}

