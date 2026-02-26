/*
* ファイル名	Stage.cpp
* タイトル	ステージ
* 作成者		久保木幹太
* 作成日		1月30日
* 更新日		1月30日
*/

//================================================================
//	インクルード
//================================================================
#include"Stage.h"
#include"managerCollider.h"
#include "SelectMap.h"
STAGE g_Stage;
void STAGE::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	g_Stage.m_model[0] = ModelLoad("asset\\model\\stadium.fbx");
	g_Stage.m_model[1] = ModelLoad("asset\\model\\stadium_lava.fbx");

	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_scale = XMFLOAT3(0.0221f, 0.0221f, 0.0221f);
	if (GetSelectedMapIndex()==0)
	{
		g_Stage.m_stageType = STAGE_TYPE::PLANE;
	}
	else
	{
		g_Stage.m_stageType = STAGE_TYPE::LAVA;
	}
		
}
void STAGE::Finalize()
{
	for (int i = 0; i < 2; i++)
	{
		ModelRelease(g_Stage.m_model[i]);
	}
}
void STAGE::Update()
{

}
void STAGE::Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		m_scale.x,
		m_scale.y,
		m_scale.z);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		m_rotation.x,
		m_rotation.y,
		m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		m_position.x,
		m_position.y,
		m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	unsigned int modelType = 0; // モデルの種類

	if (g_Stage.m_stageType == STAGE_TYPE::LAVA)
	{
		modelType = 1;
	}
	//モデルの描画リクエスト
	ModelDraw(g_Stage.m_model[modelType]);
}

const STAGE_TYPE STAGE::GetStageType()
{
	return g_Stage.m_stageType;
}
STAGE_TYPE GetStage()
{
	return g_Stage.m_stageType;
}