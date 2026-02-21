/*
* ファイル名	Selectweaponui3d.h
* タイトル	プレイヤー
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef SELECTWEAPONUI3DUI_H
#define SELECTWEAPONUI3DUI_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#include"direct3d.h"
using namespace DirectX;
#include"model.h"
#include"gameObject.h"
#include "IWeapon.h"
#include"selectWeaponTerrain.h"
#include"hitAction.h"

class SELECTWEAPONUI3D : public GameObject
{
public:
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMFLOAT3 m_velocity;
	XMFLOAT3 m_scale;
	bool m_idle;
	bool m_attack;
	int m_nowCursor;


};



void Selectweaponui3d_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Selectweaponui3d_Finalize();
void Selectweaponui3d_Update();
void Selectweaponui3d_Draw();
void Selectweaponui3d_ModelUpdate(int playerNum,int cursor);
void Selectweaponui3d_ModelAttack(int playerNum, int cursor);
#endif