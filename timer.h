/*
* ファイル名	Timer.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef TIMER_H
#define TIMER_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void Timer_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Timer_Finalize();
void Timer_Update();
void Timer_Draw();

#endif // TIMER_H