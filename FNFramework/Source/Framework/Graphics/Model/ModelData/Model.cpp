#include "Model.h"

// todo : Assimpを使ってのモデル読み込みがアニメーションがうまく対応できなかったためKDのものを利用。
//#include "../ModelLoader.h"
// ----Load関数----
//// bool ModelData::Load(const std::string& _modelName)
// {
//     ModelLoader modelLoader;
//
//     // Hack : 現在は.gltfファイルのみ対応しているので、決め打ちでAssets/models/ + fileName + .gltfを付与
//     const std::string& filePath = ModelPath::FileDir + _modelName + ModelPath::FileExtension;
//
//     if (!modelLoader.Load(filePath, *this, m_nodes))
//     {
//         FNENG_ASSERT_ERROR("モデルのロードに失敗");
//         return false;
//     }
//
//     return true;
// }

#include "Framework/KDFramework/KdGLTFLoader.h"

const std::shared_ptr<AnimationData> ModelData::GetAnimation(std::string_view _animName) const
{
    for (auto&& anim : m_spAnimations)
    {
        if (anim->Name == _animName)
        {
            return anim;
        }
    }

    return nullptr;
}

const std::shared_ptr<AnimationData> ModelData::GetAnimation(UINT _index) const
{
    return _index >= m_spAnimations.size() ? nullptr : m_spAnimations[_index];
}

const std::vector<std::shared_ptr<AnimationData>>& ModelData::GetAnimationList() const
{
    return m_spAnimations;
}

std::vector<std::shared_ptr<AnimationData>>& ModelData::WorkAnimationList()
{
    return m_spAnimations;
}

bool ModelData::IsSkinMesh()
{
    for(const auto& node: m_nodes)
    {
        if (node.IsSkinMesh) { return true; }
    }

    return false;
}

bool ModelData::Load(const std::string& _modelName)
{
    Release();

    // Hack : 現在は.gltfファイルのみ対応しているので、決め打ちでAssets/models/ + fileName + .gltfを付与
    const std::string& filePath = ModelPath::FileDir + _modelName + ModelPath::FileExtension;

    const std::string::size_type pos = std::max<signed>(static_cast<const int&>(_modelName.find_last_of('/')),
        static_cast<const int&>(_modelName.find_last_of('\\')));
    std::string fileDir = (pos == std::string::npos) ? std::string() : _modelName.substr(0, pos + 1);

    std::shared_ptr<KDFramework::KdGLTFModel> spGltfModel = KDFramework::KdLoadGLTFModel(filePath);

    if (!spGltfModel)
    {
        FNENG_ASSERT_LOG("モデルのロードに失敗", false);
        return false;
    }

    CreateNodes(spGltfModel);

    CreateMaterials(spGltfModel, ModelPath::FileDir + fileDir);

    CreateAnimations(spGltfModel);

    return true;
}

