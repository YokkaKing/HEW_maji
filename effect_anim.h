#pragma once
#include <string>
#include "DirectXMath.h"
#include "assimp/scene.h"
#include "model.h"
using namespace DirectX;
template<typename T>
T ClampT(T v, T lo, T hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}
class EffectAnim
{
public:
    void Bind(MODEL* model, const char* targetNodeName = nullptr);

  
    void PlayFrames(int startFrame, int endFrame, float fps = 30.0f, bool loop = false, float speed = 1.0f);

    void Stop();
    void Update(float deltaSeconds);


    XMMATRIX GetDeltaMatrix() const { return m_delta; }


    bool ConsumeFinished();

private:
    MODEL* m_model = nullptr;
    std::string m_targetNode;
    bool m_hasAnim = false;

    float m_ticksPerSecond = 25.0f;
    float m_durationTicks = 0.0f;

    // clip control
    bool  m_useClip = false;
    bool  m_loop = false;
    bool  m_finished = false;
    float m_speed = 1.0f;

    float m_accumTicks = 0.0f;
    float m_startTicks = 0.0f;
    float m_endTicks = 0.0f;


    XMMATRIX m_bindGlobal = XMMatrixIdentity();
    XMMATRIX m_currentGlobal = XMMatrixIdentity();
    XMMATRIX m_delta = XMMatrixIdentity();

private:
    static XMMATRIX AiToXM_Local(const aiMatrix4x4& m);

    XMMATRIX EvalTargetGlobal(float timeTicks) const;

    void EvalNodeRecursive(
        const aiNode* node,
        const aiAnimation* anim,
        float timeTicks,
        const XMMATRIX& parent,
        XMMATRIX& outTarget,
        bool& found
    ) const;
};
