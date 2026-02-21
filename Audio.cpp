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

int g_arrow_shuriken = -1;
int g_button = -1;
int g_change = -1;
int g_charge = -1;
int g_crowd = -1;
int g_cursorMove = -1;
int g_damageHammer = -1;
int g_damageSharp = -1;
int g_down = -1;
int g_fade = -1;
int g_gameStart = -1;
int g_hammer = -1;
int g_health = -1;
int g_item = -1;
int g_map = -1;
int g_roundEnd = -1;
int g_spear = -1;
int g_start = -1;
int g_sword = -1;
int g_teamLogo = -1;
int g_terrain = -1;
int g_title = -1;
int g_winner = -1;
int g_select_transform = -1;
int g_countdown = -1;
int g_go = -1;
int g_round1 = -1;
int g_round2 = -1;


void InitAudio()
{
	// XAudio生成
	XAudio2Create(&g_Xaudio, 0);
	// マスタリングボイス生成
	g_Xaudio->CreateMasteringVoice(&g_MasteringVoice);

	g_arrow_shuriken = LoadAudio("asset\\Audio\\arrow_shuriken.wav");
	g_button = LoadAudio("asset\\Audio\\button.wav");
	g_change = LoadAudio("asset\\Audio\\change.wav");
	g_charge = LoadAudio("asset\\Audio\\charge.wav");
	g_crowd = LoadAudio("asset\\Audio\\crowd.wav");
	g_cursorMove = LoadAudio("asset\\Audio\\cursorMove.wav");
	g_damageHammer = LoadAudio("asset\\Audio\\damageHammer.wav");
	g_damageSharp = LoadAudio("asset\\Audio\\damageSharp.wav");
	g_down = LoadAudio("asset\\Audio\\down.wav");
	g_fade = LoadAudio("asset\\Audio\\fade.wav");
	g_gameStart = LoadAudio("asset\\Audio\\gameStart.wav");
	g_hammer = LoadAudio("asset\\Audio\\hammer.wav");
	g_health = LoadAudio("asset\\Audio\\health.wav");
	g_item = LoadAudio("asset\\Audio\\item.wav");
	g_map = LoadAudio("asset\\Audio\\map.wav");
	g_roundEnd = LoadAudio("asset\\Audio\\roundEnd.wav");
	g_spear = LoadAudio("asset\\Audio\\spear.wav");
	g_start = LoadAudio("asset\\Audio\\start.wav");
	g_sword = LoadAudio("asset\\Audio\\sword.wav");
	g_teamLogo = LoadAudio("asset\\Audio\\teamLogo.wav");
	g_terrain = LoadAudio("asset\\Audio\\terrain.wav");
	g_title = LoadAudio("asset\\Audio\\title.wav");
	g_winner = LoadAudio("asset\\Audio\\winner.wav");
	g_select_transform = LoadAudio("asset\\Audio\\select_transform.wav");
	g_countdown = LoadAudio("asset\\Audio\\countdown.wav");
	g_go = LoadAudio("asset\\Audio\\go.wav");
	g_round1 = LoadAudio("asset\\Audio\\round_1.wav");
	g_round2 = LoadAudio("asset\\Audio\\round_2.wav");
	SetAudioVolume(g_hammer, 2.0f);
	SetAudioVolume(g_charge, 2.0f);
	SetAudioVolume(g_sword, 2.0f);
	SetAudioVolume(g_fade, 2.0f);
	SetAudioVolume(g_damageHammer, 2.0f);
	SetAudioVolume(g_damageSharp, 2.0f);
	SetAudioVolume(g_arrow_shuriken, 2.0f);





}

void UninitAudio()
{
	g_MasteringVoice->DestroyVoice();
	g_Xaudio->Release();
}

struct AUDIO
{
	IXAudio2SourceVoice* SourceVoice{};
	BYTE* SoundData{};

	int Length{};
	int PlayLength{};
	int SamplesPerSec{};

