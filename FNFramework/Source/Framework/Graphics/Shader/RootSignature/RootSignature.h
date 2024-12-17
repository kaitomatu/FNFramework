#pragma once

enum class RangeType
{
    CBV, // 定数バッファビュー
    SRV, // シェーダーリソースビュー
    UAV, // アンオーダードアクセスビュー
};

enum class TextureAddressMode
{
    Wrap, // テクスチャ座標が範囲を超えた場合、繰り返しが行われる
    Clamp, // テクスチャの範囲を超えた場合、最小値 / 最大値に制限(クランプ)される
    /**
    Border,		// テクスチャ座標が範囲を超えた場合、指定された境界色が利用される
    Mirror,		// テクスチャ座標が範囲を超えた場合、反転される
    MirrorOnce,	// テクスチャ座標が範囲を超えた場合、1度だけ反転される。テクスチャの端は境界色が利用される
    */
};

/**
* テクスチャサンプリング時に使用されるフィルタリングモード
*  テクスチャピクセルとPSでサンプリングする際に利用される
*/
enum class D3D12Filter
{
    Point, // 指定されたサンプル位置に最も近いテクスチャのピクセルの値が直接使用される(拡大時 : ドットが目立つ)
    Linear, // 指定されたサンプル位置の周囲の複数のピクセルの値を線形補間を行う		(拡大時 : 滑らか)
    Linear_Comp, // 比較機能を持ったLinearのフィルタリング

    /**
    Anisotropic,	// 異方性フィルタリング : 視点の角度によってテクスチャサンプリングの密度を変える
    */
};

/**
* ピクセルの色取得方法の制御ステート
*/
enum class SamplereState
{
    Linear_Wrap, // 線形補間を行う、テクスチャの繰り返しはあり
    Linear_Clamp, // 線形補間を行う、テクスチャの範囲外は端のピクセルを延長
    Point_Wrap, // 補間を行わない、テクスチャの繰り返しはあり
    Point_Clamp, // 補間を行わない、テクスチャの範囲外は端のピクセルを延長

    Linear_Clamp_Comp, // 比較機能を持ったLinear_Clamp

    Max,
};

class RootSignature
{
public:
    /**
    * @brief 作成
    *
    * @param rangeTypes			- レンジタイプリスト
    * @param cbvCount			- 現在設定されているCBVの数
    */
    void Create(const std::vector<RangeType>& rangeTypes, UINT& cbvCount);

    /**
    * @brief ルートシグネチャの取得
    *
    * @result ルートシグネチャのポインタ
    */
    ID3D12RootSignature* GetRootSignature() { return m_pRootSignature.Get(); }

private:

    static constexpr UINT SamplerCount = static_cast<UINT>(SamplereState::Max);

    /**
    * @brief レンジの作成
    *
    * @param pRange - レンジのポインタ
    * @param type	- レンジタイプ
    * @param count	- 登録数
    */
    void CreateRange(D3D12_DESCRIPTOR_RANGE& pRange, RangeType type, int count);

    /**
    * @brief サンプラーの作成
    *
    * @param pSamplerDesc	- サンプラーデスクのポインタ
    * @param mode			- アドレスモード
    * @param filter			- フィルター
    * @param count			- 使用サンプラー数
    */
    void CreateStaticSampler(D3D12_STATIC_SAMPLER_DESC& pSamplerDesc, TextureAddressMode mode, D3D12Filter filter,
                             int count);

    ComPtr<ID3DBlob> m_pRootBlob = nullptr;
    ComPtr<ID3D12RootSignature> m_pRootSignature = nullptr;
};
