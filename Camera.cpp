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
#include"shader.h"

//================================================================
//	グローバル変数
//================================================================
static	CAMERA	CameraObject;
static	CAMERA  Camera2Object;
XMFLOAT3		g_PlayerPosOld;
XMFLOAT3		g_Player2PosOld;
extern Controller g_Controller;

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
	//ボールの座標取得<<<<<<<<<<<<<<<<<<<<<<
	XMFLOAT3	pos = g_PlayerPosOld;//P1
	g_PlayerPosOld = GetPlayerPosition();
	//前回のボールと現在のボールの座標の差分<<<<<<<<<<<<<<<
	//P1
	pos.x = g_PlayerPosOld.x - pos.x;
	pos.y = g_PlayerPosOld.y - pos.y;
	pos.z = g_PlayerPosOld.z - pos.z;

	//カメラを移動
	//P1
	CameraObject.Position.x += pos.x;
	CameraObject.Position.y += pos.y;
	CameraObject.Position.z += pos.z;

	//注視点としてセット<<<<<<<<<<<<<<<<<<<<<<<
	//P1
	CameraObject.AtPosition.x = g_PlayerPosOld.x;
	CameraObject.AtPosition.y = g_PlayerPosOld.y;
	CameraObject.AtPosition.z = g_PlayerPosOld.z;

	//注視点を中心にカメラの位置を回転（Y軸回転）
	float	Rotation = 0.0f;
	//右スティックのX軸の傾きを取得
	Rotation = g_Controller.GetRightStickX();
	// スティックの入力値を回転速度に変換
	Rotation *= -1.5f;

	//===============================
	if (Keyboard_IsKeyDown(KK_Q))
	{
		Rotation = 1.0f;
	}
	if (Keyboard_IsKeyDown(KK_E))
	{
		Rotation = -1.0f;
	}
	//==============================

	//注視点からカメラへのベクトル
	//P1
	XMFLOAT2 vec;
	vec.x = CameraObject.Position.x - CameraObject.AtPosition.x;
	vec.y = CameraObject.Position.z - CameraObject.AtPosition.z;

	//XMFLOAT2 vec2;// 横と前後(x,z)地面に対して平行な移動
	//vec2.x = CameraObject.Position.y - CameraObject.AtPosition.y;
	//vec2.y = CameraObject.Position.z - CameraObject.AtPosition.z;// vec変数はXMFLOAT2のためyに値を入れているが実際の値はz

	//ベクトルの回転
	float co = cosf(XMConvertToRadians(Rotation));
	float si = sinf(XMConvertToRadians(Rotation));

	//P1(Rotation)Y軸回転
	CameraObject.Position.x = (vec.x * co - vec.y * si);
	CameraObject.Position.z = (vec.x * si + vec.y * co);
	CameraObject.Position.x += CameraObject.AtPosition.x;
	CameraObject.Position.z += CameraObject.AtPosition.z;

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
	XMFLOAT3	pos2 = g_Player2PosOld;//P2
	g_Player2PosOld = GetPlayer2Position();
	//P2 前回のボールと現在のボールの座標の差分
	pos2.x = g_Player2PosOld.x - pos2.x;
	pos2.y = g_Player2PosOld.y - pos2.y;
	pos2.z = g_Player2PosOld.z - pos2.z;
	//P2 カメラを移動
	Camera2Object.Position.x += pos2.x;
	Camera2Object.Position.y += pos2.y;
	Camera2Object.Position.z += pos2.z;
	//P2 注視点としてセット
	Camera2Object.AtPosition.x = g_Player2PosOld.x;
	Camera2Object.AtPosition.y = g_Player2PosOld.y;
	Camera2Object.AtPosition.z = g_Player2PosOld.z;

	float	Rotation2 = 0.0f;
	if (Keyboard_IsKeyDown(KK_LEFT))
	{
		Rotation2 = 1.0f;
	}
	if (Keyboard_IsKeyDown(KK_RIGHT))
	{
		Rotation2 = -1.0f;
	}


	//P2 注視点からカメラへのベクトル
	XMFLOAT2 vec2;
	vec2.x = Camera2Object.Position.x - Camera2Object.AtPosition.x;
	vec2.y = Camera2Object.Position.z - Camera2Object.AtPosition.z;
	//ベクトルの回転
	float co2 = cosf(XMConvertToRadians(Rotation2));
	float si2 = sinf(XMConvertToRadians(Rotation2));
	//P2(Rotation2)Y軸回転
	Camera2Object.Position.x = (vec2.x * co2 - vec2.y * si2);
	Camera2Object.Position.z = (vec2.x * si2 + vec2.y * co2);
	Camera2Object.Position.x += Camera2Object.AtPosition.x;
	Camera2Object.Position.z += Camera2Object.AtPosition.z;
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


