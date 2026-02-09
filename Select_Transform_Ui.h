#pragma once
#include "direct3d.h"


struct SELECT_TRANSFORM_CARD
{
	XMFLOAT2 pos;
	XMFLOAT2 size;
	XMFLOAT4 col; 

};

class SELECT_TRANSFORM_UI
{
	public:
	XMFLOAT2 pos;
	XMFLOAT2 size;
	XMFLOAT4 col;
	bool isUsed;
	int SelectNum;
	XMFLOAT2 Texcoord;

};

class SELECT_TRANSFORM_TIMER
{
public:
	XMFLOAT2 pos;
	XMFLOAT2 size;
	XMFLOAT4 col;
	float time;
	float frame;
};



void SelectTransformUi_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void SelectTransformUi_Finalize();
void SelectTransformUi_Update();
void SelectTransformUi_Draw();

void SetTransformUi_IsUsed(bool flg,int num);
void SetTransformUi_time(float time);
void SetTransformUi_SelectNum(int num);

//==================================================
// カードアニメーション用
//  playerIndex: 0=P1, 1=P2
//==================================================
void SelectTransformUi_CardAnim_Reset(int playerIndex);
void SelectTransformUi_CardAnim_Update(int playerIndex, int focusedIndex, bool isReady, float deltaTime);

// カードパラメータ取得
SELECT_TRANSFORM_CARD SelectTransformUi_GetCardParam(
	int playerIndex,
	int cardIndex,
	XMFLOAT2 basePos,
	float cardSpacing,
	XMFLOAT2 sizeUnfocus,
	XMFLOAT2 sizeFocus);


