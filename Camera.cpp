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
#include<math.h>

//================================================================
//	マクロ定義
//================================================================
#define ROTATION_X_UPMAX (20.0f)
#define ROTATION_X_DOWNMAX (-10.0f)
#define ROTATION_X_UPMAXP2 (15.0f)
#define ROTATION_X_DOWNMAXP2 (-10.0)
#define ROTATION_Y_MAX (90.0f)
// 履歴として保存するフレーム数 (30フレーム = 0.5秒の遅延)
// この数値を大きくするほど、追従がゆったり（遅延が大きく）になる
#define CAMERA_DELAY_FRAMES (30)
// カメラの自動回転速度 (1.0f で即座に合わせる。0.05f くらいだと滑らか)
#define CAMERA_AUTO_ROTATION_SPEED (0.05f)
#define ROTATION_Y_SPEED (0.05f)
#define ROTATION_X_SPEED (0.03f)
#define CAMERA_DISTANCE (12.0f) // プレイヤーからの距離
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

// 実体の定義
std::deque<XMFLOAT3> g_P1PositionHistory;
std::deque<XMFLOAT3> g_P2PositionHistory;
bool g_IsAutoCamera = true; // デフォルトはON

static float CameraRotationX = 0.2f; // P1カメラの垂直角度
static float CameraRotationY = 0.0f; // P1カメラの水平角度
static float Camera2RotationX = 0.2f; // P2カメラの垂直角度
static float Camera2RotationY = XM_PI; // P2カメラの水平角度（向き合うように設定）

// ユーティリティ関数：角度の差を -PI ~ PI の範囲に補正する
// これをしないと、350度から10度へ回転する時に逆回転しちゃう
float NormalizeAngle(float angle) 
{
	while (angle > XM_PI)  angle -= XM_2PI;
	while (angle < -XM_PI) angle += XM_2PI;
	return angle;
}

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
	XMFLOAT3 playerPos = GetPlayerPosition();

	// 1. 自身の座標を履歴に追加 (P2カメラが追従するために使用)
	g_P1PositionHistory.push_back(playerPos);
	if (g_P1PositionHistory.size() > CAMERA_DELAY_FRAMES) {
		g_P1PositionHistory.pop_front();
	}

	// 2. スティック入力による手動回転
	// ★重要：ご自身の 'Controller.h' 内にある、右スティックの値を取得する正しい関数名に書き換えてください。
	// 例： GetRightStickX() だったり GetAxisX(STICK_RIGHT) だったりします。
	float stickX = g_Controller[0].GetRightStickX();
	float stickY = g_Controller[0].GetRightStickY();

	CameraRotationY += stickX * ROTATION_Y_SPEED;
	CameraRotationX -= stickY * ROTATION_X_SPEED;

	// X軸（上下）の回転制限
	if (CameraRotationX > ROTATION_X_UPMAX) CameraRotationX = ROTATION_X_UPMAX;
	if (CameraRotationX < ROTATION_X_DOWNMAX) CameraRotationX = ROTATION_X_DOWNMAX;

	// 3. 自動追従 (Y軸：横回転のみを補正)
	if (g_IsAutoCamera && !g_P2PositionHistory.empty()) {
		// P2の過去（CAMERA_DELAY_FRAMES分前）の座標を取得
		XMFLOAT3 targetEnemyPos = g_P2PositionHistory.front();

		// 自分の位置から、敵の過去位置への方向を算出
		float dx = targetEnemyPos.x - playerPos.x;
		float dz = targetEnemyPos.z - playerPos.z;

		// 向くべき角度(目標角)を計算
		float targetAngleY = atan2f(dx, dz);

		// 現在のカメラ角度と目標角の最短角度差を求め、滑らかに加算する
		float angleDiff = NormalizeAngle(targetAngleY - CameraRotationY);
		CameraRotationY += angleDiff * CAMERA_AUTO_ROTATION_SPEED;
	}

	// 4. 座標と注視点の算出
	// 注視点はプレイヤーの少し上
	CameraObject.AtPosition = playerPos;
	CameraObject.AtPosition.y += 2.0f;

	// プレイヤーの背後に位置するように座標を計算
	CameraObject.Position.x = playerPos.x + sinf(CameraRotationY + XM_PI) * cosf(CameraRotationX) * CAMERA_DISTANCE;
	CameraObject.Position.y = playerPos.y + sinf(CameraRotationX) * CAMERA_DISTANCE + 2.5f;
	CameraObject.Position.z = playerPos.z + cosf(CameraRotationY + XM_PI) * cosf(CameraRotationX) * CAMERA_DISTANCE;

	// ビュー行列・プロジェクション行列の更新
	CameraObject.View = XMMatrixLookAtLH(XMLoadFloat3(&CameraObject.Position), XMLoadFloat3(&CameraObject.AtPosition), XMLoadFloat3(&CameraObject.UpVector));
	CameraObject.Projection = XMMatrixPerspectiveFovLH(CameraObject.Fov, CameraObject.Aspect, CameraObject.NearClip, CameraObject.FarClip);
}

