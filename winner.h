/*
* ファイル名	winner.h
* タイトル	プレイヤー
* 作成者		カンジェウォン
* 作成日		02/10
* 更新日		02/10
*/
#ifndef WINNER_H
#define WINNER_H

#include <d3d11.h>
#include "selectWeaponTerrain.h"
#include "model.h"

// Resultで使う保存データ
struct WINNER_RESULT_INFO
{
    int winnerPlayerId = 0;          // 1 or 2（引き分け等は0）
    int decidedRound = 0;            // 2 or 3（何ラウンド目で決着したか）
    WeaponTerrain baseWT = WeaponTerrain::NONE;
    WeaponTerrain transformWT[2] = { WeaponTerrain::NONE, WeaponTerrain::NONE };
};

// Scene切替直前に呼ぶ想定（勝者情報＆そのラウンドの選択を保存）
void Winner_SetResultInfo(
    int winnerPlayerId,
    int decidedRound,
    WeaponTerrain baseWT,
    WeaponTerrain transformA,
    WeaponTerrain transformB
);

// Result側で参照したいとき用（デバッグにも便利）
WINNER_RESULT_INFO Winner_GetResultInfo();

// Winner（表示専用）
void WinnerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void WinnerFinalize();
void WinnerUpdate(); // 中身なし
void WinnerDraw();
XMFLOAT3 GetWinnerCenterposition();
void SetStartedFlag(bool flag);
// 描画マスク（bit）
// 1: base, 2: transformA, 4: transformB
void Winner_SetDrawMask(int mask);

#endif // WINNER_H