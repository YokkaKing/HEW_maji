/*
* ファイル名	Camera.cpp
* タイトル	カメラ
* 作成者		鈴木豪
* 作成日		12月02日
* 更新日		12月17日
*/

#include"Camera.h"
#include"keyboard.h"
#include"Controller.h"
#include"Player.h"
#include"Player2.h"
#include"Viewport.h"
#include"shader.h"
#include "terrain.h"

#define ROTATION_X_MAX (45.0f)
#define ROTATION_Y_MAX (90.0f)

//================================================================
//	グローバル変数
//================================================================
static CAMERA	CameraObject;
static CAMERA  Camera2Object;
XMFLOAT3 g_PlayerPosOld;
XMFLOAT3 g_Player2PosOld;
extern Controller g_Controller;
extern TERRAIN g_Terrain;
extern PLAYER g_Player;
extern PLAYER2 g_Player2;
extern std::vector<GameObject*> g_gameObjects;

// カメラが障害物から離れるためのパディング距離
const float CAMERA_COLLISION_PADDING = 0.2f;
const float FIXED_DELTATIME = 1.0f / 60.0f;

void Camera_Initialize()
{ 
	CameraObject.Position = XMFLOAT3(0.0f, 3.0f, -4.0f);
	CameraObject.AtPosition = XMFLOAT3(0.0f, 1.0f, 0.0f);
	CameraObject.UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

	Camera2Object.Position = XMFLOAT3(2.0f, 3.0f, -4.0f);
	Camera2Object.AtPosition = XMFLOAT3(0.0f, 1.0f, 0.0f);
	Camera2Object.UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

	CameraObject.Fov = 45.0f;
	Camera2Object.Fov = 45.0f;

	CameraObject.suzukiTime = 0.0f;
	Camera2Object.suzukiTime = 0.0f;

	float width = (float)Direct3D_GetBackBufferWidth();
	float height = (float)Direct3D_GetBackBufferHeight();
	CameraObject.Aspect = (width / height) / 2;
	CameraObject.NearClip = 0.5f;
	CameraObject.FarClip = 1000.0f;

	Camera2Object.Aspect = (width / height) / 2;
	Camera2Object.NearClip = 0.5f;
	Camera2Object.FarClip = 1000.0f;

	g_PlayerPosOld = GetPlayerPosition();//<<<<<<<<<<<<<<<<
	g_Player2PosOld = GetPlayer2Position();
}

void Camera_Finalize()
{
	return;
}

