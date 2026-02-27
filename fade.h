/*
* ファイル名	fade.h
* タイトル	フェード
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef FADE_H
#define FADE_H

//================================================================
//	インクルード
//================================================================
#include"direct3d.h"
#include"sprite.h"
#include"Manager.h"

enum FADE_STATE
{
	FADE_NONE = 0,
	FADE_IN,
	FADE_OUT,

};

struct FadeObject
{
	FADE_STATE	state;			//フェード処理状態
	float		count;			//カウンター
	float		frame[3];			//フェード処理時間
	XMFLOAT4	fadecolor;		//フェード色
	SCENE		scene;			//次に切り替わるシーン
	XMFLOAT4 col;
	
};

void Fade_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Fade_Finalize();
void Fade_Update();
void Fade_Draw();

void SetFade(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene);
void SetFadeOutOnly(int fadeframe, XMFLOAT4 color, SCENE scene);
FADE_STATE GetFadeState();

#endif // FADE_H