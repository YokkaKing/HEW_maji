
//field.h
#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
using namespace DirectX;

#define BOX_RADIUS (0.5f)


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

MAPDATA* GetFieldMap();