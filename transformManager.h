#pragma once
//============================================
// transformManager.h
// 
// 変身先の武器・地形選択用ファイル
// 作成日:2026/1/27
// 最終更新日:2026/1/28
//============================================
#include "selectWeaponTerrain.h"
#include <vector>

class TransformManager
{
	struct PlayerState {
		WeaponTerrain choices[2]; //ランダムに選ばれる武器と地形のセット
		int selectedIndex = 0;    //現在カーソルがある方(0, 1)
		bool isReady = false;     //準備完了か否か
		WeaponTerrain selectedWT; //最終的に選んだ武器と地形のセット
	};

private:
	ID3D11DeviceContext* m_pContext = nullptr;
	ID3D11ShaderResourceView* m_pWeaponTextures[(int)WeaponTerrain::MAX][2];//選択時

	PlayerState m_p1;
	PlayerState m_p2;
	float m_timer = 20.0f;
	bool m_isActive = false;
public:
	TransformManager();
	~TransformManager();

	//ラウンド開始前に呼ぶ：ランダムな2択を生成
	void StartSelection(WeaponTerrain excludeP1 = WeaponTerrain::NONE,
		WeaponTerrain excludeP2 = WeaponTerrain::NONE);

	void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Finalize();
	//更新処理：戻り値が true なら全員の準備完了（またはタイムアップ）
	bool Update(float deltaTime);

	//描画処理：用意されているテクスチャを描画
	void Draw(int windowID);
	void DrawPlayerUI(const PlayerState& state, XMFLOAT2 basePos);

	//決定したデータを取得
	inGameWTselect GetPlayerSelectionWT() const {
		inGameWTselect result;
		result.player1 = m_p1.selectedWT;
		result.player2 = m_p2.selectedWT;
		return result;
	}

	bool IsActive() const { return m_isActive; }

private:
	void ShuffleChoices(PlayerState& state);
	// 内部用テクスチャ読み込み補助 (プロジェクトの読み込み方式に合わせる)
	//SRV = ShaderResourceView
	ID3D11ShaderResourceView* CreateSRV(ID3D11Device* pDevice, const char* fileName);
};