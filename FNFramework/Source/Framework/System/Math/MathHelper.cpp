#include "MathHelper.h"

bool MathHelper::Easing::EasingData::Easing(float _delta, float& _value)
{
    bool finish = false;

    if (!IsEasing)
    {
        return finish;
    }

    // 経過時間の更新
    Progress += _delta * AnimationSpeed;

    // 時間の正規化
    float t = Progress / Duration;
    t = std::clamp(t, 0.0f, 1.0f);

    if (Reverse)
    {
        t = 1.0f - t;
    }

    // イージング関数の適用
    float easedValue = ApplyEasingFunction(t);

    // カスタム範囲での値の計算
    _value = ClampValue.x + (ClampValue.y - ClampValue.x) * easedValue;

    // アニメーション終了の判定
    if (Progress >= Duration)
    {
        finish = true;

        if (IsLoop)
        {
            Progress = 0.0f;
        }
        else
        {
            IsEasing = false;
            _value = ClampValue.y;
        }
    }

    return finish;
}

bool MathHelper::Easing::EasingData::Easing(float _delta, float& _value, float _minValue, float _maxValue)
{
    bool finish = false;

    if (!IsEasing)
    {
        return finish;
    }

    // 経過時間の更新
    Progress += _delta * AnimationSpeed;

    // 時間の正規化
    float t = Progress / Duration;
    t = std::clamp(t, 0.0f, 1.0f);

    if (Reverse)
    {
        t = 1.0f - t;
    }

    // イージング関数の適用
    float easedValue = ApplyEasingFunction(t);

    // カスタム範囲での値の計算
    _value = _minValue + (_maxValue - _minValue) * easedValue;

    // アニメーション終了の判定
    if (Progress >= Duration)
    {
        finish = true;

        if (IsLoop)
        {
            Progress = 0.0f;
        }
        else
        {
            IsEasing = false;
            _value = _maxValue;
        }
    }

    return finish;
}

void MathHelper::Easing::EasingData::ImGuiUpdate()
{
    ImGui::Checkbox(U8_TEXT("イージングアニメーションするかどうか"), &IsEasing);

    if (!IsEasing) { return; }

    ImGui::Text("--------Easing Animation--------");
    {
        ImGui::Text(U8_TEXT("進捗度: %.3f"), Progress);

        ImGui::Checkbox(U8_TEXT("ループするかどうか"), &IsLoop);
        ImGui::Checkbox(U8_TEXT("逆再生するかどうか"), &Reverse);

        ImGui::DragFloat("AnimationSpeed", &AnimationSpeed, 1.f, 1.f, 1000.0f);
        ImGui::DragFloat("Duration", &Duration, 0.1f, 0.1f, 30.0f);

        ImGui::DragFloat2("ClampValue", &ClampValue.x, 0.01f, 0.0f, 1.0f);

        // 現在のタイプをインデックスとして取得
        int currentType = static_cast<int>(Type);

        // イージングタイプを選択するコンボボックス
        if (ImGui::Combo("EasingType", &currentType, g_EasingTypeItems.TypeNamePtrs.data(), g_EasingTypeItems.TypeNames.size()))
        {
            // 選択されたタイプを設定
            Type = static_cast<EasingType>(currentType);
        }
    }

}

void MathHelper::Easing::EasingData::Serialize(Json& _json) const
{
    _json[jsonKey::Easing::EasingType.data()] = g_EasingTypeItems.TypeNames[static_cast<int>(Type)];;
    _json[jsonKey::Easing::IsEasing.data()] = IsEasing;
    _json[jsonKey::Easing::IsLoop.data()] = IsLoop;
    _json[jsonKey::Easing::Reverse.data()] = Reverse;
    _json[jsonKey::Easing::AnimationSpeed.data()] = AnimationSpeed;
    _json[jsonKey::Easing::Duration.data()] = Duration;
    _json[jsonKey::Easing::ClampValue.data()] = { ClampValue.x, ClampValue.y };
}

void MathHelper::Easing::EasingData::Deserialize(const Json& _json)
{
    // EasingType の復元
    if (auto it = _json.find(jsonKey::Easing::EasingType.data()); it != _json.end())
    {
        int i = 0;
        for (; i < g_EasingTypeItems.TypeNames.size(); ++i)
        {
            if (it.value() == g_EasingTypeItems.TypeNames[i])
            {
                break;
            }
        }
        Type = static_cast<EasingType>(i);
    }

    // IsEasing の復元
    IsEasing = _json.value(jsonKey::Easing::IsEasing.data(), false);

    // IsLoop の復元
    IsLoop = _json.value(jsonKey::Easing::IsLoop.data(), true);

    // Reverse の復元
    Reverse = _json.value(jsonKey::Easing::Reverse.data(), false);

    // AnimationSpeed の復元
    AnimationSpeed = _json.value(jsonKey::Easing::AnimationSpeed.data(), 1.0f);

    // Duration の復元
    Duration = _json.value(jsonKey::Easing::Duration.data(), 1.0f);

    // ClampValue の復元
    if (auto it = _json.find(jsonKey::Easing::ClampValue.data());
        it != _json.end() && it->is_array() && it->size() >= 2)
    {
        auto clampValue = it.value();
        ClampValue = Math::Vector2{ clampValue[0].get<float>(), clampValue[1].get<float>() };
    }
}
