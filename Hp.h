/*
* ファイル名	Hp.h
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef HP_H
#define HP_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"
class HP
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
void Hp_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Hp_Finalize();
void Hp_Update();
void Hp_Draw();

#endif // HP_H