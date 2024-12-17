﻿#include "PipeLine.h"

#include "../RootSignature/RootSignature.h"

void PipeLine::SetRenderSetting(RootSignature* pRootSignature,
                                const std::vector<InputLayout>& inputLayouts, CullMode cullMode, BlendMode blendMode,
                                PrimitiveTopologyType topologyType)
{
    m_pRootSignature = pRootSignature;
    m_inputLayouts = inputLayouts;
    m_cullMode = cullMode;
    m_blendMode = blendMode;
    m_topologyType = topologyType;
}

void PipeLine::Create(std::vector<ID3DBlob*> pBlobs, const std::vector<DXGI_FORMAT> formats, bool isDepth,
                      bool isDepthMask, DXGI_FORMAT dsvFormat , int rtvCount, bool isWireFrame, bool useInstanceData)
{
    // インプットレイアウトの設定
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayouts;
    SetInputLayout(inputLayouts, m_inputLayouts, useInstanceData);

    // GraphicsPipelineStateの各種設定
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineState = {};

    // 頂点シェーダーをセット
    graphicsPipelineState.VS.pShaderBytecode = pBlobs[0]->GetBufferPointer();
    graphicsPipelineState.VS.BytecodeLength = pBlobs[0]->GetBufferSize();

    // ハルシェーダーをセット
    if (pBlobs[1])
    {
        graphicsPipelineState.HS.pShaderBytecode = pBlobs[1]->GetBufferPointer();
        graphicsPipelineState.HS.BytecodeLength = pBlobs[1]->GetBufferSize();
    }

    // ドメインシェーダーをセット
    if (pBlobs[2])
    {
        graphicsPipelineState.DS.pShaderBytecode = pBlobs[2]->GetBufferPointer();
        graphicsPipelineState.DS.BytecodeLength = pBlobs[2]->GetBufferSize();
    }

    // ジオメトリシェーダーをセット
    if (pBlobs[3])
    {
        graphicsPipelineState.GS.pShaderBytecode = pBlobs[3]->GetBufferPointer();
        graphicsPipelineState.GS.BytecodeLength = pBlobs[3]->GetBufferSize();
    }

    // ピクセルシェーダーをセット
    graphicsPipelineState.PS.pShaderBytecode = pBlobs[4]->GetBufferPointer();
    graphicsPipelineState.PS.BytecodeLength = pBlobs[4]->GetBufferSize();

    graphicsPipelineState.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // カリングモードをセット
    graphicsPipelineState.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(m_cullMode);

    // フィルターモードをセット
    if (isWireFrame)
    {
        graphicsPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; // 中身を塗りつぶさない
    }
    else
    {
        graphicsPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // 中身を塗りつぶす
    }

    // 深度設定をセット
    if (isDepth)
    {
        graphicsPipelineState.RasterizerState.DepthClipEnable = true;
        graphicsPipelineState.DepthStencilState.DepthEnable = true; // 深度バッファを利用する

        if (isDepthMask) // *アルファブレンディングの場合、falseにする
        {
            graphicsPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // 深度値書き込み
        }
        else
        {
            graphicsPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 書き込まない
        }
        graphicsPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // 小さい方を利用
        graphicsPipelineState.DSVFormat = dsvFormat;
    }
    else
    {
        graphicsPipelineState.RasterizerState.DepthClipEnable = false;
        graphicsPipelineState.DepthStencilState.DepthEnable = false;
    }

    graphicsPipelineState.BlendState.AlphaToCoverageEnable = false;

    graphicsPipelineState.BlendState.IndependentBlendEnable = false;

    // ブレンド設定
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
    SetBlendMode(blendDesc, m_blendMode);

    graphicsPipelineState.BlendState.RenderTarget[0] = blendDesc;

    graphicsPipelineState.InputLayout.pInputElementDescs = inputLayouts.data(); // レイアウト先頭アドレス

    int size = static_cast<int>(inputLayouts.size());

    graphicsPipelineState.InputLayout.NumElements = size; // レイアウト配列の要素数

    // ジオメトリシェーダーが設定されていたらPatchに
    graphicsPipelineState.PrimitiveTopologyType = (pBlobs[3] && pBlobs[4])
                                                      ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
                                                      : static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(m_topologyType);

    // RTV数をセット
    graphicsPipelineState.NumRenderTargets = rtvCount;

    // RTVフォーマットのセット
    for (int i = 0; i < rtvCount; ++i)
    {
        graphicsPipelineState.RTVFormats[i] = formats[i];
    }

    graphicsPipelineState.SampleDesc.Count = 1; // サンプリングは 1ピクセルにつき1
    graphicsPipelineState.pRootSignature = m_pRootSignature->GetRootSignature();

    auto hr = GraphicsDevice::Instance().GetDevice()->CreateGraphicsPipelineState(
        &graphicsPipelineState, IID_PPV_ARGS(&m_pPipelineState));

    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("パイプラインステートの作成に失敗しました");
    }
}

void PipeLine::SetInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElements,
                              const std::vector<InputLayout>& inputLayouts, bool useInstanceData)
{
    for (int i = 0; i < static_cast<int>(inputLayouts.size()); ++i)
    {
        if (inputLayouts[i] == InputLayout::POSITION)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
        else if (inputLayouts[i] == InputLayout::TEXCOORD)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
        else if (inputLayouts[i] == InputLayout::NORMAL)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
        else if (inputLayouts[i] == InputLayout::COLOR)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
        else if (inputLayouts[i] == InputLayout::TANGENT)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
        else if (inputLayouts[i] == InputLayout::SKININDEX)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "SKININDEX", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
        else if (inputLayouts[i] == InputLayout::SKINWEIGHT)
        {
            inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
                "SKINWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
                D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            });
        }
    }

    if(useInstanceData)
    {

        inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
            "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
            0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1
            });
        inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
            "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
            D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1
            });
        inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
            "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
            D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1
            });
        inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
            "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
            D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1
            });

        // タイリング / オフセット (float4)
        inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
            "INSTANCE_TILING_OFFSET", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
            D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1
            });

        // カラー情報 (float4)
        inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
            "INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
            D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1
            });
    }

}

void PipeLine::SetBlendMode(D3D12_RENDER_TARGET_BLEND_DESC& blendDesc, BlendMode blendMode)
{
    blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.BlendEnable = true;
    blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;

    switch (blendMode)
    {
    case BlendMode::Add:
        // 加算合成
        blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.SrcBlend = D3D12_BLEND_ONE;
        blendDesc.DestBlend = D3D12_BLEND_ONE;

        blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
        break;

    case BlendMode::Alpha:
        // 半透明ブレンディング
        blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

        blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        break;

    default:
        // ブレンディングを無効化
        blendDesc.BlendEnable = false;
        break;
    }
}
