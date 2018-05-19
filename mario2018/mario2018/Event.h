#pragma once


#ifndef Event_H
#define Event_H

#include "header.h"
#include <vector>

class Event
{
private:
	bool bState; // true = OLD, false = NEW
	unsigned int stepID;
public:
	Event(void);
	~Event(void);

	enum animationType {
		eTOP,
		eRIGHT,
		eRIGHTEND,
		eBOT,
		eLEFT,
		eBOTRIGHTEND,
		eENDBOT1,
		eENDBOT2,
		eENDPOINTS,
		eDEATHNOTHING,
		eDEATHTOP,
		eDEATHBOT,
		eNOTHING,
		ePLAYPIPERIGHT,
		ePLAYPIPETOP,
		eLOADINGMENU,
		eGAMEOVER,
		eMARIOSPRITE1,

	};

	std::vector<animationType> vOLDDir;
	std::vector<int> vOLDLength;

	std::vector<animationType> vNEWDir;
	std::vector<int> vNEWLength;

	std::vector<int> reDrawX;
	std::vector<int> reDrawY;

	enum eventType {
		eNormal,
		eEnd,
	};

	eventType eventTypeID;

	void Normal();
	void end();

	int iSpeed;

	void resetData();

	int newLevelType;
	int newMapXPos;
	int newPlayerXPos;
	int newPlayerYPos;
	bool newMoveMap;

	unsigned int iTime;
	int iDelay;

	int newCurrentLevel;
	bool inEvent;

	bool endGame;


	void Draw(SDL_Renderer* rR);

	void Animation();
	void newLevel();

	void resetRedraw();
};

#endif