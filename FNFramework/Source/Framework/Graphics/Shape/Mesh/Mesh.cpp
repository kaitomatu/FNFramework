#include "Mesh.h"

void Material::SetTextures(const std::shared_ptr<ShaderResourceTexture>& spBaseColTex,
    const std::shared_ptr<ShaderResourceTexture>& spMtRfColTex,
    const std::shared_ptr<ShaderResourceTexture>& spEmiColTex,
    const std::shared_ptr<ShaderResourceTexture>& spNmlColTex)
{
    spBaseColorTex = spBaseColTex;
    spMetallicRoughnessTex = spMtRfColTex;
    spEmissiveTex = spEmiColTex;
    spNormalTex = spNmlColTex;

    if (spMtRfColTex)
    {
        Metallic = 1.0f;
        Roughness = 1.0f;
    }
}

void Material::SetTextures(const std::string& fileDir, const std::string& baseColName, const std::string& mtRfColName,
    const std::string& emiColName, const std::string& nmlColName)
{
    std::shared_ptr<ShaderResourceTexture>	BaseColorTex = nullptr;
    std::shared_ptr<ShaderResourceTexture>	MetallicRoughnessTex = nullptr;
    std::shared_ptr<ShaderResourceTexture>	EmissiveTex = nullptr;
    std::shared_ptr<ShaderResourceTexture>	NormalTex = nullptr;

    // 基本色テクスチャ
    if (!baseColName.empty())
    {
        BaseColorTex = AssetManager::Instance().GetTexture(fileDir + baseColName);
    }

    // ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
    // 金属性・粗さマップ
    if (!mtRfColName.empty())
    {
        MetallicRoughnessTex = AssetManager::Instance().GetTexture(fileDir + mtRfColName);
    }

    // ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
    // 自己発光・エミッシブマップ
    if (!emiColName.empty())
    {
        EmissiveTex = AssetManager::Instance().GetTexture(fileDir + emiColName);
    }

    // ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
    // 法線マップ
    if (!nmlColName.empty())
    {
        NormalTex = AssetManager::Instance().GetTexture(fileDir + nmlColName);
    }

    SetTextures(BaseColorTex, MetallicRoughnessTex, EmissiveTex, NormalTex);
}

void Mesh::Create(const std::vector<MeshVertex>& vertices,
    const std::vector<MeshFace>& faces,
    const std::vector<MeshSubset>& subsets,
    bool isSkinMesh)
{
    if (vertices.empty())
    {
        FNENG_ASSERT_ERROR("頂点が一つもありません");
        return;
    }

    //===============================
    // サブセットの作成
    //===============================
    m_subsets = subsets;

    //===============================
    // バッファ / 頂点の作成
    //===============================
    CreateVertexBuffers(vertices);

    // 作成されたバッファを元に境界データを作成する
    DirectX::BoundingBox::CreateFromPoints(m_boundingBox, m_positions.size(), m_positions.data(), sizeof(Math::Vector3));
    DirectX::BoundingSphere::CreateFromPoints(m_boundingSphere, m_positions.size(), m_positions.data(), sizeof(Math::Vector3));

    //===============================
    // バッファ / インデックスの作成
    //===============================
    CreateIndexBufferAndFaceData(faces);

    // スキンメッシュかどうか
    m_isSkinMesh = isSkinMesh;

    //===============================
    // インスタンスバッファの初期化
    //===============================
    // 初期インスタンス数を設定（必要に応じて調整）
    UINT initialInstanceCount = 100; // 初期で100インスタンス分のバッファを確保
    if (!InitializeInstanceBuffer(initialInstanceCount))
    {
        FNENG_ASSERT_ERROR("インスタンスバッファの初期化に失敗しました");
        return;
    }
}

void Mesh::CreateVertexBuffers(const std::vector<MeshVertex>& _vertices)
{
    // バッファ / ビューの作成
    CreateVertexBuffer(sizeof(MeshVertex), static_cast<UINT>(_vertices.size()));

    // 頂点情報格納
    UpdateBuffer(_vertices);
}

void Mesh::UpdateBuffer(const std::vector<MeshVertex>& srcDatas)
{
    // 座標などを保存する
    m_positions.clear();
    m_positions.resize(srcDatas.size());
    for (size_t i = 0; i < m_positions.size(); ++i)
    {
        m_positions[i] = srcDatas[i].Position;
    }
    // 頂点バッファのサイズを超えている場合はエラーを出して終了
    if (sizeof(MeshVertex) * srcDatas.size() > m_vbView.SizeInBytes)
    {
        // バッファを再確保
        Vertices::CreateVertexBuffer(sizeof(MeshVertex), static_cast<UINT>(srcDatas.size()));

        FNENG_ASSERT_ERROR("更新データが頂点バッファのサイズを超えています");
        return;
    }

    m_vertexCount = static_cast<UINT>(srcDatas.size());

    // 頂点バッファに情報を描き込む
    MeshVertex* vbMap = nullptr;
    {
        auto hr = m_pVBuffer->Map(0, nullptr, (void**)&vbMap);

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR("頂点バッファのマップに失敗しました");
            return;
        }

        std::copy(std::begin(srcDatas), std::end(srcDatas), vbMap); // m_verticesの中身をvbMapにコピーする
        m_pVBuffer->Unmap(0, nullptr);
    }
}

