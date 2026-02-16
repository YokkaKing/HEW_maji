//============================================
// hitAction.cpp
// 
// 被撃時にヒットストップ・ノックバック適用
// 作成日:2026/2/09
// 最終更新日:2026/2/09
//============================================

#include "hitAction.h"

void HitAction::triggerHA(XMFLOAT3 d, float sT, float p)
{
	m_stopTimer = sT;

	//方向ベクトルを正規化
	float len = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
	if (len > 0) {
		m_kbVelocity.x = (d.x / len) * p;
		m_kbVelocity.y = (d.y / len) * p; //少し上に浮かせたい場合は要調整
		m_kbVelocity.z = (d.z / len) * p;
	}
}

void HitAction::Update(XMFLOAT3& position)
{
	if (m_stopTimer > 0.0f)
	{
		m_stopTimer -= 1.0f / 60.0f;
		return;
	}

	//ノックバックを計算して直接座標を書き換える(方法は後々変える可能性あり)
	if (fabs(m_kbVelocity.x) > 0.001f || fabs(m_kbVelocity.z) > 0.001f) {
		position.x += m_kbVelocity.x;
		position.y += m_kbVelocity.y;
		position.z += m_kbVelocity.z;

		m_kbVelocity.x *= FRICTION;
		m_kbVelocity.z *= FRICTION;
	}
}



