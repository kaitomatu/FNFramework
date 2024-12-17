#pragma once

#include "../../../BaseComponent.h"

class SpriteComponent;

// todo : クラス名が明確じゃない(操作するキャラクターと混同してしまう)ので、変更する必要がある
/**
* @class CharacterScript
* @brief 1文字では表せれないものを描画するコンポーネント
* @details
*   todo | details :  Start時にSpriteComponentを生成している関係上動的に最大桁数を変更できないので注意。
*                       また今度最大値との差分で追加する処理を検討(動的にSpriteMeshを作成するコストがわかってないため後回し)
*/
class CharacterScript
    : public BaseComponent
{
public:
    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    /**
    * @brief コンストラクタ
    * @param[in] owner - オーナーオブジェクトのポインタ
    * @param[in] name - コンポーネントの名前
    * @param[in] _enableSerialize - シリアライズをするかどうか
    */
    CharacterScript(const std::shared_ptr<GameObject>& owner, const std::string& name, bool _enableSerialize)
        : BaseComponent(owner, name, _enableSerialize, ComponentType::eDefault)
    {
    }

    ~CharacterScript() override
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    // m_checkSampleSourceTextureの設定 / 取得
    void SetCheckSampleSourceTexture(bool check) { m_checkSampleSourceTexture = check; }
    bool GetCheckSampleSourceTexture() const { return m_checkSampleSourceTexture; }

    // 最大の桁数の設定 / 取得
    void SetMaxLen(int len) { m_maxLen = len; }
    int GetMaxLen() const { return m_maxLen; }

    // 先頭から何文字目まで表示するかの設定 / 取得
    void SetDisplayLen(int len) { m_displayLen = len; }
    int GetDisplayLen() const { return m_displayLen; }

    // 文字列の基準のなる座標座標の設定 / 取得
    void SetBasePos(const Math::Vector2& pos) { m_basePos = pos; }
    const Math::Vector2& GetBasePos() const { return m_basePos; }

    // m_spSampleSourceTextureのインデックスの設定 / 取得
    void SetSamplingIdx(int idx) { m_samplingIdx = idx; }
    int GetSamplingIdx() const { return m_samplingIdx; }

    // スプライトテクスチャの追加
    void AddSpriteTexture(const std::shared_ptr<ShaderResourceTexture>& texture)
    {
        m_spSampleSourceTexture.push_back(texture);
    }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief 生成時やシーンの初めに、1度だけ呼びだされる
    * @details この関数は、このコンポーネントをインスタンス化した時に呼び出される
    */
    void Awake() override;

    /**
    * @brief Awakeを経て初期化された後、1度だけ呼びだされる
    * @details
    *	Awakeの後に呼び出される
    *	他のコンポーネントとの依存関係にある初期化処理や
    *	Awakeの段階ではできない初期化を行う
    */
    void Start() override;

    /* @brief 更新 */
    void Update() override;

private:
    /* @brief ImGui更新 */
    void ImGuiUpdate() override;
    void Release() override;

    // サンプリングするテクスチャをそのまま描画する
    void UpdateSampleSourceTexture();

    // このフラグがtrueの場合、m_spSampleSourceTextureの要素を追加順にそのまま有効にする
    bool m_checkSampleSourceTexture = false;

    // todo : 複数種類のテクスチャをサンプリングしたい場合はここを拡張してリスト管理にする必要があるのに注意
    //          拡張する際にstd::pair<int, std::string>でインデックスと名前を管理するようにする
    // スプライトを表示するインデックス : m_spSampleSourceTexture[m_samplingIdx]で利用するテクスチャを指定する
    int m_samplingIdx = 0;
    // 表示される可能性のあるテクスチャ
    std::vector<std::shared_ptr<ShaderResourceTexture>> m_spSampleSourceTexture;

    // 同時に有効化される最大のSpriteComponentの数 : この数分SpriteComponentを生成する
    int m_maxLen = 0;

    // 先頭から何文字目まで表示するか
    int m_displayLen = 0;

    // 文字列の基準のなる座標座標
    Math::Vector2 m_basePos = Math::Vector2::Zero;

private:    /* 各コンポーネントアクセス用 */

    // 描画される一つ一つのスプライト : 各要素は動的に変更されるため、weak_ptrで持つ必要がある
    std::vector<std::weak_ptr<SpriteComponent>> m_spSpriteComponents;

};

