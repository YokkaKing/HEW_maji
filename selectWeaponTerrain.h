#pragma once
//============================================
// selectWeaponTerrain.h
// 
// 武器・地形選択シーン用ファイル
// 作成日:2026/1/10
// 最終更新日:2026/1/13
//============================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
using namespace DirectX;



enum class WeaponTerrain
{
	//ここに武器 + 地形を設定していく
	//ここの番号順を.cppの画像ロード順と合わせていく
	NONE = 0,
	SWORD_WALL,
	SPEAR_HILL, //本来は丘は弓対応
	BOW_HILL,
	HAMMER_,
	SHURIKEN_,

	MAX
};

//ゲーム内で選択した武器・地形を保持
struct inGameWTselect
{
	WeaponTerrain player1 = WeaponTerrain::SWORD_WALL;
	WeaponTerrain player2 = WeaponTerrain::SWORD_WALL;
	WeaponTerrain winner = WeaponTerrain::SWORD_WALL;
};

void selectWT_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void selectWT_Finalize();
void selectWT_Update();
void selectWT_Draw(int playerID);//どちらのプレイヤーに描画しているかを判定する引数を追加 (P1:0, P2:1)

bool selectWT_IsP1Ready();
bool selectWT_IsP2Ready();

int GetPlayer1SelectedIndex();
int GetPlayer2SelectedIndex();
bool GetPlayerSelected(int playerIndex);
float GetCounter(int playerIndex);
void selectWT_Draw_Before3D();
void Selectweaponui3d_Draw();
void selectWT_Draw_After3D();
int GetControllerIndexFromPlayerNo(int playerNo);