/*
* ファイル名	Camera.cpp
* タイトル	カメラ
* 作成者		鈴木豪
* 作成日		12月02日
* 更新日		12月02日
*/

#include"Camera.h"
#include"keyboard.h"
#include"Controller.h"
#include"Player.h"
#include"Player2.h"
#include"Viewport.h"

#include "Manager.h"
#include"shader.h"

#define ROTATION_X_UPMAX (20.0f)
#define ROTATION_X_DOWNMAX (-10.0f)
#define ROTATION_X_UPMAXP2 (15.0f)
#define ROTATION_X_DOWNMAXP2 (-10.0)
#define ROTATION_Y_MAX (90.0f)
//================================================================
//	グローバル変数
//================================================================
static	CAMERA	CameraObject;
static	CAMERA  Camera2Object;
XMFLOAT3 g_PlayerPosOld;
XMFLOAT3 g_Player2PosOld;
extern Controller g_Controller[2];
static float EaseOutCubic(float t)
{
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;
	float u = 1.0f - t;
	return 1.0f - (u * u * u); // 最初速く、最後ゆっくり
}
static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

static bool  s_resultTurnRequest = false;
static bool  s_yawTurning = false;
static float s_yawFrom = 0.0f;
static float s_yawTo = 0.0f;
static float s_yawTime = 0.0f;
static float s_yawDur = 0.6f;


void Camera_Initialize()
{ 
	
	CameraObject.Position = XMFLOAT3(0.0f, 3.0f, -10.0f);
	CameraObject.AtPosition = XMFLOAT3(0.0f, 1.0f, 0.0f);
	CameraObject.UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

	CameraObject.Fov = 45.0f;

	float width = (float)Direct3D_GetBackBufferWidth();
	float height = (float)Direct3D_GetBackBufferHeight();
	CameraObject.Aspect = (width / height);
	CameraObject.NearClip = 0.5f;
	CameraObject.FarClip = 1000.0f;
	g_PlayerPosOld = GetPlayerPosition();//<<<<<<<<<<<<<<<<
	
}

