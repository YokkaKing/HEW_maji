#pragma once
#include <d3d11.h>

void Entry_Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
void Entry_Update();
void Entry_Draw();
void Entry_Finalize();


