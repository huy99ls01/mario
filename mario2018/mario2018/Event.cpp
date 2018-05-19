#include "Event.h"
#include "Core.h"

/* ******************************************** */

Event::Event(void) {
	this->iDelay = 0;
	this->endGame = false;
	this->iTime = 0;
	this->bState = true;
	this->stepID = 0;
}

Event::~Event(void) {

}

/* ******************************************** */

void Event::Draw(SDL_Renderer* rR) {
	for (unsigned int i = 0; i < reDrawX.size(); i++) {
		if (reDrawX[i] < CCore::getMap()->getMapWidth())
			CCore::getMap()->getBlock(CCore::getMap()->getMapBlock(reDrawX[i], reDrawY[i])->getBlockID())->Draw(rR, 32 * reDrawX[i] + (int)CCore::getMap()->getXPos(), CCFG::GAME_HEIGHT - 32 * reDrawY[i] - 16);
	}
}

/* ******************************************** */

void Event::Animation() {
	switch (eventTypeID) {
	case eNormal: {
		Normal();
		break;
	}
	case eEnd: {
		Normal();
		end();
		break;
	}
	default:
		Normal();
		break;
	}
}

void Event::Normal() {
	if (bState) {
		if (vOLDDir.size() > stepID) {
			if (vOLDLength[stepID] > 0) {
				switch (vOLDDir[stepID]) {
				case eTOP: // TOP
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() - iSpeed);
					vOLDLength[stepID] -= iSpeed;
					break;
				case eBOT:
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() + iSpeed);
					vOLDLength[stepID] -= iSpeed;
					break;
				case eRIGHT:
					CCore::getMap()->getPlayer()->setXPos((float)CCore::getMap()->getPlayer()->getXPos() + iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->moveAnimation();
					CCore::getMap()->getPlayer()->setMoveDirection(true);
					break;
				case eRIGHTEND:
					CCore::getMap()->setXPos((float)CCore::getMap()->getXPos() - iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->moveAnimation();
					CCore::getMap()->getPlayer()->setMoveDirection(true);
					break;
				case eLEFT:
					CCore::getMap()->getPlayer()->setXPos((float)CCore::getMap()->getPlayer()->getXPos() - iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->moveAnimation();
					CCore::getMap()->getPlayer()->setMoveDirection(false);
					break;
				case eBOTRIGHTEND: // BOT & RIGHT
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() + iSpeed);
					CCore::getMap()->setXPos((float)CCore::getMap()->getXPos() - iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->moveAnimation();
					break;
				case eENDBOT1:
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() + iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->setMarioSpriteID(10);
					break;
				case eENDBOT2:
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->setMoveDirection(false);
					break;
				case eENDPOINTS:
					if (CCore::getMap()->getMapTime() > 0) {
						CCore::getMap()->setMapTime(CCore::getMap()->getMapTime() - 1);
						CCore::getMap()->getPlayer()->setScore(CCore::getMap()->getPlayer()->getScore() + 50);
						if (CCFG::getMusic()->musicStopped) {
							CCFG::getMusic()->PlayMusic(CCFG::getMusic()->mSCORERING);
						}
					}
					else {
						vOLDLength[stepID] = 0;
						CCFG::getMusic()->StopMusic();
					}
					CCore::getMap()->getFlag()->UpdateCastleFlag();
					break;
				case eDEATHNOTHING:
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->setMarioSpriteID(0);
					break;
				case eDEATHTOP: // DEATH TOP
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() - iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->setMarioSpriteID(0);
					break;
				case eDEATHBOT: // DEATH BOT
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() + iSpeed);
					vOLDLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->setMarioSpriteID(0);
					break;
				case eNOTHING:
					vOLDLength[stepID] -= 1;
					break;
				case ePLAYPIPERIGHT:
					CCore::getMap()->getPlayer()->setXPos((float)CCore::getMap()->getPlayer()->getXPos() + iSpeed);
					vOLDLength[stepID] -= 1;
					CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cPIPE);
					break;
				case eLOADINGMENU:
					vOLDLength[stepID] -= 1;

					if (vOLDLength[stepID] < 2) {
						CCore::getMap()->setInEvent(false);
						inEvent = false;
						CCore::getMap()->getPlayer()->stopMove();

						CCFG::getMM()->getLoadingMenu()->loadingType = true;
						CCFG::getMM()->getLoadingMenu()->updateTime();
						CCFG::getMM()->setViewID(CCFG::getMM()->eGameLoading);
					}
					break;
				case eGAMEOVER:
					vOLDLength[stepID] -= 1;

					if (vOLDLength[stepID] < 2) {
						CCore::getMap()->setInEvent(false);
						inEvent = false;
						CCore::getMap()->getPlayer()->stopMove();

						CCFG::getMM()->getLoadingMenu()->loadingType = false;
						CCFG::getMM()->getLoadingMenu()->updateTime();
						CCFG::getMM()->setViewID(CCFG::getMM()->eGameLoading);

						CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cGAMEOVER);
					}
					break;

				case eMARIOSPRITE1:
					CCore::getMap()->getPlayer()->setMarioSpriteID(1);
					vOLDLength[stepID] = 0;
					break;
				}
			}
			else {
				++stepID;
				iTime = SDL_GetTicks();
			}
		}
		else {
			if (!endGame) {
				if (SDL_GetTicks() >= iTime + iDelay) {
					bState = false;
					stepID = 0;
					newLevel();
					CCore::getMap()->getPlayer()->stopMove();
					if (inEvent) {
						CCFG::getMM()->getLoadingMenu()->updateTime();
						CCFG::getMM()->getLoadingMenu()->loadingType = true;
						CCFG::getMM()->setViewID(CCFG::getMM()->eGameLoading);
						CCore::getMap()->startLevelAnimation();
					}

					CCFG::keySpace = false;
				}
			}
			else {
				CCore::getMap()->resetGameData();
				CCFG::getMM()->setViewID(CCFG::getMM()->eMainMenu);
				CCore::getMap()->setInEvent(false);
				CCore::getMap()->getPlayer()->stopMove();
				inEvent = false;
				CCFG::keySpace = false;
				endGame = false;
				stepID = 0;
			}
		}
	}
	else {
		if (vNEWDir.size() > stepID) {
			if (vNEWLength[stepID] > 0) {
				switch (vNEWDir[stepID]) {
				case eTOP:
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() - iSpeed);
					vNEWLength[stepID] -= iSpeed;
					break;
				case eBOT:
					CCore::getMap()->getPlayer()->setYPos((float)CCore::getMap()->getPlayer()->getYPos() + iSpeed);
					vNEWLength[stepID] -= iSpeed;
					break;
				case eRIGHT:
					CCore::getMap()->getPlayer()->setXPos((float)CCore::getMap()->getPlayer()->getXPos() + iSpeed);
					vNEWLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->moveAnimation();
					break;
				case eLEFT:
					CCore::getMap()->getPlayer()->setXPos((float)CCore::getMap()->getPlayer()->getXPos() - iSpeed);
					vNEWLength[stepID] -= iSpeed;
					CCore::getMap()->getPlayer()->moveAnimation();
					break;
				case ePLAYPIPETOP:
					vNEWLength[stepID] -= 1;
					CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cPIPE);
					break;
				case eNOTHING:
					vNEWLength[stepID] -= 1;
					break;
				}
			}
			else {
				++stepID;
			}
		}
		else {
			CCore::getMap()->setInEvent(false);
			CCore::getMap()->getPlayer()->stopMove();
			CCFG::getMusic()->changeMusic(true, true);
			inEvent = false;
			CCFG::keySpace = false;
			CCore::resetKeys();
		}
	}
}

void Event::end() {
	if (CCore::getMap()->getFlag() != NULL && CCore::getMap()->getFlag()->iYPos < CCFG::GAME_HEIGHT - 16 - 3 * 32 - 4) {
		CCore::getMap()->getFlag()->Update();
	}
}

void Event::newLevel() {

}

/* ******************************************** */

void Event::resetData() {
	vNEWDir.clear();
	vNEWLength.clear();
	vOLDDir.clear();
	vOLDLength.clear();
	resetRedraw();

	this->eventTypeID = eNormal;

	this->bState = true;
	this->stepID = 0;
	this->inEvent = false;
	this->endGame = false;
}

void Event::resetRedraw() {
	reDrawX.clear();
	reDrawY.clear();
}