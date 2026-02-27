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
#include <utility> // std::swap



class HP
{
public:
    XMFLOAT4 col;
    XMFLOAT2 pos;
    XMFLOAT2 size;
    float redHpLen;
    float m_Hp;
    float hpTimer;
    XMFLOAT2 shakeOffset;    // current shake offset to add to pos when drawing
    float shakeTimer;        // remaining shake time
    float shakeDuration;     // total shake duration
    float shakeMagnitude;    // maximum amplitude
    float prevHp;            // previous frame hp for change detection
};
class HP_2P
{
public:
    XMFLOAT4 col;
    XMFLOAT2 pos;
    XMFLOAT2 size;
    float redHpLen;
    float m_Hp;
    float hpTimer;
    XMFLOAT2 shakeOffset;
    float shakeTimer;
    float shakeDuration;
    float shakeMagnitude;
    float prevHp;
};
class TIMER
{
public:
    XMFLOAT2 pos;
    XMFLOAT2 size;
    XMFLOAT4 col;
    float time;
    float frame;
};
class STATUS_1P
{
public:
    XMFLOAT2 pos[3];
    XMFLOAT2 size[2];
    XMFLOAT4 col;
	int nowType;
    int nextType[2];

};
class STATUS_2P
{
public:
    XMFLOAT2 pos[3];
    XMFLOAT2 size[2];
    XMFLOAT4 col;
    int nowType;
    int nextType[2];
};
void Hp_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Hp_Finalize();
void Hp_Update();
void Hp_Draw();

float Hp_GetTime(); //時間制限用の関数
void Hp_SetTime(float time); //時間制限のリセット用

void SetIsItemAlarmUse(bool use,int type);

#endif // HP_H