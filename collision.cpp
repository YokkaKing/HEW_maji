////collision.cpp
//
//#include "collision.h"
//
//float PlayerField_Collision()
//{
//	float hit = false;
//	PLAYER* Player = GetPlayer();
//	MAPDATA* Map = GetFieldMap();
//	int i = 0;
//
//	////全てのブロックをチェック
//	while (Map[i].no != FIELD_MAX)
//	{
//		float BoxTop; //上の面(+Y)
//
//		switch (Map[i].no)
//		{
//
//		default:
//			BoxTop = Map[i].pos.y + BOX_RADIUS; // 普通のBOX
//			break;
//
//		}
//
//		// 壁としての判定処理
//		if (Map[i].pos.y - BOX_RADIUS < Player->Position.y && Player->Position.y < BoxTop - 0.1f)
//		{
//			if (Map[i].pos.z - BOX_RADIUS < Player->Position.z && Player->Position.z < Map[i].pos.z + BOX_RADIUS)
//			{
//				if (Map[i].pos.x - BOX_RADIUS < Player->Position.x + PLAYER_RADIUS && Player->Position.x < Map[i].pos.x - BOX_RADIUS)
//				{// BOXの左の面(-X)にぶつかったので座標の補正
//					Player->Position.x += (Map[i].pos.x - BOX_RADIUS) - (Player->Position.x + PLAYER_RADIUS);
//					Player->Velocity.x *= -COE; // 移動ベクトルの反転
//					hit = COLLISION_HIT::HIT_WALL_3;
//				}
//				else if (Map[i].pos.x + BOX_RADIUS > Player->Position.x - PLAYER_RADIUS && Player->Position.x > Map[i].pos.x + BOX_RADIUS)
//				{// BOXの右の面(+X)にぶつかった
//					Player->Position.x += (Map[i].pos.x + BOX_RADIUS) - (Player->Position.x - PLAYER_RADIUS);
//					Player->Velocity.x *= -COE;
//					hit = COLLISION_HIT::HIT_WALL_1;
//				}
//			}
//			else if (Map[i].pos.x - BOX_RADIUS < Player->Position.x && Player->Position.x < Map[i].pos.x + BOX_RADIUS)
//			{
//				if (Map[i].pos.z - BOX_RADIUS < Player->Position.z + PLAYER_RADIUS && Player->Position.z < Map[i].pos.z - BOX_RADIUS)
//				{// BOXの手前の面(-Z)にぶつかったので座標の補正
//					Player->Position.z += (Map[i].pos.z - BOX_RADIUS) - (Player->Position.z + PLAYER_RADIUS);
//					Player->Velocity.z *= -COE; // 移動ベクトルの反転
//					hit = COLLISION_HIT::HIT_WALL_0;
//				}
//				else if (Map[i].pos.z + BOX_RADIUS > Player->Position.z - PLAYER_RADIUS && Player->Position.z > Map[i].pos.z + BOX_RADIUS)
//				{// BOXの奥の面(+Z)にぶつかった
//					Player->Position.z += (Map[i].pos.z + BOX_RADIUS) - (Player->Position.z - PLAYER_RADIUS);
//					Player->Velocity.z *= -COE;
//					hit = COLLISION_HIT::HIT_WALL_2;
//				}
//			}
//
//
//
//		}
//		// 地面としての判定処理
//		else
//		{
//			if (Map[i].pos.z - BOX_RADIUS < Player->Position.z && Player->Position.z < Map[i].pos.z + BOX_RADIUS)
//			{
//				if (Map[i].pos.x - BOX_RADIUS < Player->Position.x && Player->Position.x < Map[i].pos.x + BOX_RADIUS)
//				{
//					if (Map[i].pos.y - BOX_RADIUS < Player->Position.y + PLAYER_RADIUS && Player->Position.y < Map[i].pos.y - BOX_RADIUS)
//					{
//						//下からあたった
//						Player->Position.y += (Map[i].pos.y - BOX_RADIUS) - (Player->Position.y + PLAYER_RADIUS);
//						Player->Velocity.y *= -COE;
//						//hit = 
//					}
//					else if (BoxTop > Player->Position.y - PLAYER_RADIUS && Player->Position.y > BoxTop)
//					{
//						//地面に当たった
//						//上から当たったので座標の補正
//						Player->Position.y += (BoxTop)-(Player->Position.y - PLAYER_RADIUS);
//						Player->Velocity.y = Player->Velocity.y * (-COE * 1.0f);//ベクトルの反転
//						hit = COLLISION_HIT::HIT_GROUND;
//					}
//
//				}
//			}
//		}
//
//
//
//
//		i++;
//	}
//
//	return hit;
//
//}
