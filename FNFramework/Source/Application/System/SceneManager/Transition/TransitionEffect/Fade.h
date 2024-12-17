#pragma once

#include "Application/System/SceneManager/Transition/Transition.h"

class SpriteComponent;

class Fade
    :public BaseTransitionEffect
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Fade(float blackOutTime, float progress)
        : BlackOutTime(blackOutTime)
        , Progress(progress)
    {}
    ~Fade() {}

    //--------------------------------
    // 初期化
    //--------------------------------
    /* @brief 初期化 */
    void Init() override;

private:

    void StartUpdate() override;
    void BlackOutUpdate() override;
    void CompleteUpdate() override;

    std::shared_ptr<SpriteComponent> m_spSpriteComponent;

    const float BlackOutTime;
    const float Progress;
    float m_sin = 0.0f;
    float m_timer = 0.0f;
};
