#pragma once

namespace KDFramework
{
    struct KdGLTFModel;
}

struct AnimationData;

namespace ModelPath
{
    static const std::string FileDir = "Assets/Model/"; // ファイルディレクトリ
    static const std::string FileExtension = ".gltf"; // ファイル拡張子
}

/**
* @class ModelData
* @brief モデルデータ
* @details モデルデータを管理するクラス / Nodeと呼ばれるMeshの集合体を持つ
*/
// Hack : 現在は.gltfファイルのみ対応しているので、.fbxなどのファイルに対応させる
//		  ModelDataはModelWorkクラスを作成して、変更の可能性がある箇所だけコピーを持つようにする(参考 : 教科書 / KdModel)
class ModelData
{
public:
    struct Node
    {
        std::string NodeName = ""; // ノード名

        Math::Matrix mLocalTransform; // ローカルトランスフォーム
        Math::Matrix mWorldTransform; // ワールドトランスフォーム

        std::shared_ptr<Mesh>	spMesh;	// メッシュ

        int                     ParentIdx = -1; // 親ノードのインデックス
        std::vector<int>        Children; // 子ノードのインデックス

        struct BoneData
        {
            std::string             Name = "";      // ボーン名
            Math::Matrix            OffsetMatrix;   // ボーンの逆行列
            int                     Index = -1;     // ボーンノードの時、先頭から何番目のボーンか？
        } Bone;

        bool                IsSkinMesh = false;
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    ModelData()
    {
    }

    ModelData(std::string_view modelName) { Load(modelName.data()); }

    ~ModelData()
    {
        Release();
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------
    /**
    * @brief ノードの取得
    * @result ノード情報
    */
    const std::vector<Node>& GetNodes() const
    {
        return m_nodes;
    }

    // 作業可能
    std::vector<Node>& WorkNodes()
    {
        return m_nodes;
    }

    /**
    * @brief ノードの取得 - ノード名からノードの情報を取得する
    * @param[in] nodeName - ノード名
    *
    * todo : 単純な全検索なので、ハッシュマップなどで高速化する
    */
    Node* FindNode(std::string_view nodeName)
    {
        for (auto&& node : m_nodes)
        {
            if (node.NodeName == nodeName)
            {
                return &node;
            }
        }

        return nullptr;
    }

    // アニメーションデータ取得
    const std::shared_ptr<AnimationData> GetAnimation(std::string_view _animName) const;
    const std::shared_ptr<AnimationData> GetAnimation(UINT _index) const;
    const std::vector<std::shared_ptr<AnimationData>>& GetAnimationList() const;
    std::vector<std::shared_ptr<AnimationData>>& WorkAnimationList();
    // アニメーションするかどうか
    bool IsAnimation() const { return !m_spAnimations.empty(); }

    bool IsSkinMesh();

    /**
    * @brief  マテリアルの取得
    * @result マテリアル情報
    */
    const std::vector<Material>& GetMaterials() const
    {
        return m_materials;
    }

    void SetBaseColorTex(const ShaderResourceTexture& tex, int idx = 0)
    {
        *m_materials[idx].spBaseColorTex = tex;
    }

    // 各種読み込み //
    void CreateNodes(const std::shared_ptr<KDFramework::KdGLTFModel>& spGltfModel);									// ノード作成
    void CreateMaterials(const std::shared_ptr<KDFramework::KdGLTFModel>& spGltfModel, const  std::string& fileDir);	// マテリアル作成
    void CreateAnimations(const std::shared_ptr<KDFramework::KdGLTFModel>& spGltfModel);								// アニメーション作成

    // 各種インデックスの取得 //
    const std::vector<int>& GetRootNodeIdxList() const { return m_rootNodeIdx; }
    const std::vector<int>& GetBoneNodeIdxList() const { return m_boneNodeIdx; }
    const std::vector<int>& GetMeshNodeIdxList() const { return m_meshNodeIdx; }

    const std::vector<int>& GetDrawMeshNodeIdxList() const { return m_drawMeshNodeIdx; }
    const std::vector<int>& GetCollisionMeshNodeIdxLists() const { return m_collisionMeshNodeIdx; }

    //--------------------------------
    // その他関数
    //--------------------------------
    /**
    * @brief モデルのロード
    * @param _modelName - モデルの名前
    * @result 成功したらtrue
    */
    bool Load(const std::string& _modelName);
    void Release();

private:

    //マテリアル配列
    std::vector<Material> m_materials;

    // すべてのノード配列
    std::vector<Node> m_nodes;

    // アニメーションデータリスト
    std::vector<std::shared_ptr<AnimationData>> m_spAnimations;

    std::vector<Node>		m_originalNodes;
    // 全ノード中、RootノードのみのIndex配列
    std::vector<int>		m_rootNodeIdx;
    // 全ノード中、ボーンノードのみのIndex配列
    std::vector<int>		m_boneNodeIdx;
    // 全ノード中、メッシュが存在するノードのみのIndexn配列
    std::vector<int>		m_meshNodeIdx;

    // 全ノード中、コリジョンメッシュが存在するノードのみのIndexn配列
    std::vector<int>		m_collisionMeshNodeIdx;
    // 全ノード中、描画するノードのみのIndexn配列
    std::vector<int>		m_drawMeshNodeIdx;
};

/**
* @class ModelWork
* @brief モデルごとの変更される可能性のあるデータを持つクラス
* @details
*   アニメーションなどのモデルごとに変更される可能性のあるデータ(ローカルのワールド行列など)を持つクラス
*/
class ModelWork
{
public:
    struct Node
    {
        // todo : マテリアルをここに追加することで、メッシュ一つだけ用意してテクスチャなどの変更により見た目を変える(ドラクエのスライム)とかもできそう

