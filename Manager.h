/*
* ファイル名	Manger.h
* タイトル	マネージャー
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef MANAGER_H
#define MANAGER_H

#include "selectWeaponTerrain.h"

enum SCENE
{
	SCENE_NONE = 0,		//何もなし
	SCENE_TEAMLOGO,    //チームロゴシーン
	SCENE_TITLE,		//タイトルシーン

	SCENE_SELECT_MAP,    //武器・地形選択
	SCENE_SELECT_WT,    //武器・地形選択
	SCENE_GAME,			//ゲームシーン
	SCENE_RESULT,		//リザルトルシーン

};

void	Manager_Initialize();
void	Manager_Finalize();
void	Manager_Update();
void	Manager_Draw_Player1();
void	Manager_Draw_Player2();

inGameWTselect Manager_GetWTselect();
void	Manager_SetWTselect(const inGameWTselect& select);
int GetRoundCount();
void	SetScene(SCENE scene);
int GetP1WinCount();
int GetP2WinCount();
void ResetWinCount();
SCENE GetScene();

#endif // MANAGER_H