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
#include"selectWeaponTerrain.h"
#include"hitAction.h"

enum class TRANSFORM_TYPE
{
	TRANSFORM_TYPE_A, // 機動力特化
	TRANSFORM_TYPE_B, // 制動・防御特化
	TRANSFORM_TYPE_NONE // 未進化
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
class PLAYER : public GameObject
{
public:
	float           FrictionRate;   // 速度減衰率
	TRANSFORM_TYPE  TransformType;  // 進化タイプ (A or B or NONE)
	int TransformTimer;
	PLAYER_STATE	State;		//状態
	std::unique_ptr<IWeapon> m_currentWeapon = nullptr; // 現在装備中の武器
	bool			m_isDead = false; // 死亡フラグ
	XMFLOAT3 m_rotation; // 武器を回転させる
	WeaponTerrain m_reservedWT[2] = { WeaponTerrain::NONE, WeaponTerrain::NONE }; // 予約された変身先
	WeaponTerrain m_currentWT = WeaponTerrain::NONE; // 現在の姿
	WeaponTerrain m_baseWT; //初期武器選択で選んだ武器を保持
	float m_moveSpeed; // 移動速度
	float m_jumpForce; // 移動速度
	bool m_isAttacked = false; // 攻撃中フラグ
	float m_moveMul = 1.0f;
	bool m_isTransformed = false; // 変身中フラグ
	HitAction m_hitAction;
public:
	//武器操作関数
	void EquipWeapon(std::unique_ptr<IWeapon> weapon); // 武器を装備する
	void OnCollision(const CollisionInfo& info)override;

	void SetReservedWT(int index, WeaponTerrain wt) {
		if (index >= 0 && index < 2) {
			m_reservedWT[index] = wt;
		}
	};

	WeaponTerrain GetReservedWT(int index) const {
		if (index >= 0 && index < 2) {
			return m_reservedWT[index];
		}
		return WeaponTerrain::NONE;
	}

	WeaponTerrain GetCurrentWT() const { return m_currentWT; }
	void SetCurrentWT(WeaponTerrain wt) { m_currentWT = wt; }

	bool isDead() const { return m_isDead; }
	void RoundReset(XMFLOAT3 startPos);
	void EquipBaseWeapon();
	HitAction& GetHitAction() { return m_hitAction; }
};

extern PLAYER g_Player;

void PlayerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp1);
void PlayerFinalize();
void PlayerUpdate();
void PlayerDraw();

XMFLOAT3 GetPlayerPosition();

void Player_Jump();
void Player_ManualMove();
float Player_GetHp();
float Player_GetMaxHp();
bool GetPlayer_IsAttacked();
void SetPlayer_IsAttacked(bool isAttacked);
PLAYER* GetPlayer();
WeaponTerrain GetSetWTP1();
void SetWTP1(WeaponTerrain wt);

#endif // PLAYER_H