	float Volume = 1.0f;   // ★追加：この音IDの音量
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
		g_Audio[index].SamplesPerSec = (int)wfx.nSamplesPerSec;
		mmioClose(hmmio, 0);
	}

	// サウンドソース生成
	g_Xaudio->CreateSourceVoice(&g_Audio[index].SourceVoice, &wfx);
	g_Audio[index].Volume = 1.0f;
	g_Audio[index].SourceVoice->SetVolume(g_Audio[index].Volume);
	assert(g_Audio[index].SourceVoice);

	return index;
}
void SetAudioVolume(int Index, float volume)
{
	if (Index < 0 || Index >= AUDIO_MAX || g_Audio[Index].SourceVoice == nullptr)
		return;

	// clamp（std::max使わない）
	if (volume < 0.0f) volume = 0.0f;
	if (volume > 4.0f) volume = 4.0f;

	g_Audio[Index].Volume = volume;
	g_Audio[Index].SourceVoice->SetVolume(volume);
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
	if (Index < 0 || Index >= AUDIO_MAX || g_Audio[Index].SourceVoice == nullptr)
		return;

	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->FlushSourceBuffers();

	XAUDIO2_BUFFER bufinfo;
	memset(&bufinfo, 0x00, sizeof(bufinfo));
	bufinfo.AudioBytes = g_Audio[Index].Length;
	bufinfo.pAudioData = g_Audio[Index].SoundData;
	bufinfo.PlayBegin = 0;
	bufinfo.PlayLength = g_Audio[Index].PlayLength;

	if (Loop)
	{
		bufinfo.LoopBegin = 0;
		bufinfo.LoopLength = g_Audio[Index].PlayLength;
		bufinfo.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);

	// ★追加：この音IDの音量を反映
	g_Audio[Index].SourceVoice->SetVolume(g_Audio[Index].Volume);

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
void PlayAudioLoopSection(int Index, float loopBeginSec, float loopEndSec)
{
    if (Index < 0 || Index >= AUDIO_MAX || g_Audio[Index].SourceVoice == nullptr)

        return;

    g_Audio[Index].SourceVoice->Stop();
    g_Audio[Index].SourceVoice->FlushSourceBuffers();

    XAUDIO2_BUFFER bufinfo;
    memset(&bufinfo, 0, sizeof(bufinfo));

    bufinfo.AudioBytes = g_Audio[Index].Length;
    bufinfo.pAudioData = g_Audio[Index].SoundData;

    // 全体は先頭から再生
    bufinfo.PlayBegin  = 0;
    bufinfo.PlayLength = g_Audio[Index].PlayLength;

    // 秒→サンプル（フレーム）変換
    unsigned int loopBegin = (unsigned int)(loopBeginSec * (float)g_Audio[Index].SamplesPerSec);
    unsigned int loopEnd   = (unsigned int)(loopEndSec   * (float)g_Audio[Index].SamplesPerSec);

    // 範囲チェック
    if (loopBegin >= (unsigned int)g_Audio[Index].PlayLength)
    {
        // ループ無しで再生だけ
		g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);
		g_Audio[Index].SourceVoice->SetVolume(g_Audio[Index].Volume); // ★追加
		g_Audio[Index].SourceVoice->Start();
        return;
    }
    if (loopEnd > (unsigned int)g_Audio[Index].PlayLength)
        loopEnd = (unsigned int)g_Audio[Index].PlayLength;

    if (loopEnd <= loopBegin + 1)
    {
        // ループ区間が短すぎる → ループ無しで再生
		g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);
		g_Audio[Index].SourceVoice->SetVolume(g_Audio[Index].Volume); // ★追加
		g_Audio[Index].SourceVoice->Start();
        return;
    }

    bufinfo.LoopBegin  = loopBegin;                 // 例：2秒
    bufinfo.LoopLength = (loopEnd - loopBegin);     // 例：1秒(2→3)
    bufinfo.LoopCount  = XAUDIO2_LOOP_INFINITE;

    g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);
    g_Audio[Index].SourceVoice->Start();
}
