/*
* ファイル名	Raycasting.h
* タイトル		カメラのレイ処理
* 作成者		鈴木豪
* 作成日		12月09日
* 更新日		12月10日
*/

#ifndef RAY_CASTING_H
#define RAY_CASTING_H

//================================================================
//	インクルード
//================================================================
//#define NOMINMAX

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include <cfloat>
#include <iostream>
#include <algorithm>
using namespace DirectX;
#include "Camera.h"
#include "gameObject.h"
#include "managerCollider.h"

//レイクラス
class Raycast
{
public:
	XMFLOAT3 m_origin;    //原点
	XMFLOAT3 m_direction; //方向ベクトル
public:
	//コンストラクタ
	Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction) : m_origin(origin), m_direction(direction)
	{
		//方向ベクトルを正規化
		XMVECTOR dir = XMLoadFloat3(&m_direction);
		dir = XMVector3Normalize(dir);
		XMStoreFloat3(&m_direction, dir);
	}

	//レイの任意の点を計算する 点P(t) = O + tD
	XMFLOAT3 GetPoint(float t) const {
		XMVECTOR O = XMLoadFloat3(&m_origin);
		XMVECTOR D = XMLoadFloat3(&m_direction);
		XMVECTOR P = XMVectorAdd(O, XMVectorScale(D, t));

		XMFLOAT3 result;
		XMStoreFloat3(&result, P);
		return result;
	}
};

//衝突判定クラス
class Ray_HitInfo
{
public:
	float m_distance = FLT_MAX; //float型で表現できる最大の有限値
	XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f }; //衝突座標
	XMFLOAT3 m_normal = { 0.0f, 0.0f, 0.0f };   //衝突面の法線ベクトル
	GameObject* m_hitObject = nullptr;
	bool m_isTransparent = false; //透過フラグ　(カメラが障害物の内部に存在している場合trueにして透過処理(ブレンド設定を変更)する必要あり)
public:
	//衝突が発生したかどうか
	bool HasHit() const { return m_distance < FLT_MAX; }

	//初期化/リセット
	void Reset()
	{
		m_distance = FLT_MAX;
		m_position = { 0.0f, 0.0f, 0.0f }; //衝突座標
		m_normal = { 0.0f, 0.0f, 0.0f };   //衝突面の法線ベクトル
	}
};

//レイと三角形の交差判定
class InterSector
{
	//===============================================
	//	変数説明
	// ----------------------------------------------
	//	v0, v1, v2 三角形の頂点
	//	hit 衝突情報を格納するHitInfoオブジェクト
	//	true / false 衝突あり / なし
	//===============================================
public:
	//===============================================
	//	関数説明
	// ----------------------------------------------
	//	レイと三角形の交差判定
	//	変数名は変数説明を参照
	//===============================================
	static bool InterSects(
		const Raycast& ray,
		const XMFLOAT3& v0,
		const XMFLOAT3& v1,
		const XMFLOAT3& v2,
		Ray_HitInfo& hit
	)
	{
		const float Epsilon = 0.0000001f;

		//頂点をXMVectorにロード
		XMVECTOR V0 = XMLoadFloat3(&v0);
		XMVECTOR V1 = XMLoadFloat3(&v1);
		XMVECTOR V2 = XMLoadFloat3(&v2);
		XMVECTOR O = XMLoadFloat3(&ray.m_origin);
		XMVECTOR D = XMLoadFloat3(&ray.m_direction);

		//三角形の辺のベクトルを計算
		XMVECTOR edge1 = XMVectorSubtract(V1, V0);
		XMVECTOR edge2 = XMVectorSubtract(V2, V0);

		//P = D x edge2（レイの方向と辺2の外積）
		XMVECTOR P = XMVector3Cross(D, edge2);

		//判定条件1：レイと三角形が平行かどうか(detが0に近いか)　det = determinantの略
		//det = edge1 ・ P
		XMVECTOR detVec = XMVector3Dot(edge1, P);
		float det = XMVectorGetX(detVec);

		//レイと三角形が平行 (detが0に近い)
		if (det > -Epsilon && det < Epsilon)
		{
			return false;
		}

		//detの逆数(inverse det)
		float invDet = 1.0f / det;

		//レイの原点から頂点0へのベクトル
		//T = O - V0
		XMVECTOR T = XMVectorSubtract(O, V0);

		//重心座標パラメータを計算　(三角形の3つの頂点の座標の平均値)
		// u = (T ・ P) * invDet
		XMVECTOR uVec = XMVector3Dot(T, P);
		float u = XMVectorGetX(uVec) * invDet;

		//uが[0, 1]の範囲外なら衝突なし
		if (u < 0.0f || u > 1.0f)
		{
			return false;
		}

		//Q = T x edge1
		XMVECTOR Q = XMVector3Cross(T, edge1);

		//重心座標パラメータを計算　(vertexのv)
		//v = (D ・ Q) * invDet
		XMVECTOR vVec = XMVector3Dot(D, Q);
		float v = XMVectorGetX(vVec) * invDet;

		//vが[0, 1]の範囲外、または u + v > 1 なら衝突なし
		if (v < 0.0f || u + v > 1.0f)
		{
			return false;
		}

		//衝突距離パラメータを計算
		//t = (edge2 ・ Q) * invDet
		XMVECTOR tVec = XMVector3Dot(edge2, Q);
		float t = XMVectorGetX(tVec) * invDet;

		//判定条件2: レイの背後、または遠すぎる
		if (t > Epsilon && t < hit.m_distance) //新しい衝突がより近いかつ、レイの背後ではない
		{
			//衝突情報を更新
			hit.m_distance = t;

			//衝突点の計算 P = O + tD
			XMFLOAT3 hitPosition = ray.GetPoint(t);
			hit.m_position = hitPosition;

			//法線の計算
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(edge1, edge2));
			XMStoreFloat3(&hit.m_normal, N);

			return true;
		}

