/*
* ファイル名	field.cpp
* タイトル	フィールド
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"field.h"
#include"Camera.h"
#include"model.h"
#include"colliderFactory.h"
#include"gameObject.h"

//================================================================
//	グローバル変数
//================================================================
MODEL* Model[FIELD_MAX];

static	ID3D11Device* g_pDevice = NULL;
static	ID3D11DeviceContext* g_pContext = NULL;
//頂点バッファ
static	ID3D11Buffer* g_VertexBuffer = NULL;
//インデックスバッファ
static	ID3D11Buffer* g_IndexBuffer = NULL;
//テクスチャ変数
static ID3D11ShaderResourceView* g_Texture;

static std::vector<std::unique_ptr<GameObject>> g_FieldObjects;

#define		BOX_NUM_VERTEX	(24)

//BOX作成関数
void	CreateBox();

//BOX頂点データ
static	Vertex3D	Box_vdata[BOX_NUM_VERTEX] =
{
	//-Z面
	{//頂点0 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),		//座標
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	//カラー
		XMFLOAT2(0.0f,0.0f)					//テクスチャ座標
	},
	{//頂点1 RIHGT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点2 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{//頂点3 RIHGT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
	//+X面
	{//頂点4 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点5 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点6 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{//頂点7 RIGHT-BOTTM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
	//+Z面
	{//頂点8 LEFT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点9 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点10 LEFT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{//頂点11 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
	//-X面
	{//頂点12 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点13 RIGHT-TOP
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点14 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{//頂点15 RIGHT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},
	//+Y面
	{//頂点16 LEFT-TOP
		XMFLOAT3(-0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.0f)
	},
	{//頂点17 RIGHT-TOP
		XMFLOAT3(0.5f, 0.5f, 0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.0f)
	},
	{//頂点17 LEFT-BOTTOM
		XMFLOAT3(-0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.25f)
	},
	{//頂点19 RIGHT-BOTTOM
		XMFLOAT3(0.5f, 0.5f, -0.5f),
		XMFLOAT3(0.0f, 1.0f, 0.0f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.25f)
	},
	//-Y面
	{//頂点20 LEFT-TOP
		XMFLOAT3(-0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,0.75f)
	},
	{//頂点21 RIGHT-TOP
		XMFLOAT3(0.5f, -0.5f, -0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,0.75f)
	},
	{//頂点22 LEFT-BOTTOM
		XMFLOAT3(-0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(0.0f,1.0f)
	},
	{//頂点23 RIGHT-BOTTOM
		XMFLOAT3(0.5f, -0.5f, 0.5f),
		XMFLOAT3(0.5f, 0.5f, 0.5f),			//法線
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		XMFLOAT2(1.0f,1.0f)
	},

};
static UINT	Box_idxdata[6 * 6] =
{
	 0,  1,  2,  2,  1,  3,		//-Z面
	 4,  5,  6,  6,  5,  7,		//+X面
	 8,  9, 10, 10,  9, 11,		//-Z面
	12, 13, 14, 14, 13, 15,		//-X面
	16, 17, 18, 18, 17, 19,		//+Y面
	20, 21, 22, 22, 21, 23,		//-Y面
};

//マップデータ配列
std::vector<MAPDATA> Map;

// マップチップ
const std::vector<std::vector<std::string>> Stage =
{
	{ // Y=0				// Z->+
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"}, // X=0
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
		{"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
	},
	{ // Y=0				// Z->+
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnonnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"}, // X=0
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
		{"nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"},
	},
};

//================================================================
//	プロトタイプ宣言
//================================================================
std::vector<std::vector<std::vector<char>>> ConvertStage();
size_t CountBlocks(const std::vector<std::vector<std::vector<char>>>& Stage);
void InitializeMap(size_t blocks);

void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	// テクスチャ読み込み
	TexMetadata metadata;
	ScratchImage image;
	LoadFromWICFile(L"Asset\\Texture\\block_field.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(),
		image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);

	auto MapChip = ConvertStage();
	size_t blocks = CountBlocks(MapChip);
	InitializeMap(blocks);

	// ブロック作成
	int loop = 0;

	char c;

	// 3D座標計算
	float posX;  // 行 → X+
	float posY;  // 高さ → Y+
	float posZ;  // 列 → Z+

	for (int y = 0; y < MapChip.size(); y++)      // 高さ
	{
		for (int x = 0; x < MapChip[y].size(); x++)  // 行 → X+
		{
			for (int z = 0; z < MapChip[y][x].size(); z++) // 列 → Z+
			{
				c = MapChip[y][x][z];

				if (c == 'n') continue;

				posX = x * 1.0f - (MapChip[y].size() / 2);
				posY = y * 1.0f;
				posZ = z * 1.0f;

				switch (c)
				{
				case 'b':
					Map[loop].pos = { posX, posY, posZ };
					Map[loop].no = FIELD_BOX;
					loop++;
					break;

				case 'o':
					Map[loop].pos = { posX, posY, posZ };
					Map[loop].no = FIELD_OBT;
					loop++;
					break;

				case 'l':
					Map[loop].pos = { posX, posY, posZ };
					Map[loop].no = FIELD_LIFT;
					loop++;
					break;

				default:
					break;
				}
			}
		}
	}

	loop++;

	Map[loop].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Map[loop].no = FIELD_MAX;

	// 当たり判定を付ける処理
	for (int i = 0; i < std::size(Map); i++)
	{
		const auto mapData = Map[i]; // マップのコピーを取得

		GameObject* object = nullptr;

		switch (Map[i].no)
		{
		case FIELD::FIELD_BOX:
			object = ColliderFactory::CreateBoxObject(
				Map[i].pos,
				{ 1.0f, 1.0f, 1.0f },
				"Wall",
				0
			);
			break;

		case FIELD::FIELD_OBT:
			/*object = ColliderFactory::CreateBoxObject(
				Map[i].pos,
				{ 1.0f, 1.0f, 1.0f },
				"Tree",
				0
			);*/
			object = ColliderFactory::CreateSphereObject(
				Map[i].pos,
				0.5f,
				"Tree",
				0
			);
			break;

		case FIELD::FIELD_LIFT:
			object = ColliderFactory::CreateBoxObject(
				Map[i].pos,
				{ 0.5f, 0.5f, 0.5f },
				"Lift",
				0
			);
			break;

		case FIELD::FIELD_MAX:
			break;

		default:
			break;
		}
	}

	//ブロックの作成
	for (int i = 0; i < FIELD_MAX; i++)
	{
		switch (i)
		{
			case FIELD_BOX:
				CreateBox();
				break;

			case FIELD_OBT:
				Model[FIELD_OBT] = ModelLoad("asset\\model\\tree.fbx");//デバッグ
				break;

			case FIELD_LIFT:
				CreateBox();
				break;
		}
	}
}
void Field_Finalize(void)
{
	for (int i = 0; i < FIELD_MAX; i++)
	{
		if (Model[i] != NULL)
		{
			ModelRelease(Model[i]);
			Model[i] = NULL;
		}
	}
	SAFE_RELEASE(g_VertexBuffer);
	SAFE_RELEASE(g_IndexBuffer);
	SAFE_RELEASE(g_Texture);

}
void Field_Draw(void)
{ 
	//シェーダーを描画パイプラインへ設定
	Shader_Begin();

	//プロジェクション行列作成
	XMMATRIX	Projection = GetProjectionMatrix();
	//ビュー行列作成
	XMMATRIX	View = GetViewMatrix();
	//先にVP変換行列を作っておく
	XMMATRIX VP = View * Projection;

	//MAPの表示
	int i = 0;
	static float rot = 0.0f;
	rot -= 0.5f;
	while (Map[i].no != FIELD_MAX)
	{
		//スケーリング行列の作成
		XMMATRIX	ScalingMatrix = XMMatrixScaling
		(
			1.0f, 1.0f, 1.0f
		);

		if (Map[i].no == FIELD::FIELD_LIFT)
		{
			ScalingMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
		}

		//平行移動行列の作成
		XMMATRIX	TranslationMatrix = XMMatrixTranslation
		(
			Map[i].pos.x, Map[i].pos.y, Map[i].pos.z
		);
		//回転行列の作成
		XMMATRIX	RotationMatrix = XMMatrixRotationRollPitchYaw
		(
			XMConvertToRadians(0.0f),
			//XMConvertToRadians(rot),
			XMConvertToRadians(0.0f),
			XMConvertToRadians(0.0f)
		);
		//ワールド行列の作成
		XMMATRIX	World = ScalingMatrix * RotationMatrix * TranslationMatrix;
		//最終的な変換行列を作成
		XMMATRIX	WVP = World * VP;	//(VP = View * Projection)

		//DirectXへ行列をセット
		Shader_SetWorldMatrix(World);
		//Shader_SetMatrix(WVP);
		
		//テクスチャをセット
		g_pContext->PSSetShaderResources(0, 1, &g_Texture);

		//頂点バッファをセット
		UINT	stride = sizeof(Vertex3D);	//頂点１個のデータサイズ
		UINT	offset = 0;
		g_pContext->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		//インデックスバッファをセット
		g_pContext->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//描画するポリゴンの種類をセット 3頂点でポリゴン１枚として表示
		g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (Map[i].no == FIELD_BOX || Map[i].no == FIELD_LIFT)
		{
			////描画リクエスト
			g_pContext->DrawIndexed(6 * 6, 0, 0);
		}
		else
		{
			ModelDraw(Model[Map[i].no]);
		}

		//ModelDraw(Test);//デバッグ

		i++;
	}


}

