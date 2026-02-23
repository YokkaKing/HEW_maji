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
#define TERRAIN_SIZE (0.5f)
#define ANTLION_POS_Y (2.7f)
#define ANTLION_POS_Y_ (-1.9f)
#define BOG_POS_Y (-0.3f)
#define TREE_POS_Y (1.0f)

//================================================================
//	インクルード
//================================================================
#include"terrain.h"
#include"keyboard.h"
#include"colliderFactory.h"
#include"managerCollider.h"
#include"debug_ostream.h"
#include"shader.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"Transform.h"
#include<string>

//================================================================
//	グローバル変数
//================================================================
// 地形オブジェクト
TERRAIN g_Terrain;

static ID3D11Device* g_pDevice;
static ID3D11DeviceContext* g_pContext;

MODEL* blockModel = nullptr;
MODEL* slopeModel = nullptr;
XMFLOAT3 g_pos[2];

int Trand = 0;

// 蟻地獄のためのデータ(endPositionのみ)他は共通のため
XMFLOAT3 g_antlionData[4] =
{
	{ 0.0f, 0.5f, 0.4f },
	{ 0.4f, 0.5f, 0.0f },
	{ 0.0f, 0.5f, -0.4f },
	{ -0.4f, 0.5f, 0.0f }
};
XMFLOAT3 g_antlionBoxData[4] =
{
	{ 9.5f, 1.0f, 1.3f },
	{ 1.3f, 1.0f, 9.5f },
	{ 9.5f, 1.0f, 1.3f },
	{ 1.3f, 1.0f, 9.5f }
};
XMFLOAT2 g_antlionData2[4] =
{
	{ 0.0f, 0.5f },
	{ 0.5f, 0.0f },
	{ 0.0f, -0.5f },
	{ -0.5f, 0.0f }
};
XMFLOAT2 g_antlionBoxData2[4] =
{
	{ 0.0f, +4.2f },
	{ +4.2f, 0.0f },
	{ 0.0f, -4.2f },
	{ -4.2f, -0.0f }
};
// 蟻地獄のためのデータ(endPositionのみ)他は共通のため
XMFLOAT3 g_antlionData3[4] =
{
	{ 0.0f, 0.5f, 0.4f },
	{ 0.4f, 0.5f, 0.0f },
	{ 0.0f, 0.5f, -0.4f },
	{ -0.4f, 0.5f, 0.0f }
};
XMFLOAT2 g_antlionData4[4] =
{
	{ 0.0f, -7.3f },
	{ -7.3f, 0.0f },
	{ 0.0f, 7.3f },
	{ 7.3f, 0.0f }
};

std::string otherModel[2][3] =
{
	/*{"asset\\model\\tree.fbx", "asset\\model\\rock.fbx", "asset\\model\\water.fbx" },
	{"asset\\model\\tree.fbx", "asset\\model\\tree.fbx", "asset\\model\\lava.fbx" }*/
	{ "asset\\model\\tree.fbx", "asset\\model\\rock.fbx", "asset\\model\\water.fbx" },
	{ "asset\\model\\rock.fbx", "asset\\model\\rock.fbx", "asset\\model\\lava.fbx" }
};

int otherM[2][6] =
{
	{ 0, 0, 1, 1, 1, 2 },
	{ 0, 0, 0, 0, 2, 2 }
};

XMFLOAT3 g_otherPos[2][6] =
{
	{
		{ -5.0f, 0.5f, -2.0f },
		{ -1.0f, 0.5f, -4.0f },
		{ 5.0f, 0.5f, -2.0f },
		{ 3.0f, 0.5f, 1.0f },
		{ 6.0f, 0.5f, 7.0f },
		{ 0.0f, 0.5f, 0.0f },
	},
	{
		{ 1.0f, 0.5f, 7.0f },
		{ 2.0f, 0.5f, -5.0f },
		{ -1.0f, 0.5f, -3.0f },
		{ -2.0f, 0.5f, 3.0f },
		{ 5.0f, 0.5f, -2.0f },
		{ 2.0f, 0.5f, 3.0f },
	}
};

XMFLOAT3 g_otherScale[2][6] =
{
	{
		{ 0.5f, 2.0f, 0.5f },
		{ 0.5f, 2.0f, 0.5f },
		{ 1.4f, 2.0f, 1.4f },
		{ 1.4f, 2.0f, 1.4f },
		{ 1.4f, 2.0f, 1.4f },
		{ 4.0f, 0.5f, 4.0f },
	},
	{
		{ 1.4f, 2.0f, 1.4f },
		{ 1.4f, 2.0f, 1.4f },
		{ 1.4f, 2.0f, 1.4f },
		{ 1.4f, 2.0f, 1.4f },
		{ 4.0f, 0.5f, 4.0f },
		{ 4.0f, 0.5f, 4.0f },
	}
};

XMFLOAT3 g_otherModelScale[2][6] =
{
	{
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
	},
	{
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f },
	}
};

XMFLOAT3 g_otherOffset[2][6] =
{
	{
		{ 0.0f, 0.5f, 0.0f },
		{ 0.0f, 0.5f, 0.0f },
		{ -0.2f, 0.0f, -0.2f },
		{ -0.2f, 0.0f, -0.2f },
		{ -0.2f, 0.0f, -0.2f },
		{ 0.0f, -0.5f, 0.0f },
	},
	{
		{ -0.2f, 0.0f, -0.2f },
		{ -0.2f, 0.0f, -0.2f },
		{ -0.2f, 0.0f, -0.2f },
		{ -0.2f, 0.0f, -0.2f },
		{ 0.0f, -0.5f, 0.0f },
		{ 0.0f, -0.5f, 0.0f },
	}
};

