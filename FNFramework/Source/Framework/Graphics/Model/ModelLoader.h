#pragma once

#ifdef _DEBUG
#pragma comment(lib,"assimp-vc143-mtd.lib")
#else
#pragma comment(lib,"assimp-vc143-mt.lib")
#endif

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"

/**
 * todo : Assimpを利用してフレームワークに落とし込めなかったため一旦保留、しばらくはKDのものを利用するため最後までこのクラスを使用しなければAssimpとこのクラスは削除する
 */
class ModelLoader
{
public:
    /**
    * @brief モデルのロード
    *
    * @param[in]  filePath - ファイルパス
    * @param[out]  modelData - ノード情報
    * @param[out] nodes - ノード情報
    * @result 成功したらtrue
    */
    bool Load(const std::string& filePath, ModelData& modelData, std::vector<ModelData::Node>& nodes);

private:
    /**
    * @brief 子ノードの解析
    * @param[in]  pNode - ノードのポインタ
    * @param[in]  parentIdx - 親ノードのインデックス
    * @param[out] modelData - ノード情報
    * @param[out] nodes - ノード情報
    * @param[in]  pScene - モデルシーンのポインタ
    * @param[in]  filePath - ファイルパス
    */
    void ProcessNodes(const aiNode* pNode, int parentIdx,
                      ModelData& modelData, std::vector<ModelData::Node>& nodes,
                      const aiScene* pScene, const std::string& filePath);

    /**
    * @brief 解析
    *
    * @param  mesh		- メッシュ情報
    * @param  pScene	- シーン情報
    * @param  pMesh     - メッシュのポインタ
    * @param  dstNode   - ノード情報
    * @param  pMaterial - マテリアルのポインタ
    * @param  filePath   - ディレクトリパス
    * @result メッシュのポインタ
    */
    void Parse(Mesh& mesh, const aiScene* pScene, const aiMesh* pMesh, ModelData::Node& dstNode,
               const aiMaterial* pMaterial, const std::string& filePath);

    /**
    * @brief マテリアルの解析
    *
    * @param  pMaterial - マテリアルのポインタ
    * @param  filePath   - ディレクトリパス
    * @result マテリアル情報
    */
    const Material ParseMaterial(const aiMaterial* pMaterial, const std::string& filePath);

    /**
     * @brief スケルトン(ボーンの階層構造など)の解析
     *
     * @param  DstBone     - ボーンの情報
     * @param  node        - ノードのポインタ
     * @param  boneInfoMap - ボーン情報マップ
     *
     * @return ボーンデータの解析に成功したらtrue
     */
    bool ParseSkeleton(ModelData::Node::BoneData& DstBone,
        const aiNode* node,
        std::unordered_map<std::string, std::pair<int, Math::Matrix>>& boneInfoMap);

    // 行列のオーダー入れ替え : Assimpは行列のオーダーが違うので変換
    void SwapMatrix4x4Order(const aiMatrix4x4& _mSrc,
                            Math::Matrix&	   _mDest) const
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                _mDest.m[i][j] = _mSrc[j][i];
            }
        }
    }

    Math::Matrix SwapMatrix4x4Order(const aiMatrix4x4& _mSrc) const
    {
        Math::Matrix m;
        SwapMatrix4x4Order(_mSrc, m);
        return m;
    }

};
