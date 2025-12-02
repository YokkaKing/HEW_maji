/*
* ファイル名	Effect.h
* タイトル	エフェクト
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef EFFECT_H
#define EFFECT_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;
#include"direct3d.h"

class EFFECT
{
	public:
		bool		Enable;
		XMFLOAT3	Position;
		int			FrameCount;	//アニメーションカウンター

};
//メイン処理関数
void Effect_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Effect_Finalize();
void Effect_Update();
void Effect_Draw();

void CreateEffect(XMFLOAT2 Position);	//エフェクト作成

#endif // EFFECT_H