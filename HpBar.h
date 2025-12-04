/*
* ファイル名	Hpbar.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef HPBAR_H
#define HPBAR_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void Hpbar_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Hpbar_Finalize();
void Hpbar_Update();
void Hpbar_Draw();

#endif // HPBAR_H