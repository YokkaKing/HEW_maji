/*
* ファイル名	Audio.h
* タイトル	サウンド
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef AUDIO_H
#define AUDIO_H

//================================================================
//	インクルード
//================================================================
#include<xaudio2.h>

void InitAudio();		//サウンドの初期化
void UninitAudio();		//サウンドの終了

int LoadAudio(const char* FileName);		//サウンドデータ読み込み
void UnloadAudio(int Index);				//サウンドデータ解放（停止）
void PlayAudio(int Index, bool Loop = false);//サウンドデータ再生
void StopAudio(int Index);

extern int arrow_shuriken;
extern int button;
extern int change;
extern int charge;
extern int crowd;
extern int cursorMove;
extern int damageHammer;
extern int damageSharp;
extern int down;
extern int fade;
extern int gameStart;
extern int hammer;
extern int health;
extern int item;
extern int map;
extern int roundEnd;
extern int spear;
extern int start;
extern int sword;
extern int teamLogo;
extern int terrain;
extern int title;
extern int winner;

/*
//以下の関数はプログラムの最初と最後に1回ずつ呼び出せばOK
void InitAudio();		//サウンドの初期化
void UninitAudio();		//サウンドの終了


//以下はシーンごとの処理

//グローバル変数
static int g_BgmID = NULL;//ロードするデータの数だけ変数が必要

//初期化時
g_BgmID = LoadAudio("asset\\Audio\\title.wav");	//サウンドデータ読み込み
PlayAudio(g_BgmID, true);	//サウンドデータ再生(ループあり)

//終了時
UnloadAudio(g_BgmID);		//サウンドデータ解放（停止）
*/

#endif // AUDIO_H