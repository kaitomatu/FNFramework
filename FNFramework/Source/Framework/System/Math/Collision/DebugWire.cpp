#include "DebugWire.h"
#include "../../../../Framework/Manager/Shader/GenericShapeShader/GenericShapeShader.h"

void DebugWire::AddDebugLine(const Math::Vector3& start, const Math::Vector3& end, const Math::Color& color)
{

    if(!m_enable) { return; }

	// 頂点データ作成
	MeshVertex data;
	data.Color = color.RGBA().v;

	// 開始位置
	data.Position = start;
	m_vertices.push_back(data);

	// 終了位置
	data.Position = end;
	m_vertices.push_back(data);
}

void DebugWire::AddDebugLine(const Math::Vector3& start, const Math::Vector3& dir, float length, const Math::Color& col)
{
    if(!m_enable) { return; }

	// デバッグラインの始点
	MeshVertex v1;
	v1.Color = col.RGBA().v;
	v1.UV = Math::Vector2::Zero;
	v1.Position = start;

	// デバッグラインの終点
	MeshVertex v2;
	v2.Color = col.RGBA().v;
	v2.UV = Math::Vector2::Zero;
	v2.Position = v1.Position + (dir * length);

	m_vertices.push_back(v1);
	m_vertices.push_back(v2);
}

void DebugWire::AddDebugBox(const Math::Vector3& min, const Math::Vector3& max, const Math::Color& col)
{
    if(!m_enable) { return; }

	// 頂点データ作成
	MeshVertex data;
	data.Color = col.RGBA().v;
	data.UV = Math::Vector2::Zero;

	// 下面
	data.Position = min; m_vertices.push_back(data);
	data.Position = Math::Vector3{ max.x, min.y, min.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ max.x, min.y, min.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ max.x, min.y, max.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ max.x, min.y, max.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ min.x, min.y, max.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ min.x, min.y, max.z }; m_vertices.push_back(data);
	data.Position = min; m_vertices.push_back(data);

	// 上面
	data.Position = Math::Vector3{ min.x, max.y, min.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ max.x, max.y, min.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ max.x, max.y, min.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ max.x, max.y, max.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ max.x, max.y, max.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ min.x, max.y, max.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ min.x, max.y, max.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ min.x, max.y, min.z }; m_vertices.push_back(data);

	// 側面
	data.Position = min; m_vertices.push_back(data);
	data.Position = Math::Vector3{ min.x, max.y, min.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ max.x, min.y, min.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ max.x, max.y, min.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ max.x, min.y, max.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ max.x, max.y, max.z }; m_vertices.push_back(data);

	data.Position = Math::Vector3{ min.x, min.y, max.z }; m_vertices.push_back(data);
	data.Position = Math::Vector3{ min.x, max.y, max.z }; m_vertices.push_back(data);
}

void DebugWire::AddDebugBox(const OBB& obb, const Math::Color& col)
{
    if (!m_enable) { return; }

    const Math::Vector3& center = obb.Center;
    const Math::Quaternion& rotation = obb.Orientation;
    const Math::Vector3& extents = obb.Extents;

    Math::Matrix rotMatrix = Math::Matrix::CreateFromQuaternion(rotation);

    std::array<Math::Vector3, 8> corners;
    corners[0] = center + Math::Vector3::Transform(Math::Vector3{ -extents.x, -extents.y, -extents.z }, rotMatrix);
    corners[1] = center + Math::Vector3::Transform(Math::Vector3{ extents.x, -extents.y, -extents.z }, rotMatrix);
    corners[2] = center + Math::Vector3::Transform(Math::Vector3{ extents.x, -extents.y,  extents.z }, rotMatrix);
    corners[3] = center + Math::Vector3::Transform(Math::Vector3{ -extents.x, -extents.y,  extents.z }, rotMatrix);
    corners[4] = center + Math::Vector3::Transform(Math::Vector3{ -extents.x,  extents.y, -extents.z }, rotMatrix);
    corners[5] = center + Math::Vector3::Transform(Math::Vector3{ extents.x,  extents.y, -extents.z }, rotMatrix);
    corners[6] = center + Math::Vector3::Transform(Math::Vector3{ extents.x,  extents.y,  extents.z }, rotMatrix);
    corners[7] = center + Math::Vector3::Transform(Math::Vector3{ -extents.x,  extents.y,  extents.z }, rotMatrix);

    for (int i = 0; i < 4; ++i)
    {
        AddDebugLine(corners[i], corners[(i + 1) % 4], col);
        AddDebugLine(corners[i + 4], corners[(i + 1) % 4 + 4], col);
        AddDebugLine(corners[i], corners[i + 4], col);
    }
}

