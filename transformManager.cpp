//============================================
// transformManager.cpp
// 
// 変身先の武器・地形選択用ファイル
// 作成日:2026/1/27
// 最終更新日:2026/1/28
//============================================
#include "transformManager.h"
#include <algorithm>
#include <random>
#include "keyboard.h"
#include "shader.h"

//コンストラクタ
TransformManager::TransformManager()
{
	// 全てのポインタを nullptr で安全に初期化
	m_pContext = nullptr;
	for (int i = 0; i < (int)WeaponTerrain::MAX; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			m_pWeaponTextures[i][j] = nullptr;
		}
	}

	// 状態の初期化
	m_isActive = false;
	m_timer = 20.0f;

	// プレイヤー状態の初期化
	m_p1.isReady = false;
	m_p1.selectedIndex = 0;
	m_p1.selectedWT = WeaponTerrain::NONE;

	m_p2.isReady = false;
	m_p2.selectedIndex = 0;
	m_p2.selectedWT = WeaponTerrain::NONE;
}

//デストラクタ
TransformManager::~TransformManager()
{
	Finalize();
}

void TransformManager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pContext = pContext;

	//変身先テクスチャのロード 選択中と非選択中で全10種
	const char* paths[6][2] = {
		"", "", //enum class WeaponTerrainにNONEを追加したため
		"asset\\texture\\Sword_Active.png", "asset\\texture\\Sword_Inactive.png",
		"asset\\texture\\Spear_Active.png", "asset\\texture\\Spear_Inactive.png",
		"asset\\texture\\Bow_Active.png", "asset\\texture\\Bow_Inactive.png",
		"asset\\texture\\Hammer_Active.png", "asset\\texture\\Hammer_Inactive.png",
		"asset\\texture\\Shuriken_Active.png", "asset\\texture\\Shuriken_Inactive.png",
	};

	for (int i = 1; i < (int)WeaponTerrain::MAX; i++) //0番(NONE)を飛ばす
	{
		m_pWeaponTextures[i][0] = CreateSRV(pDevice, paths[i][0]); //選択中(明るい)
		m_pWeaponTextures[i][1] = CreateSRV(pDevice, paths[i][1]); //非選択中(暗い)

	}


}

void TransformManager::Finalize()
{
	for (int i = 0; i < (int)WeaponTerrain::MAX; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (m_pWeaponTextures[i][j] != nullptr)
			{
				m_pWeaponTextures[i][j]->Release();
				m_pWeaponTextures[i][j] = nullptr;
			}
		}
	}
	m_pContext = nullptr;
}

bool TransformManager::Update(float deltaTime)
{
	if (!m_isActive)
	{
		return false;
	}

	//タイマー更新
	m_timer -= deltaTime;
	if (m_timer <= 0.0f)
	{
		m_timer = 0.0f;
		//時間切れのため強制的にReadyにする
		m_p1.isReady = true;
		m_p1.selectedWT = m_p1.choices[m_p1.selectedIndex]; //決定

		m_p2.isReady = true;
		m_p2.selectedWT = m_p2.choices[m_p2.selectedIndex]; //決定
	}

	//P1の入力処理
	if (!m_p1.isReady)
	{
		if (Keyboard_IsKeyDownTrigger(KK_LEFT))
		{
			m_p1.selectedIndex = 0; //左を選択
		}
		if (Keyboard_IsKeyDownTrigger(KK_RIGHT))
		{
			m_p1.selectedIndex = 1; //右を選択
		}
		if (Keyboard_IsKeyDownTrigger(KK_LEFTCONTROL))
		{
			m_p1.isReady = true;
			m_p1.selectedWT = m_p1.choices[m_p1.selectedIndex]; //決定
		}
	}
	//P2の入力処理
	if (!m_p2.isReady)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D3))
		{
			m_p2.selectedIndex = 0; //左を選択
		}
		if (Keyboard_IsKeyDownTrigger(KK_D4))
		{
			m_p2.selectedIndex = 1; //右を選択
		}
		if (Keyboard_IsKeyDownTrigger(KK_D5))
		{
			m_p2.isReady = true;
			m_p2.selectedWT = m_p2.choices[m_p2.selectedIndex]; //決定
		}
	}

	//両者が準備完了したかのチェック
	if (m_p1.isReady && m_p2.isReady)
	{
		//少し間を置いてから終了
		static float finishTimer = 0.0f;
		finishTimer += deltaTime;
		if (finishTimer > 1.0f) { //1秒待ってから
			m_isActive = false;   //選択画面終了
			finishTimer = 0.0f;
			return true;
		}
	}
	return false;
}

