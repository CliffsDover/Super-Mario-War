#ifndef __WORLD_H_
#define __WORLD_H_

#include <queue>

class MI_World;

struct WorldMapTile
{
	//Id is used for searching for AI
	short iID;

	short iType;
	short iBackgroundStyle;
	short iBackgroundSprite;
	short iForegroundSprite;

	short iConnectionType;
	bool fConnection[4];

	short iWarp;

	bool fCompleted;
	bool fAnimated;

	short iCol;
	short iRow;
};

class WorldMovingObject
{
	public:
		WorldMovingObject();
		virtual ~WorldMovingObject();

		void Init(short iCol, short iRow, short iSprite, short iInitialDirection);
		virtual void Move(short iDirection);
		virtual bool Update();
		void Draw(short iWorldOffsetX, short iWorldOffsetY);
		void FaceDirection(short iDirection);
		void SetPosition(short iCol, short iRow);

	protected:

		short ix;
		short iy;
		short iCurrentTileX;
		short iCurrentTileY;
		short iDestTileX;
		short iDestTileY;
			
		short iState;
		short iDrawSprite;
		short iDrawDirection;
		short iAnimationFrame;
		short iAnimationTimer;
	
	friend class WorldMap;
};

class WorldPlayer : public WorldMovingObject
{
	public:
	
		WorldPlayer();
		~WorldPlayer();

		void Init(short iCol, short iRow);

		void SetSprite(short iPlayer);
		void Draw(short iWorldOffsetX, short iWorldOffsetY);

	friend class WorldMap;

};

class WorldVehicle : public WorldMovingObject
{
	public:
		
		WorldVehicle();
		~WorldVehicle();

		void Init(short iCol, short iRow, short iAction, short iSprite, short iMinMoves, short iMaxMoves, bool fSpritePaces, short iInitialDirection);
		void Move();
		
		bool Update();
		void Draw(short iWorldOffsetX, short iWorldOffsetY, bool fVehiclesSleeping);

	private:

		void SetNextDest();
					
		SDL_Rect srcRects[5];

		short iMinMoves;
		short iMaxMoves;
		short iNumMoves;

		short iActionId;

		bool fEnabled;

		bool fSpritePaces;
		short iPaceOffset;
		short iPaceTimer;

	friend class WorldMap;
};

class WorldWarp
{
	public:
		WorldWarp();
		void Init(short col1, short row1, short col2, short row2);
		void GetOtherSide(short iCol, short iRow, short * iOtherCol, short * iOtherRow);

	private:
		short iCol1, iRow1;
		short iCol2, iRow2;

	friend class WorldMap;
};

class WorldMap
{
	public:

		WorldMap();
		~WorldMap();

		bool Load();
		bool Save();
		bool Save(const char * szPath);

		void New(short iWidth, short iHeight);
		void Resize(short iWidth, short iHeight);
		void Clear();

		void InitPlayer();

		bool Update(bool * fPlayerVehicleCollision);
		void Draw(short iMapOffsetX, short iMapOffsetY, bool fDrawPlayer, bool fVehiclesSleeping);
		void DrawMapToSurface(bool fInit, SDL_Surface * surface, short iMapDrawOffsetCol, short iMapDrawOffsetRow, short iAnimationFrame);

		void SetPlayerSprite(short iPlayerSprite);
		bool IsVehicleMoving();

		void GetWorldSize(short * w, short * h) {*w = iWidth; *h = iHeight;}

		void GetPlayerPosition(short * iPlayerX, short * iPlayerY);
		void SetPlayerPosition(short iPlayerCol, short iPlayerRow);

		void GetPlayerCurrentTile(short * iPlayerCurrentTileX, short * iPlayerCurrentTileY);
		void GetPlayerDestTile(short * iPlayerDestTileX, short * iPlayerDestTileY);

		short GetPlayerState();

		short GetVehicleInPlayerTile(short * iVehicleIndex);
		bool GetWarpInPlayerTile(short * iWarpCol, short * iWarpRow);

		void MovePlayer(short iDirection);
		void FacePlayer(short iDirection);
		void MoveVehicles();

		void RemoveVehicle(short iVehicleIndex);

		short NumVehiclesInTile(short iTileX, short iTileY);

		short GetVehicleStageScore(short iVehicleIndex);
		void MoveBridges();

		bool IsDoor(short iCol, short iRow);
		short UseKey(short iKeytype, short iCol, short iRow);

		short GetNextInterestingMove(short iCol, short iRow);

		void SetInitialPowerups();

	private:

		void Cleanup();
		void SetTileConnections(short iCol, short iRow);

		short iWidth;
		short iHeight;
		short iStartX, iStartY;

		short iNumStages;
		short iNumWarps;
		short iNumVehicles;

		WorldMapTile ** tiles;
		WorldPlayer player;
		WorldVehicle * vehicles;
		WorldWarp * warps;

		short iNumInitialBonuses;
		short iInitialBonuses[32];

	friend class MI_World;
	friend class WorldVehicle;

	friend int editor_edit();
	friend void AutoSetTile(short iCol, short iRow);
	friend void AutoSetPath(short iCol, short iRow);
	friend void AutoSetPathSprite(short iCol, short iRow);
	friend short AdjustForeground(short iSprite, short iCol, short iRow);
	friend void UpdateForeground(short iCol, short iRow);
};

#endif //__WORLD_H_

