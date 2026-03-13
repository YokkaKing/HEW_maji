#pragma once
#include <DirectXMath.h>

using namespace DirectX;

// 武器選択 → 変身選択 の間に入るカメラ演出
void CameraIntroSequence_Initialize();
void CameraIntroSequence_Finalize();
bool CameraIntroSequence_IsOrbitPhase();  
// 演出開始（Game_Initializeで呼ぶ）
void CameraIntroSequence_Start(
    const XMFLOAT3& arenaCenter,
    float orbitRadius = 18.0f,
    float orbitHeight = 10.0f,
    float orbitDurationSec = 2.2f,
    float moveDurationSec = 1.0f);

// 演出更新（Game_Updateで呼ぶ）
void CameraIntroSequence_Update(float dt);

// 状態
bool CameraIntroSequence_IsActive();
bool CameraIntroSequence_IsFinished();

// 必要なら強制終了
void CameraIntroSequence_ForceFinish();

bool GetCameraIntroPlayed();
void SetCameraIntroPlayed(bool played);