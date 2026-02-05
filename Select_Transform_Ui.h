#pragma once
#include "direct3d.h"


class SELECT_TRANSFORM_UI
{
	public:
	XMFLOAT2 pos;
	XMFLOAT2 size;
	XMFLOAT4 col;
	bool isUsed;
	int SelectNum;
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

void Select_Weapon_Ui_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Select_Weapon_Ui_Finalize();
void Select_Weapon_Ui_Update();
void Select_Weapon_Ui_Draw();

void SetTransformUi_IsUsed(bool flg);
void SetTransformUi_time(float time);
void SetTransformUi_SelectNum(int num);