void DebugWire::AddDebugSphere(const Math::Vector3& centerPos, const Math::Color& color, float radius, int splitCount)
{
    if(!m_enable) { return; }

	// 頂点データ作成
	MeshVertex data;
	data.UV = Math::Vector2::Zero;
	data.Color = color.RGBA().v;

	// 球上に添うように線分(頂点)を追加
	for (int i = 0; i < splitCount + 1; ++i)
	{
		// XZ面
		data.Position = centerPos;
		data.Position.x += cos(MathHelper::ConvertToRadians(static_cast<float>(i) * (360.0f / splitCount))) * radius;
		data.Position.z += sin(MathHelper::ConvertToRadians(static_cast<float>(i) * (360.0f / splitCount))) * radius;
		m_vertices.push_back(data);

		data.Position = centerPos;
		data.Position.x += cos(MathHelper::ConvertToRadians(static_cast<float>((i + 1)) * (360.0f / splitCount))) * radius;
		data.Position.z += sin(MathHelper::ConvertToRadians(static_cast<float>((i + 1)) * (360.0f / splitCount))) * radius;
		m_vertices.push_back(data);

		// XY面
		data.Position = centerPos;
		data.Position.x += cos(MathHelper::ConvertToRadians(static_cast<float>(i) * (360.0f / splitCount))) * radius;
		data.Position.y += sin(MathHelper::ConvertToRadians(static_cast<float>(i) * (360.0f / splitCount))) * radius;
		m_vertices.push_back(data);

		data.Position = centerPos;
		data.Position.x += cos(MathHelper::ConvertToRadians(static_cast<float>((i + 1)) * (360.0f / splitCount))) * radius;
		data.Position.y += sin(MathHelper::ConvertToRadians(static_cast<float>((i + 1)) * (360.0f / splitCount))) * radius;
		m_vertices.push_back(data);

		// YZ面
		data.Position = centerPos;
		data.Position.y += cos(MathHelper::ConvertToRadians(static_cast<float>(i) * (360.0f / splitCount))) * radius;
		data.Position.z += sin(MathHelper::ConvertToRadians(static_cast<float>(i) * (360.0f / splitCount))) * radius;
		m_vertices.push_back(data);

		data.Position = centerPos;
		data.Position.y += cos(MathHelper::ConvertToRadians(static_cast<float>((i + 1)) * (360.0f / splitCount))) * radius;
		data.Position.z += sin(MathHelper::ConvertToRadians(static_cast<float>((i + 1)) * (360.0f / splitCount))) * radius;
		m_vertices.push_back(data);
	}
}

void DebugWire::Init()
{
	// 頂点バッファを作成
	vertices.Vertices::CreateVertexBuffer(sizeof(MeshVertex), MaxVerticesNum);
}

void DebugWire::Draw()
{
    if(!m_enable) { return; }

	// 頂点数が2未満なら描画しない
	if (m_vertices.size() < 2) { return; }

	// 頂点バッファを作成
	vertices.UpdateBuffer(m_vertices);

	// シェーダーをセット
	ShaderManager::Instance().GetGenericShapeShader()->Begin();

	// 描画 : Meshの方のDrawではなく、Verticesの方のDrawを呼び出す
    vertices.Vertices::DrawInstanced(vertices.GetVertexCount());

	ClearVertex();
}

void DebugWire::ClearVertex()
{
    m_vertices.clear();
    vertices.ClearPositions();
}