void Camera2_Initialize()
{
	Camera2Object.Position = XMFLOAT3(2.0f, 3.0f, -4.0f);
	Camera2Object.AtPosition = XMFLOAT3(0.0f, 1.0f, 0.0f);
	Camera2Object.UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

	float width = (float)Direct3D_GetBackBufferWidth();
	float height = (float)Direct3D_GetBackBufferHeight();

	Camera2Object.Fov = 45.0f;

	Camera2Object.Aspect = (width / height);
	Camera2Object.NearClip = 0.5f;
	Camera2Object.FarClip = 1000.0f;

	g_PlayerPosOld = GetPlayerPosition();//<<<<<<<<<<<<<<<<
	g_Player2PosOld = GetPlayer2Position();
	Camera2Object.Projection = XMMatrixPerspectiveFovLH(
	XMConvertToRadians(Camera2Object.Fov),
	Camera2Object.Aspect,
	Camera2Object.NearClip,
	Camera2Object.FarClip);
	g_Player2PosOld = GetPlayer2Position();
}
void Camera_Finalize()
{
	return;
}
void Camera2_Finalize()
{
	return;
}
void Camera_Update()
{
	//========================
	// シーン遷移検知
	//========================
	static int s_prevScene = -1;
	int sceneNow = GetScene();

	// ローテーション累積値（シーン跨いで保持）
	static float nowYaw = 0.0f;     // 水平回転 (左右)
	static float nowPitch = 22.0f;  // 垂直回転 (上下)
	static float nowDistance = 6.0f; // 距離も保持（ローカルだと毎フレーム戻る）

	if (sceneNow != s_prevScene)
	{
		// Resultから出た直後に「追従差分」が暴れないように old を同期
		if (s_prevScene == SCENE_RESULT && sceneNow != SCENE_RESULT)
		{
			g_PlayerPosOld = GetPlayerPosition();
		}

		// Select画面に入った瞬間だけ初期化（毎フレーム固定しない）
		if (sceneNow == SCENE_SELECT_WT)
		{
			nowYaw = 0.0f;
			nowPitch = -10.0f;
			nowDistance = 4.0f;

			// 追従差分暴れ防止
			g_PlayerPosOld = GetPlayerPosition();

			// ★武器選択カメラの注視点を固定
			CameraObject.AtPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
		}

		// Game画面に入った瞬間だけ初期化
		if (sceneNow == SCENE_GAME) // ←あなたのゲーム用シーン定数名に合わせて
		{
			nowYaw = 0.0f;
			nowPitch = 22.0f;
			nowDistance = 6.0f;

			// 追従差分暴れ防止
			g_PlayerPosOld = GetPlayerPosition();
		}

		s_prevScene = sceneNow;
	}

	// Result中は追従差分を止める（元コード維持）
	if (sceneNow == SCENE_RESULT)
	{
		g_PlayerPosOld = GetPlayerPosition();
		return;
	}
	if (sceneNow == SCENE_SELECT_WT)
	{
		// 武器選択用の注視点（中央）
		CameraObject.AtPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

		// nowYaw / nowPitch / nowDistance は上でシーン切替時にセット済み
		XMMATRIX matRot = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(nowPitch),
			XMConvertToRadians(nowYaw),
			0.0f
		);

		XMVECTOR vOffset = XMVectorSet(0.0f, 0.0f, -nowDistance, 0.0f);
		vOffset = XMVector3TransformNormal(vOffset, matRot);

		XMVECTOR vAt = XMLoadFloat3(&CameraObject.AtPosition);
		XMVECTOR vNewPos = XMVectorAdd(vAt, vOffset);
		vNewPos = XMVectorSetY(vNewPos, XMVectorGetY(vNewPos) + 1.0f);

		XMStoreFloat3(&CameraObject.Position, vNewPos);

		// 次シーンで差分暴れしないよう同期しておく
		g_PlayerPosOld = GetPlayerPosition();
		return;
	}
	//========================
	// プレイヤー追従移動
	//========================
	XMFLOAT3 playerPos = GetPlayerPosition(); //P1
	XMFLOAT3 diff; //現在と過去のプレイヤーのposの差

	diff.x = playerPos.x - g_PlayerPosOld.x;
	diff.y = playerPos.y - g_PlayerPosOld.y;
	diff.z = playerPos.z - g_PlayerPosOld.z;

	//カメラを移動
	CameraObject.Position.x += diff.x;
	CameraObject.Position.y += diff.y;
	CameraObject.Position.z += diff.z;

	CameraObject.AtPosition.x = playerPos.x;
	CameraObject.AtPosition.y = playerPos.y;
	CameraObject.AtPosition.z = playerPos.z;

	g_PlayerPosOld.x = playerPos.x;
	g_PlayerPosOld.y = playerPos.y;
	g_PlayerPosOld.z = playerPos.z;

	//========================
	// 入力取得
	//========================
	float rotX = 0.0f;
	float rotY = 0.0f;

	float inputX = g_Controller[0].GetRightStickX() * 2.0f;
	float inputY = g_Controller[0].GetRightStickY() * 1.5f;

	if (Keyboard_IsKeyDown(KK_Q)) inputX = 1.0f;
	if (Keyboard_IsKeyDown(KK_E)) inputX = -1.0f;
	if (Keyboard_IsKeyDown(KK_UP)) inputY = 1.0f;
	if (Keyboard_IsKeyDown(KK_DOWN)) inputY = -1.0f;

	// Result/Select中は入力無効（元コード維持）
	if (GetScene() == SCENE_RESULT || GetScene() == SCENE_SELECT_WT)
	{
		inputX = 0.0f;
		inputY = 0.0f;
	}

	//========================
	// 角度更新
	//========================
	nowYaw += inputX;
	nowPitch += inputY;

	//回転制限の適用
	if (GetScene() != SCENE_RESULT)
	{
		if (nowPitch > ROTATION_X_UPMAX)  nowPitch = ROTATION_X_UPMAX;
		if (nowPitch < ROTATION_X_DOWNMAX) nowPitch = ROTATION_X_DOWNMAX;
	}

	//========================
	// 距離（シーンで変えるなら nowDistance を使う）
	//========================
	float distance = nowDistance;

	// ※ここにあった
	// if (sceneNow == SCENE_SELECT_WT) { nowYaw=0; nowPitch=-10; distance=4; }
	// は「毎フレーム固定」になってしまうので削除済み

	//========================
	// 回転行列 & 位置再計算
	//========================
	XMMATRIX matRot = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(nowPitch),
		XMConvertToRadians(nowYaw),
		0.0f
	);

	XMVECTOR vOffset = XMVectorSet(0.0f, 0.0f, -distance, 0.0f);
	vOffset = XMVector3TransformNormal(vOffset, matRot);

	XMVECTOR vAt = XMLoadFloat3(&CameraObject.AtPosition);
	XMVECTOR vNewPos = XMVectorAdd(vAt, vOffset);

	vNewPos = XMVectorSetY(vNewPos, XMVectorGetY(vNewPos) + 1.0f);

	XMStoreFloat3(&CameraObject.Position, vNewPos);

	//========================
	// FOV変更（元コード維持）
	//========================
	//if (Keyboard_IsKeyDown(KK_Z))
	//{
	//	CameraObject.Fov += 0.3f;
	//	if (CameraObject.Fov > 160.0f)
	//	{
	//		CameraObject.Fov = 160.0f;
	//	}
	//}
	//if (Keyboard_IsKeyDown(KK_X))
	//{
	//	CameraObject.Fov -= 0.3f;
	//	if (CameraObject.Fov < 5.0f)
	//	{
	//		CameraObject.Fov = 5.0f;
	//	}
	//}

	return;
}

