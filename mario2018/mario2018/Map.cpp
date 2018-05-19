#include "Map.h"
#include "CFG.h"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include <iostream>

/* ******************************************** */

Map::Map(void) {

}

Map::Map(SDL_Renderer* rR) {
	oPlayer = new Player(rR, 84, 368);

	this->currentLevelID = 0;

	this->iMapWidth = 0;
	this->iMapHeight = 0;
	this->iLevelType = 0;

	this->drawLines = false;
	this->fXPos = 0;
	this->fYPos = 0;

	this->inEvent = false;

	this->iSpawnPointID = 0;

	this->bMoveMap = true;

	this->iFrameID = 0;

	this->bTP = false;

	CCFG::getText()->setFont(rR, "font");

	oEvent = new Event();
	oFlag = NULL;

	srand((unsigned)time(NULL));

	loadGameData(rR);
	loadLVL();
}

Map::~Map(void) {
	for (std::vector<Block*>::iterator i = vBlock.begin(); i != vBlock.end(); i++) {
		delete (*i);
	}

	for (std::vector<Block*>::iterator i = vMinion.begin(); i != vMinion.end(); i++) {
		delete (*i);
	}

	delete oEvent;
	delete oFlag;
}

/* ******************************************** */

void Map::Update() {
	UpdateBlocks();

	if (!oPlayer->getInLevelAnimation()) {
		UpdateMinionBlokcs();

		UpdateMinions();

		if (!inEvent) {
			UpdatePlayer();

			++iFrameID;
			if (iFrameID > 32) {
				iFrameID = 0;
				if (iMapTime > 0) {
					--iMapTime;
					if (iMapTime == 90) {
						CCFG::getMusic()->StopMusic();
						CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cLOWTIME);
					}
					else if (iMapTime == 86) {
						CCFG::getMusic()->changeMusic(true, true);
					}

					if (iMapTime <= 0) {
						playerDeath(true, true);
					}
				}
			}
		}
		else {
			oEvent->Animation();
		}
	}
	else {
		oPlayer->powerUPAnimation();
	}

	for (unsigned int i = 0; i < lBlockDebris.size(); i++) {
		if (lBlockDebris[i]->getDebrisState() != -1) {
			lBlockDebris[i]->Update();
		}
		else {
			delete lBlockDebris[i];
			lBlockDebris.erase(lBlockDebris.begin() + i);
		}
	}

	for (unsigned int i = 0; i < lPoints.size(); i++) {
		if (!lPoints[i]->getDelete()) {
			lPoints[i]->Update();
		}
		else {
			delete lPoints[i];
			lPoints.erase(lPoints.begin() + i);
		}
	}

	for (unsigned int i = 0; i < lCoin.size(); i++) {
		if (!lCoin[i]->getDelete()) {
			lCoin[i]->Update();
		}
		else {
			lPoints.push_back(new Points(lCoin[i]->getXPos(), lCoin[i]->getYPos(), "200"));
			delete lCoin[i];
			lCoin.erase(lCoin.begin() + i);
		}
	}
}

void Map::UpdatePlayer() {
	oPlayer->Update();
	checkSpawnPoint();
}

void Map::UpdateMinions() {
	for (int i = 0; i < iMinionListSize; i++) {
		for (int j = 0, jSize = lMinion[i].size(); j < jSize; j++) {
			if (lMinion[i][j]->updateMinion()) {
				lMinion[i][j]->Update();
			}
		}
	}

	// ----- UPDATE MINION LIST ID
	for (int i = 0; i < iMinionListSize; i++) {
		for (int j = 0, jSize = lMinion[i].size(); j < jSize; j++) {
			if (lMinion[i][j]->minionSpawned) {
				if (lMinion[i][j]->minionState == -1) {
					delete lMinion[i][j];
					lMinion[i].erase(lMinion[i].begin() + j);
					jSize = lMinion[i].size();
					continue;
				}

				if (floor(lMinion[i][j]->fXPos / 160) != i) {
					lMinion[(int)floor((int)lMinion[i][j]->fXPos / 160)].push_back(lMinion[i][j]);
					lMinion[i].erase(lMinion[i].begin() + j);
					jSize = lMinion[i].size();
				}
			}
		}
	}
}

