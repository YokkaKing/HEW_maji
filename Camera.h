#pragma once

//Camera.h

#include	<d3d11.h>
#include	<DirectXMath.h>
#include	"direct3d.h"
using namespace DirectX;

class CAMERA
{
	public:
		XMFLOAT3	Position;		//座標
		XMFLOAT3	AtPosition;		//注視点
		XMFLOAT3	UpVector;		//上方ベクトル

		XMMATRIX	View;			//ビュー行列
		XMMATRIX	Projection;		//プロジェクション行列

		float		Fov;			//視野角（画角）
		float		Aspect;			//画面のアスペクト比
		float		NearClip;		//近面クリップ距離
		float		FarClip;		//遠面クリップ距離
};


void	Camera_Initialize();
void	Camera_Finalize();
void	Camera_Update();
void    Camera2_Update();
void	Camera_Draw();
void    Camera2_Draw();

void	SetCameraFov(float);
void	SetCameraAspect(float);
void	SetCameraClip(float, float);

void	SetCamera2Fov(float);
void	SetCamera2Aspect(float);
void	SetCamera2Clip(float, float);

void	SetCameraPosition(XMFLOAT3);
void	SetCameraAtPosition(XMFLOAT3);
void	SetCameraUpVector(XMFLOAT3);

void	SetCamera2Position(XMFLOAT3);
void	SetCamera2AtPosition(XMFLOAT3);
void	SetCamera2UpVector(XMFLOAT3);

XMMATRIX	GetViewMatrix();
XMMATRIX	GetProjectionMatrix();

XMMATRIX	GetViewMatrix2();
XMMATRIX	GetProjectionMatrix2();

XMFLOAT3   GetCameraAtPosition();
XMFLOAT3   GetCameraPosition();

XMFLOAT3   GetCamera2AtPosition();
XMFLOAT3   GetCamera2Position();