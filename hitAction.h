#pragma once
//============================================
// hitAction.h
// 
// 被撃時にヒットストップ・ノックバック適用
// 作成日:2026/2/09
// 最終更新日:2026/2/09
//============================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
using namespace DirectX;

class HitAction
{
private:
	float m_stopTimer = 0.0f; //ヒットストップ時間
	XMFLOAT3 m_kbVelocity = { 0.0f, 0.0f, 0.0f }; //ノックバック速度
	const float FRICTION = 0.85; //ノックバックの減衰率
public:
	//HA = HitAction d = direction sT = stopTime p = power
	void triggerHA(XMFLOAT3 d, float sT, float p);
	//更新処理
	void Update(XMFLOAT3& position);
	//ヒットストップ中かの判定
	bool IsStopping()const { return m_stopTimer > 0.0f; }
};




