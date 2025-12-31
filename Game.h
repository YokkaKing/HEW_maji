/*
* ファイル名	Game.h
* タイトル	ゲーム
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef GAME_H
#define GAME_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Game_Finalize();
void Game_Update();
void Game_Draw_Player1();
void Game_Draw_Player2();


#endif // GAME_H