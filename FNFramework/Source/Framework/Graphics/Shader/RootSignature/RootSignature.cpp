#include "RootSignature.h"

void RootSignature::Create(const std::vector<RangeType>& rangeTypes, UINT& cbvCount)
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    const int rangeCount = static_cast<int>(rangeTypes.size());

    // レンジ数分だけルートパラメータ、レンジを生成
    std::vector<D3D12_ROOT_PARAMETER> rootParams(rangeCount);
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges(rangeCount);

    // SRVの数だけSamplerDescを生成
    int samplerCount = 0;
    bool bSampler = false;
    int uavCount = 0;

    for (int i = 0; i < rangeCount; ++i)
    {
        switch (rangeTypes[i])
        {
        case RangeType::CBV:
            CreateRange(ranges[i], RangeType::CBV, cbvCount);
            rootParams[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[i].DescriptorTable.pDescriptorRanges = &ranges[i];
            rootParams[i].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            ++cbvCount;
            break;

        case RangeType::SRV:
            CreateRange(ranges[i], RangeType::SRV, samplerCount);
            rootParams[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[i].DescriptorTable.pDescriptorRanges = &ranges[i];
            rootParams[i].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            ++samplerCount;
            bSampler = true;
            break;

        case RangeType::UAV:
            CreateRange(ranges[i], RangeType::UAV, uavCount);
            rootParams[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[i].DescriptorTable.pDescriptorRanges = &ranges[i];
            rootParams[i].DescriptorTable.NumDescriptorRanges = 1;
            rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            ++uavCount;
            break;
        default:
            break;
        }
    }

    std::array<D3D12_STATIC_SAMPLER_DESC, SamplerCount> pStaticSamplerDescs = {};

    if (bSampler)
    {
        CreateStaticSampler(pStaticSamplerDescs[static_cast<int>(SamplereState::Linear_Wrap)], TextureAddressMode::Wrap,
                            D3D12Filter::Linear, 0);
        CreateStaticSampler(pStaticSamplerDescs[static_cast<int>(SamplereState::Linear_Clamp)],
                            TextureAddressMode::Clamp, D3D12Filter::Linear, 1);
        CreateStaticSampler(pStaticSamplerDescs[static_cast<int>(SamplereState::Point_Wrap)], TextureAddressMode::Wrap,
                            D3D12Filter::Point, 2);
        CreateStaticSampler(pStaticSamplerDescs[static_cast<int>(SamplereState::Point_Clamp)],
                            TextureAddressMode::Clamp, D3D12Filter::Point, 3);
        // 比較機能付きLinear_Clamp - register(s10)に設定しておく
        CreateStaticSampler(pStaticSamplerDescs[static_cast<int>(SamplereState::Linear_Clamp_Comp)],
                            TextureAddressMode::Clamp, D3D12Filter::Linear_Comp, 10);
    }

    rootSignatureDesc.pStaticSamplers = bSampler ? pStaticSamplerDescs.data() : nullptr;
    rootSignatureDesc.NumStaticSamplers = bSampler ? SamplerCount : 0; // サンプラーが設定されていれば最大数
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.pParameters = rootParams.data();
    rootSignatureDesc.NumParameters = static_cast<int>(rangeTypes.size());

    ID3DBlob* pErrorBlob = nullptr;
    // ルートシグネチャ初期化
    auto hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &m_pRootBlob,
                                          &pErrorBlob);
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("ルートシグネチャ初期化失敗");
        return;
    }
    // ルートシグネチャ作成
    hr = GraphicsDevice::Instance().GetDevice()->CreateRootSignature(0, m_pRootBlob->GetBufferPointer(),
                                                             m_pRootBlob->GetBufferSize(),
                                                             IID_PPV_ARGS(&m_pRootSignature));
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("ルートシグネチャ作成失敗");
    }
}

void RootSignature::CreateRange(D3D12_DESCRIPTOR_RANGE& pRange, RangeType type, int count)
{
    switch (type)
    {
    case RangeType::CBV:
        pRange = {};
        pRange.NumDescriptors = 1;
        pRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        pRange.BaseShaderRegister = count;
        pRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        break;
    case RangeType::SRV:
        pRange = {};
        pRange.NumDescriptors = 1;
        pRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        pRange.BaseShaderRegister = count;
        pRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        break;
    case RangeType::UAV:
        pRange = {};
        pRange.NumDescriptors = 1;
        pRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        pRange.BaseShaderRegister = count;
        pRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        break;
    default:
        break;
    }
}

void RootSignature::CreateStaticSampler(
    D3D12_STATIC_SAMPLER_DESC& pSamplerDesc,
    TextureAddressMode mode,
    D3D12Filter filter,
    int registerIndex)
{
    pSamplerDesc = {};

    // アドレスモード設定
    D3D12_TEXTURE_ADDRESS_MODE addressMode =
        (mode == TextureAddressMode::Wrap) ? D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    // フィルター設定
    D3D12_FILTER samplingFilter = {};
    switch (filter)
    {
    case D3D12Filter::Point:
        samplingFilter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        pSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        pSamplerDesc.MaxAnisotropy = 16;
        break;
    case D3D12Filter::Linear:
        samplingFilter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        pSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        pSamplerDesc.MaxAnisotropy = 16;
        break;
    case D3D12Filter::Linear_Comp:
        samplingFilter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        pSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        pSamplerDesc.MaxAnisotropy = 1;
        break;
    default:
        break;
    }

    // サンプラの設定
    pSamplerDesc.AddressU = addressMode;
    pSamplerDesc.AddressV = addressMode;
    pSamplerDesc.AddressW = addressMode;
    pSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    pSamplerDesc.Filter = samplingFilter;
    pSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    pSamplerDesc.MinLOD = 0.0f;
    pSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    pSamplerDesc.ShaderRegister = registerIndex;
}