void Field_Update(void) 
{

}

MAPDATA* GetFieldMap()
{
	return &Map[0];
}

//BOXデータを作成する
void CreateBox()
{
	{
		//頂点バッファ作成
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));//０でクリア
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex3D) * BOX_NUM_VERTEX;//格納できる頂点数*頂点サイズ
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		g_pDevice->CreateBuffer(&bd, NULL, &g_VertexBuffer);

		//頂点データを頂点バッファへコピーする
		D3D11_MAPPED_SUBRESOURCE msr;
		g_pContext->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		Vertex3D* vertex = (Vertex3D*)msr.pData;
		//頂点データをコピーする
		CopyMemory(&vertex[0], &Box_vdata[0], sizeof(Vertex3D) * BOX_NUM_VERTEX);
		//コピー完了
		g_pContext->Unmap(g_VertexBuffer, 0);

	}

	//インデックスバッファ作成
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));//０でクリア
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(UINT) * 6 * 6;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		g_pDevice->CreateBuffer(&bd, NULL, &g_IndexBuffer);

		//インデックスバッファへ書き込み
		D3D11_MAPPED_SUBRESOURCE   msr;
		g_pContext->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		UINT* index = (UINT*)msr.pData;

		//インデックスデータをバッファへコピー
		CopyMemory(&index[0], &Box_idxdata[0], sizeof(UINT) * 6 * 6);
		g_pContext->Unmap(g_IndexBuffer, 0);
	}
}

