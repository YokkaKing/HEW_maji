/*
* ファイル名	component.h
* タイトル	コンポーネント
* 作成者		久保木幹太
* 作成日		11月12日
* 更新日		11月12日
*/

#ifndef COMPONENT_H
#define COMPONENT_H

// 前方宣言（循環参照を防ぐ）
class GameObject;

//=======================================
// 基底クラス
//=======================================
class Component
{
public:
    GameObject* owner = nullptr; // 自分を所有するGameObjectへのポインタ

    virtual void Update() {}
    virtual ~Component() {}
};

#endif // COMPONENT_H