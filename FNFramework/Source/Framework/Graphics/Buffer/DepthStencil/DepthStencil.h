#pragma once

/*====================================
* DepthStencilBuffer & View作成クラス
====================================*/
class DepthStencil
{
public:
    /**
    * @brief 深度テクスチャ作成
    *
    * @param w					 - 横幅
    * @param h					 - 縦幅
    * @param format			     - バッファのフォーマット
    * @param constantData		 - 定数データかどうか
    * @remark
    * ・DXGI_FORMAT_R16_TYPELESS   - 低解像度用
    * ・DXGI_FORMAT_R32_TYPELESS   - 高解像度用
    * ・DXGI_FORMAT_R24G8_TYPELESS - Depth & Stencil
    *
    * @result 作成出来たらtrue
    */
    bool Create(int w, int h, DXGI_FORMAT format = DXGI_FORMAT_R32_TYPELESS, bool constantData = false);

    /* @brief 深度バッファのデータを初期化 */
    void ClearDSV();

    /**
    * @brief DSV番号を取得
    * @result 登録したDSV番号
    */
    UINT GetDSVNumber() const { return m_dsvNumber; }

    bool IsCreate() const
    {
        return m_pBuffer;
    }

private:
    ComPtr<ID3D12Resource> m_pBuffer = nullptr;

    UINT m_dsvNumber = 0;
};
