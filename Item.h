/*
* ファイル名	Item.h
* タイトル	アイテム制御
* 作成者		久保木幹太
* 作成日		12月31日		
* 更新日		12月31日
*/

#ifndef ITEM_H
#define ITEM_H

//================================================================
//	インクルード
//================================================================
#include<DirectXMath.h>
#include"gameObject.h"
using namespace DirectX;

enum class ITEM_TYPE
{
	NONE = 0,
	CHANGE_RECOVERY,
	HEALTH_RECOVERY
};

class ITEM_SPONER
{
public:
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;

	float m_count;
public:
	void Initialize();
	void Finalize();
	void Update();
	void Spwan();
	// ラウンドごとの初期化
	void ResetItem();
	XMFLOAT3 WherePosition();
};

class ITEM : public GameObject
{
public:
	std::shared_ptr<Collider> m_collider; // コライダーへの参照を保持
	ITEM_TYPE m_itemType;

	bool m_isScale; // 簡易アニメーションのやつ
public:
	void Set();
	void Update() override;
	void Draw() override;
	void OnCollision(const CollisionInfo& info) override;
};

// 全アイテムを削除する関数
void ClearAllItems();

#endif // ITEM_H