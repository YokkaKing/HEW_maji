//============================================
// selectWeaponTerrain.cpp
// 
// 武器・地形選択シーン用ファイル
// 作成日:2026/1/10
// 最終更新日:2026/1/21
//============================================
#include "selectWeaponTerrain.h"
#include "Manager.h"
#include "keyboard.h"
#include "fade.h"
#include "shader.h"

//武器・地形のセット総数
static const int selectCount = 5;

static ID3D11ShaderResourceView* g_TextureBG[2] = { NULL, NULL };	//テクスチャ2枚を表すオブジェクト
static ID3D11ShaderResourceView* g_TextureUI[selectCount] = { NULL };
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static inGameWTselect g_selectData;
static int g_cursorP1 = 0; 
static int g_cursorP2 = 0;

static bool g_isP1Ready = false;
static bool g_isP2Ready = false;


void selectWT_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	//初期選択状態を設定
	g_cursorP1 = 0;
	g_cursorP2 = 0;
	g_isP1Ready = false;
	g_isP2Ready = false;
	g_selectData.player1 = WeaponTerrain::SWORD_WALL;
	g_selectData.player2 = WeaponTerrain::SWORD_WALL;

	//UIテクスチャのロード
	//テクスチャ読み込みなど
	//選択画面の背景画像ロード
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\chooseBg_1.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBG[0]);
		assert(g_TextureBG[0]);//読み込み失敗時にダイアログを表示
	}
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\chooseBg_2.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBG[1]);
		assert(g_TextureBG[1]);//読み込み失敗時にダイアログを表示

	}
	//選択画面の武器アイコンロード
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\sword.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[0]);
		assert(g_TextureUI[0]);//読み込み失敗時にダイアログを表示
	}
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\spear.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[1]);
		assert(g_TextureUI[1]);//読み込み失敗時にダイアログを表示
	}
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\bow.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[2]);
		assert(g_TextureUI[2]);//読み込み失敗時にダイアログを表示
	}
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\hammer.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[3]);
		assert(g_TextureUI[3]);//読み込み失敗時にダイアログを表示
	}
	{
		TexMetadata		metadata;
		ScratchImage	image;
		LoadFromWICFile(L"asset\\texture\\shuriken.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
		CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[4]);
		assert(g_TextureUI[4]);//読み込み失敗時にダイアログを表示
	}

	//フェードインのセット
	XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(60.0f, color, FADE_IN, SCENE_GAME);

}

void selectWT_Finalize()
{
	SAFE_RELEASE(g_TextureBG[0]);
	SAFE_RELEASE(g_TextureBG[1]);
	for (int i = 0; i < selectCount; i++)
	{
		SAFE_RELEASE(g_TextureUI[i]);
	}
}

void selectWT_Update()
{
	if (g_isP1Ready == false)
	{
		//P1の選択処理 横←→で選択
		if (Keyboard_IsKeyDownTrigger(KK_LEFT))
		{
			g_cursorP1 = (g_cursorP1 + selectCount - 1) % selectCount;
		}
		if (Keyboard_IsKeyDownTrigger(KK_RIGHT))
		{
			g_cursorP1 = (g_cursorP1 + 1) % selectCount;
		}

		//決定処理 左コントロール
		if (Keyboard_IsKeyDownTrigger(KK_LEFTCONTROL))
		{
			g_isP1Ready = true;
			//決定した瞬間にデータを確定
			g_selectData.player1 = static_cast<WeaponTerrain>(g_cursorP1);
		}
		else
		{
			//準備完了のキャンセル処理
			if (Keyboard_IsKeyDownTrigger(KK_F1))g_isP1Ready = false;
		}
	}

	if (g_isP2Ready == false)
	{
		//P2の選択処理 数字キーで選択
		if (Keyboard_IsKeyDownTrigger(KK_D3))
		{
			g_cursorP2 = (g_cursorP2 + selectCount - 1) % selectCount;
		}
		if (Keyboard_IsKeyDownTrigger(KK_D4))
		{
			g_cursorP2 = (g_cursorP2 + 1) % selectCount;
		}

		//決定処理 右コントロール
		if (Keyboard_IsKeyDownTrigger(KK_RIGHTCONTROL))
		{
			g_isP2Ready = true;
			//決定した瞬間にデータを確定
			g_selectData.player2 = static_cast<WeaponTerrain>(g_cursorP2);
		}
		else
		{
			//準備完了のキャンセル処理
			if (Keyboard_IsKeyDownTrigger(KK_DELETE))g_isP2Ready = false;
		}
	}

	//両方準備完了時のみ状態遷移
	if (g_isP1Ready && g_isP2Ready)
	{
		//Managerに選択データを渡す
		Manager_SetWTselect(g_selectData);

		//ゲームシーンへフェード
		XMFLOAT4 fadeColor(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, fadeColor, FADE_STATE::FADE_OUT, SCENE_GAME);
	}
}

void selectWT_Draw(int playerID)
{
	//ウィンドウサイズを取得
	float screenWidth = (float)Direct3D_GetBackBufferWidth();
	float screenHeight = (float)Direct3D_GetBackBufferHeight();

	//背景描画
	if (g_TextureBG[playerID])
	{
		// テクスチャをセット
		g_pContext->PSSetShaderResources(0, 1, &g_TextureBG[playerID]);
		DrawSprite(XMFLOAT2(screenWidth * 0.5f, screenHeight * 0.5f), XMFLOAT2(screenWidth, screenHeight), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	int currentCursor;
	bool isReady;
	XMFLOAT4 highlightColors;

	//各プレイヤーのカーソル描画処理
	//P1
	if (playerID == 0)
	{
		currentCursor = g_cursorP1;
		isReady = g_isP1Ready;
		highlightColors = { 1.0f, 0.8f, 0.2f, 1.0f };
	}
	else //P2
	{
		currentCursor = g_cursorP2;
		isReady = g_isP2Ready;
		highlightColors = { 0.2f, 0.9f, 1.0f, 1.0f };
	}

	float startX = 460.0f;  //アイコン並びの開始位置
	float spacing = 250.0f; //アイコン同士の間隔
	float posY = 500.0f;

	for (int i = 0; i < selectCount; i++)
	{
		if (g_TextureUI[i] == NULL) continue;

		XMFLOAT2 pos = { startX + (i * spacing), posY };
		XMFLOAT2 size = { 200.0f, 200.0f };
		XMFLOAT4 color = { 0.5f, 0.5f, 0.5f, 1.0f }; //選択されていないアイコンは暗く

		//現在自分が選んでいるアイコンの強調
		if (i == currentCursor)
		{
			size = { 260.0f, 260.0f };
			color = highlightColors;
		}

		//アイコンテクスチャをセットして描画
		g_pContext->PSSetShaderResources(0, 1, &g_TextureUI[i]);
		DrawSprite(pos, size, color);
	}

	if (g_selectData.player1 == WeaponTerrain::SWORD_WALL)
	{
		//P1が剣を選んだ場合のUI描画処理

	}
	else if (g_selectData.player1 == WeaponTerrain::SPEAR_HILL)
	{
		//槍

	}
	//その他武器を同様

}

bool selectWT_IsP1Ready()
{
	return g_isP1Ready;
}

bool selectWT_IsP2Ready()
{
	return g_isP2Ready;
}
