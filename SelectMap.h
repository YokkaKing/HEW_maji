/*
* ファイル名	SelectMap.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef SELECTMAP_H
#define SELECTMAP_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"

void SelectMap_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void SelectMap_Finalize();
void SelectMap_Update();
void SelectMap_Draw();
int GetSelectedMapIndex();
#endif // SELECTMAP_H