#include "effect_anim.h"
#include <algorithm>
#include <cmath>


XMMATRIX EffectAnim::AiToXM_Local(const aiMatrix4x4& m)
{
    return XMMatrixSet(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    );
}

void EffectAnim::Bind(MODEL* model, const char* targetNodeName)
{
    m_model = model;
    m_hasAnim = false;
    m_targetNode.clear();
    m_delta = XMMatrixIdentity();
    m_bindGlobal = XMMatrixIdentity();
    m_currentGlobal = XMMatrixIdentity();

    if (!m_model || !m_model->AiScene || !m_model->AiScene->HasAnimations())
        return;

    const aiAnimation* anim = m_model->AiScene->mAnimations[0];
    if (!anim || anim->mNumChannels == 0)
        return;

    m_ticksPerSecond = (anim->mTicksPerSecond != 0.0) ? (float)anim->mTicksPerSecond : 25.0f;
    m_durationTicks = (float)anim->mDuration;

    if (targetNodeName && targetNodeName[0] != '\0')
    {
        m_targetNode = targetNodeName;
    }
    else
    {

        m_targetNode = anim->mChannels[0]->mNodeName.C_Str();
    }

    m_hasAnim = true;


    m_bindGlobal = EvalTargetGlobal(0.0f);
    m_currentGlobal = m_bindGlobal;
    m_delta = XMMatrixIdentity();
}

void EffectAnim::PlayFrames(int startFrame, int endFrame, float fps, bool loop, float speed)
{
    if (!m_hasAnim || !m_model || !m_model->AiScene) return;
    if (fps <= 0.0f) fps = 30.0f;

    const aiAnimation* anim = m_model->AiScene->mAnimations[0];


    float startSec = (float)startFrame / fps;
    float endSec = (float)endFrame / fps;

    m_startTicks = startSec * m_ticksPerSecond;
    m_endTicks = endSec * m_ticksPerSecond;


    m_startTicks = ClampT(m_startTicks, 0.0f, m_durationTicks);
    m_endTicks = ClampT(m_endTicks, 0.0f, m_durationTicks);
    if (m_endTicks < m_startTicks) std::swap(m_endTicks, m_startTicks);

    m_useClip = true;
    m_loop = loop;
    m_speed = speed;
    m_finished = false;
    m_accumTicks = 0.0f;


    m_bindGlobal = EvalTargetGlobal(m_startTicks);
    m_currentGlobal = m_bindGlobal;
    m_delta = XMMatrixIdentity();
}

void EffectAnim::Stop()
{
    m_useClip = false;
    m_finished = false;
    m_accumTicks = 0.0f;
    m_delta = XMMatrixIdentity();
}

bool EffectAnim::ConsumeFinished()
{
    bool was = m_finished;
    m_finished = false;
    return was;
}

void EffectAnim::Update(float deltaSeconds)
{
    if (!m_hasAnim || !m_model || !m_model->AiScene) return;

    const aiAnimation* anim = m_model->AiScene->mAnimations[0];
    if (!anim) return;

    float deltaTicks = deltaSeconds * m_ticksPerSecond * m_speed;
    m_accumTicks += deltaTicks;

    float timeTicks = 0.0f;

    if (m_useClip)
    {
        float clipLen = (m_endTicks - m_startTicks);
        if (clipLen <= 0.0f)
        {
            timeTicks = m_startTicks;
        }
        else if (m_loop)
        {
            float t = std::fmod(m_accumTicks, clipLen);
            if (t < 0.0f) t += clipLen;
            timeTicks = m_startTicks + t;
        }
        else
        {
            if (m_accumTicks >= clipLen)
            {
                timeTicks = m_endTicks;
                m_finished = true;
            }
            else
            {
                timeTicks = m_startTicks + m_accumTicks;
            }
        }
    }
    else
    {

        if (m_durationTicks > 0.0f)
        {
            float t = std::fmod(m_accumTicks, m_durationTicks);
            if (t < 0.0f) t += m_durationTicks;
            timeTicks = t;
        }
    }

    m_currentGlobal = EvalTargetGlobal(timeTicks);


    XMMATRIX invBind = XMMatrixInverse(nullptr, m_bindGlobal);
    m_delta = m_currentGlobal * invBind;
}

XMMATRIX EffectAnim::EvalTargetGlobal(float timeTicks) const
{
    if (!m_model || !m_model->AiScene || !m_model->AiScene->HasAnimations())
        return XMMatrixIdentity();

    const aiAnimation* anim = m_model->AiScene->mAnimations[0];
    const aiNode* root = m_model->AiScene->mRootNode;
    if (!anim || !root) return XMMatrixIdentity();

    XMMATRIX outTarget = XMMatrixIdentity();
    bool found = false;

    EvalNodeRecursive(root, anim, timeTicks, XMMatrixIdentity(), outTarget, found);


    return found ? outTarget : XMMatrixIdentity();
}

void EffectAnim::EvalNodeRecursive(
    const aiNode* node,
    const aiAnimation* anim,
    float timeTicks,
    const XMMATRIX& parent,
    XMMATRIX& outTarget,
    bool& found
) const
{
    if (!node) return;


    XMMATRIX nodeTransform = AiToXM_Local(node->mTransformation);


    const aiNodeAnim* channel = FindNodeAnim(anim, node->mName.C_Str());
    if (channel)
    {
     
        aiVector3D baseS, baseT;
        aiQuaternion baseR;
        node->mTransformation.Decompose(baseS, baseR, baseT);

        XMMATRIX T =
            (channel->mNumPositionKeys > 0)
            ? InterpolatePosition(timeTicks, channel)
            : XMMatrixTranslation(baseT.x, baseT.y, baseT.z);

        XMMATRIX R =
            (channel->mNumRotationKeys > 0)
            ? InterpolateRotation(timeTicks, channel)
            : XMMatrixRotationQuaternion(XMVectorSet(baseR.x, baseR.y, baseR.z, baseR.w));

        XMMATRIX S =
            (channel->mNumScalingKeys > 0)
            ? InterpolateScale(timeTicks, channel)
            : XMMatrixScaling(baseS.x, baseS.y, baseS.z);

        nodeTransform = S * R * T;
    }

 
    XMMATRIX globalTransform = nodeTransform * parent;

    if (!found && m_targetNode == node->mName.C_Str())
    {
        outTarget = globalTransform;
        found = true;
       
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        EvalNodeRecursive(node->mChildren[i], anim, timeTicks, globalTransform, outTarget, found);
        if (found) return;
    }
}
