/*
* ファイル名    Result.cpp
* タイトル      Result（リザルト画面）
*
* 仕様:
*  - ステージ + 初期武器モデル + 変身先モデル2つ を同時に描画
*  - UI/地形/アイテム/当たり判定/プレイヤー処理などは一切なし
*/

#include "Result.h"

#include "direct3d.h" // LIGHTOBJECT
#include "keyboard.h"
#include "fade.h"
#include "Camera.h"
#include "shader.h"
#include "Manager.h"
#include "Stage.h"
#include "Winner.h"
#include "sprite.h"
#include "Result_Ui.h"
#include "Player.h"
#include "Player2.h"
#include <cassert>
#include "ResultSystem.h"

void Result_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    ResultSystem_Initialize(pDevice, pContext);
}

void Result_Finalize()
{
    ResultSystem_Finalize();
    Player2_AllCountReset();
    Player_AllCountReset();
}

void Result_Update()
{
    ResultSystem_Update();
}

void Result_Draw_Player1()
{
    ResultSystem_Draw_Player1();
}

void Result_Draw_Player2()
{
    ResultSystem_Draw_Player2();
}

// 互換用（従来呼び出しが残っててもP1描画でとりあえず動く）
void Result_Draw()
{
    ResultSystem_Draw_Player1();
}

