#pragma once
//============================================
// generateWT.h
// 
// 武器・地形の選択データに応じて生成する用
// 作成日:2026/1/21
// 最終更新日:2026/1/21
//============================================
#include "selectWeaponTerrain.h"
#include <d3d11.h>

class PLAYER;
class PLAYER2;

void generateWT_Apply(
	const inGameWTselect& selection,
	PLAYER* pP1,
	PLAYER2* pP2,
	ID3D11Device* pDevice,
	ID3D11DeviceContext* pContext);


