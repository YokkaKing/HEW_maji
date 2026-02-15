
#ifndef RESULT_UI2_H
#define RESULT_UI2_H


#include"direct3d.h"
#include <utility> // std::swap




class RESULT_UI_2P
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
class CHALLENGE_UI_2P
{
public:
    XMFLOAT4 col[8];
    XMFLOAT2 pos[8];
    XMFLOAT2 size[8];
    bool ui_used;
    bool isEnd;
};
class SCORE_UI_2P
{
public:
    XMFLOAT4 col[7];
    XMFLOAT2 pos[7];
    XMFLOAT2 size[7];
    bool ui_used;
    bool isEnd;
    int score;
};

void Result2_Ui_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void Result2_Ui_Finalize();
void Result2_Ui_Update();
void Result2_Ui_Draw();
           
void Result2_SetupResultUi(bool flg, int winner);
void Result2_SetResultIsEnd(bool flg);
void Result2_SetChallengeUi(bool flg);
void Result2_SetChallengeIsEnd(bool flg);
void Result2_SetScoreUi(bool flg);
void Result2_SetScoreIsEnd(bool flg);
bool Result2_IsResultAttackReady();
bool Result2_IsChallengeAttackReady();
bool Result2_IsScoreAttackReady();
#endif // RESULT_UI_H