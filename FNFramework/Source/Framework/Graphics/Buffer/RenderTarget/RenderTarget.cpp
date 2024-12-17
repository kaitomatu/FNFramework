#include "RenderTarget.h"

#include "../../../Graphics/Shader/Shader.h"

void RenderTarget::SetRootDescriptorTable(const Shader& shader, int index)
{
    int cbvCount = static_cast<int>(shader.GetCBVCount());
    m_renderTargetTexture.Set(cbvCount + index);
}

bool RenderTarget::Create(int w, int h,
    int mipLevel, int arraySize,
    DXGI_FORMAT colorFormat,
    DXGI_FORMAT depthFormat,
    const Math::Color& clearCol,
    bool constantData)
{
    if (!CreateRTTex(w, h, mipLevel, arraySize, colorFormat, clearCol, constantData))
    {
        FNENG_ASSERT_ERROR("RenderTargetの作成に失敗しました");
        return false;
    }

    if (depthFormat != DXGI_FORMAT_UNKNOWN)
    {
        if (!m_depthStencil.Create(w, h, depthFormat, constantData))
        {
            FNENG_ASSERT_ERROR("DepthStencilBufferの作成に失敗しました");
            return false;
        }
    }

    m_width = w;
    m_height = h;
    m_rtvClearColor = clearCol;

    return true;
}

void RenderTarget::ClearRTV()
{
    const D3D12_CPU_DESCRIPTOR_HANDLE& rtvH = GraphicsDevice::Instance().GetRTVHeap()->GetCPUHandle(m_rtvNumber);
    GraphicsDevice::Instance().ClearRTV(rtvH, m_rtvClearColor);
}

bool RenderTarget::CreateRTTex(int w, int h, int mipLevel, int arraySize, DXGI_FORMAT format,
                               const Math::Vector4& clearCol, bool constantData)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = static_cast<UINT>(w); //	横幅
    desc.Height = static_cast<UINT>(h); //	縦幅
    desc.DepthOrArraySize = static_cast<UINT16>(arraySize); //	配列サイズ
    desc.MipLevels = static_cast<UINT16>(mipLevel); //	ミップマップレベル
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;

    clearValue.Color[0] = clearCol.x;
    clearValue.Color[1] = clearCol.y;
    clearValue.Color[2] = clearCol.z;
    clearValue.Color[3] = clearCol.w;

    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProp.CreationNodeMask = 1;
    heapProp.VisibleNodeMask = 1;

    HRESULT hr = GraphicsDevice::Instance().GetDevice()->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        &clearValue,
        IID_PPV_ARGS(m_pRenderTargetResource.ReleaseAndGetAddressOf())
    );

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("RenderTargetの作成に失敗しました");
        return false;
    }

    int srvNumber = GraphicsDevice::Instance().GetCBVSRVUAVHeap()->CreateSRV(m_pRenderTargetResource.Get(), constantData);
    m_renderTargetTexture.SetSRVNumber(srvNumber);
    m_renderTargetTexture.InitFromD3DResource(m_pRenderTargetResource.Get(), srvNumber);

    // バッファを作成
    // ToDo | FIXME : 要検証だが、GameScene内でcreate関数を呼んでもm_rtvNumberが2しかならない
    // RTVHeapReset関数を見直す必要がある
    //
    // ShaderManager::Init -> GameScene::Init
    //  この間にRTVHeap::Resetが呼ばれてm_nextRegisterNumberが2からスタートするようになっている
    m_rtvNumber = GraphicsDevice::Instance().GetRTVHeap()->CreateRTV(m_renderTargetTexture.WorkBuffer(), format, constantData);

    return true;
}