void Camera_Update()
{
	//プレイヤーの座標取得<<<<<<<<<<<<<<<<<<<<<<
	XMFLOAT3 playerPos = GetPlayerPosition();//P1
	XMFLOAT3 diff; //現在と過去のプレイヤーのposの差
	//前回のプレイヤーと現在のプレイヤーの座標の差分<<<<<<<<<<<<<<<
	//P1
	diff.x = playerPos.x - g_PlayerPosOld.x;
	diff.y = playerPos.y - g_PlayerPosOld.y;
	diff.z = playerPos.z - g_PlayerPosOld.z;

	//カメラを移動
	//P1
	CameraObject.Position.x += diff.x;
	CameraObject.Position.y += diff.y;
	CameraObject.Position.z += diff.z;

	//注視点としてセット<<<<<<<<<<<<<<<<<<<<<<<
	//P1
	CameraObject.AtPosition.x = playerPos.x;
	CameraObject.AtPosition.y = playerPos.y;
	CameraObject.AtPosition.z = playerPos.z;

	g_PlayerPosOld.x = playerPos.x;
	g_PlayerPosOld.y = playerPos.y;
	g_PlayerPosOld.z = playerPos.z;

	//ローテーション変数初期化
	float rotX = 0.0f;
	float rotY = 0.0f;

	//回転角度の累積と制限
	//現在の累積角度を保持する静的変数 (初期値 0.0f)
	static float nowYaw = 0.0f;   // 水平回転 (左右)
	static float nowPitch = 0.0f; // 垂直回転 (上下)

	//コントローラー・キーボードからの入力を取得
	float inputX = g_Controller.GetRightStickX() * -2.0f;
	float inputY = g_Controller.GetRightStickY() * 1.5f;

	if (Keyboard_IsKeyDown(KK_Q)) inputX = 1.0f;
	if (Keyboard_IsKeyDown(KK_E)) inputX = -1.0f;
	if (Keyboard_IsKeyDown(KK_UP)) inputY = 1.0f;
	if (Keyboard_IsKeyDown(KK_DOWN)) inputY = -1.0f;

	//角度を更新
	nowYaw += inputX;
	nowPitch += inputY;

	//回転制限の適用
	//左右180度制限 (-90度 から +90度 = 合計180度)
	if (nowYaw > ROTATION_Y_MAX)  nowYaw = ROTATION_Y_MAX;
	if (nowYaw < -ROTATION_Y_MAX) nowYaw = -ROTATION_Y_MAX;

	//上下45度制限 (水平を0度として +-45度)
	if (nowPitch > ROTATION_X_MAX)  nowPitch = ROTATION_X_MAX;
	if (nowPitch < -ROTATION_X_MAX) nowPitch = -ROTATION_X_MAX;

	//座標の再計算
	//プレイヤーからの基本距離 (初期設定 -4.0f に基づき 4.0f)
	float distance = 4.0f;

	//回転行列の作成 (Yaw, Pitch, Roll)
	XMMATRIX matRot = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(nowPitch),
		XMConvertToRadians(nowYaw),
		0.0f
	);

	//プレイヤーの背面を基準にしたオフセットベクトルを回転させる
	XMVECTOR vOffset = XMVectorSet(0.0f, 0.0f, -distance, 0.0f);
	vOffset = XMVector3TransformNormal(vOffset, matRot);

	//プレイヤー座標（注視点）にオフセットを加算
	XMVECTOR vAt = XMLoadFloat3(&CameraObject.AtPosition);
	XMVECTOR vNewPos = XMVectorAdd(vAt, vOffset);

	//高さを少し上げる調整 (プレイヤーの足元ではなく腰から頭付近を映す)
	vNewPos = XMVectorSetY(vNewPos, XMVectorGetY(vNewPos) + 1.0f);

	XMStoreFloat3(&CameraObject.Position, vNewPos);
	//===========================================
	//	レイキャスティング処理(クリッピング回避)
	//===========================================

	//理想的なカメラ位置を保存 
	XMFLOAT3 P_ideal = CameraObject.Position;
	XMFLOAT3 P_player = CameraObject.AtPosition;

	XMVECTOR V_ideal = XMLoadFloat3(&P_ideal);
	XMVECTOR V_player = XMLoadFloat3(&P_player);
	XMVECTOR idealToPlayer = XMVectorSubtract(V_player, V_ideal);
	float maxDistance = XMVectorGetX(XMVector3Length(idealToPlayer));//レイの最大距離
	XMVECTOR dir = XMVector3Normalize(idealToPlayer);

	XMFLOAT3 dir_f3;
	XMStoreFloat3(&dir_f3, dir);

	//Raycast 構造体を初期化
	Raycast cameraRay(P_ideal, dir_f3);

	//衝突情報の初期化
	Ray_HitInfo nearestHit;
	nearestHit.m_distance = maxDistance;
	nearestHit.m_hitObject = nullptr;

	//全ての透過オブジェクトフラグをリセット
	for (auto obj : g_gameObjects)
	{
		obj->m_isTransparent = false;
	}

	//--- 地形オブジェクトに対する衝突判定 ---
	//PerformCameraRaycast(g_Terrain.hills, cameraRay, nearestHit);
	PerformCameraRaycast(g_gameObjects, cameraRay, nearestHit);
	//------------------------------------------

	//衝突オブジェクトの処理
	if (nearestHit.m_hitObject != nullptr)
	{
		//衝突したオブジェクトがterrain.wallだった場合透過フラグを立てる
		//※他の障害物オブジェクトでも同様の処理を入れる必要あり
		if (nearestHit.m_hitObject->m_tag == "HILL" || nearestHit.m_hitObject->m_tag == "WALL")
		{
			//プレイヤーが視認できない (レイがプレイヤーに到達する前に障害物に遮られた)
			//衝突した障害物に透過フラグを立てる
			nearestHit.m_hitObject->m_isTransparent = true;
		}
	}


	//FOVの変更(P1)
	if (Keyboard_IsKeyDown(KK_Z))
	{
		CameraObject.Fov += 0.3f;
		if (CameraObject.Fov > 160.0f)
		{
			CameraObject.Fov = 160.0f;
		}

	}
	if (Keyboard_IsKeyDown(KK_X))
	{
		CameraObject.Fov -= 0.3f;
		if (CameraObject.Fov < 5.0f)
		{
			CameraObject.Fov = 5.0f;
		}
	}

	return;
}