std::vector<std::vector<std::vector<char>>> ConvertStage()
{
	std::vector<std::vector<std::vector<char>>> map;

	map.reserve(Stage.size());

	for (const auto& layer : Stage) // Y方向
	{
		map.emplace_back();
		map.back().reserve(layer.size());

		for (const auto& row : layer) // Z方向
		{
			// string → char 配列に変換
			map.back().emplace_back(row.begin(), row.end());
		}
	}

	return map;
}

size_t CountBlocks(const std::vector<std::vector<std::vector<char>>>& Stage)
{
	size_t count = 0;

	for (const auto& layer : Stage)
	{
		for (const auto& row : layer)
		{
			for (char c : row)
			{
				if (c != 'n')
					count++;
			}
		}
	}

	count += 2;

	return count;
}

void InitializeMap(size_t blocks)
{
	Map.clear();
	Map.reserve(blocks); // あらかじめ必要数を確保

	for (size_t i = 0; i < blocks; i++)
	{
		Map.push_back(MAPDATA{}); // 空の MAPDATA を追加
	}
}

void SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay)
{
	// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
	std::unique_ptr<GameObject> obj_owner(
		ColliderFactory::CreateBoxObject(pos, scl, tag, lay)
	);

	GameObject* raw_ptr = obj_owner.get(); // 生のポインタを取得（参照用）

	if (raw_ptr != nullptr)
	{
		g_FieldObjects.push_back(std::move(obj_owner));
	}
}
void SetObject(XMFLOAT3 pos, float radius, std::string tag, int lay)
{
	// ファクトリの戻り値 (生のポインタ) を unique_ptr で受け取り、所有権を確保
	std::unique_ptr<GameObject> obj_owner(
		ColliderFactory::CreateSphereObject(pos, radius, tag, lay)
	);

	GameObject* raw_ptr = obj_owner.get(); // 生のポインタを取得（参照用）

	if (raw_ptr != nullptr)
	{
		g_FieldObjects.push_back(std::move(obj_owner));
	}
}