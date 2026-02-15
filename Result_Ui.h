
#ifndef RESULT_UI_H
#define RESULT_UI_H


#include"direct3d.h"
#include <utility> // std::swap



class RESULT_UI
{
public:
    XMFLOAT4 col;
    XMFLOAT2 pos[2];
    XMFLOAT2 size[2];
    bool ui_used;
    bool win_used;
    bool lose_used;
    bool isEnd;
};

class CHALLENGE_UI
{
public:
    XMFLOAT4 col[8];
    XMFLOAT2 pos[8];
    XMFLOAT2 size[8];
    bool ui_used;
    bool isEnd;

};

class SCORE_UI
{
public:
    XMFLOAT4 col[7];
    XMFLOAT2 pos[7];
    XMFLOAT2 size[7];
    bool ui_used;
    bool isEnd;
    int score;

};


void Result_Ui_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Result_Ui_Finalize();
void Result_Ui_Update();
void Result_Ui_Draw();

void SetupResultUi(bool flg, int winner);

void SetResultIsEnd(bool flg);
void SetChallengeUi(bool flg);
void SetChallengeIsEnd(bool flg);
void SetScoreUi(bool flg);
void SetScoreIsEnd(bool flg);
bool ResultUi_IsResultAttackReady();
bool ResultUi_IsChallengeAttackReady();
bool ResultUi_IsScoreAttackReady();
#endif // RESULT_UI_H