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

void STAGE::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	m_model[0] = ModelLoad("asset\\model\\stadium.fbx");

	m_position = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_scale = XMFLOAT3(40.0f, 1.0f, 40.0f);
	m_tag = "Wall";
	m_layer = 0;

	// プレイヤーの当たり判定の追加
	auto collider = AddComponent<BoxCollider>(this, m_scale);
	ManagerCollider::AddCollider(collider);
}
void STAGE::Finalize()
{
	ModelRelease(m_model[0]);
}
void STAGE::Update()
{

}
void STAGE::Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		0.017f,
		0.017f,
		0.017f);
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

	//モデルの描画リクエスト
	ModelDraw(m_model[0]);
}
void STAGE::OnCollision(const CollisionInfo& info)
{

}