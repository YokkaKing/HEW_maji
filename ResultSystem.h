#pragma once
#include <d3d11.h>
#include "selectWeaponTerrain.h"

// P1/P2それぞれの「ベース + 変身2つ」
struct RESULT_LOADOUT
{
    WeaponTerrain baseWT = WeaponTerrain::NONE;
    WeaponTerrain t0 = WeaponTerrain::NONE;
    WeaponTerrain t1 = WeaponTerrain::NONE;
};

struct RESULT_MATCH_INFO
{
    int winnerId = 1; // 1 or 2
    int decidedRound = 0;

    RESULT_LOADOUT p1;
    RESULT_LOADOUT p2;
};

// Manager側（試合終了時）でセット
void ResultSystem_SetMatchInfo(const RESULT_MATCH_INFO& info);

// Resultシーンから呼ぶ
void ResultSystem_Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx);
void ResultSystem_Finalize();
void ResultSystem_Update();

// Split描画（Manager_Draw_Player1/2 から呼ぶ）
void ResultSystem_Draw_Player1();
void ResultSystem_Draw_Player2();
