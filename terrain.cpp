/*
* ファイル名	terrain.cpp
* タイトル	地形
* 作成者		久保木幹太
* 作成日		11月25日
* 更新日		12月02日
*/

//================================================================
//	マクロ定義
//================================================================
#define TERRAIN_SIZE (0.25f)

//================================================================
//	インクルード
//================================================================
#include"terrain.h"
#include"keyboard.h"
#include"colliderFactory.h"
#include"debug_ostream.h"
#include"shader.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include<string>

//================================================================
//	グローバル変数
//================================================================
// 地形オブジェクト
TERRAIN g_Terrain;

static ID3D11Device* g_pDevice;
static ID3D11DeviceContext* g_pContext;

MODEL* blockModel = nullptr;

//================================================================
//	一文字0.25立法メートルとする
//	n -> 何もなし
//	a -> 当たり判定
//================================================================
// 地形::丘 の座標データ
const std::vector<std::vector<std::vector<std::string>>> Hill =
{
	{ // Y = 0 // Z->+ // X↓+
		{"anananananan"},
		{"nnnnnnnnnnna"},
		{"annnnnnnnnnn"},
		{"nnnnnnnnnnna"},
		{"annnnnnnnnnn"},
		{"nnnnnnnnnnna"},
		{"annnnnnnnnnn"},
		{"nnnnnnnnnnna"},
		{"annnnnnnnnnn"},
		{"nnnnnnnnnnna"},
		{"annnnnnnnnnn"},
		{"nananananana"},
	},
	{ // Y = 0.25
		{"nnnnnnnnnnnn"},
		{"nananananann"},
		{"nnnnnnnnnnan"},
		{"nannnnnnnnnn"},
		{"nnnnnnnnnnan"},
		{"nannnnnnnnnn"},
		{"nnnnnnnnnnan"},
		{"nannnnnnnnnn"},
		{"nnnnnnnnnnan"},
		{"nannnnnnnnnn"},
		{"nnananananan"},
		{"nnnnnnnnnnnn"},
	},
	{ // Y = 0.5
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnanananannn"},
		{"nnnnnnnnnann"},
		{"nnannnnnnnnn"},
		{"nnnnnnnnnann"},
		{"nnannnnnnnnn"},
		{"nnnnnnnnnann"},
		{"nnannnnnnnnn"},
		{"nnnanananann"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
	},
	{ // Y = 0.75
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnananannnn"},
		{"nnnnnnnnannn"},
		{"nnnannnnnnnn"},
		{"nnnnnnnnannn"},
		{"nnnannnnnnnn"},
		{"nnnnananannn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
	},
	{ // Y = 0.75
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnanannnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnanannnn"},
		{"nnnnnnannnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
	},
	{ // Y = 1
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnannnnn"},
		{"nnnnnnannnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
		{"nnnnnnnnnnnn"},
	},
};

//================================================================
//	同じ文字は絶対に4文字入力すること
//	各文字を頂点とし、それぞれを結んだ四角形を当たり判定とする
//	n -> 何もなし
//	文字は分かりやすいやつでa,b,cとか
//================================================================
const std::vector<std::vector<std::vector<std::string>>> Hills =
{// -> プレイヤーの初期視点
	// 16個
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"annnnnnnnnnnnnnnnnna"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"annnnnnnnnnnnnnnnnna"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nbnnnnnnnnnnnnnnnnbn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nbnnnnnnnnnnnnnnnnbn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nncnnnnnnnnnnnnnncnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nncnnnnnnnnnnnnnncnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnndnnnnnnnnnnnndnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnndnnnnnnnnnnnndnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnennnnnnnnnnennnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnennnnnnnnnnennnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnfnnnnnnnnfnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnfnnnnnnnnfnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnngnnnnnngnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnngnnnnnngnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnhnnnnhnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnhnnnnhnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnninninnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnninninnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
	{
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnjjnnnnnnnnn"},
		{"nnnnnnnnnjjnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnn"},
	},
};

// 壁の当たり判定
const std::vector<std::vector<std::vector<std::string>>> Walls =
{
	{
		{"annnnnnnna"},
		{"cnnnnnnnnd"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"cnnnnnnnnd"},
		{"bnnnnnnnnb"},
	},
	{
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
	},
	{
		{"annnnnnnna"},
		{"cnnnnnnnnd"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"cnnnnnnnnd"},
		{"bnnnnnnnnb"},
	},
};

void TerrainInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	g_Terrain.m_moveTerrain[0] = ModelLoad("asset\\model\\hill.fbx");
	g_Terrain.m_moveTerrain[1] = ModelLoad("asset\\model\\wall.fbx");
	blockModel = ModelLoad("asset\\model\\block.fbx");

	g_Terrain.m_motherPosition[0] = GetPlayerPosition();
	g_Terrain.m_motherPosition[1] = GetPlayer2Position();
	g_Terrain.m_terrainScale[0] = { 5.0f, 5.0f, 5.0f };
	g_Terrain.m_terrainScale[1] = { 10.0f, 6.5f, 10.0f };
	g_Terrain.m_terrainRotation[0] = { 0.0f,0.0f,0.0f };
	g_Terrain.m_terrainRotation[1] = { 0.0f,0.0f,0.0f };

	g_Terrain.m_motherPosition[0].y -= 5.0f;
	g_Terrain.m_motherPosition[1].y -= 3.0f;

	g_Terrain.m_coolTime[0] = 0.0f;
	g_Terrain.m_coolTime[1] = 0.0f;

	//g_Terrain.PixelObjects(Hill, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition);

	// 丘の当たり判定
	g_Terrain.SimpleObjects(Hills, { 0.25f, 0.25f, 0.25f }, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition[0]);
	g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::WALL, g_Terrain.m_motherPosition[1]);
	//hal::dout << "座標 : (" << g_Terrain.slopes[0]->m_position.x << "," << g_Terrain.slopes[0]->m_position.y << "," << g_Terrain.slopes[0]->m_position.z << ")\n";
}
void TerrainFinalize()
{
	//ModelRelease(g_Terrain.m_model);
	ModelRelease(blockModel);

	for (int i = 0; i < 2; i++)
	{
		ModelRelease(g_Terrain.m_moveTerrain[i]);
		g_Terrain.m_isChange[i] = false;
	}

	g_Terrain.terrainObjects.clear();
	g_Terrain.hills.clear();
	g_Terrain.walls.clear();
	g_Terrain.trees.clear();
}
void TerrainUpdate()
{
	if (Keyboard_IsKeyDown(KK_R))
	{
		// クールタイムがあれば発動できない
		if (g_Terrain.m_coolTime[0] <= 0)
		{
			g_Terrain.m_isChange[0] = true; // 1Pの変身を確認
			g_Terrain.m_coolTime[0] = 60.0f;
		}
	}

	if (Keyboard_IsKeyDown(KK_L))
	{
		// クールタイムがあれば発動できない
		if (g_Terrain.m_coolTime[1] <= 0)
		{
			g_Terrain.m_isChange[1] = true; // 2Pの変身を確認
			g_Terrain.m_coolTime[1] = 60.0f;
		}
	}

	// 変身したら-する
	if (g_Terrain.m_isChange[0])
	{
		// クールタイムあったら-する
		if (g_Terrain.m_coolTime[0] > 0.0f)
		{
			g_Terrain.m_coolTime[0] -= 1.0f / 10.0f;
		}
		else // 無かったら
		{
			g_Terrain.m_coolTime[0] = 0.0f;	// クールタイムをなくす
			g_Terrain.m_isChange[0] = false; // 変身を解く
		}
	}
	// 変身したら-する
	if (g_Terrain.m_isChange[1])
	{
		// クールタイムあったら-する
		if (g_Terrain.m_coolTime[1] > 0.0f)
		{
			g_Terrain.m_coolTime[1] -= 1.0f / 10.0f;
		}
		else // 無かったら
		{
			g_Terrain.m_coolTime[1] = 0.0f;	// クールタイムをなくす
			g_Terrain.m_isChange[1] = false; // 変身を解く
		}
	}

	// 変身してなければ追従P1
	if (!g_Terrain.m_isChange[0])
	{
		g_Terrain.m_motherPosition[0] = GetPlayerPosition();
		g_Terrain.m_motherPosition[0].y -= 5.0f; // 下に設定
		g_Terrain.UpdateObject(g_Terrain.hills, g_Terrain.m_motherPosition[0], FALSE); // P1の地形の当たり判定
	}
	else
	{
		// 変身したら上昇
		if (g_Terrain.m_motherPosition[0].y < -0.5f)
		{
			g_Terrain.m_motherPosition[0].y += 0.1f;
			g_Terrain.UpdateObject(g_Terrain.hills, g_Terrain.m_motherPosition[0], TRUE); // P1の地形の当たり判定
		}
	}

	// 変身してなければ追従P2
	if (!g_Terrain.m_isChange[1])
	{
		g_Terrain.m_motherPosition[1] = GetPlayer2Position();
		g_Terrain.m_motherPosition[1].y -= 3.0f; // 下に設定
		g_Terrain.UpdateObject(g_Terrain.walls, g_Terrain.m_motherPosition[1], FALSE); // P2の地形の当たり判定
	}
	else
	{
		// 変身したら上昇
		if (g_Terrain.m_motherPosition[1].y < 1.5f)
		{
			g_Terrain.m_motherPosition[1].y += 0.1f;
			g_Terrain.UpdateObject(g_Terrain.walls, g_Terrain.m_motherPosition[1], TRUE); // P2の地形の当たり判定
		}
	}
}
void TerrainDraw()
{
	// 1Pが変身してないときは描画しない
	if (g_Terrain.m_isChange[0])
	{
		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.m_terrainScale[0].x,
			g_Terrain.m_terrainScale[0].y,
			g_Terrain.m_terrainScale[0].z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			g_Terrain.m_terrainRotation[0].x,
			g_Terrain.m_terrainRotation[0].y,
			g_Terrain.m_terrainRotation[0].z);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.m_motherPosition[0].x,
			g_Terrain.m_motherPosition[0].y,
			g_Terrain.m_motherPosition[0].z);
		XMMATRIX	world = scale * rotation * translation;

		//変換行列作成
		XMMATRIX	view = GetViewMatrix();
		XMMATRIX	projection = GetProjectionMatrix();
		XMMATRIX	wvp = world * view * projection;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);
		// Shader_SetMatrix(wvp);

		//モデルの描画リクエスト
		ModelDraw(g_Terrain.m_moveTerrain[0]);
	}
	// 2Pが変身してないときは描画しない
	if (g_Terrain.m_isChange[1])
	{
		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.m_terrainScale[1].x,
			g_Terrain.m_terrainScale[1].y,
			g_Terrain.m_terrainScale[1].z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			g_Terrain.m_terrainRotation[1].x,
			g_Terrain.m_terrainRotation[1].y,
			g_Terrain.m_terrainRotation[1].z);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.m_motherPosition[1].x,
			g_Terrain.m_motherPosition[1].y,
			g_Terrain.m_motherPosition[1].z);
		XMMATRIX	world = scale * rotation * translation;

		//変換行列作成
		XMMATRIX	view = GetViewMatrix();
		XMMATRIX	projection = GetProjectionMatrix();
		XMMATRIX	wvp = world * view * projection;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);
		// Shader_SetMatrix(wvp);

		//モデルの描画リクエスト
		ModelDraw(g_Terrain.m_moveTerrain[1]);
	}

	/*
	for (int i = 0; i < g_Terrain.hills.size(); i++)
	{
		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.hills[i]->m_scale.x,
			g_Terrain.hills[i]->m_scale.y,
			g_Terrain.hills[i]->m_scale.z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			g_Terrain.hills[i]->m_rotation.x,
			g_Terrain.hills[i]->m_rotation.y,
			g_Terrain.hills[i]->m_rotation.z);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.hills[i]->m_position.x,
			g_Terrain.hills[i]->m_position.y,
			g_Terrain.hills[i]->m_position.z);
		XMMATRIX	world = scale * rotation * translation;

		//変換行列作成
		XMMATRIX	view = GetViewMatrix();
		XMMATRIX	projection = GetProjectionMatrix();
		XMMATRIX	wvp = world * view * projection;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);
		// Shader_SetMatrix(wvp);

		//モデルの描画リクエスト
		ModelDraw(blockModel);
	}

	for (int i = 0; i < g_Terrain.walls.size(); i++)
	{
		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.walls[i]->m_scale.x,
			g_Terrain.walls[i]->m_scale.y,
			g_Terrain.walls[i]->m_scale.z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			g_Terrain.walls[i]->m_rotation.x,
			g_Terrain.walls[i]->m_rotation.y,
			g_Terrain.walls[i]->m_rotation.z);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.walls[i]->m_position.x,
			g_Terrain.walls[i]->m_position.y,
			g_Terrain.walls[i]->m_position.z);
		XMMATRIX	world = scale * rotation * translation;

		//変換行列作成
		XMMATRIX	view = GetViewMatrix();
		XMMATRIX	projection = GetProjectionMatrix();
		XMMATRIX	wvp = world * view * projection;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);
		// Shader_SetMatrix(wvp);

		//モデルの描画リクエスト
		ModelDraw(blockModel);
	}
	*/
}
// 簡単な四角形の当たり判定を作る場合
void TERRAIN::SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay)
{
	// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
	std::unique_ptr<GameObject> obj_owner(
		ColliderFactory::CreateBoxObject(pos, scl, tag, lay)
	);

	GameObject* raw_ptr = obj_owner.get(); // 生のポインタを取得（参照用）

	if (raw_ptr != nullptr)
	{
		if (raw_ptr->m_tag == "HILL") hills.push_back(raw_ptr);
		if (raw_ptr->m_tag == "Wall") walls.push_back(raw_ptr);
		if (raw_ptr->m_tag == "TREE") trees.push_back(raw_ptr);

		terrainObjects.push_back(std::move(obj_owner));
	}
}
// string型で書いたオブジェクトの当たり判定をchar型にして効率よくする
std::vector<std::vector<std::vector<char>>> TERRAIN::ConvertTerrain(std::vector<std::vector<std::vector<std::string>>> terrain)
{
	std::vector<std::vector<std::vector<char>>> obj;

	obj.reserve(terrain.size()); // 事前にY軸分の容量を確保

	for (const auto& layer : terrain) // Y軸方向
	{
		obj.emplace_back();					// 新しく枠を作る(次元の追加)
		obj.back().reserve(layer.size());	// Z軸分の容量を確保

		for (const auto& row : layer) // rowはstd::vector<std::string>
		{
			std::vector<char> rowChars;
			for (const auto& str : row) // strはstd::string
			{
				rowChars.insert(rowChars.end(), str.begin(), str.end());
			}
			obj.back().push_back(std::move(rowChars));
		}

	}

	return obj;
}
// char型になったオブジェクトの当たり判定がいくつあるのか数える
size_t TERRAIN::CountObjects(const std::vector<std::vector<std::vector<char>>>& obj)
{
	size_t count = 0;

	for (const auto& layer : obj)
	{
		for (const auto& row : layer)
		{
			for (char c : row)
			{
				if (c == 'a')
				{
					count++;
				}
			}
		}
	}

	return count;
}
// 受け取った総量のオブジェクトの当たり判定をデータとして格納する
std::vector<TERRAIN_OBJECT> TERRAIN::InitializeObject(const std::vector<std::vector<std::vector<char>>>& terrainChip, TERRAIN_TYPE type)
{
	std::vector<TERRAIN_OBJECT> terrain;

	size_t objects = 0;	// 何個オブジェクトがあるか

	objects = CountObjects(terrainChip); // オブジェクトの数を格納

	terrain.clear();
	terrain.reserve(objects);	// オブジェクトの数の分だけ事前に容量を確保

	for (size_t i = 0; i < objects; i++)
	{
		terrain.push_back(TERRAIN_OBJECT{}); // 空のTERRAIN_OBJECTを追加
	}

	int loop = 0;	// 何回ループしたか
	char c;			// 文字を取り出す

	XMFLOAT3 centerNo; // 真ん中の番号
	centerNo.y = (terrainChip.size() / 2);							// Y層の半分
	centerNo.x = (terrainChip[centerNo.y].size() / 2);				// X層の半分
	centerNo.z = (terrainChip[centerNo.y][centerNo.x].size() / 2);	// Z層の半分
	XMFLOAT3 distance;	// 現在の座標が、どれだけ中心と差があるか

	for (int y = 0; y < terrainChip.size(); y++)
	{
		for (int x = 0; x < terrainChip[y].size(); x++)
		{
			for (int z = 0; z < terrainChip[y][x].size(); z++)
			{
				c = terrainChip[y][x][z]; // 文字を取り出す

				distance.x = (x - centerNo.x) * TERRAIN_SIZE;	// Xが中心からどれだけ離れているか
				distance.y = (y - centerNo.y) * TERRAIN_SIZE;	// Yが中心からどれだけ離れているか
				distance.z = (z - centerNo.z) * TERRAIN_SIZE;	// Zが中心からどれだけ離れているか

				switch (c)
				{
				case 'a':
					terrain[loop].m_distance = distance;	// 中心からどれだけ離れているか渡す
					terrain[loop].m_size = { TERRAIN_SIZE, TERRAIN_SIZE, TERRAIN_SIZE };
					terrain[loop].m_type = type;			// 種類を格納
					loop++;
					break;

				default:
					break;
				}
			}
		}
	}

	return terrain;
}
// char型になったオブジェクトの種類がいくつあるか数える
size_t TERRAIN::CountObjectType(const std::vector<std::vector<std::vector<char>>>& obj)
{
	size_t count = 0;

	for (const auto& layer : obj)
	{
		for (const auto& row : layer)
		{
			for (char c : row)
			{
				if (c != 'n') // nじゃなかったら数える
				{
					count++;
				}
			}
		}
	}

	count /= 4; // 文字はそれぞれ4文字ずつあるから4で割る

	return count;
}
// char型になったオブジェクトの頂点を求めて、1つのオブジェクトとして認識する
std::vector<XMFLOAT6> TERRAIN::VolumeObject(const std::vector<std::vector<std::vector<char>>>& terrainChip, const size_t objectType, XMFLOAT3 size)
{
	// サイズチェック
	if (objectType == 0 || terrainChip.empty()) {
		return std::vector<XMFLOAT6>();
	}

	std::vector<XMFLOAT6> MIX_VALUE(objectType);
	std::vector<std::vector<XMFLOAT3>> vertexGroups(objectType); // 各タイプの頂点リスト
	std::vector<char> moji(objectType);

	int typeCount = 0;
	char c;

	// 配列サイズを取得
	int sizeY = terrainChip.size();
	int sizeZ = terrainChip[0].size();
	int sizeX = terrainChip[0][0].size();

	// 真ん中の座標を計算
	XMFLOAT3 centerPos;
	centerPos.x = (sizeX * size.x) / 2.0f;
	centerPos.y = (sizeY * size.y) / 2.0f;
	centerPos.z = (sizeZ * size.z) / 2.0f;

	// Y → Z → X の順でループ
	for (int y = 0; y < sizeY; y++)
	{
		for (int z = 0; z < sizeZ; z++)
		{
			for (int x = 0; x < sizeX; x++)
			{
				c = terrainChip[y][z][x];

				if (c == 'n') continue; // スキップ

				// セルの開始位置（角）を計算
				XMFLOAT3 worldPos;
				worldPos.x = x * size.x;
				worldPos.y = y * size.y;
				worldPos.z = z * size.z;

				// 既存の文字タイプを検索
				int foundIndex = -1;
				for (int i = 0; i < typeCount; i++)
				{
					if (moji[i] == c)
					{
						foundIndex = i;
						break;
					}
				}

				if (foundIndex == -1) // 新しい文字タイプ
				{
					if (typeCount >= objectType) continue; // 範囲チェック

					moji[typeCount] = c;
					vertexGroups[typeCount].push_back(worldPos);
					typeCount++;
				}
				else // 既存の文字タイプ
				{
					vertexGroups[foundIndex].push_back(worldPos);
				}
			}
		}
	}

	// 各タイプのバウンディングボックスを計算
	for (int i = 0; i < typeCount; i++)
	{
		if (vertexGroups[i].empty()) continue;

		// 4頂点だけ使う（最初の4つ）
		int vertexCount = (std::min)(4, (int)vertexGroups[i].size());

		XMFLOAT3 min = vertexGroups[i][0];
		XMFLOAT3 max = vertexGroups[i][0];

		// 4頂点分だけチェック
		for (int j = 0; j < vertexCount; j++)
		{
			min.x = (std::min)(min.x, vertexGroups[i][j].x);
			min.y = (std::min)(min.y, vertexGroups[i][j].y);
			min.z = (std::min)(min.z, vertexGroups[i][j].z);

			max.x = (std::max)(max.x, vertexGroups[i][j].x);
			max.y = (std::max)(max.y, vertexGroups[i][j].y);
			max.z = (std::max)(max.z, vertexGroups[i][j].z);
		}

		// バウンディングボックスのサイズを計算
		MIX_VALUE[i].size.x = (max.x - min.x) + size.x;
		MIX_VALUE[i].size.y = (max.y - min.y) + size.y;
		MIX_VALUE[i].size.z = (max.z - min.z) + size.z;

		// バウンディングボックスの中心を計算
		XMFLOAT3 boxCenter;
		boxCenter.x = (min.x + max.x + size.x) / 2.0f;
		boxCenter.y = (min.y + max.y + size.y) / 2.0f;
		boxCenter.z = (min.z + max.z + size.z) / 2.0f;

		// マザーポジションからの相対座標を計算
		MIX_VALUE[i].pos.x = boxCenter.x - centerPos.x;
		MIX_VALUE[i].pos.y = boxCenter.y - centerPos.y;
		MIX_VALUE[i].pos.z = boxCenter.z - centerPos.z;
	}

	return MIX_VALUE;
}
// 受け取った総量のオブジェクトの当たり判定をデータとして格納する
std::vector<TERRAIN_OBJECT> TERRAIN::InitializeObject(const std::vector<std::vector<std::vector<char>>>& terrainChip, std::vector<XMFLOAT6> mixVal, const size_t objectType, TERRAIN_TYPE type)
{
	std::vector<TERRAIN_OBJECT> terrain(objectType); // サイズを指定

	for (size_t i = 0; i < objectType && i < mixVal.size(); i++) // 範囲チェック
	{
		terrain[i].m_distance = mixVal[i].pos;
		terrain[i].m_size = mixVal[i].size;
		terrain[i].m_type = type;
	}

	return terrain;
}
// 当たり判定の座標を更新する
void TERRAIN::UpdateObject(std::vector<GameObject*> terrain, XMFLOAT3 motherPosition, bool move)
{
	if (!move)
	{
		XMFLOAT3 pos;
		// 変身してなければプレイヤーと同じ動きをする
		for (int i = 0; i < terrain.size(); i++)
		{
			pos.x = motherPosition.x + terrain[i]->m_velocity.x;
			pos.y = motherPosition.y + terrain[i]->m_velocity.y;
			pos.z = motherPosition.z + terrain[i]->m_velocity.z;

			terrain[i]->m_position = pos;
		}
	}
	else
	{
		for (int i = 0; i < terrain.size(); i++)
		{
			terrain[i]->Move(0.0f, 0.1f, 0.0f);
		}
	}
}
// 当たり判定を作る
void TERRAIN::CreateHit(std::vector<TERRAIN_OBJECT> terrain, XMFLOAT3 motherPosition)
{
	XMFLOAT3 pos;

	// terrainの数だけ繰り返す
	for (int i = 0; i < terrain.size(); i++)
	{
		// distanceを使って座標を求める
		pos.x = motherPosition.x + terrain[i].m_distance.x;
		pos.y = motherPosition.y + terrain[i].m_distance.y;
		pos.z = motherPosition.z + terrain[i].m_distance.z;

		switch (terrain[i].m_type)
		{
		case TERRAIN_TYPE::HILL:
			SetObject(pos, terrain[i].m_size, "HILL", 0);
			hills[i]->m_position = pos;	// 座標を格納
			hills[i]->m_velocity = terrain[i].m_distance;
			hills[i]->m_scale = terrain[i].m_size;
			break;

		case TERRAIN_TYPE::WALL:
			SetObject(pos, terrain[i].m_size, "Wall", 0);
			walls[i]->m_position = pos;	// 座標を格納
			walls[i]->m_velocity = terrain[i].m_distance;
			walls[i]->m_scale = terrain[i].m_size;
			break;

		case TERRAIN_TYPE::TREE:
			break;

		case TERRAIN_TYPE::MAX:
			break;

		default:
			break;
		}
	}
}
// ブロック単位で当たり判定を設置できる
void TERRAIN::PixelObjects(const std::vector<std::vector<std::vector<std::string>>> terrain, TERRAIN_TYPE type, XMFLOAT3 motherPosition)
{
	// 当たり判定を追加する
	CreateHit(InitializeObject(ConvertTerrain(terrain), type), motherPosition);
}
// 直方体や立方体など、大まかな範囲で当たり判定を設置できる
void TERRAIN::SimpleObjects(const std::vector<std::vector<std::vector<std::string>>> terrain, XMFLOAT3 size, TERRAIN_TYPE type, XMFLOAT3 motherPosition)
{
	auto conTerrain = ConvertTerrain(terrain);
	auto countType = CountObjectType(conTerrain);
	// 当たり判定を追加する
	CreateHit(InitializeObject(conTerrain, VolumeObject(conTerrain, countType, size), countType, type), motherPosition);
}