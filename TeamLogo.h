/*
* ファイル名	TeamLogo.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef TEAMLOGO_H
#define TEAMLOGO_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void TeamLogo_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void TeamLogo_Finalize();
void TeamLogo_Update();
void TeamLogo_Draw();

#endif // TEAMLOGO_H