#pragma once

namespace RenderingData
{
    namespace Model
    {
        // 描画タイプ
        enum class RenderType
        {
            eShadow = 1 << 0, // シャドウマップ描画
            eLit = 1 << 1, // ライティング描画
            eUnlit = 1 << 2, // ライティングなし描画
        };

        namespace ImGui
        {
            // ※ 変更するたびに手動で増やしてください ※
            // ここを変更しないとModelComponent::ImGui内で描画法を設定できなくなります
            static constexpr int RenderTypeCount = 3;
        }

        // モデルの描画タイプを比較する
        static bool HasRenderTypeFlag(
            UINT _renderType,
            RenderType flag)
        {
            return _renderType & static_cast<UINT>(flag);
        }
    }

    namespace Sprite
    {
        // 描画時に利用するスプライトのデータ
        struct Sprite
        {
            enum Order
            {
                eBackGround = 0, // 背景
                eDefault = 100,  // デフォルト
                eFront = 200,    // 前面
            };

            Sprite() = default;
            Sprite(
                const std::shared_ptr<SpriteMesh>& _vertex, // 描画用頂点配列
                const Math::Vector4& _pixelPos,             // x, y, w, h座標(ピクセル)
                const std::shared_ptr<ShaderResourceTexture>& _maskTex = nullptr, // マスクテクスチャ
                const std::shared_ptr<Math::Rectangle>& _srcRect = nullptr,        // 元画像のRECT : nullptr で上の x, y, h ,w で指定した範囲が描画される
                const Math::Color& _color = Color::White,   // 色(RGBA) : nullptr で色はセットしない(前回の描画時の色が使用される)
                const Math::Matrix& _worldMatrix = Math::Matrix::Identity, // ワールド行列
                const Math::Vector2& _pivot = { 0.5, 0.5f },
                int _order = Order::eDefault,
                const Math::Vector2& _tilling = { 1.0f, 1.0f },
                const Math::Vector2& _offset = { 0.0f, 0.0f })
                : Vertex(_vertex)
                , PixelPos(_pixelPos), SrcRect(_srcRect)
                , MaskTex(_maskTex)
                , Color(_color)
                , WorldMatrix(_worldMatrix)
                , Pivot(_pivot)
                , Order(_order)
                , Tilling(_tilling)
                , Offset(_offset)
            {
            }

            std::shared_ptr<SpriteMesh> Vertex;
            Math::Vector4               PixelPos;
            std::shared_ptr<ShaderResourceTexture> MaskTex;
            std::shared_ptr<Math::Rectangle> SrcRect;
            Math::Color                 Color = Color::White;
            Math::Matrix                WorldMatrix;
            Math::Vector2               Pivot = { 0.5, 0.5f };
            int                         Order = Order::eDefault; // ソートオーダー
            Math::Vector2               Tilling = { 1.0f, 1.0f };
            Math::Vector2               Offset = { 0.0f, 0.0f };
        };
    }
}

/**
* @class Renderer
* @brief アセットデータを描画するクラス
* @details
*/
class Renderer
    : public utl::Singleton<Renderer>
{
    friend class utl::Singleton<Renderer>;

public:

    struct InstancedRenderEntry
    {
        // モデル一つずつのデータ
        std::vector<InstanceData> InstanceDataList;
        std::vector<ModelWork*> ModelWorkList;
    };

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    //-----------------------
    // 描画用データの追加
    //-----------------------

    void AddRenderingModelData(
        const std::shared_ptr<ModelWork>& spModelWork,
        const Math::Matrix& worldMatrix,
        UINT renderType,
        const Math::Vector4& color,
        const Math::Vector2& tilling,
        const Math::Vector2& offset)
    {
        if (!spModelWork)
        {
            return;
        }

        // インスタンスデータを作成
        InstanceData instanceData = {};
        instanceData.mWorld = worldMatrix;
        instanceData.TilingOffset = Math::Vector4{
            tilling.x,
            tilling.y,
            offset.x,
            offset.y };
        instanceData.Color = color;

        if (IsRenderTypeLit(renderType))
        {
            // GBuffer描画用データを追加
            auto& instancedRenderEntry = m_GBufferRenderData[spModelWork->GetModelData()];
            instancedRenderEntry.InstanceDataList.emplace_back(instanceData);
            instancedRenderEntry.ModelWorkList.emplace_back(spModelWork.get());
        }

        if (IsRenderTypeShadow(renderType))
        {
            // シャドウマップ描画用データを追加
            auto& instancedRenderEntry = m_ShadowMapRenderData[spModelWork->GetModelData()];
            instancedRenderEntry.InstanceDataList.emplace_back(instanceData);
            instancedRenderEntry.ModelWorkList.emplace_back(spModelWork.get());
        }
    }

    void AddRenderingSpriteData(const RenderingData::Sprite::Sprite& _spritData)
    {
        // ソートする基準：_order でソート
        auto it = std::lower_bound(m_spriteList.begin(), m_spriteList.end(), _spritData.Order,
            [](const RenderingData::Sprite::Sprite& data, int order) {
                return data.Order < order; // オーダーが小さい順にソート
            });

        // 指定した位置に挿入
        m_spriteList.emplace(it, _spritData);
    }

    //--------------------------------
    // その他関数
    //--------------------------------

    /*
    * @brief 描画処理
    * @details
    *	モデルデータを描画する
    *	外部からこれを呼び出すことで描画処理を行う
    */
    void Render();

private:
    /* @brief モデル描画 */
    void DrawModel();

    /* @brief スプライト描画 */
    void DrawSprite();

    /* @brief リストのデータを削除する */
    void ClearList();

    // キーを ModelData にし、値に InstancedRenderEntry を持つマップ
    std::unordered_map<std::shared_ptr<ModelData>, InstancedRenderEntry> m_GBufferRenderData;
    std::unordered_map<std::shared_ptr<ModelData>, InstancedRenderEntry> m_ShadowMapRenderData;

    // スプライトリスト
    std::list<RenderingData::Sprite::Sprite> m_spriteList;

    //--------------------------------
    // 描画タイプの設定
    //--------------------------------
    bool IsRenderTypeLit(UINT _renderType)
    {
        return HasRenderTypeFlag(_renderType, RenderingData::Model::RenderType::eLit);
    }

    bool IsRenderTypeShadow(UINT _renderType)
    {
        return HasRenderTypeFlag(_renderType, RenderingData::Model::RenderType::eShadow);
    }

    bool IsRenderTypeUnlit(UINT _renderType)
    {
        return HasRenderTypeFlag(_renderType, RenderingData::Model::RenderType::eUnlit);
    }

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    Renderer()
    {
    }

    ~Renderer() override
    {
    }
};
