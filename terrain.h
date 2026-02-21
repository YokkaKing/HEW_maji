/*
* ファイル名	terrain.h
* タイトル	地形
* 作成者		久保木幹太
* 作成日		11月25日
* 更新日		12月02日
*/

#ifndef TERRAIN_H
#define TERRAIN_H

//================================================================
//	マクロ定義
//================================================================
#define MOVE_TERRAIN_TYPE (5)
#define CHANGE_FLAG (2)
 
//================================================================
//	インクルード
//================================================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
using namespace DirectX;
#include"gameObject.h"
#include<string>
#include"selectWeaponTerrain.h"

// 新しい型 座標と大きさを格納する
struct XMFLOAT6
{
	XMFLOAT3 pos;
	XMFLOAT3 size;
};

// 地形の種類
enum class TERRAIN_TYPE
{
	HILL = 0,
	WALL,
	TREE,
	BOG,

	MAX
};

void TerrainInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain p1Set, WeaponTerrain p2Set);
void TerrainFinalize();
void TerrainUpdate();
void TerrainDraw();
void TerrainSet(WeaponTerrain set, bool playerSelect); // 何の地形か、何Pか

class TERRAIN_OBJECT
{
public:
	XMFLOAT3 m_position;	// 座標
	XMFLOAT3 m_distance;	// 中心からどれだけ離れた場所にいるか
	XMFLOAT3 m_size;		// 大きさ
	TERRAIN_TYPE m_type;	// どの種類の地形か
};

class TERRAIN : public GameObject
{
public:
	// マザーポジション,こいつが動くと他が連動して動く
	XMFLOAT3 m_motherPosition[2] = {};
	// 作成したオブジェクトを保存する器
	std::vector<std::unique_ptr<GameObject>> terrainObjects;
	std::vector<GameObject*> hills[2];	// 丘の当たり判定の全て
	std::vector<GameObject*> walls[2];	// 壁の当たり判定の全て
	std::vector<GameObject*> trees[2];	// 木の当たり判定の全て
	std::vector<GameObject*> ants[2];	// 蟻地獄の当たり判定の全て
	std::vector<GameObject*> bogs[2];	// 沼地の当たり判定の全て

	MODEL* m_moveTerrain[MOVE_TERRAIN_TYPE] = {}; // 地形で使うモデル
	XMFLOAT3 m_terrainScale[MOVE_TERRAIN_TYPE] = {}; // 各地形の大きさ
	XMFLOAT3 m_terrainRotation[MOVE_TERRAIN_TYPE] = {};	// 各地形の回転
	bool m_isChange[CHANGE_FLAG] = { false, false }; // 変身したか
	XMFLOAT3 m_terrainScaling[MOVE_TERRAIN_TYPE] = {};
	FLOAT m_coolTime[2] = {}; // 変身時間(仮) 今後は他のファイルから持ってくる予定
public:
	void SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay, int select);

	//================================================================
	//	複数の当たり判定を細かく設定したい用
	//================================================================
	// string型で書いたオブジェクトの当たり判定をchar型にして効率よくする
	std::vector<std::vector<std::vector<char>>> ConvertTerrain(std::vector<std::vector<std::vector<std::string>>> terrain);
	// char型になったオブジェクトの当たり判定がいくつあるのか数える
	size_t CountObjects(const std::vector<std::vector<std::vector<char>>>& obj);
	// 受け取った総量のオブジェクトの当たり判定をデータとして格納する
	std::vector<TERRAIN_OBJECT> InitializeObject(const std::vector<std::vector<std::vector<char>>>& terrainChip, TERRAIN_TYPE type);
	//================================================================
	//	大きな当たり判定を作り出す用
	//================================================================
		// char型になったオブジェクトの種類がいくつあるか数える
	size_t CountObjectType(const std::vector<std::vector<std::vector<char>>>& obj);
	// char型になったオブジェクトの頂点を求めて、1つのオブジェクトとして認識する
	std::vector<XMFLOAT6> VolumeObject(const std::vector<std::vector<std::vector<char>>>& terrainChip, const size_t objectType, XMFLOAT3 size);
	// 受け取った総量のオブジェクトの当たり判定をデータとして格納する
	std::vector<TERRAIN_OBJECT> InitializeObject(const std::vector<std::vector<std::vector<char>>>& terrainChip, std::vector<XMFLOAT6> mixVal, const size_t objectType, TERRAIN_TYPE type);

public:
	// 各オブジェクトを更新する処理
	void UpdateObject(std::vector<GameObject*> terrain, XMFLOAT3 motherPosition, bool move);
	// 自動で当たり判定を作り出す
	void CreateHit(std::vector<TERRAIN_OBJECT> terrain, XMFLOAT3 motherPosition, int select);

public:
	void ClearPlayerObjects(WeaponTerrain set, int select); // プレイヤーに追従していた地形を解放する
	void PixelObjects(const std::vector<std::vector<std::vector<std::string>>> terrain, TERRAIN_TYPE type, XMFLOAT3 motherPosition, int select);
	void SimpleObjects(const std::vector<std::vector<std::vector<std::string>>> terrain, XMFLOAT3 size, TERRAIN_TYPE type, XMFLOAT3 motherPosition, int select);

public:
	// 蟻地獄のオブジェクトを作る
	void CreateAnt(XMFLOAT3 motherPosition, int select);
};

#endif // TERRAIN_H