bool Mesh::InitializeInstanceBuffer(UINT initialInstanceCount)
{
    UINT bufferSize = initialInstanceCount * sizeof(InstanceData);
    m_currentInstanceCapacity = initialInstanceCount;

    CreateInstanceBufferResource(bufferSize);

    // 初期化データをゼロクリア
    InstanceData* pMappedData = nullptr;
    HRESULT hr = m_pInstanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData));
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("インスタンスバッファのマップに失敗しました");
        return false;
    }
    memset(pMappedData, 0, bufferSize);
    m_pInstanceBuffer->Unmap(0, nullptr);

    // インスタンスバッファビューの設定
    m_instanceBufferView.BufferLocation = m_pInstanceBuffer->GetGPUVirtualAddress();
    m_instanceBufferView.SizeInBytes = bufferSize;
    m_instanceBufferView.StrideInBytes = sizeof(InstanceData);

    return true;
}

void Mesh::CreateInstanceBufferResource(UINT bufferSize)
{
    // ヒーププロパティの設定（UPLOADヒープ）
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    // リソースディスクリプタの設定
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    // デバイスの取得
    auto device = GraphicsDevice::Instance().GetDevice();

    // バッファの作成
    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_pInstanceBuffer)
    );
    if (FAILED(hr))
    {
        FNENG_ASSERT_ERROR("インスタンスバッファの作成に失敗しました");
        return;
    }
}

void Mesh::UpdateInstanceBuffer(const std::vector<InstanceData>& instanceDataList)
{
    UINT instanceCount = static_cast<UINT>(instanceDataList.size());
    UINT bufferSize = sizeof(InstanceData) * instanceCount;

    // バッファの容量チェックとリサイズ
    if (m_pInstanceBuffer == nullptr || m_currentInstanceCapacity < instanceCount)
    {
        // 古いバッファを解放
        m_pInstanceBuffer.Reset();

        // 新しいバッファを作成
        m_currentInstanceCapacity = instanceCount;

        auto device = GraphicsDevice::Instance().GetDevice();
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        HRESULT hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_pInstanceBuffer));

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR("インスタンスバッファの作成に失敗しました");
            return;
        }

        // インスタンスバッファビューを設定
        m_instanceBufferView.BufferLocation = m_pInstanceBuffer->GetGPUVirtualAddress();
        m_instanceBufferView.SizeInBytes = bufferSize;
        m_instanceBufferView.StrideInBytes = sizeof(InstanceData);
    }

    // データのコピー
    void* pData = nullptr;
    m_pInstanceBuffer->Map(0, nullptr, &pData);
    memcpy(pData, instanceDataList.data(), bufferSize);
    m_pInstanceBuffer->Unmap(0, nullptr);
}

void Mesh::DrawInstanced(UINT instanceCount) const
{
    const auto& pCmdList = GraphicsDevice::Instance().GetCmdList();

    // 頂点バッファビューの配列を設定（頂点バッファとインスタンスバッファ）
    D3D12_VERTEX_BUFFER_VIEW vbViews[] = { m_vbView, m_instanceBufferView };
    pCmdList->IASetVertexBuffers(0, _countof(vbViews), vbViews);

    // インデックスバッファの設定（存在する場合）
    if (m_ibView.SizeInBytes > 0)
    {
        pCmdList->IASetIndexBuffer(&m_ibView);
        pCmdList->DrawIndexedInstanced(m_faces.size() * 3, instanceCount, 0, 0, 0);
    }
    else
    {
        pCmdList->DrawInstanced(m_vertexCount, instanceCount, 0, 0);
    }
}

void Mesh::DrawSubset(UINT subsetNo) const
{
    // 範囲外のサブセットはスキップ
    if (subsetNo >= (int)m_subsets.size()) { return; }
    // 面数が0なら描画スキップ
    if (m_subsets[subsetNo].FaceCount == 0) { return; }

    const auto& pCmdList = GraphicsDevice::Instance().GetCmdList();

    // 頂点バッファビューの配列を設定（頂点バッファとインスタンスバッファ）
    D3D12_VERTEX_BUFFER_VIEW vbViews[] = { m_vbView, m_instanceBufferView };
    pCmdList->IASetVertexBuffers(0, _countof(vbViews), vbViews);

    // インデックスバッファの設定（存在する場合）
    if (m_ibView.SizeInBytes > 0)
    {
        pCmdList->IASetIndexBuffer(&m_ibView);
        pCmdList->DrawIndexedInstanced(m_subsets[subsetNo].FaceCount * 3, m_instanceCount, m_subsets[subsetNo].FaceStart * 3, 0, 0);
    }
    else
    {
        pCmdList->DrawInstanced(m_vertexCount, m_instanceCount, 0, 0);
    }
}

void Mesh::DrawSubsetInstanced(UINT subsetIndex, UINT instanceCount) const
{
    const auto& pCmdList = GraphicsDevice::Instance().GetCmdList();
    
    // 頂点バッファビューの配列を設定（頂点バッファとインスタンスバッファ）
    D3D12_VERTEX_BUFFER_VIEW vbViews[] = { m_vbView, m_instanceBufferView };
    pCmdList->IASetVertexBuffers(0, _countof(vbViews), vbViews);

    // インデックスバッファの設定
    pCmdList->IASetIndexBuffer(&m_ibView);

    // 描画コール
    const MeshSubset& subset = m_subsets[subsetIndex];
    pCmdList->DrawIndexedInstanced(subset.FaceCount * 3, instanceCount, subset.FaceStart * 3, 0, 0);
}
