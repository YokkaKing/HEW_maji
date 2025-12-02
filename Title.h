/*
* ファイル名	Title.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef TITLE_H
#define TITLE_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void Title_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Title_Finalize();
void Title_Update();
void Title_Draw();

#endif // TITLE_H