        std::string NodeName; // ノード名
        Math::Matrix mLocalTransform; // ローカルトランスフォーム
        Math::Matrix mWorldTransform; // ワールドトランスフォーム

        void Copy(const ModelData::Node& rNode)
        {
            NodeName = rNode.NodeName;
            mLocalTransform = rNode.mLocalTransform;
            mWorldTransform = rNode.mLocalTransform;
        }
    };

    //--------------------------------
    // コンストラクタ / デストラクタ
    //--------------------------------
    ModelWork()
    {
    }

    ModelWork(const std::shared_ptr<ModelData>& _spModel) { SetModelData(_spModel); }

    ~ModelWork()
    {
    }

    //--------------------------------
    // ゲッター / セッター
    //--------------------------------

    // 有効フラグ
    bool IsEnable() const { return (m_enable && m_spData); }
    void SetEnable(bool _flag) { m_enable = _flag; }

    bool IsSkinMesh() const { return m_spData ? m_spData->IsSkinMesh() : false; }

    //----------------------
    // モデルデータの取得
    const std::shared_ptr<ModelData> GetModelData() const { return m_spData; }
    // メッシュ取得 : m_coppiedNodesより大きい
    const std::shared_ptr<Mesh> GetMeshes(UINT _index) const;

    //----------------------
    // ノードの取得
    // モデルデータのノードリスト取得
    const std::vector<ModelData::Node>& GetDataNodes() const
    {
        if (!m_spData)
        {
            FNENG_ASSERT_ERROR("モデルデータが存在しません");
        }
        return m_spData->GetNodes();
    }

    // コピーノードのリスト取得
    const std::vector<Node>& GetNodes() const { return m_coppiedNodes; }

    std::vector<Node>& WorkNodes()
    {
        m_needCalcNode = true;
        return m_coppiedNodes;
    }

    // 文字列でのノード検索
    const ModelData::Node* FindDataNode(std::string_view _name) const;
    const Node* FindNode(std::string_view _name) const;
    Node* FindWorkNode(std::string_view _name);

    //----------------------
    // アニメーションデータ取得
    const std::shared_ptr<AnimationData> GetAnimation(std::string_view animName) const
    {
        return !m_spData ? nullptr : m_spData->GetAnimation(animName);
    }

    const std::shared_ptr<AnimationData> GetAnimation(int index) const
    {
        return !m_spData ? nullptr : m_spData->GetAnimation(index);
    }

    bool IsAnimation() const { return m_spData ? false : m_spData->IsAnimation(); }

    // モデル設定：コピーノードの生成
    void SetModelData(const std::shared_ptr<ModelData>& _rModel);
    void SetModelData(std::string_view _fileName);

    bool NeedCalcNodeMatrices() const { return m_needCalcNode; }

    //--------------------------------
    // その他関数
    //--------------------------------

    // ボーンの行列を計算
    void CalcNodeMatrices();

private:
    //  再起呼び出し用の関数
    void RecCaclNodeMatrices(int _nodeIdx, int _parentNodeIdx = -1);

    // 有効フラグ
    bool m_enable = true;

    // モデルデータの参照 : これを元にノードのデータを作成する
    std::shared_ptr<ModelData> m_spData = nullptr;
    // 動作中に変化する可能性のあるノードデータ
    std::vector<Node> m_coppiedNodes;

    // Dirtyフラグ
    bool m_needCalcNode = false;
};
