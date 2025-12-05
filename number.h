/*
* ファイル名	Number.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef NUMBER_H
#define NUMBER_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void Number_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Number_Finalize();
void Number_Update();
void Number_Draw();

#endif // NUMBER_H