void Camera2_Update()
{
	//プレイヤーの座標取得<<<<<<<<<<<<<<<<<<<<<<
	XMFLOAT3 player2Pos = GetPlayer2Position();//P1
	XMFLOAT3 diff; //現在と過去のプレイヤーのposの差
	//前回のプレイヤーと現在のプレイヤーの座標の差分<<<<<<<<<<<<<<<
	//P1
	diff.x = player2Pos.x - g_Player2PosOld.x;
	diff.y = player2Pos.y - g_Player2PosOld.y;
	diff.z = player2Pos.z - g_Player2PosOld.z;

	//カメラを移動
	//P1
	Camera2Object.Position.x += diff.x;
	Camera2Object.Position.y += diff.y;
	Camera2Object.Position.z += diff.z;

	//注視点としてセット<<<<<<<<<<<<<<<<<<<<<<<
	//P1
	Camera2Object.AtPosition.x = player2Pos.x;
	Camera2Object.AtPosition.y = player2Pos.y;
	Camera2Object.AtPosition.z = player2Pos.z;

	g_Player2PosOld.x = player2Pos.x;
	g_Player2PosOld.y = player2Pos.y;
	g_Player2PosOld.z = player2Pos.z;

	//ローテーション変数初期化
	float rotX = 0.0f;
	float rotY = 0.0f;

	//回転角度の累積と制限
	//現在の累積角度を保持する静的変数 (初期値 0.0f)
	static float nowYaw = 0.0f;   // 水平回転 (左右)
	static float nowPitch = 0.0f; // 垂直回転 (上下)

	//コントローラー・キーボードからの入力を取得
	float inputX = g_Controller.GetRightStickX() * -2.0f;
	float inputY = g_Controller.GetRightStickY() * 1.5f;

	if (Keyboard_IsKeyDown(KK_Y)) inputX = 1.0f;
	if (Keyboard_IsKeyDown(KK_I)) inputX = -1.0f;
	if (Keyboard_IsKeyDown(KK_G)) inputY = 1.0f;
	if (Keyboard_IsKeyDown(KK_V)) inputY = -1.0f;

	//角度を更新
	nowYaw += inputX;
	nowPitch += inputY;

	//回転制限の適用
	//左右180度制限 (-90度 から +90度 = 合計180度)
	if (nowYaw > ROTATION_Y_MAX)  nowYaw = ROTATION_Y_MAX;
	if (nowYaw < -ROTATION_Y_MAX) nowYaw = -ROTATION_Y_MAX;

	//上下45度制限 (水平を0度として +-45度)
	if (nowPitch > ROTATION_X_MAX)  nowPitch = ROTATION_X_MAX;
	if (nowPitch < -ROTATION_X_MAX) nowPitch = -ROTATION_X_MAX;

	//座標の再計算
	//プレイヤーからの基本距離 (初期設定 -4.0f に基づき 4.0f)
	float distance = 4.0f;

	//回転行列の作成 (Yaw, Pitch, Roll)
	XMMATRIX matRot = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(nowPitch),
		XMConvertToRadians(nowYaw),
		0.0f
	);

	//プレイヤーの背面を基準にしたオフセットベクトルを回転させる
	XMVECTOR vOffset = XMVectorSet(0.0f, 0.0f, -distance, 0.0f);
	vOffset = XMVector3TransformNormal(vOffset, matRot);

	//プレイヤー座標（注視点）にオフセットを加算
	XMVECTOR vAt = XMLoadFloat3(&Camera2Object.AtPosition);
	XMVECTOR vNewPos = XMVectorAdd(vAt, vOffset);

	//高さを少し上げる調整 (プレイヤーの足元ではなく腰から頭付近を映す)
	vNewPos = XMVectorSetY(vNewPos, XMVectorGetY(vNewPos) + 1.0f);

	XMStoreFloat3(&Camera2Object.Position, vNewPos);
	//=====================================
	//	レイキャスティング処理
	//=====================================
	
	//理想的なカメラ位置を保存 
	XMFLOAT3 P_ideal2 = Camera2Object.Position;
	XMFLOAT3 P_player2 = Camera2Object.AtPosition;

	XMVECTOR V_ideal2 = XMLoadFloat3(&P_ideal2);
	XMVECTOR V_player2 = XMLoadFloat3(&P_player2);
	XMVECTOR idealToPlayer2 = XMVectorSubtract(V_player2, V_ideal2);
	float maxDistance2 = XMVectorGetX(XMVector3Length(idealToPlayer2));
	XMVECTOR dir = XMVector3Normalize(idealToPlayer2);

	XMFLOAT3 dir_f3;
	XMStoreFloat3(&dir_f3, dir);

	//Raycast 構造体を初期化
	Raycast cameraRay2(P_ideal2, dir_f3);

	//衝突情報の初期化
	Ray_HitInfo nearestHit2;
	nearestHit2.m_distance = maxDistance2;
	nearestHit2.m_hitObject = nullptr;

		//--- 地形オブジェクトに対する衝突判定 ---
		PerformCameraRaycast(g_gameObjects, cameraRay2, nearestHit2);
		//PerformCameraRaycast(g_Terrain.walls, cameraRay2, nearestHit2);
		//------------------------------------------

	//衝突オブジェクトの処理
		if (nearestHit2.m_hitObject != nullptr)
		{
			//衝突したオブジェクトがterrain.wallだった場合透過フラグを立てる
			//※他の障害物オブジェクトでも同様の処理を入れる必要あり
			if (nearestHit2.m_hitObject->m_tag == "HILL" || nearestHit2.m_hitObject->m_tag == "WALL")
			{
				//プレイヤーが視認できない (レイがプレイヤーに到達する前に障害物に遮られた)
				//衝突した障害物に透過フラグを立てる
				nearestHit2.m_hitObject->m_isTransparent = true;
			}
		}

	//FOVの変更(P2)
	if (Keyboard_IsKeyDown(KK_N))
	{
		Camera2Object.Fov += 0.3f;
		if (Camera2Object.Fov > 160.0f)
		{
			Camera2Object.Fov = 160.0f;
		}

	}
	if (Keyboard_IsKeyDown(KK_M))
	{
		Camera2Object.Fov -= 0.3f;
		if (Camera2Object.Fov < 5.0f)
		{
			Camera2Object.Fov = 5.0f;
		}
	}

	return;
}

