/*
* ファイル名	sprite.h
* タイトル	スプライト
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef SPRITE_H
#define SPRITE_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include"direct3d.h"
#include<DirectXMath.h>
using namespace DirectX;


void DrawSprite(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col);
void DrawSpriteEx(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc);
void InitializeSprite();	//スプライト初期化
void FinalizeSprite();	//スプライト終了
void DrawSpriteScroll(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, XMFLOAT2 texcoord);
void DrawSpriteExRotation(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc, float radian);
void DrawSprite(XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc);//行列使用版

// 頂点構造体
struct Vertex
{
	XMFLOAT3 position; // 頂点座標  //XMFLOAT3へ変更
	XMFLOAT4 color;		//頂点カラー（R,G,B,A）
	XMFLOAT2 texCoord;	//テクスチャ座標
};

#endif // SPRITE_H