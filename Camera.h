/*
* ファイル名	Camera.h
* タイトル	カメラ
* 作成者		鈴木豪
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef CAMERA_H
#define CAMERA_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#include"direct3d.h"
using namespace DirectX;
#include "gameObject.h"
#include "Raycasting.h"

class CAMERA
{
	public:
		XMFLOAT3 Position;		//座標 (実際のカメラの位置)
		XMFLOAT3 AtPosition;		//注視点
		XMFLOAT3 UpVector;		//上方ベクトル

		XMFLOAT3 IdealPosition;	//障害物がない場合のカメラの理想的な位置

		XMMATRIX View;			//ビュー行列
		XMMATRIX Projection;		//プロジェクション行列

		float Fov;			//視野角（画角）
		float Aspect;			//画面のアスペクト比
		float NearClip;		//近面クリップ距離
		float FarClip;		//遠面クリップ距離

		float suzukiTime = 0.0f;   //障害物がない時間が続いた秒数
		const float returnDelay = 3.0f;  //カメラを理想位置に戻し始めるまでの遅延時間
		const float returnSpeed = 5.0f;  //カメラを理想位置まで戻す速度

		bool CameraIsInsideObj = false; //カメラが障害物オブジェクト内部にいるかどうか
};

void Camera_Initialize();
void Camera_Finalize();
void Camera_Update();
void Camera2_Update();
void Camera_Draw();
void Camera2_Draw();

//カメラレイ組み込み用関数
void PerformCameraRaycast(const std::vector<GameObject*>& gameObjects, Raycast& cameraRay, Ray_HitInfo& nearestHit);

void SetCameraFov(float);
void SetCameraAspect(float);
void SetCameraClip(float, float);

void SetCamera2Fov(float);
void SetCamera2Aspect(float);
void SetCamera2Clip(float, float);

void SetCameraPosition(XMFLOAT3);
void SetCameraAtPosition(XMFLOAT3);
void SetCameraUpVector(XMFLOAT3);

void SetCamera2Position(XMFLOAT3);
void SetCamera2AtPosition(XMFLOAT3);
void SetCamera2UpVector(XMFLOAT3);

CAMERA& GetCameraObj();
CAMERA& GetCamera2Obj();

XMMATRIX GetViewMatrix();
XMMATRIX GetProjectionMatrix();

XMMATRIX GetViewMatrix2();
XMMATRIX GetProjectionMatrix2();

XMFLOAT3 GetCameraAtPosition();
XMFLOAT3 GetCameraPosition();

XMFLOAT3 GetCamera2AtPosition();
XMFLOAT3 GetCamera2Position();

#endif // CAMERA_H