void Map::UpdateMinionsCollisions() {
	// ----- COLLISIONS
	for (int i = 0; i < iMinionListSize; i++) {
		for (unsigned int j = 0; j < lMinion[i].size(); j++) {
			if (!lMinion[i][j]->collisionOnlyWithPlayer  && lMinion[i][j]->deadTime < 0) {

				for (unsigned int k = j + 1; k < lMinion[i].size(); k++) {
					if (!lMinion[i][k]->collisionOnlyWithPlayer  && lMinion[i][k]->deadTime < 0) {
						if (lMinion[i][j]->getXPos() < lMinion[i][k]->getXPos()) {
							if (lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX >= lMinion[i][k]->getXPos() && lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX <= lMinion[i][k]->getXPos() + lMinion[i][k]->iHitBoxX && ((lMinion[i][j]->getYPos() <= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY) || (lMinion[i][k]->getYPos() <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY))) {
								if (lMinion[i][j]->killOtherUnits && lMinion[i][j]->moveSpeed > 0 && lMinion[i][k]->minionSpawned) {
									lMinion[i][k]->setMinionState(-2);
									lMinion[i][j]->collisionWithAnotherUnit();
								}

								if (lMinion[i][k]->killOtherUnits && lMinion[i][k]->moveSpeed > 0 && lMinion[i][j]->minionSpawned) {
									lMinion[i][j]->setMinionState(-2);
									lMinion[i][k]->collisionWithAnotherUnit();
								}

								if (lMinion[i][j]->getYPos() - 4 <= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY && lMinion[i][j]->getYPos() + 4 >= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY) {
									lMinion[i][k]->onAnotherMinion = true;
								}
								else if (lMinion[i][k]->getYPos() - 4 <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i][k]->getYPos() + 4 >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY) {
									lMinion[i][j]->onAnotherMinion = true;
								}
								else {
									lMinion[i][j]->collisionEffect();
									lMinion[i][k]->collisionEffect();
								}
							}
						}
						else {
							if (lMinion[i][k]->getXPos() + lMinion[i][k]->iHitBoxX >= lMinion[i][j]->getXPos() && lMinion[i][k]->getXPos() + lMinion[i][k]->iHitBoxX <= lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX && ((lMinion[i][j]->getYPos() <= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY) || (lMinion[i][k]->getYPos() <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY))) {
								if (lMinion[i][j]->killOtherUnits && lMinion[i][j]->moveSpeed > 0 && lMinion[i][k]->minionSpawned) {
									lMinion[i][k]->setMinionState(-2);
									lMinion[i][j]->collisionWithAnotherUnit();
								}

								if (lMinion[i][k]->killOtherUnits && lMinion[i][k]->moveSpeed > 0 && lMinion[i][j]->minionSpawned) {
									lMinion[i][j]->setMinionState(-2);
									lMinion[i][k]->collisionWithAnotherUnit();
								}

								if (lMinion[i][j]->getYPos() - 4 <= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY && lMinion[i][j]->getYPos() + 4 >= lMinion[i][k]->getYPos() + lMinion[i][k]->iHitBoxY) {
									lMinion[i][k]->onAnotherMinion = true;
								}
								else if (lMinion[i][k]->getYPos() - 4 <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i][k]->getYPos() + 4 >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY) {
									lMinion[i][j]->onAnotherMinion = true;
								}
								else {
									lMinion[i][j]->collisionEffect();
									lMinion[i][k]->collisionEffect();
								}
							}
						}
					}
				}


				if (i + 1 < iMinionListSize) {
					for (unsigned int k = 0; k < lMinion[i + 1].size(); k++) {
						if (!lMinion[i + 1][k]->collisionOnlyWithPlayer  && lMinion[i + 1][k]->deadTime < 0) {
							if (lMinion[i][j]->getXPos() < lMinion[i + 1][k]->getXPos()) {
								if (lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX >= lMinion[i + 1][k]->getXPos() && lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX <= lMinion[i + 1][k]->getXPos() + lMinion[i + 1][k]->iHitBoxX && ((lMinion[i][j]->getYPos() <= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY) || (lMinion[i + 1][k]->getYPos() <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY))) {
									if (lMinion[i][j]->killOtherUnits && lMinion[i][j]->moveSpeed > 0 && lMinion[i + 1][k]->minionSpawned) {
										lMinion[i + 1][k]->setMinionState(-2);
										lMinion[i][j]->collisionWithAnotherUnit();
									}

									if (lMinion[i + 1][k]->killOtherUnits && lMinion[i + 1][k]->moveSpeed > 0 && lMinion[i][j]->minionSpawned) {
										lMinion[i][j]->setMinionState(-2);
										lMinion[i + 1][k]->collisionWithAnotherUnit();
									}

									if (lMinion[i][j]->getYPos() - 4 <= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY && lMinion[i][j]->getYPos() + 4 >= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY) {
										lMinion[i + 1][k]->onAnotherMinion = true;
									}
									else if (lMinion[i + 1][k]->getYPos() - 4 <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i + 1][k]->getYPos() + 4 >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY) {
										lMinion[i][j]->onAnotherMinion = true;
									}
									else {
										lMinion[i][j]->collisionEffect();
										lMinion[i + 1][k]->collisionEffect();
									}
								}
							}
							else {
								if (lMinion[i + 1][k]->getXPos() + lMinion[i + 1][k]->iHitBoxX >= lMinion[i][j]->getXPos() && lMinion[i + 1][k]->getXPos() + lMinion[i + 1][k]->iHitBoxX < lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX && ((lMinion[i][j]->getYPos() <= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY) || (lMinion[i + 1][k]->getYPos() <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY))) {
									if (lMinion[i][j]->killOtherUnits && lMinion[i][j]->moveSpeed > 0 && lMinion[i + 1][k]->minionSpawned) {
										lMinion[i + 1][k]->setMinionState(-2);
										lMinion[i][j]->collisionWithAnotherUnit();
									}

									if (lMinion[i + 1][k]->killOtherUnits && lMinion[i + 1][k]->moveSpeed > 0 && lMinion[i][j]->minionSpawned) {
										lMinion[i][j]->setMinionState(-2);
										lMinion[i + 1][k]->collisionWithAnotherUnit();
									}

									if (lMinion[i][j]->getYPos() - 4 <= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY && lMinion[i][j]->getYPos() + 4 >= lMinion[i + 1][k]->getYPos() + lMinion[i + 1][k]->iHitBoxY) {
										lMinion[i + 1][k]->onAnotherMinion = true;
									}
									else if (lMinion[i + 1][k]->getYPos() - 4 <= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY && lMinion[i + 1][k]->getYPos() + 4 >= lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY) {
										lMinion[i][j]->onAnotherMinion = true;
									}
									else {
										lMinion[i][j]->collisionEffect();
										lMinion[i + 1][k]->collisionEffect();
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (!inEvent && !oPlayer->getInLevelAnimation()) {

		for (int i = getListID(-(int)fXPos + oPlayer->getXPos()) - (getListID(-(int)fXPos + oPlayer->getXPos()) > 0 ? 1 : 0), iSize = i + 2; i < iSize; i++) {
			for (unsigned int j = 0, jSize = lMinion[i].size(); j < jSize; j++) {
				if (lMinion[i][j]->deadTime < 0) {
					if ((oPlayer->getXPos() - fXPos >= lMinion[i][j]->getXPos() && oPlayer->getXPos() - fXPos <= lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX) || (oPlayer->getXPos() - fXPos + oPlayer->getHitBoxX() >= lMinion[i][j]->getXPos() && oPlayer->getXPos() - fXPos + oPlayer->getHitBoxX() <= lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX)) {
						if (lMinion[i][j]->getYPos() - 2 <= oPlayer->getYPos() + oPlayer->getHitBoxY() && lMinion[i][j]->getYPos() + 16 >= oPlayer->getYPos() + oPlayer->getHitBoxY()) {
							lMinion[i][j]->collisionWithPlayer(true);
						}
						else if ((lMinion[i][j]->getYPos() <= oPlayer->getYPos() + oPlayer->getHitBoxY() && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= oPlayer->getYPos() + oPlayer->getHitBoxY()) || (lMinion[i][j]->getYPos() <= oPlayer->getYPos() && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= oPlayer->getYPos())) {
							lMinion[i][j]->collisionWithPlayer(false);
						}
					}
				}
			}
		}
	}
}

void Map::UpdateBlocks() {
	vBlock[2]->getSprite()->Update();
	vBlock[8]->getSprite()->Update();
	vBlock[29]->getSprite()->Update();
	vBlock[55]->getSprite()->Update();
	vBlock[57]->getSprite()->Update();
	vBlock[67]->getSprite()->Update();
	vBlock[68]->getSprite()->Update();
	vBlock[69]->getSprite()->Update();
	vBlock[70]->getSprite()->Update();
}

void Map::UpdateMinionBlokcs() {
	vMinion[0]->getSprite()->Update();
	vMinion[4]->getSprite()->Update();
}

/* ******************************************** */

void Map::Draw(SDL_Renderer* rR) {
	DrawMap(rR);
	DrawMinions(rR);

	for (unsigned int i = 0; i < lPoints.size(); i++) {
		lPoints[i]->Draw(rR);
	}

	for (unsigned int i = 0; i < lCoin.size(); i++) {
		lCoin[i]->Draw(rR);
	}

	for (unsigned int i = 0; i < lBlockDebris.size(); i++) {
		lBlockDebris[i]->Draw(rR);
	}

	for (unsigned int i = 0; i < vLevelText.size(); i++) {
		CCFG::getText()->Draw(rR, vLevelText[i]->getText(), vLevelText[i]->getXPos() + (int)fXPos, vLevelText[i]->getYPos());
	}


	oPlayer->Draw(rR);

	if (inEvent) {
		oEvent->Draw(rR);
	}

	DrawGameLayout(rR);
}

void Map::DrawMap(SDL_Renderer* rR) {
	if (oFlag != NULL) {
		oFlag->DrawCastleFlag(rR, vBlock[51]->getSprite()->getTexture());
	}
	int x = getStartBlock();
	for (int i = getStartBlock(), iEnd = getEndBlock(); i < iEnd && i < iMapWidth; i++) {
		for (int j = iMapHeight - 1; j >= 0; j--) {
			if (lMap[i][j]->getBlockID() != 0) {

				vBlock[lMap[i][j]->getBlockID()]->Draw(rR, 32 * i + (int)fXPos, CCFG::GAME_HEIGHT - 32 * j - 16 - lMap[i][j]->updateYPos());
			}
		}
	}

	if (oFlag != NULL) {
		oFlag->Draw(rR, vBlock[oFlag->iBlockID]->getSprite()->getTexture());
	}
}

void Map::DrawMinions(SDL_Renderer* rR) {
	for (int i = 0; i < iMinionListSize; i++) {
		for (int j = 0, jSize = lMinion[i].size(); j < jSize; j++) {
			lMinion[i][j]->Draw(rR, vMinion[lMinion[i][j]->getBloockID()]->getSprite()->getTexture());
		}
	}
}

void Map::DrawGameLayout(SDL_Renderer* rR) {
	CCFG::getText()->Draw(rR, "PETER", 54, 16);

	if (oPlayer->getScore() < 100) {
		CCFG::getText()->Draw(rR, "00000" + std::to_string(oPlayer->getScore()), 54, 32);
	}
	else if (oPlayer->getScore() < 1000) {
		CCFG::getText()->Draw(rR, "000" + std::to_string(oPlayer->getScore()), 54, 32);
	}
	else if (oPlayer->getScore() < 10000) {
		CCFG::getText()->Draw(rR, "00" + std::to_string(oPlayer->getScore()), 54, 32);
	}
	else if (oPlayer->getScore() < 100000) {
		CCFG::getText()->Draw(rR, "0" + std::to_string(oPlayer->getScore()), 54, 32);
	}
	else {
		CCFG::getText()->Draw(rR, std::to_string(oPlayer->getScore()), 54, 32);
	}

	CCFG::getText()->Draw(rR, "HANOI", 462, 16);
	CCFG::getText()->Draw(rR, getLevelName(), 480, 32);

	if (iLevelType != 1) {
		vBlock[2]->Draw(rR, 268, 32);
	}
	else {
		vBlock[57]->Draw(rR, 268, 32);
	}
	CCFG::getText()->Draw(rR, "y", 286, 32);
	CCFG::getText()->Draw(rR, (oPlayer->getCoins() < 10 ? "0" : "") + std::to_string(oPlayer->getCoins()), 302, 32);

	CCFG::getText()->Draw(rR, "TIME", 672, 16);
	if (CCFG::getMM()->getViewID() == CCFG::getMM()->eGame) {
		if (iMapTime > 100) {
			CCFG::getText()->Draw(rR, std::to_string(iMapTime), 680, 32);
		}
		else if (iMapTime > 10) {
			CCFG::getText()->Draw(rR, "0" + std::to_string(iMapTime), 680, 32);
		}
		else {
			CCFG::getText()->Draw(rR, "00" + std::to_string(iMapTime), 680, 32);
		}
	}
}

void Map::DrawLines(SDL_Renderer* rR) {
	SDL_SetRenderDrawBlendMode(rR, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(rR, 255, 255, 255, 128);

	for (int i = 0; i < CCFG::GAME_WIDTH / 32 + 1; i++) {
		SDL_RenderDrawLine(rR, 32 * i - (-(int)fXPos) % 32, 0, 32 * i - (-(int)fXPos) % 32, CCFG::GAME_HEIGHT);
	}

	for (int i = 0; i < CCFG::GAME_HEIGHT / 32 + 1; i++) {
		SDL_RenderDrawLine(rR, 0, 32 * i - 16 + (int)fYPos, CCFG::GAME_WIDTH, 32 * i - 16 + (int)fYPos);
	}

	for (int i = 0; i < CCFG::GAME_WIDTH / 32 + 1; i++) {
		for (int j = 0; j < CCFG::GAME_HEIGHT / 32; j++) {
			CCFG::getText()->Draw(rR, std::to_string(i + (-((int)fXPos + (-(int)fXPos) % 32)) / 32), 32 * i + 16 - (-(int)fXPos) % 32 - CCFG::getText()->getTextWidth(std::to_string(i + (-((int)fXPos + (-(int)fXPos) % 32)) / 32), 8) / 2, CCFG::GAME_HEIGHT - 9 - 32 * j, 8);
			CCFG::getText()->Draw(rR, std::to_string(j), 32 * i + 16 - (-(int)fXPos) % 32 - CCFG::getText()->getTextWidth(std::to_string(j), 8) / 2 + 1, CCFG::GAME_HEIGHT - 32 * j, 8);
		}
	}

	SDL_SetRenderDrawBlendMode(rR, SDL_BLENDMODE_NONE);
}

/* ******************************************** */

void Map::moveMap(int nX, int nY) {
	if (fXPos + nX > 0) {
		oPlayer->updateXPos((int)(nX - fXPos));
		fXPos = 0;
	}
	else {

		this->fXPos += nX;
	}
}

int Map::getStartBlock() {
	return (int)(-fXPos - (-(int)fXPos) % 32) / 32;
}

int Map::getEndBlock() {
	return (int)(-fXPos - (-(int)fXPos) % 32 + CCFG::GAME_WIDTH) / 32 + 2;
}

/* ******************************************** */

/* ******************************************** */
/* ---------------- COLLISION ---------------- */

Vector2* Map::getBlockID(int nX, int nY) {
	return new Vector2((int)(nX < 0 ? 0 : nX) / 32, (int)(nY > CCFG::GAME_HEIGHT - 16 ? 0 : (CCFG::GAME_HEIGHT - 16 - nY + 32) / 32));
}

int Map::getBlockIDX(int nX) {
	return (int)(nX < 0 ? 0 : nX) / 32;
}

int Map::getBlockIDY(int nY) {
	return (int)(nY > CCFG::GAME_HEIGHT - 16 ? 0 : (CCFG::GAME_HEIGHT - 16 - nY + 32) / 32);
}

bool Map::checkCollisionLB(int nX, int nY, int nHitBoxY, bool checkVisible) {
	return checkCollision(getBlockID(nX, nY + nHitBoxY), checkVisible);
}

bool Map::checkCollisionLT(int nX, int nY, bool checkVisible) {
	return checkCollision(getBlockID(nX, nY), checkVisible);
}

bool Map::checkCollisionLC(int nX, int nY, int nHitBoxY, bool checkVisible) {
	return checkCollision(getBlockID(nX, nY + nHitBoxY), checkVisible);
}

bool Map::checkCollisionRC(int nX, int nY, int nHitBoxX, int nHitBoxY, bool checkVisible) {
	return checkCollision(getBlockID(nX + nHitBoxX, nY + nHitBoxY), checkVisible);
}

bool Map::checkCollisionRB(int nX, int nY, int nHitBoxX, int nHitBoxY, bool checkVisible) {
	return checkCollision(getBlockID(nX + nHitBoxX, nY + nHitBoxY), checkVisible);
}

bool Map::checkCollisionRT(int nX, int nY, int nHitBoxX, bool checkVisible) {
	return checkCollision(getBlockID(nX + nHitBoxX, nY), checkVisible);
}

bool Map::checkCollision(Vector2* nV, bool checkVisible) {
	bool output = vBlock[lMap[nV->getX()][nV->getY()]->getBlockID()]->getCollision() && (checkVisible ? vBlock[lMap[nV->getX()][nV->getY()]->getBlockID()]->getVisible() : true);
	delete nV;
	return output;
}

void Map::checkCollisionOnTopOfTheBlock(int nX, int nY) {
	switch (lMap[nX][nY + 1]->getBlockID()) {
	case 29: case 71: case 72: case 73:// COIN
		lMap[nX][nY + 1]->setBlockID(0);
		lCoin.push_back(new Coin(nX * 32 + 7, CCFG::GAME_HEIGHT - nY * 32 - 48));
		CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cCOIN);
		oPlayer->setCoins(oPlayer->getCoins() + 1);
		return;
		break;
	}

	for (int i = (nX - nX % 5) / 5, iEnd = i + 3; i < iEnd && i < iMinionListSize; i++) {
		for (unsigned int j = 0; j < lMinion[i].size(); j++) {
			if (!lMinion[i][j]->collisionOnlyWithPlayer && lMinion[i][j]->getMinionState() >= 0 && ((lMinion[i][j]->getXPos() >= nX * 32 && lMinion[i][j]->getXPos() <= nX * 32 + 32) || (lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX >= nX * 32 && lMinion[i][j]->getXPos() + lMinion[i][j]->iHitBoxX <= nX * 32 + 32))) {
				if (lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY >= CCFG::GAME_HEIGHT - 24 - nY * 32 && lMinion[i][j]->getYPos() + lMinion[i][j]->iHitBoxY <= CCFG::GAME_HEIGHT - nY * 32 + 16) {
					lMinion[i][j]->moveDirection = !lMinion[i][j]->moveDirection;
					lMinion[i][j]->setMinionState(-2);
				}
			}
		}
	}
}

/* ---------------- COLLISION ---------------- */
/* ******************************************** */
/* ----------------- MINIONS ----------------- */

int Map::getListID(int nX) {
	return (int)(nX / 160);
}

void Map::addPoints(int X, int Y, std::string sText, int iW, int iH) {
	lPoints.push_back(new Points(X, Y, sText, iW, iH));
}

void Map::addGoombas(int iX, int iY, bool moveDirection) {
	lMinion[getListID(iX)].push_back(new Goombas(iX, iY, iLevelType == 0 || iLevelType == 4 ? 0 : iLevelType == 1 ? 8 : 10, moveDirection));
}


void Map::lockMinions() {
	for (unsigned int i = 0; i < lMinion.size(); i++) {
		for (unsigned int j = 0; j < lMinion[i].size(); j++) {
			lMinion[i][j]->lockMinion();
		}
	}
}
void Map::addText(int X, int Y, std::string sText) {
	vLevelText.push_back(new LevelText(X, Y, sText));
}

int Map::getNumOfMinions() {
	int iOutput = 0;

	for (int i = 0, size = lMinion.size(); i < size; i++) {
		iOutput += lMinion[i].size();
	}

	return iOutput;
}

/* ----------------- MINIONS ----------------- */
/* ******************************************** */

/* ---------- LOAD GAME DATA ---------- */

void Map::loadGameData(SDL_Renderer* rR) {
	std::vector<std::string> tSprite;
	std::vector<unsigned int> iDelay;

	// ----- 0 Transparent ( vo hinh ) -----
	tSprite.push_back("transp");
	iDelay.push_back(0);
	vBlock.push_back(new Block(0, new Sprite(rR, tSprite, iDelay, false), false, true, false, false));
	tSprite.clear();
	iDelay.clear();
	// ----- 1 -----
	tSprite.push_back("gnd_red_1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(1, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 2 -----
	tSprite.push_back("coin_0");
	iDelay.push_back(300);
	tSprite.push_back("coin_2");
	iDelay.push_back(30);
	tSprite.push_back("coin_1");
	iDelay.push_back(130);
	tSprite.push_back("coin_2");
	iDelay.push_back(140);
	vBlock.push_back(new Block(2, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 3 -----
	tSprite.push_back("bush_center_0");
	iDelay.push_back(0);
	vBlock.push_back(new Block(3, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 4 -----
	tSprite.push_back("bush_center_1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(4, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 5 -----
	tSprite.push_back("bush_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(5, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 6 -----
	tSprite.push_back("bush_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(6, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 7 -----
	tSprite.push_back("bush_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(7, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 8 -----
	tSprite.push_back("blockq_0");
	iDelay.push_back(300);
	tSprite.push_back("blockq_2");
	iDelay.push_back(30);
	tSprite.push_back("blockq_1");
	iDelay.push_back(130);
	tSprite.push_back("blockq_2");
	iDelay.push_back(140);
	vBlock.push_back(new Block(8, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 9 -----
	tSprite.push_back("blockq_used");
	iDelay.push_back(0);
	vBlock.push_back(new Block(9, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 10 -----
	tSprite.push_back("grass_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(10, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 11 -----
	tSprite.push_back("grass_center");
	iDelay.push_back(0);
	vBlock.push_back(new Block(11, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 12 -----
	tSprite.push_back("grass_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(12, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 13 -----
	tSprite.push_back("brickred");
	iDelay.push_back(0);
	vBlock.push_back(new Block(13, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 14 -----
	tSprite.push_back("cloud_left_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(14, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 15 -----
	tSprite.push_back("cloud_left_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(15, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 16 -----
	tSprite.push_back("cloud_center_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(16, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 17 -----
	tSprite.push_back("cloud_center_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(17, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 18 -----
	tSprite.push_back("cloud_right_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(18, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 19 -----
	tSprite.push_back("cloud_right_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(19, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 20 -----
	tSprite.push_back("pipe_left_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(20, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 21 -----
	tSprite.push_back("pipe_left_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(21, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 22 -----
	tSprite.push_back("pipe_right_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(22, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 23 -----
	tSprite.push_back("pipe_right_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(23, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 24 BlockQ2 -----
	tSprite.push_back("transp");
	iDelay.push_back(0);
	vBlock.push_back(new Block(24, new Sprite(rR, tSprite, iDelay, false), true, false, true, false));
	tSprite.clear();
	iDelay.clear();
	// ----- 25 -----
	tSprite.push_back("gnd_red2");
	iDelay.push_back(0);
	vBlock.push_back(new Block(25, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 26 -----
	tSprite.push_back("gnd1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(26, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 27 -----
	tSprite.push_back("gnd1_2");
	iDelay.push_back(0);
	vBlock.push_back(new Block(27, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 28 -----
	tSprite.push_back("brick1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(28, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 29 -----
	tSprite.push_back("coin_use0");
	iDelay.push_back(300);
	tSprite.push_back("coin_use2");
	iDelay.push_back(30);
	tSprite.push_back("coin_use1");
	iDelay.push_back(130);
	tSprite.push_back("coin_use2");
	iDelay.push_back(140);
	vBlock.push_back(new Block(29, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 30 -----
	tSprite.push_back("pipe1_left_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(30, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 31 -----
	tSprite.push_back("pipe1_left_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(31, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 32 -----
	tSprite.push_back("pipe1_right_bot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(32, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 33 -----
	tSprite.push_back("pipe1_right_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(33, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 34 -----
	tSprite.push_back("pipe1_hor_bot_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(34, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 35 -----
	tSprite.push_back("pipe1_hor_top_center");
	iDelay.push_back(0);
	vBlock.push_back(new Block(35, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 36 -----
	tSprite.push_back("pipe1_hor_top_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(36, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 37 -----
	tSprite.push_back("pipe1_hor_bot_center");
	iDelay.push_back(0);
	vBlock.push_back(new Block(37, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 38 -----
	tSprite.push_back("pipe1_hor_bot_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(38, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 39 -----
	tSprite.push_back("pipe1_hor_top_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(39, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 40 -----
	tSprite.push_back("end0_l");
	iDelay.push_back(0);
	vBlock.push_back(new Block(40, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 41 -----
	tSprite.push_back("end0_dot");
	iDelay.push_back(0);
	vBlock.push_back(new Block(41, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 42 -----
	tSprite.push_back("end0_flag");
	iDelay.push_back(0);
	vBlock.push_back(new Block(42, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 43 -----
	tSprite.push_back("castle0_brick");
	iDelay.push_back(0);
	vBlock.push_back(new Block(43, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 44 -----
	tSprite.push_back("castle0_top0");
	iDelay.push_back(0);
	vBlock.push_back(new Block(44, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 45 -----
	tSprite.push_back("castle0_top1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(45, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 46 -----
	tSprite.push_back("castle0_center_center_top");
	iDelay.push_back(0);
	vBlock.push_back(new Block(46, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 47 -----
	tSprite.push_back("castle0_center_center");
	iDelay.push_back(0);
	vBlock.push_back(new Block(47, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 48 -----
	tSprite.push_back("castle0_center_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(48, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 49 -----
	tSprite.push_back("castle0_center_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(49, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 50 -----
	tSprite.push_back("coin_an0");
	iDelay.push_back(0);
	tSprite.push_back("coin_an1");
	iDelay.push_back(0);
	tSprite.push_back("coin_an2");
	iDelay.push_back(0);
	tSprite.push_back("coin_an3");
	iDelay.push_back(0);
	vBlock.push_back(new Block(50, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 51 -----
	tSprite.push_back("castle_flag");
	iDelay.push_back(0);
	vBlock.push_back(new Block(51, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 52 -----
	tSprite.push_back("firework0");
	iDelay.push_back(0);
	vBlock.push_back(new Block(52, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 53 -----
	tSprite.push_back("firework1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(53, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 54 -----
	tSprite.push_back("firework2");
	iDelay.push_back(0);
	vBlock.push_back(new Block(54, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 55 -----
	tSprite.push_back("blockq1_0");
	iDelay.push_back(300);
	tSprite.push_back("blockq1_2");
	iDelay.push_back(30);
	tSprite.push_back("blockq1_1");
	iDelay.push_back(130);
	tSprite.push_back("blockq1_2");
	iDelay.push_back(140);
	vBlock.push_back(new Block(55, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 56 -----
	tSprite.push_back("blockq1_used");
	iDelay.push_back(0);
	vBlock.push_back(new Block(56, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 57 -----
	tSprite.push_back("coin1_0");
	iDelay.push_back(300);
	tSprite.push_back("coin1_2");
	iDelay.push_back(30);
	tSprite.push_back("coin1_1");
	iDelay.push_back(130);
	tSprite.push_back("coin1_2");
	iDelay.push_back(140);
	vBlock.push_back(new Block(57, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 58 -----
	tSprite.push_back("pipe_hor_bot_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(58, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 59 -----
	tSprite.push_back("pipe_hor_top_center");
	iDelay.push_back(0);
	vBlock.push_back(new Block(59, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 60 -----
	tSprite.push_back("pipe_hor_top_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(60, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 61 -----
	tSprite.push_back("pipe_hor_bot_center");
	iDelay.push_back(0);
	vBlock.push_back(new Block(61, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 62 -----
	tSprite.push_back("pipe_hor_bot_left");
	iDelay.push_back(0);
	vBlock.push_back(new Block(62, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 63 -----
	tSprite.push_back("pipe_hor_top_right");
	iDelay.push_back(0);
	vBlock.push_back(new Block(63, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 64 -----
	tSprite.push_back("block_debris0");
	iDelay.push_back(0);
	vBlock.push_back(new Block(64, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 65 -----
	tSprite.push_back("block_debris1");
	iDelay.push_back(0);
	vBlock.push_back(new Block(65, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 66 -----
	tSprite.push_back("block_debris2");
	iDelay.push_back(0);
	vBlock.push_back(new Block(66, new Sprite(rR, tSprite, iDelay, false), false, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 67 -----
	tSprite.push_back("coin_use00");
	iDelay.push_back(300);
	tSprite.push_back("coin_use02");
	iDelay.push_back(30);
	tSprite.push_back("coin_use01");
	iDelay.push_back(130);
	tSprite.push_back("coin_use02");
	iDelay.push_back(140);
	vBlock.push_back(new Block(67, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 68 -----
	tSprite.push_back("coin_use20");
	iDelay.push_back(300);
	tSprite.push_back("coin_use22");
	iDelay.push_back(30);
	tSprite.push_back("coin_use21");
	iDelay.push_back(130);
	tSprite.push_back("coin_use22");
	iDelay.push_back(140);
	vBlock.push_back(new Block(68, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 69 -----
	tSprite.push_back("coin_use30");
	iDelay.push_back(300);
	tSprite.push_back("coin_use32");
	iDelay.push_back(30);
	tSprite.push_back("coin_use31");
	iDelay.push_back(130);
	tSprite.push_back("coin_use32");
	iDelay.push_back(140);
	vBlock.push_back(new Block(69, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 70 ----- ENDUSE
	tSprite.push_back("transp");
	iDelay.push_back(0);
	vBlock.push_back(new Block(70, new Sprite(rR, tSprite, iDelay, false), false, false, true, true));
	tSprite.clear();
	iDelay.clear();


	// --------------- MINION ---------------

	// ----- 0 -----
	tSprite.push_back("goombas_0");
	iDelay.push_back(200);
	tSprite.push_back("goombas_1");
	iDelay.push_back(200);
	vMinion.push_back(new Block(0, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 1 -----
	tSprite.push_back("goombas_ded");
	iDelay.push_back(0);
	vMinion.push_back(new Block(1, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 2 -----
	tSprite.push_back("mushroom");
	iDelay.push_back(0);
	vMinion.push_back(new Block(2, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 3 -----
	tSprite.push_back("mushroom_1up");
	iDelay.push_back(0);
	vMinion.push_back(new Block(3, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 4 -----
	tSprite.push_back("star_0");
	iDelay.push_back(75);
	tSprite.push_back("star_1");
	iDelay.push_back(75);
	tSprite.push_back("star_2");
	iDelay.push_back(75);
	tSprite.push_back("star_3");
	iDelay.push_back(75);
	vMinion.push_back(new Block(4, new Sprite(rR, tSprite, iDelay, false), true, false, false, true));
	tSprite.clear();
	iDelay.clear();
	// ----- 5 -----
	tSprite.push_back("flower0");
	iDelay.push_back(50);
	tSprite.push_back("flower1");
	iDelay.push_back(50);
	tSprite.push_back("flower2");
	iDelay.push_back(50);
	tSprite.push_back("flower3");
	iDelay.push_back(50);
	vMinion.push_back(new Block(5, new Sprite(rR, tSprite, iDelay, false), true, false, true, true));
	tSprite.clear();
	iDelay.clear();


	iBlockSize = vBlock.size();
	iMinionSize = vMinion.size();
}

/* ******************************************** */

void Map::clearMap() {
	for (int i = 0; i < iMapWidth; i++) {
		for (int j = 0; j < iMapHeight; j++) {
			delete lMap[i][j];
		}
		lMap[i].clear();
	}
	lMap.clear();

	this->iMapWidth = this->iMapHeight = 0;

	if (oFlag != NULL) {
		delete oFlag;
		oFlag = NULL;
	}

	oEvent->eventTypeID = oEvent->eNormal;

	clearLevelText();
}

void Map::clearMinions() {
	for (int i = 0; i < iMinionListSize; i++) {
		for (int j = 0, jSize = lMinion[i].size(); j < jSize; j++) {
			delete lMinion[i][j];
			jSize = lMinion[i].size();
		}
		lMinion[i].clear();
	}


}



/* ******************************************** */

void Map::setBackgroundColor(SDL_Renderer* rR) {
	switch (iLevelType) {
	case 0: case 2:
		SDL_SetRenderDrawColor(rR, 93, 148, 252, 255);
		break;
	case 1:
		SDL_SetRenderDrawColor(rR, 0, 0, 0, 255);
		break;
	default:
		SDL_SetRenderDrawColor(rR, 93, 148, 252, 255);
		break;
	}
}

std::string Map::getLevelName() {
	return "" + std::to_string(1 + currentLevelID / 4) + "-" + std::to_string(currentLevelID % 4 + 1);
}

void Map::loadMinionsLVL_1_1() {
	clearMinions();

	addGoombas(704, 368, true);
	addGoombas(680, 200, true);

	addGoombas(1280, 368, true);

	addGoombas(1632, 368, true);
	addGoombas(1680, 368, true);

	addGoombas(2560, 112, true);
	addGoombas(2624, 112, true);

	addGoombas(3104, 368, true);
	addGoombas(3152, 368, true);


	addGoombas(3648, 368, true);
	addGoombas(3696, 368, true);

	addGoombas(3968, 368, true);
	addGoombas(4016, 368, true);

	addGoombas(4096, 368, true);
	addGoombas(4144, 368, true);

	addGoombas(5568, 368, true);
	addGoombas(5612, 368, true);
}



void Map::createMap() {


	for (int i = 0; i < iMapWidth; i += 5) {
		std::vector<Minion*> temp;
		lMinion.push_back(temp);
	}

	iMinionListSize = lMinion.size();



	for (int i = 0; i < iMapWidth; i++) {
		std::vector<MapLevel*> temp;
		for (int i = 0; i < iMapHeight; i++) {
			temp.push_back(new MapLevel(0));
		}

		lMap.push_back(temp);
	}
	this->bTP = false;
}

void Map::checkSpawnPoint() {
	if (getNumOfSpawnPoints() > 1) {
		for (int i = iSpawnPointID + 1; i < getNumOfSpawnPoints(); i++) {
			if (getSpawnPointXPos(i) > oPlayer->getXPos() - fXPos && getSpawnPointXPos(i) < oPlayer->getXPos() - fXPos + 128) {
				iSpawnPointID = i;
			}
		}
	}
}

int Map::getNumOfSpawnPoints() {
	switch (currentLevelID) {
	case 0: case 1:
		return 2;
	}

	return 1;
}

int Map::getSpawnPointXPos(int iID) {
	switch (currentLevelID) {
	case 0:
		switch (iID) {
		case 0:
			return 84;
		case 1:
			return 82 * 32;
		}
	}

	return 84;
}

int Map::getSpawnPointYPos(int iID) {

	return CCFG::GAME_HEIGHT - 48 - oPlayer->getHitBoxY();
}

void Map::setSpawnPoint() {
	float X = (float)getSpawnPointXPos(iSpawnPointID);

	if (X > 6 * 32) {
		fXPos = -(X - 6 * 32);
		oPlayer->setXPos(6 * 32);
		oPlayer->setYPos((float)getSpawnPointYPos(iSpawnPointID));
	}
	else {
		fXPos = 0;
		oPlayer->setXPos(X);
		oPlayer->setYPos((float)getSpawnPointYPos(iSpawnPointID));
	}

	oPlayer->setMoveDirection(true);
}

void Map::resetGameData() {
	this->currentLevelID = 0;
	this->iSpawnPointID = 0;

	oPlayer->setCoins(0);
	oPlayer->setScore(0);
	oPlayer->resetPowerLVL();

	oPlayer->setNumOfLives(3);

	setSpawnPoint();

	loadLVL();
}

void Map::loadLVL() {

	switch (currentLevelID) {
	case 0:
		loadLVL_1_1();
		break;

	}
}

// ---------- LEVELTEXT -----

void Map::clearLevelText() {
	for (unsigned int i = 0; i < vLevelText.size(); i++) {
		delete vLevelText[i];
	}

	vLevelText.clear();
}

void Map::loadLVL_1_1() {
	clearMap();

	this->iMapWidth = 260;
	this->iMapHeight = 25;
	this->iLevelType = 0;
	this->iMapTime = 400;

	// ---------- LOAD LISTS ----------
	createMap();

	// ----- MINIONS
	loadMinionsLVL_1_1();


	// ----- GND -----

	structGND(0, 0, 69, 2);

	structGND(71, 0, 15, 2);

	structGND(89, 0, 64, 2);

	structGND(155, 0, 85, 2);

	// ----- GND -----

	// ----- GND 2 -----

	structGND2(134, 2, 4, true);
	structGND2(140, 2, 4, false);
	structGND2(148, 2, 4, true);
	structGND2(152, 2, 1, 4);
	structGND2(155, 2, 4, false);
	structGND2(181, 2, 8, true);
	structGND2(189, 2, 1, 8);

	structGND2(198, 2, 1, 1);

	// ----- GND 2 -----

	// --- P ---
	structBrick(2, 5, 1, 7);
	structBrick(3, 8, 3, 1);
	structBrick(3, 11, 3, 1);
	structBrick(5, 9, 1, 2);

	// --- E ---
	structBrick(7, 5, 1, 7);
	structBrick(8, 11, 3, 1);
	structBrick(8, 8, 3, 1);
	structBrick(8, 5, 3, 1);

	// --- T ---
	structBrick(12, 11, 5, 1);
	structBrick(14, 5, 1, 6);

	// --- E ---
	structBrick(18, 5, 1, 7);
	structBrick(19, 5, 2, 1);
	structBrick(19, 8, 3, 1);
	structBrick(19, 11, 3, 1);

	// --- T ---
	structBrick(23, 11, 5, 1);
	structBrick(25, 5, 1, 6);

	// --- E ---
	structBrick(29, 5, 1, 7);
	structBrick(30, 5, 3, 1);
	structBrick(30, 8, 3, 1);
	structBrick(30, 11, 3, 1);


	//--- R ---
	structBrick(34, 5, 1, 7);
	structBrick(35, 11, 3, 1);
	structBrick(36, 8, 2, 1);
	structBrick(37, 9, 1, 2);
	structBrick(35, 7, 1, 1);
	structBrick(36, 6, 1, 1);
	structBrick(37, 5, 1, 1);

	// --- H ---
	structBrick(39, 5, 1, 7);
	structBrick(40, 8, 2, 1);
	structBrick(42, 5, 1, 7);

	// --- A ---
	structBrick(46, 11, 2, 1);
	structBrick(45, 5, 1, 6);
	structBrick(46, 8, 2, 1);
	structBrick(48, 5, 1, 6);

	// --- O ---
	structBrick(50, 5, 1, 7);
	structBrick(51, 11, 2, 1);
	structBrick(51, 5, 2, 1);
	structBrick(53, 5, 1, 7);


	// ----- BRICK -----

	struckBlockQ(16, 5, 1);
	structBrick(20, 5, 1, 1);
	struckBlockQ(21, 5, 1);
	lMap[21][5]->setSpawnMushroom(true);

	struckBlockQ(22, 9, 1);
	struckBlockQ(23, 5, 1);


	struckBlockQ2(64, 6, 1);
	lMap[64][6]->setSpawnMushroom(true);
	lMap[64][6]->setPowerUP(false);

	structBrick(77, 5, 1, 1);
	struckBlockQ(78, 5, 1);
	lMap[78][5]->setSpawnMushroom(true);
	structBrick(79, 5, 1, 1);

	structBrick(80, 9, 8, 1);
	structBrick(91, 9, 3, 1);
	struckBlockQ(94, 9, 1);
	structBrick(94, 5, 1, 1);
	lMap[94][5]->setNumOfUse(4);

	structBrick(100, 5, 2, 1);

	struckBlockQ(106, 5, 1);
	struckBlockQ(109, 5, 1);
	struckBlockQ(109, 9, 1);
	lMap[109][9]->setSpawnMushroom(true);
	struckBlockQ(112, 5, 1);

	structBrick(118, 5, 1, 1);

	structBrick(121, 9, 3, 1);

	structBrick(128, 9, 1, 1);
	struckBlockQ(129, 9, 2);
	structBrick(131, 9, 1, 1);

	structBrick(129, 5, 2, 1);

	structBrick(168, 5, 2, 1);
	struckBlockQ(170, 5, 1);
	structBrick(171, 5, 1, 1);

	lMap[101][5]->setSpawnStar(true);

	// ----- BRICK -----

	// ----- PIPES -----

	//structPipe(28, 2, 1);
	//structPipe(38, 2, 2);
	//structPipe(46, 2, 3);
	structPipe(57, 2, 3);
	structPipe(163, 2, 1);
	structPipe(179, 2, 1);

	// ----- PIPES -----

	// ----- END

	structEnd(198, 3, 9);
	structCastleSmall(202, 2);

	// ----- MAP 1_1_2 -----

	this->iLevelType = 1;

	structGND(240, 0, 17, 2);

	structBrick(240, 2, 1, 11);
	structBrick(244, 2, 7, 3);
	structBrick(244, 12, 7, 1);

	structPipeVertical(255, 2, 10);
	structPipeHorizontal(253, 2, 1);

	structCoins(244, 5, 7, 1);
	structCoins(244, 7, 7, 1);
	structCoins(245, 9, 5, 1);

	// ----- END LEVEL

	this->iLevelType = 0;
}


/* ******************************************** */

// ----- POS 0 = TOP, 1 = BOT
bool Map::blockUse(int nX, int nY, int iBlockID, int POS) {
	if (POS == 0) {
		switch (iBlockID) {
		case 8: case 55: // ----- BlockQ
			if (lMap[nX][nY]->getSpawnMushroom()) {
				if (lMap[nX][nY]->getPowerUP()) {
					if (oPlayer->getPowerLVL() == 0) {
						lMinion[getListID(32 * nX)].push_back(new Mushroom(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, true, nX, nY));
					}
					else {
						lMinion[getListID(32 * nX)].push_back(new Flower(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, nX, nY));
					}
				}
				else {
					lMinion[getListID(32 * nX)].push_back(new Mushroom(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, false, nX, nY));
				}
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cMUSHROOMAPPER);
			}
			else {
				lCoin.push_back(new Coin(nX * 32 + 7, CCFG::GAME_HEIGHT - nY * 32 - 48));
				oPlayer->setScore(oPlayer->getScore() + 200);
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cCOIN);
				oPlayer->setCoins(oPlayer->getCoins() + 1);
			}

			if (lMap[nX][nY]->getNumOfUse() > 1) {
				lMap[nX][nY]->setNumOfUse(lMap[nX][nY]->getNumOfUse() - 1);
			}
			else {
				lMap[nX][nY]->setNumOfUse(0);
				lMap[nX][nY]->setBlockID(9);
			}

			lMap[nX][nY]->startBlockAnimation();
			checkCollisionOnTopOfTheBlock(nX, nY);
			break;
		case 13: case 28: // ----- Brick
			if (lMap[nX][nY]->getSpawnStar()) {
				lMap[nX][nY]->setBlockID(56);
				lMinion[getListID(32 * nX)].push_back(new Star(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, nX, nY));
				lMap[nX][nY]->startBlockAnimation();
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cMUSHROOMAPPER);
			}
			else if (lMap[nX][nY]->getSpawnMushroom()) {
				lMap[nX][nY]->setBlockID(56);
				if (lMap[nX][nY]->getPowerUP()) {
					if (oPlayer->getPowerLVL() == 0) {
						lMinion[getListID(32 * nX)].push_back(new Mushroom(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, true, nX, nY));
					}
					else {
						lMinion[getListID(32 * nX)].push_back(new Flower(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, nX, nY));
					}
				}
				else {
					lMinion[getListID(32 * nX)].push_back(new Mushroom(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, false, nX, nY));
				}
				lMap[nX][nY]->startBlockAnimation();
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cMUSHROOMAPPER);
			}
			else if (lMap[nX][nY]->getNumOfUse() > 0) {
				lCoin.push_back(new Coin(nX * 32 + 7, CCFG::GAME_HEIGHT - nY * 32 - 48));
				oPlayer->setScore(oPlayer->getScore() + 200);
				oPlayer->setCoins(oPlayer->getCoins() + 1);

				lMap[nX][nY]->setNumOfUse(lMap[nX][nY]->getNumOfUse() - 1);
				if (lMap[nX][nY]->getNumOfUse() == 0) {
					lMap[nX][nY]->setBlockID(56);
				}

				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cCOIN);

				lMap[nX][nY]->startBlockAnimation();
			}
			else {
				if (oPlayer->getPowerLVL() > 0) {
					lMap[nX][nY]->setBlockID(0);
					lBlockDebris.push_back(new BlockDebris(nX * 32, CCFG::GAME_HEIGHT - 48 - nY * 32));
					CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cBLOCKBREAK);
				}
				else {
					lMap[nX][nY]->startBlockAnimation();
					CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cBLOCKHIT);
				}
			}

			checkCollisionOnTopOfTheBlock(nX, nY);
			break;
		case 24: // ----- BlockQ2
			if (lMap[nX][nY]->getSpawnMushroom()) {
				if (lMap[nX][nY]->getPowerUP()) {
					if (oPlayer->getPowerLVL() == 0) {
						lMinion[getListID(32 * nX)].push_back(new Mushroom(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, true, nX, nY));
					}
					else {
						lMinion[getListID(32 * nX)].push_back(new Flower(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, nX, nY));
					}
				}
				else {
					lMinion[getListID(32 * nX)].push_back(new Mushroom(32 * nX, CCFG::GAME_HEIGHT - 16 - 32 * nY, false, nX, nY));
				}
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cMUSHROOMAPPER);
			}
			else {
				lCoin.push_back(new Coin(nX * 32 + 7, CCFG::GAME_HEIGHT - nY * 32 - 48));
				oPlayer->setCoins(oPlayer->getCoins() + 1);
				oPlayer->setScore(oPlayer->getScore() + 200);
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cCOIN);

				lMap[nX][nY]->startBlockAnimation();
			}

			lMap[nX][nY]->setBlockID(56);
			checkCollisionOnTopOfTheBlock(nX, nY);
			break;
			break;
		}
	}
	else if (POS == 1) {
		switch (iBlockID) {
		case 21: case 23: case 31: case 33:
			pipeUse();
			break;
		case 40: case 41:  // End
			EndUse();
			break;
		default:
			break;
		}
	}

	switch (iBlockID) {
	case 29: case 67: case 68: case 69:// COIN
		lMap[nX][nY]->setBlockID(0);
		oPlayer->addCoin();
		CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cCOIN);
		return false;
		break;
	case 36: case 38: case 60: case 62:
		pipeUse();
		break;

		break;
	default:
		break;
	}

	return true;
}

void Map::pipeUse() {
	for (unsigned int i = 0; i < lPipe.size(); i++) {
		lPipe[i]->checkUse();
	}
}

void Map::EndUse() {
	inEvent = true;

	oEvent->resetData();
	oPlayer->resetJump();
	oPlayer->stopMove();



	CCFG::getMusic()->StopMusic();
	CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cLEVELEND);

	oEvent->eventTypeID = oEvent->eEnd;

	if (oPlayer->getYPos() < CCFG::GAME_HEIGHT - 16 - 10 * 32) {
		oFlag->iPoints = 5000;
	}
	else if (oPlayer->getYPos() < CCFG::GAME_HEIGHT - 16 - 8 * 32) {
		oFlag->iPoints = 2000;
	}
	else if (oPlayer->getYPos() < CCFG::GAME_HEIGHT - 16 - 6 * 32) {
		oFlag->iPoints = 500;
	}
	else if (oPlayer->getYPos() < CCFG::GAME_HEIGHT - 16 - 4 * 32) {
		oFlag->iPoints = 200;
	}
	else {
		oFlag->iPoints = 100;
	}



	oEvent->vOLDDir.push_back(oEvent->eRIGHTEND);
	oEvent->vOLDLength.push_back(oPlayer->getHitBoxX());

	oEvent->vOLDDir.push_back(oEvent->eENDBOT1);
	oEvent->vOLDLength.push_back((CCFG::GAME_HEIGHT - 16 - 32 * 2) - oPlayer->getYPos() - oPlayer->getHitBoxY() - 2);

	oEvent->vOLDDir.push_back(oEvent->eENDBOT2);
	oEvent->vOLDLength.push_back((CCFG::GAME_HEIGHT - 16 - 32 * 2) - oPlayer->getYPos() - oPlayer->getHitBoxY() - 2);

	oEvent->vOLDDir.push_back(oEvent->eRIGHTEND);
	oEvent->vOLDLength.push_back(oPlayer->getHitBoxX());

	oEvent->vOLDDir.push_back(oEvent->eBOTRIGHTEND);
	oEvent->vOLDLength.push_back(32);

	oEvent->vOLDDir.push_back(oEvent->eRIGHTEND);
	oEvent->vOLDLength.push_back(132);

	oEvent->iSpeed = 3;

	switch (currentLevelID) {
	case 0: {
		oEvent->newLevelType = 100;

		oEvent->iDelay = 1500;
		oEvent->newCurrentLevel = 1;
		oEvent->inEvent = true;

		oEvent->newMapXPos = -210 * 32;
		oEvent->newPlayerXPos = 64;
		oEvent->newPlayerYPos = CCFG::GAME_HEIGHT - 48 - oPlayer->getHitBoxY();
		oEvent->newMoveMap = false;

		oPlayer->setMoveDirection(true);

		oEvent->reDrawX.push_back(204);
		oEvent->reDrawY.push_back(2);
		oEvent->reDrawX.push_back(204);
		oEvent->reDrawY.push_back(3);
		oEvent->reDrawX.push_back(205);
		oEvent->reDrawY.push_back(2);
		oEvent->reDrawX.push_back(205);
		oEvent->reDrawY.push_back(3);
		break;
	}

	}

	oEvent->vOLDDir.push_back(oEvent->eENDPOINTS);
	oEvent->vOLDLength.push_back(iMapTime);

	oEvent->vOLDDir.push_back(oEvent->eNOTHING);
	oEvent->vOLDLength.push_back(128);
}


void Map::EndBonus() {
	inEvent = true;

	oEvent->resetData();
	oPlayer->resetJump();
	oPlayer->stopMove();

	oEvent->eventTypeID = oEvent->eNormal;

	oEvent->iSpeed = 3;

	oEvent->newLevelType = iLevelType;
	oEvent->newCurrentLevel = currentLevelID;
	oEvent->newMoveMap = true;
	oEvent->iDelay = 0;
	oEvent->inEvent = false;

	oEvent->newMoveMap = true;

	oEvent->vOLDDir.push_back(oEvent->eNOTHING);
	oEvent->vOLDLength.push_back(1);
}

void Map::playerDeath(bool animation, bool instantDeath) {
	if ((oPlayer->getPowerLVL() == 0 && !oPlayer->getUnkillAble()) || instantDeath) {
		inEvent = true;

		oEvent->resetData();
		oPlayer->resetJump();
		oPlayer->stopMove();

		oEvent->iDelay = 150;
		oEvent->newCurrentLevel = currentLevelID;

		oEvent->newMoveMap = bMoveMap;

		oEvent->eventTypeID = oEvent->eNormal;

		oPlayer->resetPowerLVL();

		if (animation) {
			oEvent->iSpeed = 4;
			oEvent->newLevelType = iLevelType;

			oPlayer->setYPos(oPlayer->getYPos() + 4.0f);

			oEvent->vOLDDir.push_back(oEvent->eDEATHNOTHING);
			oEvent->vOLDLength.push_back(30);

			oEvent->vOLDDir.push_back(oEvent->eDEATHTOP);
			oEvent->vOLDLength.push_back(64);

			oEvent->vOLDDir.push_back(oEvent->eDEATHBOT);
			oEvent->vOLDLength.push_back(CCFG::GAME_HEIGHT - oPlayer->getYPos() + 128);
		}
		else {
			oEvent->iSpeed = 4;
			oEvent->newLevelType = iLevelType;

			oEvent->vOLDDir.push_back(oEvent->eDEATHTOP);
			oEvent->vOLDLength.push_back(1);
		}

		oEvent->vOLDDir.push_back(oEvent->eNOTHING);
		oEvent->vOLDLength.push_back(64);

		if (oPlayer->getNumOfLives() > 1) {
			oEvent->vOLDDir.push_back(oEvent->eLOADINGMENU);
			oEvent->vOLDLength.push_back(90);

			oPlayer->setNumOfLives(oPlayer->getNumOfLives() - 1);

			CCFG::getMusic()->StopMusic();
			CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cDEATH);
		}
		else {
			oEvent->vOLDDir.push_back(oEvent->eGAMEOVER);
			oEvent->vOLDLength.push_back(90);

			oPlayer->setNumOfLives(oPlayer->getNumOfLives() - 1);

			CCFG::getMusic()->StopMusic();
			CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cDEATH);
		}
	}
	else if (!oPlayer->getUnkillAble()) {
		oPlayer->setPowerLVL(oPlayer->getPowerLVL() - 1);
	}
}

void Map::startLevelAnimation() {


	switch (currentLevelID) {
	case 0:

		break;
	}
}

/* ******************************************** */

void Map::structGND(int X, int Y, int iWidth, int iHeight) {
	for (int i = 0; i < iWidth; i++) {
		for (int j = 0; j < iHeight; j++) {
			lMap[X + i][Y + j]->setBlockID(1);
		}
	}
}



// ----- true = LEFT, false = RIGHT -----
void Map::structGND2(int X, int Y, int iSize, bool bDir) {
	if (bDir) {
		for (int i = 0, k = 1; i < iSize; i++) {
			for (int j = 0; j < k; j++) {
				lMap[X + i][Y + j]->setBlockID(25);
			}
			++k;
		}
	}
	else {
		for (int i = 0, k = 1; i < iSize; i++) {
			for (int j = 0; j < k; j++) {
				lMap[X + iSize - 1 - i][Y + j]->setBlockID(25);
			}
			++k;
		}
	}
}

void Map::structGND2(int X, int Y, int iWidth, int iHeight) {
	for (int i = 0; i < iWidth; i++) {
		for (int j = 0; j < iHeight; j++) {
			lMap[X + i][Y + j]->setBlockID(25);
		}
	}
}

void Map::structPipe(int X, int Y, int iHeight) {
	for (int i = 0; i < iHeight; i++) {
		lMap[X][Y + i]->setBlockID(20);
		lMap[X + 1][Y + i]->setBlockID(22);
	}

	lMap[X][Y + iHeight]->setBlockID(21);
	lMap[X + 1][Y + iHeight]->setBlockID(23);
}

void Map::structPipeVertical(int X, int Y, int iHeight) {
	for (int i = 0; i < iHeight + 1; i++) {
		lMap[X][Y + i]->setBlockID(20);
		lMap[X + 1][Y + i]->setBlockID(22);
	}
}

void Map::structPipeHorizontal(int X, int Y, int iWidth) {
	lMap[X][Y]->setBlockID(62);
	lMap[X][Y + 1]->setBlockID(60);

	for (int i = 0; i < iWidth; i++) {
		lMap[X + 1 + i][Y]->setBlockID(61);
		lMap[X + 1 + i][Y + 1]->setBlockID(59);
	}

	lMap[X + 1 + iWidth][Y]->setBlockID(58);
	lMap[X + 1 + iWidth][Y + 1]->setBlockID(63);
}

void Map::structBrick(int X, int Y, int iWidth, int iHeight) {
	for (int i = 0; i < iWidth; i++) {
		for (int j = 0; j < iHeight; j++) {
			lMap[X + i][Y + j]->setBlockID(13);
		}
	}
}

void Map::struckBlockQ(int X, int Y, int iWidth) {
	for (int i = 0; i < iWidth; i++) {
		lMap[X + i][Y]->setBlockID(8);
	}
}

void Map::struckBlockQ2(int X, int Y, int iWidth) {
	for (int i = 0; i < iWidth; i++) {
		lMap[X + i][Y]->setBlockID(24);
	}
}

void Map::structCoins(int X, int Y, int iWidth, int iHeight) {
	for (int i = 0; i < iWidth; i++) {
		for (int j = 0; j < iHeight; j++) {
			lMap[X + i][Y + j]->setBlockID(71);
		}
	}
}

void Map::structEnd(int X, int Y, int iHeight) {
	for (int i = 0; i < iHeight; i++) {
		lMap[X][Y + i]->setBlockID(40);
	}

	oFlag = new Flag(X * 32 - 16, Y + iHeight + 72);

	lMap[X][Y + iHeight]->setBlockID(41);

	for (int i = Y + iHeight + 1; i < Y + iHeight + 4; i++) {
		lMap[X][i]->setBlockID(70);
	}
}

void Map::structCastleSmall(int X, int Y) {
	for (int i = 0; i < 2; i++) {
		lMap[X][Y + i]->setBlockID(43);
		lMap[X + 1][Y + i]->setBlockID(43);
		lMap[X + 3][Y + i]->setBlockID(43);
		lMap[X + 4][Y + i]->setBlockID(43);

		lMap[X + 2][Y + i]->setBlockID(47);
	}

	lMap[X + 2][Y + 1]->setBlockID(46);

	for (int i = 0; i < 5; i++) {
		lMap[X + i][Y + 2]->setBlockID(i == 0 || i == 4 ? 45 : 44);
	}

	lMap[X + 1][Y + 3]->setBlockID(48);
	lMap[X + 2][Y + 3]->setBlockID(43);
	lMap[X + 3][Y + 3]->setBlockID(49);

	for (int i = 0; i < 3; i++) {
		lMap[X + i + 1][Y + 4]->setBlockID(45);
	}
}





/* ******************************************** */

void Map::setBlockID(int X, int Y, int iBlockID) {
	if (X >= 0 && X < iMapWidth) {
		lMap[X][Y]->setBlockID(iBlockID);
	}
}

/* ******************************************** */

Player* Map::getPlayer() {
	return oPlayer;
}


float Map::getXPos() {
	return fXPos;
}

void Map::setXPos(float iXPos) {
	this->fXPos = iXPos;
}

float Map::getYPos() {
	return fYPos;
}

void Map::setYPos(float iYPos) {
	this->fYPos = iYPos;
}

int Map::getLevelType() {
	return iLevelType;
}

void Map::setLevelType(int iLevelType) {
	this->iLevelType = iLevelType;
}

int Map::getCurrentLevelID() {
	return currentLevelID;
}

void Map::setCurrentLevelID(int currentLevelID) {
	if (this->currentLevelID != currentLevelID) {
		this->currentLevelID = currentLevelID;
		oEvent->resetRedraw();
		loadLVL();
		iSpawnPointID = 0;
	}

	this->currentLevelID = currentLevelID;
}



void Map::setSpawnPointID(int iSpawnPointID) {
	this->iSpawnPointID = iSpawnPointID;
}

int Map::getMapTime() {
	return iMapTime;
}

void Map::setMapTime(int iMapTime) {
	this->iMapTime = iMapTime;
}

int Map::getMapWidth() {
	return iMapWidth;
}

bool Map::getMoveMap() {
	return bMoveMap;
}

void Map::setMoveMap(bool bMoveMap) {
	this->bMoveMap = bMoveMap;
}

bool Map::getDrawLines() {
	return drawLines;
}

void Map::setDrawLines(bool drawLines) {
	this->drawLines = drawLines;
}

/* ******************************************** */

Event* Map::getEvent() {
	return oEvent;
}

bool Map::getInEvent() {
	return inEvent;
}

void Map::setInEvent(bool inEvent) {
	this->inEvent = inEvent;
}

/* ******************************************** */

Block* Map::getBlock(int iID) {
	return vBlock[iID];
}

MapLevel* Map::getMapBlock(int iX, int iY) {
	return lMap[iX][iY];
}

Flag* Map::getFlag() {
	return oFlag;
}