void ModelData::CreateNodes(const std::shared_ptr<KDFramework::KdGLTFModel>& spGltfModel)
{
    m_nodes.resize(spGltfModel->Nodes.size());

    for (UINT i = 0; i < spGltfModel->Nodes.size(); i++)
    {
        // 入力元ノード
        const KDFramework::KdGLTFNode& rSrcNode = spGltfModel->Nodes[i];

        // 出力先のノード参照
        Node& rDstNode = m_nodes[i];

        if (rSrcNode.IsMesh)
        {
            // メッシュ作成
            rDstNode.spMesh = std::make_shared<Mesh>();

            if (rDstNode.spMesh)
            {
                rDstNode.spMesh->Create(rSrcNode.Mesh.Vertices, rSrcNode.Mesh.Faces, rSrcNode.Mesh.Subsets,
                                        rSrcNode.Mesh.IsSkinMesh);
            }

            // メッシュノードリストにインデックス登録
            m_meshNodeIdx.push_back(i);
        }

        // ノード情報セット
        rDstNode.NodeName = rSrcNode.Name;

        rDstNode.mLocalTransform = rSrcNode.LocalTransform;
        rDstNode.mWorldTransform = rSrcNode.WorldTransform;
        rDstNode.Bone.OffsetMatrix = rSrcNode.InverseBindMatrix;

        rDstNode.IsSkinMesh = rSrcNode.Mesh.IsSkinMesh;

        rDstNode.Bone.Index = rSrcNode.BoneNodeIndex;

        rDstNode.ParentIdx = rSrcNode.Parent;
        rDstNode.Children = rSrcNode.Children;

        // 当たり判定用ノード検索
        if (rDstNode.NodeName.find("Col") != std::string::npos)
        {
            // 判定用ノードに割り当て
            m_collisionMeshNodeIdx.push_back(i);
        }
        else
        {
            // 描画ノードに割り当て
            m_drawMeshNodeIdx.push_back(i);
        }
    }

    for (UINT nodeIdx = 0; nodeIdx < spGltfModel->Nodes.size(); nodeIdx++)
    {
        // ルートノードのIndexリスト
        if (spGltfModel->Nodes[nodeIdx].Parent == -1) { m_rootNodeIdx.push_back(nodeIdx); }

        // ボーンノードのIndexリスト
        int boneIdx = spGltfModel->Nodes[nodeIdx].BoneNodeIndex;

        if (boneIdx >= 0)
        {
            if (boneIdx >= (int)m_boneNodeIdx.size()) { m_boneNodeIdx.resize(boneIdx + 1); }

            m_boneNodeIdx[boneIdx] = nodeIdx;
        }
    }

    // 当たり判定用ノードが1つも見つからなければ、m_drawMeshNodeと同じ割り当てを行い、見た目 = 当たり判定とする
    if (!m_collisionMeshNodeIdx.size())
    {
        m_collisionMeshNodeIdx = m_drawMeshNodeIdx;
    }

}

void ModelData::CreateMaterials(const std::shared_ptr<KDFramework::KdGLTFModel>& spGltfModel, const std::string& fileDir)
{
    //マテリアル配列を受け取れるサイズのメモリを確保
    m_materials.resize(spGltfModel->Materials.size());

    for (UINT i = 0; i < m_materials.size(); ++i)
    {
        // src = sourceの略
        // dst = destinationの略
        const KDFramework::KdGLTFMaterial& rSrcMaterial = spGltfModel->Materials[i];
        Material& rDstMaterial = m_materials[i];

        // 名前
        rDstMaterial.Name = rSrcMaterial.Name;

        // 各テクスチャの設定
        rDstMaterial.SetTextures(fileDir, rSrcMaterial.BaseColorTexName,
                                 rSrcMaterial.MetallicRoughnessTexName, rSrcMaterial.EmissiveTexName,
                                 rSrcMaterial.NormalTexName);

        // 基本色
        rDstMaterial.BaseColor = rSrcMaterial.BaseColor;

        // 金属性・粗さ
        rDstMaterial.Metallic = rSrcMaterial.Metallic;
        rDstMaterial.Roughness = rSrcMaterial.Roughness;

        // 自己発光・エミッシブ
        rDstMaterial.Emissive = rSrcMaterial.Emissive;
    }
}

void ModelData::CreateAnimations(const std::shared_ptr<KDFramework::KdGLTFModel>& spGltfModel)
{
    // アニメーションデータ
    m_spAnimations.resize(spGltfModel->Animations.size());

    for (UINT i = 0; i < m_spAnimations.size(); ++i)
    {
        const KDFramework::KdGLTFAnimationData& rSrcAnimation = *spGltfModel->Animations[i];

        m_spAnimations[i] = std::make_shared<AnimationData>();
        AnimationData& rDstAnimation = *(m_spAnimations[i]);

        rDstAnimation.Name = rSrcAnimation.m_name;

        rDstAnimation.MaxFrame = rSrcAnimation.m_maxLength;

        rDstAnimation.Channels.resize(rSrcAnimation.m_nodes.size());

        for (UINT j = 0; j < rDstAnimation.Channels.size(); ++j)
        {
            rDstAnimation.Channels[j].NodeOffset = rSrcAnimation.m_nodes[j]->m_nodeOffset;
            rDstAnimation.Channels[j].Translations = rSrcAnimation.m_nodes[j]->m_translations;
            rDstAnimation.Channels[j].Rotations = rSrcAnimation.m_nodes[j]->m_rotations;
            rDstAnimation.Channels[j].Scales = rSrcAnimation.m_nodes[j]->m_scales;
        }
    }
}

