/*
* ファイル名	sprite.cpp
* タイトル	スプライト
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"sprite.h"

//================================================================
//	グローバル変数
//================================================================
static constexpr int NUM_VERTEX = 6; // 使用できる最大頂点数
static ID3D11Buffer* g_pVertexBuffer = nullptr; // 頂点バッファ
// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void InitializeSprite()
{
	g_pDevice = Direct3D_GetDevice();

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex3D) * NUM_VERTEX;//<<<<<<<格納する最大頂点数
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	g_pDevice->CreateBuffer(&bd, NULL, &g_pVertexBuffer);
}

void FinalizeSprite()
{
	g_pVertexBuffer->Release();	//頂点バッファの解放
}


//=====================================
//スプライト描画
//=====================================
void DrawSprite(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col)
{

	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex3D* v = (Vertex3D*)msr.pData;

	// 指定の位置に指定のサイズ、色の四角形を描画する /////////テクスチャ追加
	v[0].position = { pos.x - (size.x / 2), pos.y - (size.y / 2), 0.0f };
	v[0].normal = { 0.0f, 0.0f, 0.0f };
	v[0].color = col;
	v[0].texCoord = { 0.0f, 0.0f };

	v[1].position = { pos.x + (size.x / 2), pos.y - (size.y / 2), 0.0f };
	v[1].normal = { 0.0f, 0.0f, 0.0f };
	v[1].color = col;
	v[1].texCoord = { 1.0f, 0.0f };

	v[2].position = { pos.x - (size.x / 2), pos.y + (size.y / 2), 0.0f };
	v[2].normal = { 0.0f, 0.0f, 0.0f };
	v[2].color = col;
	v[2].texCoord = { 0.0f, 1.0f };

	v[3].position = { pos.x + (size.x / 2), pos.y + (size.y / 2), 0.0f };
	v[3].normal = { 0.0f, 0.0f, 0.0f };
	v[3].color = col;
	v[3].texCoord = { 1.0f, 1.0f };


	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);//頂点１つあたりのサイズを指定
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定　ポリゴンの描画ルール的なもの
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(4, 0);//表示に使用する頂点数を指定

}

void DrawSpriteUV(
	XMFLOAT2 pos,
	XMFLOAT2 size,
	XMFLOAT4 col,
	float u0, float v0,
	float u1, float v1,float angle)
{
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	Vertex3D* v = (Vertex3D*)msr.pData;

	const float deg = angle;
	const float rad = deg * (3.14159265f / 180.0f);
	const float c = cosf(rad);
	const float s = sinf(rad);

	auto Rotate = [&](float x, float y) -> XMFLOAT2
		{

			return XMFLOAT2(x * c - y * s, x * s + y * c);
		};



	const float halfW = fabsf(size.x) * 0.5f;
	const float halfH = fabsf(size.y) * 0.5f;

	XMFLOAT2 p0 = Rotate(-halfW, -halfH);
	XMFLOAT2 p1 = Rotate(+halfW, -halfH);
	XMFLOAT2 p2 = Rotate(-halfW, +halfH);
	XMFLOAT2 p3 = Rotate(+halfW, +halfH);


	v[0].position = { pos.x + p0.x, pos.y + p0.y, 0.0f };
	v[0].color = col;
	v[0].texCoord = { u0, v0 };

	v[1].position = { pos.x + p1.x, pos.y + p1.y, 0.0f };
	v[1].color = col;
	v[1].texCoord = { u1, v0 };

	v[2].position = { pos.x + p2.x, pos.y + p2.y, 0.0f };
	v[2].color = col;
	v[2].texCoord = { u0, v1 };

	v[3].position = { pos.x + p3.x, pos.y + p3.y, 0.0f };
	v[3].color = col;
	v[3].texCoord = { u1, v1 };

	g_pContext->Unmap(g_pVertexBuffer, 0);

	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	g_pContext->Draw(4, 0);
}

void DrawSpriteEx(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc)
{
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();


	float w = 1.0f / wc;
	float h = 1.0f / hc;
	float texX = (bno % wc) * w;
	float texY = (bno / wc) * h;

	
	bool flipX = (size.x < 0.0f);
	bool flipY = (size.y < 0.0f);

	
	float halfW = fabsf(size.x) * 0.5f;
	float halfH = fabsf(size.y) * 0.5f;

	
	float u0 = texX;
	float u1 = texX + w;
	float v0 = texY;
	float v1 = texY + h;

	
	if (flipX) std::swap(u0, u1);
	if (flipY) std::swap(v0, v1);

	// vertex buffer lock
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex3D* v = (Vertex3D*)msr.pData;

	
	v[0].position = { pos.x - halfW, pos.y - halfH, 0.0f };
	v[0].color = col;
	v[0].texCoord = { u0, v0 };

	v[1].position = { pos.x + halfW, pos.y - halfH, 0.0f };
	v[1].color = col;
	v[1].texCoord = { u1, v0 };

	v[2].position = { pos.x - halfW, pos.y + halfH, 0.0f };
	v[2].color = col;
	v[2].texCoord = { u0, v1 };

	v[3].position = { pos.x + halfW, pos.y + halfH, 0.0f };
	v[3].color = col;
	v[3].texCoord = { u1, v1 };

	// unlock
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// set vertex buffer
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// draw
	g_pContext->Draw(4, 0);
}

void DrawSpriteEx(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, float hp,bool isPlayer1)
{
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	float percent = hp * 0.01;
	float w = percent;
	float h = 1.0f;
	float texX = 0;
	float texY = 0;
	if (isPlayer1)
	{
		float texX = 1;
		float texY = 1;
	}
	//float sizeX = size.x * percent;
	bool flipX = (size.x < 0.0f);
	bool flipY = (size.y < 0.0f);

	float halfW = fabsf(size.x) * 0.5f;
	float halfH = fabsf(size.y) * 0.5f;
	float u0 = texX;
	float u1 = texX + w;
	float v0 = texY;
	float v1 = texY + h;
	if (isPlayer1)
	{
		halfW = halfW * percent;
		pos.x = pos.x + (fabsf(size.x) * 0.5f) - halfW;
		u0 = texX - w;
		u1 = texX;
		v0 = texY - h;
		v1 = texY;
	}
	else
	{
		halfW = halfW * percent;
		pos.x = pos.x - (fabsf(size.x) * 0.5f) + halfW;
	}
	if (flipX) std::swap(u0, u1);
	if (flipY) std::swap(v0, v1);

	// vertex buffer lock
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex3D* v = (Vertex3D*)msr.pData;


	v[0].position = { pos.x - halfW, pos.y - halfH, 0.0f };
	v[0].color = col;
	v[0].texCoord = { u0, v0 };

	v[1].position = { pos.x + halfW, pos.y - halfH, 0.0f };
	v[1].color = col;
	v[1].texCoord = { u1, v0 };

	v[2].position = { pos.x - halfW, pos.y + halfH, 0.0f };
	v[2].color = col;
	v[2].texCoord = { u0, v1 };

	v[3].position = { pos.x + halfW, pos.y + halfH, 0.0f };
	v[3].color = col;
	v[3].texCoord = { u1, v1 };

	// unlock
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// set vertex buffer
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// draw
	g_pContext->Draw(4, 0);
}
void DrawSpriteEx(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc,float angle)
{
	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();


	float w = 1.0f / wc;
	float h = 1.0f / hc;
	float texX = (bno % wc) * w;
	float texY = (bno / wc) * h;


	bool flipX = (size.x < 0.0f);
	bool flipY = (size.y < 0.0f);


	float halfW = fabsf(size.x) * 0.5f;
	float halfH = fabsf(size.y) * 0.5f;


	float u0 = texX;
	float u1 = texX + w;
	float v0 = texY;
	float v1 = texY + h;


	if (flipX) std::swap(u0, u1);
	if (flipY) std::swap(v0, v1);

	const float deg = angle;
	const float rad = deg * (3.14159265f / 180.0f);
	const float c = cosf(rad);
	const float s = sinf(rad);

	auto Rotate = [&](float x, float y) -> XMFLOAT2
		{
			
			return XMFLOAT2(x * c - y * s, x * s + y * c);
		};


	XMFLOAT2 p0 = Rotate(-halfW, -halfH);
	XMFLOAT2 p1 = Rotate(+halfW, -halfH);
	XMFLOAT2 p2 = Rotate(-halfW, +halfH);
	XMFLOAT2 p3 = Rotate(+halfW, +halfH);

	// vertex buffer lock
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex3D* v = (Vertex3D*)msr.pData;

	v[0].position = { pos.x + p0.x, pos.y + p0.y, 0.0f };
	v[0].color = col;
	v[0].texCoord = { u0, v0 };

	v[1].position = { pos.x + p1.x, pos.y + p1.y, 0.0f };
	v[1].color = col;
	v[1].texCoord = { u1, v0 };

	v[2].position = { pos.x + p2.x, pos.y + p2.y, 0.0f };
	v[2].color = col;
	v[2].texCoord = { u0, v1 };

	v[3].position = { pos.x + p3.x, pos.y + p3.y, 0.0f };
	v[3].color = col;
	v[3].texCoord = { u1, v1 };

	// unlock
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// set vertex buffer
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// draw
	g_pContext->Draw(4, 0);
}

void DrawSpriteScroll(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col,
	XMFLOAT2 texcoord)
{

	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex3D* v = (Vertex3D*)msr.pData;

	// 指定の位置に指定のサイズ、色の四角形を描画する /////////テクスチャ追加
	v[0].position = { pos.x - (size.x / 2), pos.y - (size.y / 2), 0.0f };
	v[0].color = col;
	v[0].texCoord = { texcoord.x, texcoord.y };

	v[1].position = { pos.x + (size.x / 2), pos.y - (size.y / 2), 0.0f };
	v[1].color = col;
	v[1].texCoord = { 1.0f + texcoord.x, texcoord.y };

	v[2].position = { pos.x - (size.x / 2), pos.y + (size.y / 2), 0.0f };
	v[2].color = col;
	v[2].texCoord = { texcoord.x, 1.0f + texcoord.y };

	v[3].position = { pos.x + (size.x / 2), pos.y + (size.y / 2), 0.0f };
	v[3].color = col;
	v[3].texCoord = { 1.0f + texcoord.x, 1.0f + texcoord.y };


	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);//頂点１つあたりのサイズを指定
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定　ポリゴンの描画ルール的なもの
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(4, 0);//表示に使用する頂点数を指定}
}


void DrawSpriteAnimation(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col,
	XMFLOAT2 texcoord)
{

	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex3D* v = (Vertex3D*)msr.pData;

	// 指定の位置に指定のサイズ、色の四角形を描画する /////////テクスチャ追加
	v[0].position = { pos.x - (size.x / 2), pos.y - (size.y / 2), 0.0f };
	v[0].color = col;
	v[0].texCoord = { 0, 0 };

	v[1].position = { pos.x + (size.x / 2), pos.y - (size.y / 2), 0.0f };
	v[1].color = col;
	v[1].texCoord = { texcoord.x, 0 };

	v[2].position = { pos.x - (size.x / 2), pos.y + (size.y / 2), 0.0f };
	v[2].color = col;
	v[2].texCoord = { 0, texcoord.y };

	v[3].position = { pos.x + (size.x / 2), pos.y + (size.y / 2), 0.0f };
	v[3].color = col;
	v[3].texCoord = { texcoord.x, texcoord.y };


	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);//頂点１つあたりのサイズを指定
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定　ポリゴンの描画ルール的なもの
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(4, 0);//表示に使用する頂点数を指定}
}
void DrawSpriteExRotation(XMFLOAT2 pos, XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc, float radian)
{

	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex3D* v = (Vertex3D*)msr.pData;

	//ブロックの縦横サイズを計算
	float w = 1.0f / wc;
	float h = 1.0f / hc;
	//bnoの左上のテクスチャ座標を計算
	float x = (bno % wc) * w;
	float y = (bno / wc) * h;

	// 指定の位置に指定のサイズ、色の四角形を描画する /////////テクスチャ追加
	v[0].position = { -(size.x / 2), -(size.y / 2), 0.0f };
	v[0].color = col;
	v[0].texCoord = { x, y };

	v[1].position = { (size.x / 2), -(size.y / 2), 0.0f };
	v[1].color = col;
	v[1].texCoord = { x + w, y };

	v[2].position = { -(size.x / 2), (size.y / 2), 0.0f };
	v[2].color = col;
	v[2].texCoord = { x, y + h };

	v[3].position = { (size.x / 2), (size.y / 2), 0.0f };
	v[3].color = col;
	v[3].texCoord = { x + w, y + h };

	//回転処理
	float co = cosf(radian);
	float si = sinf(radian);
	for (int i = 0; i < 4; i++)
	{
		float x = v[i].position.x;
		float y = v[i].position.y;
		v[i].position.x = (x * co - y * si) + pos.x;//回転させてから表示位置まで平行移動
		v[i].position.y = (x * si + y * co) + pos.y;//
	}


	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);//頂点１つあたりのサイズを指定
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定　ポリゴンの描画ルール的なもの
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(4, 0);//表示に使用する頂点数を指定
}

void DrawSprite(XMFLOAT2 size, XMFLOAT4 col, int bno, int wc, int hc)
{

	g_pDevice = Direct3D_GetDevice();
	g_pContext = Direct3D_GetDeviceContext();

	// 頂点バッファをロックする
	D3D11_MAPPED_SUBRESOURCE msr;
	g_pContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	// 頂点バッファへの仮想ポインタを取得
	Vertex3D* v = (Vertex3D*)msr.pData;

	//ブロックの縦横サイズを計算
	float w = 1.0f / wc;
	float h = 1.0f / hc;
	//bnoの左上のテクスチャ座標を計算
	float x = (bno % wc) * w;
	float y = (bno / wc) * h;

	// 指定の位置に指定のサイズ、色の四角形を描画する /////////テクスチャ追加
	v[0].position = { -(size.x / 2), -(size.y / 2), 0.0f };
	v[0].color = col;
	v[0].texCoord = { x, y };

	v[1].position = { (size.x / 2), -(size.y / 2), 0.0f };
	v[1].color = col;
	v[1].texCoord = { x + w, y };

	v[2].position = { -(size.x / 2), (size.y / 2), 0.0f };
	v[2].color = col;
	v[2].texCoord = { x, y + h };

	v[3].position = { (size.x / 2), (size.y / 2), 0.0f };
	v[3].color = col;
	v[3].texCoord = { x + w, y + h };


	// 頂点バッファのロックを解除
	g_pContext->Unmap(g_pVertexBuffer, 0);

	// 頂点バッファを描画パイプラインに設定
	UINT stride = sizeof(Vertex3D);//頂点１つあたりのサイズを指定
	UINT offset = 0;
	g_pContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定　ポリゴンの描画ルール的なもの
	g_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画命令発行
	g_pContext->Draw(4, 0);//表示に使用する頂点数を指定
}