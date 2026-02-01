/*
* ファイル名	Player2.h
* タイトル	プレイヤー2
* 作成者		鈴木豪
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef PLAYER2_H
#define PLAYER2_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#include"direct3d.h"
using namespace DirectX;
#include"model.h"
#include"gameObject.h"
#include"IWeapon.h"
#include"selectWeaponTerrain.h"

enum class EVOLUTION_TYPE2
{
	EVOLUTION_TYPE_A, // 機動力特化
	EVOLUTION_TYPE_B, // 制動・防御特化
	EVOLUTION_TYPE_NONE // 未進化
};

//プレイヤーの状態
enum PLAYER2_STATE
{
	PLAYER2_STATE_IDLE = 0,	//何もしない
	PLAYER2_STATE_MOVE,		//移動
	PLAYER2_STATE_DIRECTION,	//方向指示
	PLAYER2_STATE_POWER,		//威力指示
};

//プレイヤー構造体
class PLAYER2 : public GameObject
{
public:
	float           FrictionRate;   // 速度減衰率
	EVOLUTION_TYPE2  EvolutionType;  // 進化タイプ (A or B or NONE)
	int EvolutionTimer;
	PLAYER2_STATE	State;		//状態
	std::unique_ptr<IWeapon> m_currentWeapon = nullptr; // 現在装備中の武器
	bool			m_isDead = false; // 死亡フラグ
	bool 		  m_isAttacked = false; // 攻撃を受けたかどうか
	XMFLOAT3 m_rotation; // 武器を回転させる
	//武器操作関数
public:
	void EquipWeapon(std::unique_ptr<IWeapon> weapon); // 武器を装備する
	void OnCollision(const CollisionInfo& info)override;
};

void Player2Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp2);
void Player2Finalize();
void Player2Update();
void Player2Draw();

XMFLOAT3 GetPlayer2Position();

void Player2_Jump();
void Player2_ManualMove();
float Player2_GetHp();
float Player2_GetMaxHp();
bool GetPlayer2_IsAttacked();
void SetPlayer2_IsAttacked(bool isAttacked);
PLAYER2* GetPlayer2();
WeaponTerrain GetSetWTP2();

#endif // PLAYER2_H