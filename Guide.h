#pragma once
#ifndef ENEMY_GUIDE_H
#define ENEMY_GUIDE_H

#include <d3d11.h>
#include <DirectXMath.h>

class Guide {
public:
    static void Initialize(ID3D11Device* pDevice);
    static void Finalize();
    static void Draw(bool isPlayer1); //各プレイヤーの描画ループ内で呼ぶ

private:
    static DirectX::XMFLOAT2 ProjectWorldToScreen(DirectX::XMFLOAT3 worldPos, DirectX::XMMATRIX view, DirectX::XMMATRIX proj);
};


#endif
