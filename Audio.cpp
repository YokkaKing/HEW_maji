/*
* ファイル名	Audio.cpp
* タイトル	サウンド
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;
#include"direct3d.h"
#include"shader.h"
#include"sprite.h"
#include"keyboard.h"
#include"audio.h"

//================================================================
//	グローバル変数
//================================================================
static IXAudio2* g_Xaudio{};
static IXAudio2MasteringVoice* g_MasteringVoice{};

int arrow_shuriken = -1;
int button = -1;
int change = -1;
int charge = -1;
int crowd = -1;
int cursorMove = -1;
int damageHammer = -1;
int damageSharp = -1;
int down = -1;
int fade = -1;
int gameStart = -1;
int hammer = -1;
int health = -1;
int item = -1;
int map = -1;
int roundEnd = -1;
int spear = -1;
int start = -1;
int sword = -1;
int teamLogo = -1;
int terrain = -1;
int title = -1;
int winner = -1;

void InitAudio()
{
	// XAudio生成
	XAudio2Create(&g_Xaudio, 0);
	// マスタリングボイス生成
	g_Xaudio->CreateMasteringVoice(&g_MasteringVoice);

	arrow_shuriken = LoadAudio("asset\\Audio\\arrow_shuriken.wav");
	button = LoadAudio("asset\\Audio\\button.wav");
	change = LoadAudio("asset\\Audio\\change.wav");
	charge = LoadAudio("asset\\Audio\\charge.wav");
	crowd = LoadAudio("asset\\Audio\\crowd.wav");
	cursorMove = LoadAudio("asset\\Audio\\cursorMove.wav");
	damageHammer = LoadAudio("asset\\Audio\\damageHammer.wav");
	damageSharp = LoadAudio("asset\\Audio\\damageSharp.wav");
	down = LoadAudio("asset\\Audio\\down.wav");
	fade = LoadAudio("asset\\Audio\\fade.wav");
	gameStart = LoadAudio("asset\\Audio\\gameStart.wav");
	hammer = LoadAudio("asset\\Audio\\hammer.wav");
	health = LoadAudio("asset\\Audio\\health.wav");
	item = -LoadAudio("asset\\Audio\\item.wav");
	map = LoadAudio("asset\\Audio\\map.wav");
	roundEnd = LoadAudio("asset\\Audio\\roundEnd.wav");
	spear = LoadAudio("asset\\Audio\\spear.wav");
	start = LoadAudio("asset\\Audio\\start.wav");
	sword = LoadAudio("asset\\Audio\\sword.wav");
	teamLogo = LoadAudio("asset\\Audio\\teamLogo.wav");
	terrain = LoadAudio("asset\\Audio\\terrain.wav");
	title = LoadAudio("asset\\Audio\\title.wav");
	winner = LoadAudio("asset\\Audio\\winner.wav");
}

void UninitAudio()
{
	g_MasteringVoice->DestroyVoice();
	g_Xaudio->Release();
}

struct AUDIO
{
	IXAudio2SourceVoice*	SourceVoice{};
	BYTE*					SoundData{};

	int						Length{};
	int						PlayLength{};
};

#define AUDIO_MAX 100
static AUDIO g_Audio[AUDIO_MAX]{};

int LoadAudio(const char *FileName)
{
	int index = -1;

	for (int i = 0; i < AUDIO_MAX; i++)
	{
		if (g_Audio[i].SourceVoice == nullptr)
		{
			index = i;
			break;
		}
	}

	if (index == -1)
		return -1;

	// サウンドデータ読込
	WAVEFORMATEX wfx = { 0 };

	{
		HMMIO hmmio = NULL;
		MMIOINFO mmioinfo = { 0 };
		MMCKINFO riffchunkinfo = { 0 };
		MMCKINFO datachunkinfo = { 0 };
		MMCKINFO mmckinfo = { 0 };
		UINT32 buflen;
		LONG readlen;

		hmmio = mmioOpen((LPSTR)FileName, &mmioinfo, MMIO_READ);
		assert(hmmio);

		riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(hmmio, &riffchunkinfo, NULL, MMIO_FINDRIFF);

		mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		if (mmckinfo.cksize >= sizeof(WAVEFORMATEX))
		{
			mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx));
		}
		else
		{
			PCMWAVEFORMAT pcmwf = { 0 };
			mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf));
			memset(&wfx, 0x00, sizeof(wfx));
			memcpy(&wfx, &pcmwf, sizeof(pcmwf));
			wfx.cbSize = 0;
		}
		mmioAscend(hmmio, &mmckinfo, 0);

		datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		buflen = datachunkinfo.cksize;
		g_Audio[index].SoundData = new unsigned char[buflen];
		readlen = mmioRead(hmmio, (HPSTR)g_Audio[index].SoundData, buflen);

		g_Audio[index].Length = readlen;
		g_Audio[index].PlayLength = readlen / wfx.nBlockAlign;

		mmioClose(hmmio, 0);
	}

	// サウンドソース生成
	g_Xaudio->CreateSourceVoice(&g_Audio[index].SourceVoice, &wfx);
	assert(g_Audio[index].SourceVoice);

	return index;
}

void UnloadAudio(int Index)
{
	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->DestroyVoice();

	delete[] g_Audio[Index].SoundData;
	g_Audio[Index].SoundData = nullptr;
}

void PlayAudio(int Index, bool Loop)
{
	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->FlushSourceBuffers();

	// バッファ設定
	XAUDIO2_BUFFER bufinfo;

	memset(&bufinfo, 0x00, sizeof(bufinfo));
	bufinfo.AudioBytes = g_Audio[Index].Length;
	bufinfo.pAudioData = g_Audio[Index].SoundData;
	bufinfo.PlayBegin = 0;
	bufinfo.PlayLength = g_Audio[Index].PlayLength;

	// ループ設定
	if (Loop)
	{
		bufinfo.LoopBegin = 0;
		bufinfo.LoopLength = g_Audio[Index].PlayLength;
		bufinfo.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);

	// 再生
	g_Audio[Index].SourceVoice->Start();
}

void StopAudio(int Index)
{
	if (Index < 0 || Index >= AUDIO_MAX || g_Audio[Index].SourceVoice == nullptr)
	{
		return;
	}

	// 再生を停止
	g_Audio[Index].SourceVoice->Stop();

	// 次回再生時に最初から流れるようにバッファをクリア
	g_Audio[Index].SourceVoice->FlushSourceBuffers();
}