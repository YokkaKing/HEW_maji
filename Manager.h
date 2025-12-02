
//Manager.h
#pragma once

enum SCENE
{
	SCENE_NONE = 0,		//何もなし
	SCENE_TITLE,		//タイトルシーン
	SCENE_GAME,			//ゲームシーン
	SCENE_RESULT,		//リザルトルシーン

};

void	Manager_Initialize();
void	Manager_Finalize();
void	Manager_Update();
void	Manager_Draw();

void	SetScene(SCENE scene);

