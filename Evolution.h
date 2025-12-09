/*
* ファイル名	Evolution.h
* タイトル	進化
* 作成者		イトイ
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef EVOLUTION_H
#define EVOLUTION_H

//================================================================
//	インクルード
//================================================================
#include"Player.h" 
#include"Player2.h"
#include<DirectXMath.h>
using namespace DirectX;

// 進化システム初期化 
void EvolutionInitialize();

// プレイヤーを進化させるメイン関数 
void EvolvePlayer();

// 進化タイプに応じたパラメーター補正を適用
void ApplyEvolutionEffect();


//================================
void EvolvePlayer2();
void ApplyEvolutionEffect2();
void EvolvePlayer3();
void ApplyEvolutionEffect3();
//===============================
#endif // EVOLUTION_H