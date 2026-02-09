/*
* ファイル名	Transform.h
* タイトル	進化
* 作成者		イトイ
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

//================================================================
//	インクルード
//================================================================
#include"Player.h" 
#include"Player2.h"
#include<DirectXMath.h>
using namespace DirectX;

// 進化システム初期化 
void TransformInitialize(WeaponTerrain selectP1_A, WeaponTerrain selectP1_B, WeaponTerrain selectP2_A, WeaponTerrain selectP2_B);

// プレイヤーを進化させるメイン関数 
void TransformPlayer();

// 進化タイプに応じたパラメーター補正を適用
void ApplyTransformEffect();

//================================
void TransformPlayer2();
void ApplyTransformEffect2();
//===============================

bool GetIsUsedA_P1();
bool GetIsUsedB_P1();
bool GetIsUsedA_P2();
bool GetIsUsedB_P2();
void SetIsUsed_P1(int no, bool flag);
void SetIsUsed_P2(int no, bool flag);

extern WeaponTerrain g_TransformA_P1; // P1の2回目選択
extern WeaponTerrain g_TransformB_P1; // P1の3回目選択
extern WeaponTerrain g_TransformA_P2; // P2の2回目選択
extern WeaponTerrain g_TransformB_P2; // P2の3回目選択
#endif //TRANSFORM_H