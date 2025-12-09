/*
* ファイル名	Hp2.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef HP2_H
#define HP2_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"
class HP2
{
public:
    XMFLOAT4 col;
    XMFLOAT2 pos;
    XMFLOAT2 size;
    float maxHpBarSizeX;
    float maxHpBarPosX;
    float hpOldSizeX;
    float m_hp;
};
void Hp2_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Hp2_Finalize();
void Hp2_Update();
void Hp2_Draw();

#endif // HP2_H