/*
* ファイル名    Result.h
* タイトル      Result（リザルト画面）
*
* 仕様（最低限）:
*  - Resultは「ステージ + 初期武器モデル + 変身先モデル2つ」だけ描画
*  - UI/地形/当たり判定/アイテム等は一切描画しない
*  - カメラは1つのビューでOK（左右同じにする想定）
*/

#ifndef RESULT_H
#define RESULT_H

#include <d3d11.h>

void Result_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Result_Finalize();
void Result_Update();
void Result_Draw_Player1();
void Result_Draw_Player2();
void Result_Draw();

#endif // RESULT_H
