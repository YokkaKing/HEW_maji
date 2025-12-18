/*
* ファイル名	Player.h
* タイトル	プレイヤー
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef PLAYER_H
#define PLAYER_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#include"direct3d.h"
using namespace DirectX;
#include"model.h"
#include"gameObject.h"
#include "IWeapon.h"

enum class EVOLUTION_TYPE
{
	EVOLUTION_TYPE_A, // 機動力特化
	EVOLUTION_TYPE_B, // 制動・防御特化
	EVOLUTION_TYPE_NONE // 未進化
};

//プレイヤーの状態
enum PLAYER_STATE
{
	PLAYER_STATE_IDLE = 0,	//何もしない
	PLAYER_STATE_MOVE,		//移動
	PLAYER_STATE_DIRECTION,	//方向指示
	PLAYER_STATE_POWER,		//威力指示
	PLAYER_STATE_JUMP,		//ジャンプ
};

//プレイヤー構造体
class PLAYER: public GameObject
{
public:
	float           FrictionRate;   // 速度減衰率
	EVOLUTION_TYPE  EvolutionType;  // 進化タイプ (A or B or NONE)
	PLAYER_STATE	State;		//状態
	IWeapon* m_currentWeapon = nullptr; // 現在装備中の武器
	float			m_maxHp = 100.0f; // 最大体力
	float			m_currentHp;	 // 現在の体力
	bool			m_isDead = false; // 死亡フラグ
	XMFLOAT3 m_rotation; // 武器を回転させる
	//武器操作関数
	void EquipWeapon(IWeapon* weapon); // 武器を装備する
	void TryAttack(const XMFLOAT3& direction); // 攻撃を試みる

public:
	void OnCollision(const CollisionInfo& info)override;
	void TakeDamage(float damage);
};

void PlayerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void PlayerFinalize();
void PlayerUpdate();
void PlayerDraw();

void PlayerDrawHUD();

XMFLOAT3 GetPlayerPosition();

void Player_Jump();
void Player_ManualMove();
float Player_GetHP();
float Player_GetMaxHp();
PLAYER* GetPlayer();

#endif // PLAYER_H