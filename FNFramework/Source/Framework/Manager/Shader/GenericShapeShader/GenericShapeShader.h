#pragma once

/**
* @class VertexShader
* @brief 頂点描画用クラス
*/
class GenericShapeShader
    : public Shader
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    GenericShapeShader() { Init(); }

    ~GenericShapeShader() override
    {
    }

    bool Begin() override;   // 頂点描画

private:
    //--------------------------------
    // その他関数
    //--------------------------------
    /* @brief 初期化 */
    void Init();
};