void Camera2_Update()
{
	XMFLOAT3 player2Pos = GetPlayer2Position();

	// 1. 自身の座標を履歴に追加 (P1カメラが追従するために使用)
	g_P2PositionHistory.push_back(player2Pos);
	if (g_P2PositionHistory.size() > CAMERA_DELAY_FRAMES) {
		g_P2PositionHistory.pop_front();
	}

	// 2. スティック入力による手動回転
	// ★重要：P2側の右スティック取得関数名に書き換えてください。
	float stickX = g_Controller[1].GetRightStickX();
	float stickY = g_Controller[1].GetRightStickY();

	Camera2RotationY += stickX * ROTATION_Y_SPEED;
	Camera2RotationX -= stickY * ROTATION_X_SPEED;

	if (Camera2RotationX > ROTATION_X_UPMAX) Camera2RotationX = ROTATION_X_UPMAX;
	if (Camera2RotationX < ROTATION_X_DOWNMAX) Camera2RotationX = ROTATION_X_DOWNMAX;

	// 3. 自動追従 (Y軸：横回転のみ)
	if (g_IsAutoCamera && !g_P1PositionHistory.empty()) {
		// P1の過去の座標を取得
		XMFLOAT3 targetEnemyPos = g_P1PositionHistory.front();

		float dx = targetEnemyPos.x - player2Pos.x;
		float dz = targetEnemyPos.z - player2Pos.z;

		float targetAngleY = atan2f(dx, dz);

		float angleDiff = NormalizeAngle(targetAngleY - Camera2RotationY);
		Camera2RotationY += angleDiff * CAMERA_AUTO_ROTATION_SPEED;
	}

	// 4. 座標と注視点の算出
	Camera2Object.AtPosition = player2Pos;
	Camera2Object.AtPosition.y += 2.0f;

	Camera2Object.Position.x = player2Pos.x + sinf(Camera2RotationY + XM_PI) * cosf(Camera2RotationX) * CAMERA_DISTANCE;
	Camera2Object.Position.y = player2Pos.y + sinf(Camera2RotationX) * CAMERA_DISTANCE + 2.5f;
	Camera2Object.Position.z = player2Pos.z + cosf(Camera2RotationY + XM_PI) * cosf(Camera2RotationX) * CAMERA_DISTANCE;

	Camera2Object.View = XMMatrixLookAtLH(XMLoadFloat3(&Camera2Object.Position), XMLoadFloat3(&Camera2Object.AtPosition), XMLoadFloat3(&Camera2Object.UpVector));
	Camera2Object.Projection = XMMatrixPerspectiveFovLH(Camera2Object.Fov, Camera2Object.Aspect, Camera2Object.NearClip, Camera2Object.FarClip);
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