		return false;
	
	}

	//===============================================
	//	関数説明
	// ----------------------------------------------
	//	レイとBoxColliderの交差判定
	//	変数名は変数説明を参照
	//===============================================
	static bool InterSects(
		const Raycast& ray,
		const BoxCollider* boxCollider,
		Ray_HitInfo& hit
	)
	{
		//BoxColliderとGameObjectの情報を取得
		GameObject* owner = boxCollider->owner;
		XMFLOAT3 halfSize = boxCollider->HalfSize();
		const float Epsilon = 0.001f;

		//ワールド座標からローカル座標への変換行列を計算

		//スケール、回転、平行移動を含むワールド行列
		XMMATRIX WorldMatrix = 
			XMMatrixScaling(owner->m_scale.x, owner->m_scale.y, owner->m_scale.z) *
			XMMatrixRotationRollPitchYaw(owner->m_rotation.x, owner->m_rotation.y, owner->m_rotation.z) *
			XMMatrixTranslation(owner->m_position.x, owner->m_position.y, owner->m_position.z);

		//逆行列 (Inverse Matrix) を計算
		XMVECTOR det;
		XMMATRIX InverseWorldMatrix = XMMatrixInverse(&det, WorldMatrix);

		//レイをローカル座標に変換(レイの原点と方向)
		//レイの原点: P_local = P_world * M_inverse
		//W = World, L = Local
		XMVECTOR RayOrigin_W = XMLoadFloat3(&ray.m_origin);
		XMVECTOR RayOrigin_L = XMVector3TransformCoord(RayOrigin_W, InverseWorldMatrix);

		//レイの方向: D_local = D_world * R_inverse (回転成分のみを適用)
		XMMATRIX InverseRotationMatrix = XMMatrixRotationRollPitchYaw(-owner->m_rotation.x, -owner->m_rotation.y, -owner->m_rotation.z);
		XMVECTOR RayDirection_W = XMLoadFloat3(&ray.m_direction);
		XMVECTOR RayDirection_L = XMVector3TransformNormal(RayDirection_W, InverseRotationMatrix);

		//ローカルレイをRaycast構造体に格納
		Raycast localRay(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0)); //コンストラクタを一時的に使用
		XMStoreFloat3(&localRay.m_origin, RayOrigin_L);
		XMStoreFloat3(&localRay.m_direction, RayDirection_L);

		//ローカルレイとAABB（軸並行境界ボックス）の交差判定 (Slab法)

		float tmin = -FLT_MAX;
		float tmax = FLT_MAX;

		//X軸の判定
		for (int i = 0; i < 3; ++i)
		{
			//ボックスの境界 (ローカル座標)
			XMFLOAT3 Min_L = { -halfSize.x, -halfSize.y, -halfSize.z };
			XMFLOAT3 Max_L = { halfSize.x, halfSize.y, halfSize.z };
			
			//halfSize.x, halfSize.y, halfSize.zの値を取得
			//float half_size_i = (&halfSize.x)[i];
			//float min_b = -half_size_i; //-HalfSize
			//float max_b = half_size_i;  //+HalfSize

			float origin = (&localRay.m_origin.x)[i];
			float direction = (&localRay.m_direction.x)[i];

			float min_b = (&Min_L.x)[i];
			float max_b = (&Max_L.x)[i];

			////Y軸(i == 1)の衝突判定を無視する
			//if (i == 1)
			//{
			//	//Y軸方向の衝突判定を完全にスキップして、X, Z軸のみでtmin/tmaxを制約する
			//	continue;
			//}

			if (fabs(direction) < Epsilon) {
				//レイが軸と平行な場合
				if (origin < min_b || origin > max_b)
					return false; //ボックスの外側にいる
			}
			else {
				//スラブ境界との衝突距離を計算
				float t1 = (min_b - origin) / direction;
				float t2 = (max_b - origin) / direction;

				//t1, t2 の順序を入れ替える
				if (t1 > t2) std::swap(t1, t2);

				//全ての軸で共通する衝突区間を更新
				tmin = (std::max)(tmin, t1);
				tmax = (std::min)(tmax, t2);

				//区間が重ならない
				if (tmin > tmax) return false;
			}
		}

		//結果の検証とHitInfoの更新

		//衝突距離 tmin が有効な衝突距離 (レイの原点より前方) かつ、既存のヒットより近い場合
		//tmin > Epsilon はレイがオブジェクトの外側から入っていることを保証
		if (tmin > Epsilon && tmin < hit.m_distance)
		{
			//既存のヒット距離より近い場合のみ更新
			hit.m_distance = tmin;

			//m_hitObjectの設定 コライダーのowner(GameObject*)を格納する
			hit.m_hitObject = boxCollider->owner;

			//衝突点の計算 (ローカル座標)
			XMFLOAT3 hitPosition_L = localRay.GetPoint(tmin);

			//法線の計算 (ローカル座標)
			XMVECTOR Normal_L = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			//ローカル座標でどの面（軸）で衝突したかを判定し、法線ベクトルを設定
			for (int i = 0; i < 3; ++i) {
				float pos = (&hitPosition_L.x)[i];
				float size = (&halfSize.x)[i];

				//許容誤差を考慮して面判定
				if (fabs(pos - size) < 0.01f) {
					(&Normal_L.m128_f32[i])[0] = 1.0f; // 正の面 (+X, +Y, +Z)
					break;
				}
				if (fabs(pos + size) < 0.01f) {
					(&Normal_L.m128_f32[i])[0] = -1.0f; // 負の面 (-X, -Y, -Z)
					break;
				}
			}

			//衝突位置と法線をワールド座標に戻す

			//衝突位置 (座標): L -> W
			XMVECTOR HitPos_L = XMLoadFloat3(&hitPosition_L);
			XMVECTOR HitPosition_W = XMVector3TransformCoord(HitPos_L, WorldMatrix);
			XMStoreFloat3(&hit.m_position, HitPosition_W);

			//法線 (ベクトル): L -> W (回転のみ)
			XMMATRIX WorldRotationMatrix = XMMatrixRotationRollPitchYaw(owner->m_rotation.x, owner->m_rotation.y, owner->m_rotation.z);
			XMVECTOR Normal_W = XMVector3TransformNormal(Normal_L, WorldRotationMatrix);
			XMStoreFloat3(&hit.m_normal, Normal_W);

			return true;
		}

		return false;
	}
	
};

#endif // RAY_CASTING_H