void Camera2_Update()
{
	if (GetScene() == SCENE_RESULT)
	{
		g_Player2PosOld = GetPlayer2Position();
		return;
	}

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
	static float nowPitch = 22.0f; // 垂直回転 (上下)

	//コントローラー・キーボードからの入力を取得
	float inputX = g_Controller[1].GetRightStickX() * 2.0f;
	float inputY = g_Controller[1].GetRightStickY() * 1.5f;

	if (Keyboard_IsKeyDown(KK_Y)) inputX = 1.0f;
	if (Keyboard_IsKeyDown(KK_I)) inputX = -1.0f;
	if (Keyboard_IsKeyDown(KK_G)) inputY = 1.0f;
	if (Keyboard_IsKeyDown(KK_V)) inputY = -1.0f;

	//角度を更新
	nowYaw += inputX;
	nowPitch += inputY;

	//回転制限の適用
	//左右180度制限 (-90度 から +90度 = 合計180度)
	//if (nowYaw > ROTATION_Y_MAX)  nowYaw = ROTATION_Y_MAX;
	//if (nowYaw < -ROTATION_Y_MAX) nowYaw = -ROTATION_Y_MAX;

	//上下45度制限 (水平を0度として +-45度)
	if (nowPitch > ROTATION_X_UPMAXP2)  nowPitch = ROTATION_X_UPMAXP2;
	if (nowPitch < ROTATION_X_DOWNMAXP2) nowPitch = ROTATION_X_DOWNMAXP2;

	//座標の再計算
	//プレイヤーからの基本距離 (初期設定 -4.0f に基づき 4.0f)
	float distance = 6.0f;

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
void Camera_ResultRequestTurn120(float durationSec)
{
	// 回転中なら無視（連打対策）
	if (s_yawTurning) return;

	s_resultTurnRequest = true;
	s_yawDur = (durationSec <= 0.0f) ? 0.0001f : durationSec;
}

bool Camera_IsResultMoving()
{
	return s_yawTurning;
}

