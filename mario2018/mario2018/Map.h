#pragma once

#ifndef MAP_H
#define MAP_H

#include "header.h"
#include "IMG.h"
#include "Block.h"
#include "MapLevel.h"
#include "Player.h"
//#include "PlayerFireBall.h" 
#include "Vector2.h"
#include "LevelText.h"
#include "Goombas.h"
#include "Mushroom.h"
#include "Star.h"
#include "Flag.h"
//#include "Toad.h"
//#include "FireBall.h""
#include "Pipe.h"
#include "Flower.h"
#include "Event.h"
#include "Coin.h"
#include "Points.h"
#include "BlockDebris.h"
#include <vector>


class Map
{
private:
	float fXPos, fYPos;

	std::vector<Block*> vBlock;
	int iBlockSize; // Size of vBlock

	std::vector<Block*> vMinion;
	int iMinionSize; // Size of vBlock

	std::vector<std::vector<MapLevel*>> lMap;
	int iMapWidth, iMapHeight;

	std::vector<BlockDebris*> lBlockDebris;

	std::vector<LevelText*> vLevelText;


	int currentLevelID;
	int iLevelType; // 0, 1, 2;
	bool underWater;

	int iSpawnPointID;

	bool bMoveMap;

	int iFrameID;
	int iMapTime;

	bool inEvent;
	Event* oEvent;

	// ----- PLAYER -----
	Player* oPlayer;

	// ----- MINIONS -----
	std::vector<std::vector<Minion*>> lMinion;
	int iMinionListSize;

	int getListID(int nXPos);

	int getNumOfMinions();

	// ----- MINIONS -----


	// ----- POINTS & COIN -----

	std::vector<Coin*> lCoin;
	std::vector<Points*> lPoints;

	// ----- POINTS & COIN -----

	// ----- PIPEEVENTS -----

	std::vector<Pipe*> lPipe;

	// ----- ENDEVENTS

	Flag* oFlag;

	bool drawLines;

	// ---------- Methods ----------

	int getStartBlock();
	int getEndBlock();


	// ----- Load -----
	void loadGameData(SDL_Renderer* rR);



	void createMap();

	void checkSpawnPoint();
	int getNumOfSpawnPoints();
	int getSpawnPointXPos(int iID);
	int getSpawnPointYPos(int iID);

	void loadLVL_1_1();
	void loadMinionsLVL_1_1();

	void clearPipeEvents();
	void loadPipeEventsLVL_1_1();

	void clearLevelText();
	void clearMap();
	void clearMinions();

	bool bTP; // -- TP LOOP

	void pipeUse();
	void EndUse();
	void EndBoss();
	void EndBonus();

public:
	Map(void);
	Map(SDL_Renderer* rR);
	~Map(void);

	void Update();


	void moveMap(int iX, int iY);

	void UpdatePlayer();
	void UpdateMinions();
	void UpdateMinionsCollisions();
	void UpdateBlocks();
	void UpdateMinionBlokcs();

	void Draw(SDL_Renderer* rR);
	void DrawMap(SDL_Renderer* rR);
	void DrawMinions(SDL_Renderer* rR);
	void DrawGameLayout(SDL_Renderer* rR);
	void DrawLines(SDL_Renderer* rR);

	void addPoints(int X, int Y, std::string sText, int iW, int iH);
	void addGoombas(int iX, int iY, bool moveDirection);

	void addText(int X, int Y, std::string sText);

	void lockMinions();

	bool blockUse(int nX, int nY, int iBlockID, int POS);

	// ----- LOAD
	void resetGameData();
	void loadLVL();
	void setBackgroundColor(SDL_Renderer* rR);
	std::string getLevelName();
	void startLevelAnimation();

	void structGND(int X, int Y, int iWidth, int iHeight);
	void structGND2(int X, int Y, int iSize, bool bDir); // true = LEFT, false = RIGHT
	void structGND2(int X, int Y, int iWidth, int iHeight);

	void structPipe(int X, int Y, int iHeight);
	void structPipeVertical(int X, int Y, int iHeight);
	void structPipeHorizontal(int X, int Y, int iWidth);

	void structCoins(int X, int Y, int iWidth, int iHeight);

	void structBrick(int X, int Y, int iWidth, int iHeight);
	void struckBlockQ(int X, int Y, int iWidth);
	void struckBlockQ2(int X, int Y, int iWidth);

	void structEnd(int X, int Y, int iHeight);

	void structCastleSmall(int X, int Y);

	void setBlockID(int X, int Y, int iBlockID);


	// ********** COLLISION
	Vector2* getBlockID(int nX, int nY);
	int getBlockIDX(int nXl);
	int getBlockIDY(int nY);

	// ----- LEFT
	bool checkCollisionLB(int nX, int nY, int nHitBoxY, bool checkVisible);
	bool checkCollisionLT(int nX, int nY, bool checkVisible);
	// ----- CENTER
	bool checkCollisionLC(int nX, int nY, int nHitBoxY, bool checkVisible);
	bool checkCollisionRC(int nX, int nY, int nHitBoxX, int nHitBoxY, bool checkVisible);
	// ----- RIGHT
	bool checkCollisionRB(int nX, int nY, int nHitBoxX, int nHitBoxY, bool checkVisible);
	bool checkCollisionRT(int nX, int nY, int nHitBoxX, bool checkVisible);

	bool checkCollision(Vector2* nV, bool checkVisible);

	void checkCollisionOnTopOfTheBlock(int nX, int nY);
	// ********** COLLISION
	void playerDeath(bool animation, bool instantDeath);
	// -------- get & set -----------
	float getXPos();
	void setXPos(float iYPos);
	float getYPos();
	void setYPos(float iYPos);

	bool getDrawLines();
	void setDrawLines(bool drawLines);

	int getCurrentLevelID();
	void setCurrentLevelID(int currentLevelID);

	int getLevelType();
	void setLevelType(int iLevelType);

	int getMapWidth();

	void setSpawnPointID(int iSpawnPointID);

	void setSpawnPoint();

	Block* getBlock(int iID);
	Player * getPlayer();
	MapLevel* getMapBlock(int iX, int iY);

	bool getMoveMap();
	void setMoveMap(bool bMoveMap);

	int getMapTime();
	void setMapTime(int iMapTime);

	Flag* getFlag();

	Event* getEvent();
	bool getInEvent();
	void setInEvent(bool inEvent);

};
#endif
