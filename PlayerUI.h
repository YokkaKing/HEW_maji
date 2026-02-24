#ifndef PLAYER_NAME_UI_H
#define PLAYER_NAME_UI_H

#include <d3d11.h>
#include <DirectXMath.h>

class PlayerUI {
public:
    static void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    static void Finalize();
    static void Update();
    static void Draw(bool isPlayer1);

private:
    // 3D座標をスクリーン座標(2D)に変換するヘルパー関数
    static DirectX::XMFLOAT2 ProjectWorldToScreen(DirectX::XMFLOAT3 worldPos, DirectX::XMMATRIX view, DirectX::XMMATRIX proj);    static ID3D11ShaderResourceView* m_pTexture;
};

#endif