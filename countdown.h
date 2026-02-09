#pragma once
#include "direct3d.h"
void CountdownUI_Initialize(ID3D11Device* pDevice);
void CountdownUI_Finalize();

// dt: seconds (e.g. 1/60)
void CountdownUI_Update(float dt);

// 始まるときのカウント
// 例: CountdownUI_Start(4.0f);  -> 3,2,1,GO
void CountdownUI_Start(float seconds);

//これがtrueの間、プレイヤーを更新してはいけない（ゲームプレイをブロック）。
bool CountdownUI_IsBlockingGameplay();


void CountdownUI_DrawLabel(int label);
// 開始の 3,2,1,GO を描画（3,2,1だけSEを鳴らす）
void CountdownUI_DrawStart();

// 終了の 5..1 を描画（無音）
void CountdownUI_DrawEnd(float remain);

int  CountdownUI_GetStartLabel();         
int  CountdownUI_GetEndLabel(float remain);
