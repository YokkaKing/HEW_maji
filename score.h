/*
* ファイル名	Score.h
* タイトル	タイトル
* 作成者		カンジェウォン
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef SCORE_H
#define SCORE_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"
#include <utility> // std::swap



class SCORE
{
public:
    XMFLOAT4 col[5];
    XMFLOAT2 pos[5];
    XMFLOAT2 size[5];
    float frame[5];
};

void Score_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Score_Finalize();
void Score_Update();
void Score_Draw();
void Score_BeginShow(int roundResult);
#endif // SCORE_H