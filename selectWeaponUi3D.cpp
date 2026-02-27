
/* ファイル名	Selectweaponui3d.cpp
* タイトル	プレイヤー
* 作成者		カンジェウォン
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	マクロ定義
//================================================================


//================================================================
//	インクルード
//================================================================
#include"keyboard.h"
#include"Controller.h"
#include"Selectweaponui3d.h"
#include"Camera.h"
#include"shader.h"
#include"Transform.h"
#include"colliderFactory.h"
#include"debug_ostream.h"
#include"fade.h"
#include"sword.h"
#include"spear.h"
#include"hammer.h"
#include"arrow.h"
#include"syuriken.h"
#include"terrain.h"
#include<memory>
#include"generateWT.h"
#include "model.h"
#include"Audio.h"
#include"selectWeaponTerrain.h"
#include"Item.h"

//================================================================
//	グローバル変数
//================================================================
SELECTWEAPONUI3D	g_Selectweaponui3d[2];
static ID3D11Device* U_pDevice;
static ID3D11DeviceContext* U_pContext;
static LIGHTOBJECT		U_Light;
static MODEL* g_playerModels[2][5] = { nullptr };
void Selectweaponui3d_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	Camera_Initialize();	//カメラ初期化

	XMFLOAT4	para;

	para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);//環境光の色
	U_Light.SetAmbient(para);

	para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);//光の色
	U_Light.SetDiffuse(para);

	para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);//光方向
	float	len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
	para.x /= len;
	para.y /= len;
	para.z /= len;
	U_Light.SetDirection(para);//光の方向（正規化済）
	U_pDevice = pDevice;
	U_pContext = pContext;
	for (int i = 0; i < 2; i++)
	{
		g_Selectweaponui3d[i].m_position = XMFLOAT3(0.0f, 0.0f, 3.0f);
		g_Selectweaponui3d[i].m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Selectweaponui3d[i].m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

		g_Selectweaponui3d[i].m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
		g_Selectweaponui3d[i].m_model = ModelLoad("asset\\model\\default_sword.fbx");
		g_Selectweaponui3d[i].m_idle = false;
		g_Selectweaponui3d[i].m_attack = false;
		g_Selectweaponui3d[i].m_nowCursor = 0;
		ModelPlayClip(g_Selectweaponui3d[i].m_model, 0, 60, 60.0f, true);
	}
	g_playerModels[0][0] = ModelLoad("asset\\model\\default_sword.fbx");
	g_playerModels[0][1] = ModelLoad("asset\\model\\default_spear.fbx");
	g_playerModels[0][2] = ModelLoad("asset\\model\\default_bow.fbx");
	g_playerModels[0][3] = ModelLoad("asset\\model\\default_hammer.fbx");
	g_playerModels[0][4] = ModelLoad("asset\\model\\default_shuriken.fbx");

	g_playerModels[1][0] = ModelLoad("asset\\model\\default_sword.fbx");
	g_playerModels[1][1] = ModelLoad("asset\\model\\default_spear.fbx");
	g_playerModels[1][2] = ModelLoad("asset\\model\\default_bow.fbx");
	g_playerModels[1][3] = ModelLoad("asset\\model\\default_hammer.fbx");
	g_playerModels[1][4] = ModelLoad("asset\\model\\default_shuriken.fbx");

}
void Selectweaponui3d_Finalize()
{
	for (int i = 0; i < 2; i++)
	{
		ModelRelease(g_Selectweaponui3d[i].m_model);
	}

	Camera_Finalize();	//カメラ終了処理
}
void	Selectweaponui3d_Update()
{
	Camera_Update();

	for (int i = 0; i < 2; i++)
	{
		
		if (g_Selectweaponui3d[i].m_attack && ModelConsumeClipFinished(g_Selectweaponui3d[i].m_model))
		{

			g_Selectweaponui3d[i].m_attack = false;
			g_Selectweaponui3d[i].m_idle = false;
			if (!g_Selectweaponui3d[i].m_idle)
			{
				switch (g_Selectweaponui3d[i].m_nowCursor)
				{
				case 0:
					g_Selectweaponui3d[i].m_model = g_playerModels[i][g_Selectweaponui3d[i].m_nowCursor];
					ModelPlayClip(g_Selectweaponui3d[i].m_model, 0, 60, 60.0f, true);
					break;
				case 1:
					g_Selectweaponui3d[i].m_model = g_playerModels[i][g_Selectweaponui3d[i].m_nowCursor];

					ModelPlayClip(g_Selectweaponui3d[i].m_model, 0, 120, 60.0f, true);
					break;
				case 2:
					g_Selectweaponui3d[i].m_model = g_playerModels[i][g_Selectweaponui3d[i].m_nowCursor];

					ModelPlayClip(g_Selectweaponui3d[i].m_model, 0, 60, 60.0f, true);
					break;
				case 3:
					g_Selectweaponui3d[i].m_model = g_playerModels[i][g_Selectweaponui3d[i].m_nowCursor];

					ModelPlayClip(g_Selectweaponui3d[i].m_model, 0, 120, 60.0f, true);
					break;
				case 4:
					g_Selectweaponui3d[i].m_model = g_playerModels[i][g_Selectweaponui3d[i].m_nowCursor];

					ModelPlayClip(g_Selectweaponui3d[i].m_model, 0, 60, 60.0f, true);
					break;
				default:
					break;

				}
				g_Selectweaponui3d[i].m_idle = true;
			}
			
		}
		ModelUpdateAnimation(g_Selectweaponui3d[i].m_model, 1.0f / 60.0f);
	}

	

}

void Selectweaponui3d_Draw()
{
	Shader_Begin();
	U_Light.SetEnable(TRUE);			//ライティングON
	Shader_SetLight(U_Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);
	Camera_Draw();		//Drawの最初で呼ぶ！
	Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());

	XMMATRIX	scale = XMMatrixScaling(
		0.025f,
		0.025f,
		0.025f);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
		g_Selectweaponui3d[0].m_rotation.x,
		g_Selectweaponui3d[0].m_rotation.y-(XM_PI*0.2f),
		g_Selectweaponui3d[0].m_rotation.z);

	XMMATRIX translation = XMMatrixTranslation(
		g_Selectweaponui3d[0].m_position.x-2.5f,
		g_Selectweaponui3d[0].m_position.y - 1.2f,
		g_Selectweaponui3d[0].m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	Shader_SetBones(g_Selectweaponui3d[0].m_model);
	//モデルの描画リクエスト
	if (GetPlayerSelected(0) && GetCounter(0) >= 90.0f)
	{
		ModelDraw(g_Selectweaponui3d[0].m_model);
	}


	rotation = XMMatrixRotationRollPitchYaw(
		g_Selectweaponui3d[1].m_rotation.x,
		g_Selectweaponui3d[1].m_rotation.y + (XM_PI * 0.2f),
		g_Selectweaponui3d[1].m_rotation.z);

	translation = XMMatrixTranslation(
		g_Selectweaponui3d[1].m_position.x + 2.5f,
		g_Selectweaponui3d[1].m_position.y - 1.2f,
		g_Selectweaponui3d[1].m_position.z);
	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	Shader_SetBones(g_Selectweaponui3d[1].m_model);
	//モデルの描画リクエスト
	if (GetPlayerSelected(1)&&GetCounter(1)>=90.0f)
	{
		ModelDraw(g_Selectweaponui3d[1].m_model);
	}
	U_Light.SetEnable(FALSE);			//ライティングON
	Shader_SetLight(U_Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);
}
void Selectweaponui3d_ModelUpdate(int playerNum,int cursor)
{
	if (playerNum ==1)
	{
		g_Selectweaponui3d[0].m_model = g_playerModels[0][cursor];
	
		switch (cursor)
		{
		case 0:
			
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 0, 60, 60.0f, true);
			break;
		case 1:
		
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 0, 120, 60.0f, true);
			break;
		case 2:
			
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 0, 60, 60.0f, true);
			break;
		case 3:
		
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 0, 120, 60.0f, true);
			break;
		case 4:
			
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 0, 60, 60.0f, true);
			break;
		default:
			break;

		}
		g_Selectweaponui3d[0].m_nowCursor = cursor;
		g_Selectweaponui3d[0].m_attack = false;
		g_Selectweaponui3d[0].m_idle = true;


	}
	if (playerNum == 2)
	{
		g_Selectweaponui3d[1].m_model = g_playerModels[1][cursor];
		switch (cursor)
		{
		case 0:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 0, 60, 60.0f, true);
			break;
		case 1:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 0, 120, 60.0f, true);
			break;
		case 2:
		
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 0, 60, 60.0f, true);
			break;
		case 3:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 0, 120, 60.0f, true);
			break;
		case 4:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 0, 60, 60.0f, true);
			break;
		default:
			break;
		}
		g_Selectweaponui3d[1].m_nowCursor = cursor;
		g_Selectweaponui3d[1].m_attack = false;
		g_Selectweaponui3d[1].m_idle = true;

	}
}
void Selectweaponui3d_ModelAttack(int playerNum, int cursor)
{
	if (playerNum == 1)
	{
		g_Selectweaponui3d[0].m_model = g_playerModels[0][cursor];
		switch (cursor)
		{
		case 0:
		
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 167, 225, 60.0f, false, 2.0f);
			break;
		case 1:
			
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 420, 477, 60.0f, false, 2.0f);
			break;
		case 2:
			
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 301, 418, 60.0f, false, 4.0f);
			break;
		case 3:
			
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 260, 420, 60.0f, false, 2.0f);
			break;
		case 4:
	
			ModelPlayClip(g_Selectweaponui3d[0].m_model, 151, 208, 60.0f, false, 2.0f);
			break;
		default:
			break;

		}
		g_Selectweaponui3d[0].m_attack = true;
	}
	if (playerNum == 2)
	{
		g_Selectweaponui3d[1].m_model = g_playerModels[1][cursor];
		switch (cursor)
		{
		case 0:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 167, 225, 60.0f, false, 2.0f);
			break;
		case 1:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 420, 477, 60.0f, false, 2.0f);
			break;
		case 2:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 301,418, 60.0f, false, 4.0f);
			break;
		case 3:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 260, 420, 60.0f, false, 2.0f);
			break;
		case 4:
			
			ModelPlayClip(g_Selectweaponui3d[1].m_model, 151, 208, 60.0f, false, 2.0f);
			break;
		default:
			break;
		}
		g_Selectweaponui3d[1].m_attack = true;
	
	}
}


