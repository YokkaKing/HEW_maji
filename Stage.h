/*
* ファイル名	Stage.h
* タイトル	ステージ
* 作成者		久保木幹太
* 作成日		1月30日
* 更新日		1月30日
*/

#ifndef STAGE_H
#define STAGE_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#include"direct3d.h"
using namespace DirectX;
#include"model.h"
#include"gameObject.h"
#include"Camera.h"

class STAGE : public GameObject
{
public:
	MODEL* m_model[2];
	ID3D11Device* g_pDevice;
	ID3D11DeviceContext* g_pContext;

public:
	void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Finalize();
	void Update();
	void Draw();
	void OnCollision(const CollisionInfo& info) override;
};

#endif // STAGE_H