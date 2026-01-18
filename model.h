/*
* ファイル名	model.h
* タイトル	モデル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

#ifndef MODEL_H
#define MODEL_H

//================================================================
//	インクルード
//================================================================
#include<unordered_map>
#include"assimp/cimport.h"
#include"assimp/scene.h"
#include"assimp/postprocess.h"
#include"assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")
#include"d3d11.h"
#include"DirectXMath.h"
using namespace DirectX;
#include"direct3d.h"

struct BoneInfo
{
    XMMATRIX offset;        // inverse bind pose
    XMMATRIX finalTransform;
};

struct MODEL
{
    const aiScene* AiScene = nullptr;

    ID3D11Buffer** VertexBuffer;
    ID3D11Buffer** IndexBuffer;

    std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture;
    std::unordered_map<std::string, UINT> BoneMap;
    std::vector<BoneInfo> Bones;

    float AnimationTimeTicks = 0.0f;

    XMMATRIX GlobalInverse;
    //アニメーションクリップ関連
    bool UseClip = false;
    bool ClipLoop = true;            // true: ループ, false: 1回だけ
    bool ClipJustFinished = false;   // 1回再生クリップが終わったら true
    float ClipStartTicks = 0.0f;
    float ClipEndTicks = 0.0f;
    float ClipLengthTicks = 0.0f;

    // 再生速度 (1.0 = 標準, 0.5 = 半速, 2.0 = 2倍速)
    float PlaySpeed = 1.0f;

    // クリップ再生時に使用する速度（UseClip=true のときは優先して使われる）
    float ClipPlaySpeed = 1.0f;
};


MODEL* ModelLoad(const char* FileName);
void ModelRelease(MODEL* model);

void ModelDraw(MODEL* model);

const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
XMMATRIX InterpolatePosition(float time, const aiNodeAnim* channel);
XMMATRIX InterpolateRotation(float time, const aiNodeAnim* channel);
XMMATRIX InterpolateScale(float time, const aiNodeAnim* channel);

void ReadNodeHierarchy(
    MODEL* model,
    float animTime,
    const aiNode* node,
    const XMMATRIX& parentTransform);

void ModelUpdateAnimation(MODEL* model, float deltaTimeSeconds);

// フレームをもとにクリップを設定 (0 ~ 60 Idle 61 ~ 120 Walkだと　ModelPlayClip(model, 61, 120);でwalkアニメ実装 )
void ModelPlayClip(MODEL* model, int startFrame, int endFrame, float fps = 30.0f, bool loop = true, float speed = 1.0f);
void ModelStopClip(MODEL* model);
//アニメーション全体のスピードを設定
void ModelSetPlaySpeed(MODEL* model, float speed);
// クリップ完了フラグを取得してクリアするユーティリティ
bool ModelConsumeClipFinished(MODEL* model);
#endif 