void Camera_Draw()
{ 
	//P1プロジェクション行列作成
	CameraObject.Projection = XMMatrixPerspectiveFovLH
	(
		XMConvertToRadians(CameraObject.Fov),
		CameraObject.Aspect,
		CameraObject.NearClip,
		CameraObject.FarClip
	);

	//ビュー行列作成
	XMVECTOR	vpos = XMVectorSet(
		CameraObject.Position.x,
		CameraObject.Position.y,
		CameraObject.Position.z,
		0.0f);
	XMVECTOR	vAt = XMVectorSet(
		CameraObject.AtPosition.x,
		CameraObject.AtPosition.y,
		CameraObject.AtPosition.z,
		0.0f
	);
	XMVECTOR	vUp = XMVectorSet(
		CameraObject.UpVector.x,
		CameraObject.UpVector.y,
		CameraObject.UpVector.z,
		0.0f
	);
	CameraObject.View = XMMatrixLookAtLH(
		vpos,
		vAt,
		vUp
	);

	
	return;

}

void Camera2_Draw()
{
	//P2プロジェクション行列作成
	Camera2Object.Projection = XMMatrixPerspectiveFovLH
	(
		XMConvertToRadians(Camera2Object.Fov),
		Camera2Object.Aspect,
		Camera2Object.NearClip,
		Camera2Object.FarClip
	);

	//ビュー行列作成
	XMVECTOR	vpos2 = XMVectorSet(
		Camera2Object.Position.x,
		Camera2Object.Position.y,
		Camera2Object.Position.z,
		0.0f);
	XMVECTOR	vAt2 = XMVectorSet(
		Camera2Object.AtPosition.x,
		Camera2Object.AtPosition.y,
		Camera2Object.AtPosition.z,
		0.0f
	);
	XMVECTOR	vUp2 = XMVectorSet(
		Camera2Object.UpVector.x,
		Camera2Object.UpVector.y,
		Camera2Object.UpVector.z,
		0.0f
	);
	Camera2Object.View = XMMatrixLookAtLH(
		vpos2,
		vAt2,
		vUp2
	);

	return;
}