std::string g_otherTag[2][6] =
{
	{
		"WALL",
		"WALL",
		"WALL",
		"WALL",
		"WALL",
		"WATER"
	},
	{
		"WALL",
		"WALL",
		"WALL",
		"WALL",
		"LAVA",
		"LAVA"
	}
};

//================================================================
//	一文字0.25立法メートルとする
//	n -> 何もなし
//	a -> 当たり判定
//================================================================
// 地形::丘 の座標データ
//const std::vector<std::vector<std::vector<std::string>>> Hill =
//{
//	{ // Y = 0 // Z->+ // X↓+
//		{"anananananan"},
//		{"nnnnnnnnnnna"},
//		{"annnnnnnnnnn"},
//		{"nnnnnnnnnnna"},
//		{"annnnnnnnnnn"},
//		{"nnnnnnnnnnna"},
//		{"annnnnnnnnnn"},
//		{"nnnnnnnnnnna"},
//		{"annnnnnnnnnn"},
//		{"nnnnnnnnnnna"},
//		{"annnnnnnnnnn"},
//		{"nananananana"},
//	},
//	{ // Y = 0.25
//		{"nnnnnnnnnnnn"},
//		{"nananananann"},
//		{"nnnnnnnnnnan"},
//		{"nannnnnnnnnn"},
//		{"nnnnnnnnnnan"},
//		{"nannnnnnnnnn"},
//		{"nnnnnnnnnnan"},
//		{"nannnnnnnnnn"},
//		{"nnnnnnnnnnan"},
//		{"nannnnnnnnnn"},
//		{"nnananananan"},
//		{"nnnnnnnnnnnn"},
//	},
//	{ // Y = 0.5
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnanananannn"},
//		{"nnnnnnnnnann"},
//		{"nnannnnnnnnn"},
//		{"nnnnnnnnnann"},
//		{"nnannnnnnnnn"},
//		{"nnnnnnnnnann"},
//		{"nnannnnnnnnn"},
//		{"nnnanananann"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//	},
//	{ // Y = 0.75
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnananannnn"},
//		{"nnnnnnnnannn"},
//		{"nnnannnnnnnn"},
//		{"nnnnnnnnannn"},
//		{"nnnannnnnnnn"},
//		{"nnnnananannn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//	},
//	{ // Y = 0.75
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnanannnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnanannnn"},
//		{"nnnnnnannnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//	},
//	{ // Y = 1
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnannnnn"},
//		{"nnnnnnannnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//		{"nnnnnnnnnnnn"},
//	},
//};

//================================================================
//	同じ文字は絶対に4文字入力すること
//	各文字を頂点とし、それぞれを結んだ四角形を当たり判定とする
//	n -> 何もなし
//	文字は分かりやすいやつでa,b,cとか
//================================================================

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

// 沼の当たり判定
const std::vector<std::vector<std::vector<std::string>>> Bogs =
{
	{
		{"annnnnnnna"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"nnnnnnnnnn"},
		{"annnnnnnna"},
	},
};

// 木の当たり判定
const std::vector<std::vector<std::vector<std::string>>> Trees =
{
	{
		{"nnnaaaaaaannn"},
		{"nnannnnnnnann"},
		{"nannnnnnnnnan"},
		{"annnnnnnnnnna"},
		{"annnnnnnnnnna"},
		{"annnnnnnnnnna"},
		{"annnnnnnnnnna"},
		{"annnnnnnnnnna"},
		{"annnnnnnnnnna"},
		{"annnnnnnnnnna"},
		{"nannnnnnnnnan"},
		{"nnannnnnnnann"},
		{"nnnaaaaaaannn"},
	}
};

void TerrainInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain p1Set, WeaponTerrain p2Set)
{
	Trand = rand() % 2; // 乱数

	//============================================
	//	新しい引数のp1,2Setはプレイヤーが選択した 
	//	武器と地形に応じて条件式でモデルをロードする
	//	ためのデータ保持用引数。
	//============================================

	g_pDevice = pDevice;
	g_pContext = pContext;

	g_Terrain.m_moveTerrain[0] = ModelLoad("asset\\model\\hill.fbx");
	g_Terrain.m_moveTerrain[1] = ModelLoad("asset\\model\\wall.fbx");
	g_Terrain.m_moveTerrain[2] = ModelLoad("asset\\model\\antlion.fbx");
	g_Terrain.m_moveTerrain[3] = ModelLoad("asset\\model\\numa.fbx");
	g_Terrain.m_moveTerrain[4] = ModelLoad("asset\\model\\trees.fbx");
	blockModel = ModelLoad("asset\\model\\block.fbx");
	slopeModel = ModelLoad("asset\\model\\block3.fbx");

	g_Terrain.m_motherPosition[0] = GetPlayerPosition();
	g_Terrain.m_motherPosition[1] = GetPlayer2Position();
	g_Terrain.m_terrainScale[0] = { 5.0f, 5.0f, 5.0f };
	g_Terrain.m_terrainScale[1] = { 5.0f, 6.5f, 5.0f };
	g_Terrain.m_terrainScale[2] = { 5.0f, 5.5f, 5.0f };
	g_Terrain.m_terrainScale[3] = { 5.0f, 5.0f, 5.0f };
	g_Terrain.m_terrainScale[4] = { 5.0f, 5.0f, 5.0f };
	g_Terrain.m_terrainRotation[0] = { 0.0f,0.0f,0.0f };
	g_Terrain.m_terrainRotation[1] = { 0.0f,0.0f,0.0f };
	g_Terrain.m_terrainRotation[2] = { 0.0f,0.0f,0.0f };
	g_Terrain.m_terrainRotation[3] = { 0.0f,0.0f,0.0f };
	g_Terrain.m_terrainRotation[4] = { 0.0f,0.0f,0.0f };

	g_Terrain.m_terrainScaling[0] = { 5.0f, 5.0f, 5.0f };
	g_Terrain.m_terrainScaling[1] = { 10.0f, 6.5f, 10.0f };
	g_Terrain.m_terrainScaling[2] = { 10.0f, 8.0f, 10.0f };
	g_Terrain.m_terrainScaling[3] = { 10.0f, 5.0f, 10.0f };
	g_Terrain.m_terrainScaling[4] = { 7.0f, 7.0f, 7.0f };

	g_Terrain.m_motherPosition[0].y -= 5.0f;
	g_Terrain.m_motherPosition[1].y -= 5.0f;

	g_Terrain.m_coolTime[0] = 0.0f;
	g_Terrain.m_coolTime[1] = 0.0f;

	g_pos[0] = {};
	g_pos[1] = {};

	//g_Terrain.PixelObjects(Hill, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition);

	// 丘の当たり判定
	//g_Terrain.SimpleObjects(Hills, { 0.25f, 0.25f, 0.25f }, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition[0]);
	//g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::WALL, g_Terrain.m_motherPosition[1]);
	//hal::dout << "座標 : (" << g_Terrain.slopes[0]->m_position.x << "," << g_Terrain.slopes[0]->m_position.y << "," << g_Terrain.slopes[0]->m_position.z << ")\n";

	XMFLOAT3 initPosWall = g_Terrain.m_motherPosition[1];
	XMFLOAT3 initPosHill = g_Terrain.m_motherPosition[0];

	for (int i = 0; i < 6; i++)
	{
		g_Terrain.m_otherScale[i] = g_otherScale[Trand][i];
		g_Terrain.m_otherModelScale[i] = g_otherModelScale[Trand][i];
		g_Terrain.m_otherPosition[i] = g_otherPos[Trand][i];
	}
	for (int i = 0; i < 3; i++)
	{
		g_Terrain.m_otherModel[i] = ModelLoad(otherModel[Trand][i].c_str());
	}

	g_Terrain.other.resize(6);

	for (int i = 0; i < 6; i++)
	{
		g_Terrain.other[i] = ColliderFactory::CreateBoxObject(
			g_Terrain.m_otherPosition[i],
			g_Terrain.m_otherScale[i],
			g_otherTag[Trand][i],
			0
		);
		g_Terrain.other[i]->m_isStatic = true;
	}

	//======================================================
	//	各プレイヤーに固定された座標で地形をセットしている現状
	//	鈴木ができるのはここまでです。
	//	あとは地形システムを作った久保木に任せます。
	//	下のスイッチ文はそれっぽいので活用してもいいよ
	//======================================================

	//選択された武器・地形データに応じて地形を生成
	switch (p1Set)
	{
	case WeaponTerrain::SWORD_WALL:
		//g_Terrain.SimpleObjects(Trees, { 0.25f, 2.0f, 0.25f }, TERRAIN_TYPE::TREE, g_Terrain.m_motherPosition[1], 0); // 相手を中心に生えるから
		g_Terrain.PixelObjects(Trees, TERRAIN_TYPE::TREE, g_Terrain.m_motherPosition[1], 0);
		break;
	case WeaponTerrain::SPEAR_HILL:
		//g_Terrain.SimpleObjects(Hills, { 0.25f, 0.25f, 0.25f }, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition[0], 0);
		g_Terrain.CreateAnt(g_Terrain.m_motherPosition[0], 0);
		break;
	case WeaponTerrain::BOW_HILL:
		g_Terrain.SimpleObjects(Bogs, { 1.0f, 2.5f, 1.0f }, TERRAIN_TYPE::BOG, g_Terrain.m_motherPosition[0], 0);
		break;
	case WeaponTerrain::HAMMER_:
		g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition[1], 1);
		break;
	case WeaponTerrain::SHURIKEN_:
		g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::WALL, g_Terrain.m_motherPosition[0], 0);
		break;

	default:
		break;
	}
	//プレイヤー2
	switch (p2Set)
	{
	case WeaponTerrain::SWORD_WALL:
		//g_Terrain.SimpleObjects(Trees, { 0.25f, 2.0f, 0.25f }, TERRAIN_TYPE::TREE, g_Terrain.m_motherPosition[0], 1); // 相手を中心に生えるから
		g_Terrain.PixelObjects(Trees, TERRAIN_TYPE::TREE, g_Terrain.m_motherPosition[0], 1);
		break;
	case WeaponTerrain::SPEAR_HILL:
		//g_Terrain.SimpleObjects(Hills, { 0.25f, 0.25f, 0.25f }, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition[1], 1);
		g_Terrain.CreateAnt(g_Terrain.m_motherPosition[1], 1);
		break;
	case WeaponTerrain::BOW_HILL:
		g_Terrain.SimpleObjects(Bogs, { 1.0f, 2.5f, 1.0f }, TERRAIN_TYPE::BOG, g_Terrain.m_motherPosition[1], 1);
		//g_Terrain.PixelObjects(Hills, TERRAIN_TYPE::HILL, initPosHill);
		break;
	case WeaponTerrain::HAMMER_:
		g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::HILL, g_Terrain.m_motherPosition[1], 1);
		break;
	case WeaponTerrain::SHURIKEN_:
		g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::WALL, g_Terrain.m_motherPosition[1], 1);
		break;
	default:
		break;
	}
}
void TerrainFinalize()
{
	//ModelRelease(g_Terrain.m_model);
	ModelRelease(blockModel);

	for (int i = 0; i < 4; i++)
	{
		ModelRelease(g_Terrain.m_moveTerrain[i]);
	}

	for (int i = 0; i < CHANGE_FLAG; i++)
	{
		g_Terrain.m_isChange[i] = false;
	}

	g_Terrain.terrainObjects.clear();

	for (int i = 0; i < 2; i++)
	{
		g_Terrain.hills[i].clear();
		g_Terrain.walls[i].clear();
		g_Terrain.trees[i].clear();
		g_Terrain.ants[i].clear();
		g_Terrain.bogs[i].clear();
	}

	g_Terrain.other.clear();

	for (int i = 0; i < 6; i++)
	{
		g_Terrain.m_otherScale[i] = {};
		g_Terrain.m_otherModelScale[i] = {};
		g_Terrain.m_otherPosition[i] = {};
	}
	for (int i = 0; i < 3; i++)
	{
		ModelRelease(g_Terrain.m_otherModel[i]);
	}
}
void TerrainUpdate()
{
	if (GetPlayer_IsTransformed())
	{
		TerrainSet(GetSetWTP1(), FALSE);
		// クールタイムがあれば発動できない
		if (g_Terrain.m_coolTime[0] <= 0)
		{
			g_Terrain.m_isChange[0] = true; // 1Pの変身を確認
			g_Terrain.m_coolTime[0] = 20.0f;
			g_pos[0] = g_Terrain.m_motherPosition[1];
		}
	}

	if (GetPlayer2_IsTransformed())
	{
		TerrainSet(GetSetWTP2(), TRUE);
		// クールタイムがあれば発動できない
		if (g_Terrain.m_coolTime[1] <= 0)
		{
			g_Terrain.m_isChange[1] = true; // 2Pの変身を確認
			g_Terrain.m_coolTime[1] = 20.0f;
			g_pos[1] = g_Terrain.m_motherPosition[0];
		}
	}

	//if (Keyboard_IsKeyDown(KK_L))
	//{
	//	g_Terrain.m_isChange[0] = true; // 2Pの変身を確認
	//	g_Terrain.m_coolTime[0] = 20.0f;
	//}

	// 変身したら-する
	if (g_Terrain.m_isChange[0])
	{
		// クールタイムあったら-する
		if (g_Terrain.m_coolTime[0] > 0.0f)
		{
			g_Terrain.m_coolTime[0] -= 1.0f / 60.0f;
		}
		else // 無かったら
		{
			g_Terrain.m_coolTime[0] = 0.0f;	// クールタイムをなくす
			g_Terrain.m_isChange[0] = false; // 変身を解く
			g_pos[0] = {};
		}
	}
	// 変身したら-する
	if (g_Terrain.m_isChange[1])
	{
		// クールタイムあったら-する
		if (g_Terrain.m_coolTime[1] > 0.0f)
		{
			g_Terrain.m_coolTime[1] -= 1.0f / 60.0f;
		}
		else // 無かったら
		{
			g_Terrain.m_coolTime[1] = 0.0f;	// クールタイムをなくす
			g_Terrain.m_isChange[1] = false; // 変身を解く
			g_pos[1] = {};
		}
	}

	// 変身してなければ追従P1
	if (!g_Terrain.m_isChange[0])
	{
		g_Terrain.m_motherPosition[0] = GetPlayerPosition();
		g_Terrain.m_motherPosition[0].y = -5.0f; // 下に設定
		switch (GetSetWTP1())
		{
		case WeaponTerrain::SWORD_WALL:
			g_Terrain.UpdateObject(g_Terrain.trees[0], g_Terrain.m_motherPosition[1], FALSE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::SPEAR_HILL:
			g_Terrain.UpdateObject(g_Terrain.ants[0], g_Terrain.m_motherPosition[0], FALSE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::BOW_HILL:
			g_Terrain.UpdateObject(g_Terrain.bogs[0], g_Terrain.m_motherPosition[0], FALSE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::HAMMER_:
			g_Terrain.UpdateObject(g_Terrain.walls[0], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::SHURIKEN_:
			g_Terrain.UpdateObject(g_Terrain.walls[0], g_Terrain.m_motherPosition[0], FALSE); // P1の地形の当たり判定
			break;
		}
	}
	else
	{
		float posY = 0.0f;

		switch (GetSetWTP1())
		{
		case WeaponTerrain::SWORD_WALL:
			posY = TREE_POS_Y;
			break;
		case WeaponTerrain::SPEAR_HILL:
			posY = ANTLION_POS_Y;
			break;
		case WeaponTerrain::BOW_HILL:
			posY = BOG_POS_Y;
			break;
		case WeaponTerrain::HAMMER_:
			posY = 0.7f;
			break;
		case WeaponTerrain::SHURIKEN_:
			posY = 0.7f;
			break;
		default:
			break;
		}

		// 変身したら上昇
		if (g_Terrain.m_motherPosition[0].y < posY)
		{
			g_Terrain.m_motherPosition[0].y += 0.1f;
			switch (GetSetWTP1())
			{
			case WeaponTerrain::SWORD_WALL:
				g_Terrain.UpdateObject(g_Terrain.trees[0], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::SPEAR_HILL:
				g_Terrain.UpdateObject(g_Terrain.ants[0], g_Terrain.m_motherPosition[0], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::BOW_HILL:
				g_Terrain.UpdateObject(g_Terrain.bogs[0], g_Terrain.m_motherPosition[0], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::HAMMER_:
				g_Terrain.UpdateObject(g_Terrain.walls[0], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::SHURIKEN_:
				g_Terrain.UpdateObject(g_Terrain.walls[0], g_Terrain.m_motherPosition[0], TRUE); // P1の地形の当たり判定
				break;
			}
		}
	}

	// 変身してなければ追従P2
	if (!g_Terrain.m_isChange[1])
	{
		g_Terrain.m_motherPosition[1] = GetPlayer2Position();
		g_Terrain.m_motherPosition[1].y = -5.0f; // 下に設定
		switch (GetSetWTP2())
		{
		case WeaponTerrain::SWORD_WALL:
			g_Terrain.UpdateObject(g_Terrain.trees[1], g_Terrain.m_motherPosition[0], FALSE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::SPEAR_HILL:
			g_Terrain.UpdateObject(g_Terrain.ants[1], g_Terrain.m_motherPosition[1], FALSE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::BOW_HILL:
			g_Terrain.UpdateObject(g_Terrain.bogs[1], g_Terrain.m_motherPosition[1], FALSE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::HAMMER_:
			g_Terrain.UpdateObject(g_Terrain.walls[1], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
			break;
		case WeaponTerrain::SHURIKEN_:
			g_Terrain.UpdateObject(g_Terrain.walls[1], g_Terrain.m_motherPosition[1], FALSE); // P1の地形の当たり判定
			break;
		}
	}
	else
	{
		float posY = 0.0f;

		switch (GetSetWTP2())
		{
		case WeaponTerrain::SWORD_WALL:
			posY = TREE_POS_Y;
			break;
		case WeaponTerrain::SPEAR_HILL:
			posY = ANTLION_POS_Y;
			break;
		case WeaponTerrain::BOW_HILL:
			posY = BOG_POS_Y;
			break;
		case WeaponTerrain::HAMMER_:
			posY = 0.7f;
			break;
		case WeaponTerrain::SHURIKEN_:
			posY = 0.7f;
			break;
		default:
			break;
		}

		// 変身したら上昇
		if (g_Terrain.m_motherPosition[1].y < posY)
		{
			g_Terrain.m_motherPosition[1].y += 0.1f;
			switch (GetSetWTP2())
			{
			case WeaponTerrain::SWORD_WALL:
				g_Terrain.UpdateObject(g_Terrain.trees[1], g_Terrain.m_motherPosition[0], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::SPEAR_HILL:
				g_Terrain.UpdateObject(g_Terrain.ants[1], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::BOW_HILL:
				g_Terrain.UpdateObject(g_Terrain.bogs[1], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::HAMMER_:
				g_Terrain.UpdateObject(g_Terrain.walls[1], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
				break;
			case WeaponTerrain::SHURIKEN_:
				g_Terrain.UpdateObject(g_Terrain.walls[1], g_Terrain.m_motherPosition[1], TRUE); // P1の地形の当たり判定
				break;
			}
		}
	}
}
void TerrainDraw()
{
	for (int i = 0; i < 6; i++)
	{
		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.m_otherModelScale[i].x,
			g_Terrain.m_otherModelScale[i].y,
			g_Terrain.m_otherModelScale[i].z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			0.0f,
			0.0f,
			0.0f);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.m_otherPosition[i].x + g_otherOffset[Trand][i].x,
			g_Terrain.m_otherPosition[i].y + g_otherOffset[Trand][i].y,
			g_Terrain.m_otherPosition[i].z + g_otherOffset[Trand][i].z);
		XMMATRIX	world = scale * rotation * translation;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);

		g_Terrain.other[i]->m_position = g_Terrain.m_otherPosition[i];

		ModelDraw(g_Terrain.m_otherModel[otherM[Trand][i]]);
	}

	// 1Pが変身してないときは描画しない
	if (g_Terrain.m_isChange[0])
	{
		int no = 0;

		switch (GetSetWTP1())
		{
		case WeaponTerrain::SWORD_WALL:
			no = 4;
			break;
		case WeaponTerrain::SPEAR_HILL:
			no = 2;
			break;
		case WeaponTerrain::BOW_HILL:
			no = 3;
			break;
		case WeaponTerrain::HAMMER_:
			no = 1;
			break;
		case WeaponTerrain::SHURIKEN_:
			no = 1;
			break;
		default:
			break;
		}

		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.m_terrainScaling[no].x,
			g_Terrain.m_terrainScaling[no].y,
			g_Terrain.m_terrainScaling[no].z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			g_Terrain.m_terrainRotation[0].x,
			g_Terrain.m_terrainRotation[0].y,
			g_Terrain.m_terrainRotation[0].z);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.m_motherPosition[0].x,
			g_Terrain.m_motherPosition[0].y,
			g_Terrain.m_motherPosition[0].z);
		if (no == 4)
		{

			translation = XMMatrixTranslation(
				g_pos[0].x,
				g_Terrain.m_motherPosition[0].y,
				g_pos[0].z);
		}
		XMMATRIX	world = scale * rotation * translation;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);

		switch (GetSetWTP1())
		{
		case WeaponTerrain::SWORD_WALL:
			ModelDraw(g_Terrain.m_moveTerrain[4]);
			break;
		case WeaponTerrain::SPEAR_HILL:
			ModelDraw(g_Terrain.m_moveTerrain[2]);
			break;
		case WeaponTerrain::BOW_HILL:
			ModelDraw(g_Terrain.m_moveTerrain[3]);
			break;
		case WeaponTerrain::HAMMER_:
			ModelDraw(g_Terrain.m_moveTerrain[1]);
			break;
		case WeaponTerrain::SHURIKEN_:
			ModelDraw(g_Terrain.m_moveTerrain[1]);
			break;
		default:
			break;
		}
	}
	// 2Pが変身してないときは描画しない
	if (g_Terrain.m_isChange[1])
	{
		int no = 0;

		switch (GetSetWTP2())
		{
		case WeaponTerrain::SWORD_WALL:
			no = 4;
			break;
		case WeaponTerrain::SPEAR_HILL:
			no = 2;
			break;
		case WeaponTerrain::BOW_HILL:
			no = 3;
			break;
		case WeaponTerrain::HAMMER_:
			no = 1;
			break;
		case WeaponTerrain::SHURIKEN_:
			no = 1;
			break;
		default:
			break;
		}

		//ワールド行列作成
		XMMATRIX	scale = XMMatrixScaling(
			g_Terrain.m_terrainScaling[no].x,
			g_Terrain.m_terrainScaling[no].y,
			g_Terrain.m_terrainScaling[no].z);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			g_Terrain.m_terrainRotation[1].x,
			g_Terrain.m_terrainRotation[1].y,
			g_Terrain.m_terrainRotation[1].z);
		XMMATRIX	translation = XMMatrixTranslation(
			g_Terrain.m_motherPosition[1].x,
			g_Terrain.m_motherPosition[1].y,
			g_Terrain.m_motherPosition[1].z);
		if (no == 4)
		{
			translation = XMMatrixTranslation(
				g_pos[1].x,
				g_Terrain.m_motherPosition[1].y,
				g_pos[1].z);
		}
		XMMATRIX	world = scale * rotation * translation;

		//シェーダーへ行列をセット
		Shader_SetWorldMatrix(world);

		switch (GetSetWTP2())
		{
		case WeaponTerrain::SWORD_WALL:
			ModelDraw(g_Terrain.m_moveTerrain[4]);
			break;
		case WeaponTerrain::SPEAR_HILL:
			ModelDraw(g_Terrain.m_moveTerrain[2]);
			break;
		case WeaponTerrain::BOW_HILL:
			ModelDraw(g_Terrain.m_moveTerrain[3]);
			break;
		case WeaponTerrain::HAMMER_:
			ModelDraw(g_Terrain.m_moveTerrain[1]);
			break;
		case WeaponTerrain::SHURIKEN_:
			ModelDraw(g_Terrain.m_moveTerrain[1]);
			break;
		default:
			break;
		}
	}
}
void TerrainSet(WeaponTerrain set, bool playerSelect)
{
	// --- 追加：重複呼び出し防止用のフラグ ---
	static WeaponTerrain lastSetP1 = (WeaponTerrain)-1;
	static WeaponTerrain lastSetP2 = (WeaponTerrain)-1;
	static bool firstCallP1 = true;
	static bool firstCallP2 = true;

	if (!playerSelect) { // P1の場合
		if (!firstCallP1 && lastSetP1 == set) return; // 変化がなければ何もしない
		lastSetP1 = set;
		firstCallP1 = false;
	}
	else { // P2の場合
		if (!firstCallP2 && lastSetP2 == set) return; // 変化がなければ何もしない
		lastSetP2 = set;
		firstCallP2 = false;
	}
	// ----------------------------------------

	int select = 0;
	if (playerSelect)
	{
		select = 1;
	}

	switch (select)
	{
	case 0:
		g_Terrain.ClearPlayerObjects(GetSetWTP1(), select);
		break;
	case 1:
		g_Terrain.ClearPlayerObjects(GetSetWTP2(), select);
		break;
	}

	int swordSelect = 0;
	if (select == 0)
	{
		swordSelect = 1;
	}

	switch (set)
	{
	case WeaponTerrain::SWORD_WALL:
		//g_Terrain.SimpleObjects(Trees, { 0.25f, 2.0f, 0.25f }, TERRAIN_TYPE::TREE, g_Terrain.m_motherPosition[swordSelect], select);
		g_Terrain.PixelObjects(Trees, TERRAIN_TYPE::TREE, g_Terrain.m_motherPosition[swordSelect], select);
		break;
	case WeaponTerrain::SPEAR_HILL:
		g_Terrain.CreateAnt(g_Terrain.m_motherPosition[select], select);
		break;
	case WeaponTerrain::BOW_HILL:
		g_Terrain.SimpleObjects(Bogs, { 1.0f, 2.5f, 1.0f }, TERRAIN_TYPE::BOG, g_Terrain.m_motherPosition[select], select);
		break;
	case WeaponTerrain::HAMMER_:
		g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::WALL, g_Terrain.m_motherPosition[select], select);
		break;
	case WeaponTerrain::SHURIKEN_:
		g_Terrain.SimpleObjects(Walls, { 1.0f, 1.0f, 1.0f }, TERRAIN_TYPE::WALL, g_Terrain.m_motherPosition[select], select);
		break;
	default:
		break;
	}
}
// プレイヤーに追従していた地形を解放する
void TERRAIN::ClearPlayerObjects(WeaponTerrain set, int select)
{
	// 範囲外ならスキップ
	if (select < 0 || select >= 2)
	{
		return;
	}

	std::vector<GameObject*>* targetList = nullptr;

	switch (set)
	{
	case WeaponTerrain::SWORD_WALL:
		targetList = &trees[select];
		break;
	case WeaponTerrain::SPEAR_HILL:
		targetList = &ants[select];
		break;
	case WeaponTerrain::BOW_HILL:
		targetList = &bogs[select];
		break;
	case WeaponTerrain::HAMMER_:
		targetList = &walls[select];
		break;
	case WeaponTerrain::SHURIKEN_:
		targetList = &walls[select];
		break;
	default:
		break;
	}

	// リリースするものがなければスキップ
	if (targetList == nullptr || targetList->empty()) return;

	for (auto* rawPtr : *targetList)
	{
		if (rawPtr)
		{
			if (rawPtr == nullptr) continue;

			// GameObjectの死亡フラグを立てる
			rawPtr->m_isDead = true;

			// Colliderコンポーネントを取得して無効化する
			// gameObject.h にある GetColliders テンプレート関数を利用
			auto colliders = rawPtr->GetColliders();
			for (auto& col : colliders)
			{
				if (col)
				{
					col->SetEnable(false);
					ManagerCollider::RemoveCollider(col);
				}
			}
		}

		// 実体（unique_ptr）のリストから削除してメモリを解放
		auto it = std::remove_if(terrainObjects.begin(), terrainObjects.end(),
			[rawPtr](const std::unique_ptr<GameObject>& obj) {
				return obj.get() == rawPtr;
			});

		if (it != terrainObjects.end()) {
			terrainObjects.erase(it, terrainObjects.end());
		}
	}

	// 管理リストをクリア
	targetList->clear();
}
// 簡単な四角形の当たり判定を作る場合
void TERRAIN::SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay, int select)
{
	// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
	std::unique_ptr<GameObject> obj_owner(
		ColliderFactory::CreateBoxObject(pos, scl, tag, lay)
	);

	GameObject* raw_ptr = obj_owner.get(); // 生のポインタを取得（参照用）

	if (raw_ptr != nullptr)
	{
		if (raw_ptr->m_tag == "HILL") hills[select].push_back(raw_ptr);
		if (raw_ptr->m_tag == "WALL") walls[select].push_back(raw_ptr);
		if (raw_ptr->m_tag == "TREEP1") trees[0].push_back(raw_ptr);
		if (raw_ptr->m_tag == "TREEP2") trees[1].push_back(raw_ptr);
		if (raw_ptr->m_tag == "BOGP1") bogs[0].push_back(raw_ptr);
		if (raw_ptr->m_tag == "BOGP2") bogs[1].push_back(raw_ptr);

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
		// 変身前：プレイヤーに追従させる（ここを同期させる）
		for (auto& obj : terrain)
		{
			// velocityを「初期の相対距離」として保持しているなら、これでOK
			obj->m_position.x = motherPosition.x + obj->m_velocity.x;
			obj->m_position.y = motherPosition.y + obj->m_velocity.y;
			obj->m_position.z = motherPosition.z + obj->m_velocity.z;
		}
	}
	else
	{
		// 変身後：キー入力と同じように「増分」だけを足す
		for (auto& obj : terrain)
		{
			if (obj->m_tag == "SlopeP1" || obj->m_tag == "SlopeP2")
			{
				if (obj->m_position.y < ANTLION_POS_Y_)
				{
					obj->m_position.y += 0.1f; // 上昇
				}
				
				if (obj->m_position.y >= ANTLION_POS_Y_)
				{
					obj->m_position.y = ANTLION_POS_Y_;
				}
			}
			else
			{
				obj->m_position.y += 0.1f; // 上昇
			}
		}
	}
}
// 当たり判定を作る
void TERRAIN::CreateHit(std::vector<TERRAIN_OBJECT> terrain, XMFLOAT3 motherPosition, int select)
{
	XMFLOAT3 pos;

	std::string b_tag;
	std::string t_tag;
	b_tag = "BOGP1";
	t_tag = "TREEP1";
	if (select == 1)
	{
		b_tag = "BOGP2";
		t_tag = "TREEP2";
	}

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
			SetObject(pos, terrain[i].m_size, "HILL", 0, select);
			hills[select][i]->m_position = pos;	// 座標を格納
			hills[select][i]->m_velocity = terrain[i].m_distance;
			hills[select][i]->m_scale = terrain[i].m_size;
			break;

		case TERRAIN_TYPE::WALL:
			SetObject(pos, terrain[i].m_size, "WALL", 0, select);
			walls[select][i]->m_position = pos;	// 座標を格納
			walls[select][i]->m_velocity = terrain[i].m_distance;
			walls[select][i]->m_scale = terrain[i].m_size;
			break;

		case TERRAIN_TYPE::TREE:
			SetObject(pos, terrain[i].m_size, t_tag, 0, select);
			trees[select][i]->m_position = pos;	// 座標を格納
			trees[select][i]->m_velocity = terrain[i].m_distance;
			trees[select][i]->m_scale = terrain[i].m_size;
			break;

		case TERRAIN_TYPE::BOG:
			SetObject(pos, terrain[i].m_size, b_tag, 0, select);
			bogs[select][i]->m_position = pos;	// 座標を格納
			bogs[select][i]->m_velocity = terrain[i].m_distance;
			bogs[select][i]->m_scale = terrain[i].m_size;
			break;

		case TERRAIN_TYPE::MAX:
			break;

		default:
			break;
		}
	}
}
// ブロック単位で当たり判定を設置できる
void TERRAIN::PixelObjects(const std::vector<std::vector<std::vector<std::string>>> terrain, TERRAIN_TYPE type, XMFLOAT3 motherPosition, int select)
{
	// 当たり判定を追加する
	CreateHit(InitializeObject(ConvertTerrain(terrain), type), motherPosition, select);
}
// 直方体や立方体など、大まかな範囲で当たり判定を設置できる
void TERRAIN::SimpleObjects(const std::vector<std::vector<std::vector<std::string>>> terrain, XMFLOAT3 size, TERRAIN_TYPE type, XMFLOAT3 motherPosition, int select)
{
	auto conTerrain = ConvertTerrain(terrain);
	auto countType = CountObjectType(conTerrain);
	// 当たり判定を追加する
	CreateHit(InitializeObject(conTerrain, VolumeObject(conTerrain, countType, size), countType, type), motherPosition, select);
}

// 蟻地獄のオブジェクトを作る
void TERRAIN::CreateAnt(XMFLOAT3 motherPosition, int select)
{
	std::string tag = "SlopeP1";

	if (select)
	{
		tag = "SlopeP2"; // 専用のタグをつける
	}

	//// 四方分坂を作る
	//for (int i = 0; i < 4; i++)
	//{
	//	// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
	//	std::unique_ptr<GameObject> antObj(
	//		ColliderFactory::CreateTrapezoidSlopeObject(
	//			{ 0.0f, 0.0f, 0.0f },
	//			g_antlionData[i],
	//			7.5f,
	//			7.5f,
	//			0.4f,
	//			tag,
	//			0
	//		)
	//	);

	//	antObj->m_position = motherPosition;
	//	antObj->m_velocity.x = g_antlionData2[i].x; // dataの数値を代入
	//	antObj->m_velocity.z = g_antlionData2[i].y; // dataの数値を代入
	//	GameObject* raw_ptr = antObj.get(); // 生のポインタを取得（参照用）

	//	if (raw_ptr != nullptr)
	//	{
	//		ants[select].push_back(raw_ptr);
	//		terrainObjects.push_back(std::move(antObj));
	//	}
	//}

	// 四方分坂を作る(外から上る方)
	for (int i = 0; i < 4; i++)
	{
		// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
		std::unique_ptr<GameObject> antObj(
			ColliderFactory::CreateTrapezoidSlopeObject(
				{ 0.0f, 0.0f, 0.0f },
				g_antlionData3[i],
				10.0f,
				10.0f,
				0.4f,
				tag,
				0
			)
		);

		antObj->m_position = motherPosition;
		antObj->m_position.z += g_antlionData4[i].y;
		antObj->m_position.x += g_antlionData4[i].x;
		GameObject* raw_ptr = antObj.get(); // 生のポインタを取得（参照用）

		if (raw_ptr != nullptr)
		{
			ants[select].push_back(raw_ptr);
			terrainObjects.push_back(std::move(antObj));
		}
	}

	//// 四方分の壁を作る
	//for (int i = 0; i < 4; i++)
	//{
	//	// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
	//	std::unique_ptr<GameObject> antObj(
	//		ColliderFactory::CreateBoxObject(
	//			{ 0.0f, 0.0f, 0.0f },
	//			g_antlionBoxData[i],
	//			"WALL",
	//			0
	//		)
	//	);

	//	antObj->m_position = motherPosition;
	//	antObj->m_position.y -= 1.0f;
	//	antObj->m_velocity.y = -1.0f;

	//	antObj->m_position.x += g_antlionBoxData2[i].x;
	//	antObj->m_velocity.x = g_antlionBoxData2[i].x;
	//	antObj->m_position.z += g_antlionBoxData2[i].y;
	//	antObj->m_velocity.z = g_antlionBoxData2[i].y;

	//	//antObj->m_velocity.x = g_antlionData2[i].x; // dataの数値を代入
	//	//antObj->m_velocity.z = g_antlionData2[i].y; // dataの数値を代入
	//	GameObject* raw_ptr = antObj.get(); // 生のポインタを取得（参照用）

	//	if (raw_ptr != nullptr)
	//	{
	//		ants[select].push_back(raw_ptr);
	//		terrainObjects.push_back(std::move(antObj));
	//	}
	//}
}