#include "SpriteMesh.h"

void SpriteMesh::DrawInstanced(UINT _vertexCount) const
{
    const auto& pCmdList = GraphicsDevice::Instance().GetCmdList();;
    pCmdList->IASetVertexBuffers(0, 1, &m_vbView);
    pCmdList->IASetIndexBuffer(&m_ibView);
    pCmdList->DrawIndexedInstanced(_vertexCount, 1, 0, 0, 0);
}

void SpriteMesh::DrawInstanced() const
{
    DrawInstanced(m_instanceCount);
}

void SpriteMesh::Create()
{
    //===============================
    // 頂点バッファ / ビューの作成
    //===============================
    CreateVertexBuffer(sizeof(SpriteMeshVertex), static_cast<UINT>(m_vertices.size()));

    // インデックスデータ
    std::vector<MeshFace> _faces =
    {
        {0, 1, 2},
        {1, 3, 2}
    };

    // インスタンス数格納
    m_instanceCount = static_cast<UINT>(_faces.size() * 3);

    m_vertexCount = static_cast<UINT>(m_vertices.size());

    //===============================
    // バッファ / インデックスの作成
    //===============================
    CreateIndexBufferAndFaceData(_faces);
}

void SpriteMesh::SpriteVertexSetting(
    const std::shared_ptr<ShaderResourceTexture>& _tex,
    const Math::Vector4& _pixelPos,
    const std::shared_ptr<Math::Rectangle>& _srcRect,
    const Math::Vector2& _pivot)
{
    SpriteVertexSetting(
        {
            static_cast<float>(_tex->GetWidth()),
            static_cast<float>(_tex->GetHeight())
        },
        _pixelPos,
        _srcRect,
        _pivot);
}

void SpriteMesh::SpriteVertexSetting(const Math::Vector2& _spriteSize, const Math::Vector4& _pixelPos,
    const std::shared_ptr<Math::Rectangle>& _srcRect, const Math::Vector2& _pivot)
{
    // UV
    Math::Vector2 uvMin = { 0, 0 };
    Math::Vector2 uvMax = { 1, 1 };
    if (_srcRect)
    {
        uvMin.x = static_cast<float>(_srcRect->x) / _spriteSize.x;
        uvMin.y = static_cast<float>(_srcRect->y) / _spriteSize.y;

        uvMax.x = static_cast<float>(_srcRect->x + _srcRect->width) / _spriteSize.x;
        uvMax.y = static_cast<float>(_srcRect->y + _srcRect->height) / _spriteSize.y;
    }

    // 頂点作成
    float x1 = _pixelPos.x;
    float y1 = _pixelPos.y;
    float x2 = _pixelPos.x + _pixelPos.z;
    float y2 = _pixelPos.y + _pixelPos.w;

    // 基準点(_pivot)ぶんずらす
    x1 -= _pivot.x * _pixelPos.z;
    x2 -= _pivot.x * _pixelPos.z;
    y1 -= _pivot.y * _pixelPos.w;
    y2 -= _pivot.y * _pixelPos.w;

    // 頂点データの設定
    m_vertices[0] = { {x1, y1, 0}, {uvMin.x, uvMax.y} };
    m_vertices[1] = { {x1, y2, 0}, {uvMin.x, uvMin.y} };
    m_vertices[2] = { {x2, y1, 0}, {uvMax.x, uvMax.y} };
    m_vertices[3] = { {x2, y2, 0}, {uvMax.x, uvMin.y} };

    // 設定された頂点情報をバッファに書き込む
    UpdateBuffer();
}

void SpriteMesh::UpdateBuffer()
{
    // 頂点バッファが設定されていなければ警告を出す
    if(m_vertices[0].Position.z <= -1)
    {
        FNENG_ASSERT_LOG("頂点バッファの座標が設定されていません",/* isOutput = */ true)
    }

    // 頂点バッファに情報を描き込む
    SpriteMeshVertex* vbMap = nullptr;
    {
        auto hr = m_pVBuffer->Map(0, nullptr, (void**)&vbMap);

        if (FAILED(hr))
        {
            FNENG_ASSERT_ERROR("頂点バッファのマップに失敗しました");
            return;
        }

        std::copy(std::begin(m_vertices), std::end(m_vertices), vbMap); // m_verticesの中身をvbMapにコピーする
        m_pVBuffer->Unmap(0, nullptr);
    }
}

void SpriteMesh::Release()
{
    if(m_spMainTex)
    {
        m_spMainTex.reset();
        m_spMainTex = nullptr;
    }
}