void	SetCameraFov(float fov)
{
	CameraObject.Fov = fov;
}
void	SetCameraAspect(float asp)
{ 
	CameraObject.Aspect = asp;
}
void	SetCameraClip(float n, float f)
{ 
	CameraObject.NearClip = n;
	CameraObject.FarClip = f;
}

void	SetCamera2Fov(float fov)
{
	Camera2Object.Fov = fov;
}
void	SetCamera2Aspect(float asp)
{
	Camera2Object.Aspect = asp;
}
void	SetCamera2Clip(float n, float f)
{
	Camera2Object.NearClip = n;
	Camera2Object.FarClip = f;
}

void	SetCameraPosition(XMFLOAT3 pos)
{
	CameraObject.Position = pos;
}
void	SetCameraAtPosition(XMFLOAT3 at)
{
	CameraObject.AtPosition = at;
}
void	SetCameraUpVector(XMFLOAT3 up)
{ 
	CameraObject.UpVector = up;
}

void	SetCamera2Position(XMFLOAT3 pos)
{
	Camera2Object.Position = pos;
}
void	SetCamera2AtPosition(XMFLOAT3 at)
{
	Camera2Object.AtPosition = at;
}
void	SetCamera2UpVector(XMFLOAT3 up)
{
	Camera2Object.UpVector = up;
}

CAMERA& GetCameraObj()
{
	return CameraObject;
}

CAMERA& GetCamera2Obj()
{
	return Camera2Object;
}

XMMATRIX	GetViewMatrix()
{ 
	return	CameraObject.View;
}
XMMATRIX	GetProjectionMatrix()
{
	return	CameraObject.Projection;
}

XMMATRIX	GetViewMatrix2()
{
	return	Camera2Object.View;
}
XMMATRIX	GetProjectionMatrix2()
{
	return	Camera2Object.Projection;
}

XMFLOAT3 GetCameraAtPosition()
{
	return CameraObject.AtPosition;
}

XMFLOAT3 GetCameraPosition()
{
	return CameraObject.Position;
}

XMFLOAT3 GetCamera2AtPosition()
{
	return Camera2Object.AtPosition;
}

XMFLOAT3 GetCamera2Position()
{
	return Camera2Object.Position;
}