void ModelData::Release()
{
    m_materials.clear();

    m_nodes.clear();

    m_rootNodeIdx.clear();
    m_boneNodeIdx.clear();
    m_meshNodeIdx.clear();
}

//============================================================
// ModelWork
//============================================================
const std::shared_ptr<Mesh> ModelWork::GetMeshes(UINT _index) const
{
    if (m_coppiedNodes.size() <= _index) { return {}; }

    return GetDataNodes()[_index].spMesh;
}

const ModelData::Node* ModelWork::FindDataNode(std::string_view _name) const
{
    if (m_spData == nullptr) { return nullptr; }

    return m_spData->FindNode(_name.data());
}

const ModelWork::Node* ModelWork::FindNode(std::string_view _name) const
{
    // ノード検索：文字列
    for (auto&& node : m_coppiedNodes)
    {
        if (node.NodeName == _name.data())
        {
            return &node;
        }
    }

    return nullptr;
}

ModelWork::Node* ModelWork::FindWorkNode(std::string_view _name)
{
    // 可変ノード検索：文字列
    for (auto&& node : m_coppiedNodes)
    {
        if (node.NodeName == _name.data())
        {
            m_needCalcNode = true;

            return &node;
        }
    }

    return nullptr;
}

void ModelWork::SetModelData(const std::shared_ptr<ModelData>& _rModel)
{
    if(!_rModel)
    {
        FNENG_ASSERT_LOG("モデルが存在しません", false)
        return;
    }

    // モデル設定：コピーノードの生成
    m_spData = _rModel;

    UINT nodeSize = static_cast<UINT>(_rModel->GetNodes().size());

    m_coppiedNodes.resize(nodeSize);

    // ノードのコピーを生成
    for (UINT i = 0; i < nodeSize; ++i)
    {
        m_coppiedNodes[i].Copy(_rModel->GetNodes()[i]);
    }

    m_needCalcNode = true;
}

void ModelWork::SetModelData(std::string_view _fileName)

{        // モデルの読み込みに失敗したらデフォルトのモデルを返す

    SetModelData(AssetManager::Instance().GetModelData(_fileName));
}

void ModelWork::CalcNodeMatrices()
{
    // ルートノードから各ノードの行列を計算していく
    if (!m_spData)
    {
        FNENG_ASSERT_ERROR("モデルのデータが存在しないノードの行列計算");
        return;
    }

    // 全ボーン行列を書き込み
    for (auto&& nodeIdx : m_spData->GetRootNodeIdxList())
    {
        RecCaclNodeMatrices(nodeIdx);
    }

    m_needCalcNode = false;
}

void ModelWork::RecCaclNodeMatrices(int _nodeIdx, int _parentNodeIdx)
{
    // ノード行列計算用の再起用関数
    const auto& data = m_spData->GetNodes()[_nodeIdx];
    auto& work = m_coppiedNodes[_nodeIdx];

    // 親との行列を合成
    if (_parentNodeIdx >= 0)
    {
        auto& parent = m_coppiedNodes[data.ParentIdx];

        work.mWorldTransform = work.mLocalTransform * parent.mWorldTransform;
    }
    // 親が居ない場合は親は自分自身とする
    else
    {
        work.mWorldTransform = work.mLocalTransform;
    }

    for (int childNodeIdx : data.Children)
    {
        RecCaclNodeMatrices(childNodeIdx, _nodeIdx);
    }
}
