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
	PLAYER2_STATE	State;		//状態
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
	void SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay);
	void TakeDamage(float damage);
};

void Player2Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Player2Finalize();
void Player2Update();
void Player2Draw();

XMFLOAT3 GetPlayer2Position();

void Player2_Jump();
void Player2_ManualMove();
float Player2_GetHp();
float Player2_GetMaxHp();
PLAYER2* GetPlayer2();

#endif // PLAYER2_H