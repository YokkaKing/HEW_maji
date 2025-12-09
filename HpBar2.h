/*
* ファイル名	HpBar2.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef HpBar2_H
#define HpBar2_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void HpBar2_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void HpBar2_Finalize();
void HpBar2_Update();
void HpBar2_Draw();

#endif // HpBar2_H