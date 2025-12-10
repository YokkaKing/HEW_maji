/*
* ファイル名	field.h
* タイトル	フィールド
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef FIELD_H
#define FIELD_H

//================================================================
//	マクロ定義
//================================================================
#define BOX_RADIUS (0.5f)

//================================================================
//	インクルード
//================================================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
#include<string>
using namespace DirectX;

//MAP構成ブロックの種類
enum FIELD
{
	FIELD_BOX = 0,	

	FIELD_OBT,

	FIELD_LIFT,

	FIELD_MAX
};
//MAPデータ構造体
class MAPDATA
{
	public:
		XMFLOAT3	pos;	//ブロックの座標
		FIELD		no;		//ブロックの種類
		//その他必要な物は追加する
};
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Field_Finalize(void);
void Field_Draw(void);
void Field_Update(void);

void SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay);
void SetObject(XMFLOAT3 pos, float radius, std::string tag, int lay);

MAPDATA* GetFieldMap();

#endif // FIELD_H