void PerformCameraRaycast(const std::vector<GameObject*>& gameObjects, Raycast& cameraRay, Ray_HitInfo& nearestHit)
{
	for (GameObject* gameObj : gameObjects)
	{
		if (gameObj && gameObj->m_isEnable)
		{ //有効なオブジェクトのみ
			auto colliders = gameObj->GetColliders<Collider>();

			for (const auto& collider : colliders)
			{
				Ray_HitInfo tempHit = nearestHit;

				//if (collider->type == ColliderType::Box)
				if(auto boxCollider = std::dynamic_pointer_cast<BoxCollider>(collider))
				{//BoxColliderの場合

					//const BoxCollider* boxCollider = static_cast<const BoxCollider*>(collider.get());
					//InterSector::InterSects(cameraRay, boxCollider, nearestHit);

					//shared_ptrから生のポインタ(boxCollider.get())を取得して渡す
					bool hit = InterSector::InterSects(cameraRay, boxCollider.get(), tempHit);

					//InterSectsがtrueを返した場合(より近い衝突)を検出した場合
					//tempHitの持つ全ての情報(m_hitObject含む)でnearesthitを完全に更新する
					if (hit)
					{
						nearestHit = tempHit;
					}
				}
				// SphereColliderなど、他のコライダーがある場合はここに追加


			}
		}
	}
}
//====================================
//	メモ書き
//====================================

	//XMFLOAT2 vec2;// 横と前後(x,z)地面に対して平行な移動
	//vec2.x = CameraObject.Position.y - CameraObject.AtPosition.y;
	//vec2.y = CameraObject.Position.z - CameraObject.AtPosition.z;// vec変数はXMFLOAT2のためyに値を入れているが実際の値はz

	// Rotation2(X軸回転)
	//CameraObject.Position.y = (vec2.x * co2 - vec2.y * si2);
	//CameraObject.Position.z = (vec2.x * si2 + vec2.y * co2);
	//CameraObject.Position.y += CameraObject.AtPosition.y;
	//CameraObject.Position.z += CameraObject.AtPosition.z;

	////vecを正規化する
	//float len = sqrtf(vec.x * vec.x + vec.y * vec.y);
	//vec.x /= len;
	//vec.y /= len;

	////注視点の方向へ移動する
	//float	speed = 0.0f;
	//if (Keyboard_IsKeyDown(KK_W))
	//{
	//	speed = -0.1f;
	//}
	//if (Keyboard_IsKeyDown(KK_S))
	//{
	//	speed = 0.1f;
	//}

	////今回の移動量ベクトル
	//vec.x *= speed;
	//vec.y *= speed;

	////座標と注視点へ移動量を加算
	//CameraObject.Position.x += vec.x;
	//CameraObject.Position.z += vec.y;
	//CameraObject.AtPosition.x += vec.x;
	//CameraObject.AtPosition.z += vec.y;

	//if (Keyboard_IsKeyDown(KK_Q))
	//{
	//	Rotation = 1.0f;
	//}
	//if (Keyboard_IsKeyDown(KK_E))
	//{
	//	Rotation = -1.0f;
	//}
	//if (Keyboard_IsKeyDown(KK_LEFT))
	//{
	//	Rotation = 1.0f;
	//}
	//if (Keyboard_IsKeyDown(KK_RIGHT))
	//{
	//	Rotation = -1.0f;
	//}

	//注視点からカメラへのベクトル
	//P1
	//XMFLOAT2 vec;
	//vec.x = CameraObject.Position.x - CameraObject.AtPosition.x;
	//vec.y = CameraObject.Position.z - CameraObject.AtPosition.z;

	////ベクトルの回転
	//float co = cosf(XMConvertToRadians(Rotation));
	//float si = sinf(XMConvertToRadians(Rotation));

	////P1(Rotation)Y軸回転
	//CameraObject.Position.x = (vec.x * co - vec.y * si);
	//CameraObject.Position.z = (vec.x * si + vec.y * co);
	//CameraObject.Position.x += CameraObject.AtPosition.x;
	//CameraObject.Position.z += CameraObject.AtPosition.z;

