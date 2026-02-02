#include "generateWT.h"
#include "Player.h"
#include "Player2.h"
#include "terrain.h"

//武器生成用
#include "sword.h"
#include "spear.h"
#include "hammer.h"
#include "arrow.h"
#include "syuriken.h"

//選択データに応じた武器を生成
template <typename T>
void AssignWeaponToPlayer(GameObject* pPlayerObj, T* pOwner, WeaponTerrain type, bool isP1)
{
	std::unique_ptr<IWeapon> newWeapon = nullptr;

	switch (type)
	{
	case WeaponTerrain::SWORD_WALL: newWeapon = std::make_unique<Sword>(pPlayerObj, isP1); break;
	case WeaponTerrain::SPEAR_HILL: newWeapon = std::make_unique<Spear>(pPlayerObj, isP1); break;
	case WeaponTerrain::BOW_HILL: newWeapon = std::make_unique<Arrow>(pPlayerObj, isP1); break;
	case WeaponTerrain::HAMMER_: newWeapon = std::make_unique<Hammer>(pPlayerObj, isP1); break;
	case WeaponTerrain::SHURIKEN_:newWeapon = std::make_unique<Shuriken>(pPlayerObj, isP1); break;
	}

	if (newWeapon != nullptr)
	{
		// 引数で受け取った pOwner (pP1 or pP2) の武器スロットに移動
		pOwner->m_currentWeapon = std::move(newWeapon);
	}
}


void generateWT_Apply(const inGameWTselect& selection, PLAYER* pP1, PLAYER2* pP2, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	//プレイヤー1の武器を生成
	AssignWeaponToPlayer(pP1, pP1, selection.player1, true);
	//プレイヤー2の武器を生成
	AssignWeaponToPlayer(pP2, pP2, selection.player2, false);
}