void TransformManager::StartSelection(WeaponTerrain excludeP1, WeaponTerrain excludeP2)
{
	m_isActive = true;
	m_timer = 20.0f;

	m_p1.isReady = false;
	m_p2.isReady = false;
	m_p1.selectedIndex = 0;
	m_p2.selectedIndex = 0;

	//ShuffleChoices(m_p1);
	//ShuffleChoices(m_p2);

	// NONE(0)を除いた武器の数 (MAX=6なら5種類)
	int weaponCount = (int)WeaponTerrain::MAX - 1;

	// ------------
	// P1 の抽選
	// ------------
	for (int i = 0; i < 2; i++)
	{
		WeaponTerrain candidate;
		bool isValid = false;

		// 有効な武器が出るまで回す
		while (!isValid)
		{
			// 1 ～ weaponCount のランダム値
			int r = (rand() % weaponCount) + 1;
			candidate = (WeaponTerrain)r;

			// チェック1: 引数で指定された「除外武器(1回目に選んだやつ)」ならNG
			if (candidate == excludeP1) continue;

			// チェック2: 今回の2択の中で重複していたらNG (choices[0]と同じなら弾く)
			if (i == 1 && candidate == m_p1.choices[0]) continue;

			// ここまで来ればOK
			isValid = true;
		}
		m_p1.choices[i] = candidate;
	}

	// ----------
	// P2 の抽選 
	// ----------
	for (int i = 0; i < 2; i++)
	{
		WeaponTerrain candidate;
		bool isValid = false;

		while (!isValid)
		{
			int r = (rand() % weaponCount) + 1;
			candidate = (WeaponTerrain)r;

			if (candidate == excludeP2) continue;
			if (i == 1 && candidate == m_p2.choices[0]) continue;

			isValid = true;
		}
		m_p2.choices[i] = candidate;
	}
}

void TransformManager::ShuffleChoices(PlayerState& state)
{
	std::vector<WeaponTerrain> pool = {
		WeaponTerrain::SWORD_WALL,
		WeaponTerrain::SPEAR_HILL,
		WeaponTerrain::BOW_HILL,
		WeaponTerrain::HAMMER_,
		WeaponTerrain::SHURIKEN_
	};

	//武器と地形のセットをランダムで表示するための処理
	std::random_device random;
	std::mt19937 g(random());

	std::shuffle(pool.begin(), pool.end(), g);

	//シャッフルされた武器と地形のセットをプレイヤーの選択肢に割り当てる
	state.choices[0] = pool[0];
	state.choices[1] = pool[1];
	state.selectedIndex = 0;

}

void TransformManager::Draw(int windowID)
{
	if (!m_isActive || !m_pContext)
	{
		return;
	}

	//シェーダーリセット
	Shader_Begin();

	//画面サイズを取得
	float sw = (float)Direct3D_GetBackBufferWidth();
	float sh = (float)Direct3D_GetBackBufferHeight();

	XMMATRIX matProj = XMMatrixOrthographicOffCenterLH(0.0f, sw, sh, 0.0f, 0.0f, 1.0f);
	Shader_SetMatrix(matProj);

	//ワールド行列リセット
	Shader_SetWorldMatrix(XMMatrixIdentity());

	//ブレンド有効化
	SetBlendState(BLENDSTATE_ALFA);

	//描画座標用変数
	float centerX = sw * 0.5f;
	float centerY = sh * 0.5f;

	if (windowID == 0)
	{
		DrawPlayerUI(m_p1, XMFLOAT2(centerX, centerY));
	}
	else if (windowID == 1)
	{
		DrawPlayerUI(m_p2, XMFLOAT2(centerX, centerY));
	}
}

//変身先選択用関数
void TransformManager::DrawPlayerUI(const PlayerState& state, XMFLOAT2 basePos)
{
	float cardSpacing = 200.0f; //変身先UIの間隔

	for (int i = 0; i < 2; i++)
	{
		WeaponTerrain type = state.choices[i];
		XMFLOAT2 pos = { basePos.x + (i == 0 ? -cardSpacing : cardSpacing), basePos.y };

		//選択中ならインデックス[0](明るい)、そうでなければ[1](暗い)を使用
		int textureState = (state.selectedIndex == i) ? 0 : 1;
		ID3D11ShaderResourceView* pTex = m_pWeaponTextures[(int)type][textureState];

		if (pTex) {
			m_pContext->PSSetShaderResources(0, 1, &pTex);

			//選択中の強調サイズ
			XMFLOAT2 size = (state.selectedIndex == i) ? XMFLOAT2(240, 340) : XMFLOAT2(200, 300);

			DrawSprite(pos, size, DirectX::XMFLOAT4(1, 1, 1, 1));
		}
	}

}

//内部用テクスチャ読み込み補助
ID3D11ShaderResourceView* TransformManager::CreateSRV(ID3D11Device* pDevice, const char* fileName) {
	//char* から wchar_t* (ワイド文字列) への変換
	wchar_t wFileName[MAX_PATH];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wFileName, fileName, _TRUNCATE);

	//DirectXTex を使用したロード処理
	TexMetadata metadata;
	ScratchImage image;

	//WICファイル（PNG等）からロード
	HRESULT hr = LoadFromWICFile(wFileName, WIC_FLAGS_FORCE_SRGB, &metadata, image);
	if (FAILED(hr)) {
		//ロード失敗時のデバッグ出力など（必要に応じて）
		char buf[512];
		sprintf_s(buf, "【Texture Error】ロード失敗: %s (Error Code: 0x%08X)\n", fileName, hr);
		OutputDebugStringA(buf); // Visual Studioの出力ウィンドウに表示
		return nullptr;
	}
	OutputDebugStringA("【Texture Success】ロード完了\n");

	//シェーダーリソースビューの作成
	ID3D11ShaderResourceView* pSRV = nullptr;
	hr = DirectX::CreateShaderResourceView(
		pDevice,
		image.GetImages(),
		image.GetImageCount(),
		metadata,
		&pSRV
	);

	if (FAILED(hr)) {
		return nullptr;
	}

	//成功確認のテスト
	assert(pSRV);

	return pSRV;
}




