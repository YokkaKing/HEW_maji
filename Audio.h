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
void PlayAudioLoopSection(int Index, float loopBeginSec, float loopEndSec);
void SetAudioVolume(int Index, float volume);
extern int g_arrow_shuriken;
extern int g_button;
extern int g_change;
extern int g_charge1;
extern int g_charge2;
extern int g_charge3;
extern int g_charge4;
extern int g_charge5;

extern int g_crowd;
extern int g_cursorMove;
extern int g_damageHammer;
extern int g_damageSharp;
extern int g_down;
extern int g_fade;
extern int g_gameStart;
extern int g_hammer;
extern int g_health;
extern int g_item;
extern int g_map;
extern int g_roundEnd;
extern int g_spear;
extern int g_start;
extern int g_sword;
extern int g_teamLogo;
extern int g_terrain;
extern int g_title;
extern int g_winner;
extern int g_select_transform;
extern int g_countdown;
extern int g_go;
extern int g_round1;
extern int g_round2;
extern int g_result;
extern int g_ko;
extern int g_score;
extern int g_titleSword;

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