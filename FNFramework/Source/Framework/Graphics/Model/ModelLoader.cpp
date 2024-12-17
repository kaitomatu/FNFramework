// #include "ModelLoader.h"
//
// bool ModelLoader::Load(const std::string& filePath, ModelData& modelData, std::vector<ModelData::Node>& nodes)
// {
//     Assimp::Importer importer;
//
//     int flag = 0;
//     flag |= aiProcess_CalcTangentSpace;
//     flag |= aiProcess_Triangulate;
//     flag |= aiProcess_ConvertToLeftHanded;
//
//     const aiScene* pScene = importer.ReadFile(filePath, flag);
//
//     if (!pScene)
//     {
//         FNENG_ASSERT_ERROR("モデルのファイルが見つかりません");
//         return false;
//     }
//
//     nodes.clear();
//     // 予測されるノード数をあらかじめ確保する
//     nodes.reserve(pScene->mNumMeshes + pScene->mRootNode->mNumChildren);
//
//     // ルートからのノード情報の解析
//     ProcessNodes(pScene->mRootNode, -1, modelData, nodes, pScene, filePath);
//
//     // 当たり判定用ノードが一つも設定されていない場合は、描画用ノード = 当たり判定用ノードとする
//     if (modelData.GetCollisionMeshNodeIdxLists().empty())
//     {
//         for (int drawMeshNodeIdx : modelData.GetMeshNodeIdxList())
//         {
//             modelData.AddCollisionMeshNodeIdx(drawMeshNodeIdx);
//         }
//     }
//
//     //---------------------------------------------------
//     // アニメーションデータの読み込み
//     //---------------------------------------------------
//     auto& animationList = modelData.WorkAnimationList();
//
//     for (unsigned int i = 0; i < pScene->mNumAnimations; ++i)
//     {
//         aiAnimation* pAnimation = pScene->mAnimations[i];
//
//         std::shared_ptr<AnimationData> spAnimaData = std::make_shared<AnimationData>();
//         spAnimaData->Name = pAnimation->mName.C_Str();
//         spAnimaData->MaxFrame = static_cast<float>(pAnimation->mDuration);
//
//         spAnimaData->Channels.resize(pAnimation->mNumChannels);
//
//         for (unsigned int j = 0; j < pAnimation->mNumChannels; ++j)
//         {
//             auto& srcChannel = spAnimaData->Channels[j];
//             const auto& dstChannel = pAnimation->mChannels[j];
//             srcChannel.NodeOffset = j;
//
//             for (unsigned int k = 0; k < dstChannel->mNumPositionKeys; ++k)
//             {
//                 AnimKeyVector3 translation;
//                 translation.m_time = static_cast<float>(dstChannel->mPositionKeys[k].mTime);
//
//                 translation.m_vec.x = dstChannel->mPositionKeys[k].mValue.x;
//                 translation.m_vec.y = dstChannel->mPositionKeys[k].mValue.y;
//                 translation.m_vec.z = dstChannel->mPositionKeys[k].mValue.z;
//
//                 srcChannel.Translations.emplace_back(translation);
//             }
//
//             for (unsigned int k = 0; k < dstChannel->mNumRotationKeys; ++k)
//             {
//                 AnimKeyQuaternion rotation;
//                 rotation.m_time = static_cast<float>(dstChannel->mRotationKeys[k].mTime);
//
//                 rotation.m_quat.x = dstChannel->mRotationKeys[k].mValue.x;
//                 rotation.m_quat.y = dstChannel->mRotationKeys[k].mValue.y;
//                 rotation.m_quat.z = dstChannel->mRotationKeys[k].mValue.z;
//                 rotation.m_quat.w = dstChannel->mRotationKeys[k].mValue.w;
//
//                 srcChannel.Rotations.emplace_back(rotation);
//             }
//
//             for (unsigned int k = 0; k < dstChannel->mNumScalingKeys; ++k)
//             {
//                 AnimKeyVector3 scale;
//                 scale.m_time = static_cast<float>(dstChannel->mScalingKeys[k].mTime);
//
//                 scale.m_vec.x = dstChannel->mScalingKeys[k].mValue.x;
//                 scale.m_vec.y = dstChannel->mScalingKeys[k].mValue.y;
//                 scale.m_vec.z = dstChannel->mScalingKeys[k].mValue.z;
//
//                 srcChannel.Scales.emplace_back(scale);
//             }
//         }
//
//         animationList.emplace_back(spAnimaData);
//     }
//
//     return true;
// }
//
// void ModelLoader::ProcessNodes(const aiNode* pNode, int parentIdx,
//                                ModelData& modelData, std::vector<ModelData::Node>& nodes,
//                                const aiScene* pScene, const std::string& filePath)
// {
//     // 現在のノードのインデックスを取得
//     int currentIndex = static_cast<int>(nodes.size()); // ルートノードの場合は0が入る
//
//     // ノード情報の追加
//     nodes.emplace_back(); // ノードを追加
//     ModelData::Node& DstNode = nodes.back();
//
//     DstNode.NodeName = pNode->mName.C_Str();
//     DstNode.ParentIdx = parentIdx;
//
//     // ローカルトランスフォームの取得
//     // ※ Assimp と DirectX::SimpleMath::Matrix では行列の並びが異なるので変換
//     DstNode.mLocalTransform = SwapMatrix4x4Order(pNode->mTransformation);
//
//     // ルートノードかどうか
//     if(!pNode->mParent)
//     {
//         // ルートノードならば、ワールドトランスフォームはローカルトランスフォームと同じ
//         DstNode.mWorldTransform = DstNode.mLocalTransform;
//
//         modelData.AddRootNodeIdx(currentIndex);
//     }
//     else
//     {
//         // Assimpは自動的にROOTというノードが作成されるっぽいので名前で判定する
//          if(pNode->mParent->mName.C_Str() == std::string("ROOT"))
//         {
//             modelData.AddRootNodeIdx(currentIndex);
//         }
//
//         // ルートノードじゃない場合は親がいるはずなので、親のワールドトランスフォームを掛けてワールドトランスフォームを求める
//         DstNode.mWorldTransform = DstNode.mLocalTransform * nodes[parentIdx].mWorldTransform;
//     }
//
//     // 当たり判定用ノードの検索
//     if (DstNode.NodeName.find("Col") != std::string::npos)
//     {
//         modelData.AddCollisionMeshNodeIdx(currentIndex);
//     }
//
//     //---------------------------------------------------
//     // メッシュの作成 / マテリアルの解析
//     //---------------------------------------------------
//     // メッシュの処理
//     for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
//     {
//         // メッシュの取得
//         const aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
//
//         // マテリアルの取得
//         const aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
//
//         // メッシュの解析
//         std::shared_ptr<Mesh> spMesh = std::make_shared<Mesh>();
//         Parse(*spMesh, pScene, pMesh, DstNode, pMaterial, filePath);
//
//         DstNode.spMeshes.push_back(spMesh);
//
//         // メッシュノードリストにインデックス登録
//         modelData.AddMeshNodeIdx(currentIndex);
//     }
//
//     //---------------------------------------------------
//     // 子ノードの処理 : 再帰的に処理
//     //---------------------------------------------------
//     for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
//     {
//         int childIndex = static_cast<int>(nodes.size());
//         ProcessNodes(pNode->mChildren[i], currentIndex, modelData, nodes, pScene, filePath);
//         nodes[currentIndex].ChildrenIdx.push_back(childIndex);
//     }
// }
//
// void ModelLoader::Parse(
//     Mesh& mesh,
//     const aiScene* pScene,
//     const aiMesh* pMesh,
//     ModelData::Node& dstNode,
//     const aiMaterial* pMaterial,
//     const std::string& filePath)
// {
//     //---------------------------------------------------
//     // 頂点情報の格納
//     //---------------------------------------------------
//     std::vector<MeshVertex> vertices; // 頂点情報
//
//     vertices.resize(pMesh->mNumVertices);
//
//     // メッシュ情報格納
//     for (UINT i = 0; i < pMesh->mNumVertices; ++i)
//     {
//         // 頂点座標格納
//         vertices[i].Position.x = pMesh->mVertices[i].x;
//         vertices[i].Position.y = pMesh->mVertices[i].y;
//         vertices[i].Position.z = pMesh->mVertices[i].z;
//
//         // UV情報が設定されていたら...
//         if (pMesh->HasTextureCoords(0))
//         {
//             //vertices[i].UV.x = 1 - static_cast<float>(pMesh->mTextureCoords[0][i].x);
//             //vertices[i].UV.y = 1 - static_cast<float>(pMesh->mTextureCoords[0][i].y);
//             vertices[i].UV.x = pMesh->mTextureCoords[0][i].x;
//             vertices[i].UV.y = pMesh->mTextureCoords[0][i].y;
//         }
//
//         // 法線情報格納
//         vertices[i].Normal.x = pMesh->mNormals[i].x;
//         vertices[i].Normal.y = pMesh->mNormals[i].y;
//         vertices[i].Normal.z = pMesh->mNormals[i].z;
//
//         // 接線情報が設定されていたら...
//         if (pMesh->HasTangentsAndBitangents())
//         {
//             vertices[i].Tangent.x = pMesh->mTangents[i].x;
//             vertices[i].Tangent.y = pMesh->mTangents[i].y;
//             vertices[i].Tangent.z = pMesh->mTangents[i].z;
//         }
//
//         // 色情報が設定されていたら...
//         if (pMesh->HasVertexColors(i))
//         {
//             Math::Color color;
//             color.x = pMesh->mColors[0][i].r;
//             color.y = pMesh->mColors[0][i].g;
//             color.z = pMesh->mColors[0][i].b;
//             color.w = pMesh->mColors[0][i].a;
//
//             vertices[i].Color = color.RGBA().v;
//         }
//
//         // ボーン情報の初期化
//         for(UINT j = 0; j < 4; ++j)
//         {
//             vertices[i].BoneIDs[j] = 0;
//             vertices[i].BoneWeights[j] = 0.0f;
//         }
//     }
//
//     //---------------------------------------------------
//     // 面情報の格納
//     //---------------------------------------------------
//     std::vector<MeshFace> faces; // 面情報
//
//     faces.resize(pMesh->mNumFaces);
//
//     for (UINT i = 0; i < pMesh->mNumFaces; ++i)
//     {
//         const aiFace& face = pMesh->mFaces[i];
//
//         // 面情報格納
//         for(UINT j = 0; j < 3; ++j)
//         {
//             faces[i].Idx[j] = face.mIndices[j];
//         }
//     }
//
//     // 読み込んだ情報からメッシュを作成する
//     mesh.Create(vertices, faces, ParseMaterial(pMaterial, filePath));
//
//     //---------------------------------------------------
//     // ボーン情報の作成
//     //---------------------------------------------------
//     std::unordered_map<std::string, std::pair<int, Math::Matrix>> boneInfo = {};
//     std::vector<UINT> boneCounts;
//     boneCounts.resize(vertices.size(), 0);
//
//     for (UINT i = 0; i < pMesh->mNumBones; ++i)
//     {
//         const aiBone* pBone = pMesh->mBones[i];
//
//         // ボーンの逆行列を取得
//         // ※ Assimp と DirectX::SimpleMath::Matrix では行列の並びが異なるので変換
//         dstNode.Bone.OffsetMatrix = SwapMatrix4x4Order(pBone->mOffsetMatrix);
//
//         boneInfo[pBone->mName.C_Str()] = { i, dstNode.Bone.OffsetMatrix };
//
//         // そのボーンを持つ各頂点をループする
//         for (UINT j = 0; j < pBone->mNumWeights; j++)
//         {
//             UINT id = pBone->mWeights[j].mVertexId;
//             float weight = pBone->mWeights[j].mWeight;
//
//             boneCounts[id]++;
//
//             switch (boneCounts[id])
//             {
//             case 1:
//                 vertices[id].BoneIDs[0] =static_cast<short>(i);
//                 vertices[id].BoneWeights[0] = weight;
//                 break;
//
//             case 2:
//                 vertices[id].BoneIDs[1] =static_cast<short>(i);
//                 vertices[id].BoneWeights[1] = weight;
//                 break;
//
//             case 3:
//                 vertices[id].BoneIDs[2] = static_cast<short>(i);
//                 vertices[id].BoneWeights[2] = weight;
//                 break;
//
//             case 4:
//                 vertices[id].BoneIDs[3] = static_cast<short>(i);
//                 vertices[id].BoneWeights[3] = weight;
//                 break;
//
//             default:
//                 FNENG_ASSERT_LOG("頂点にボーンを割り当てられない", false)
//                 break;
//             }
//         }
//     }
//
//     // ボーンが一つもない場合は、スキンメッシュではないのでデータは読み込まない
//     if(pMesh->mNumBones <= 0)
//     {
//         //　すべての重みの合計が 1 になるように重みを正規化する。
//         for (auto& v : vertices)
//         {
//             std::array<float, 4>& boneWeights = v.BoneWeights;
//
//             float totalWeight = boneWeights[0] + boneWeights[1] + boneWeights[2] + boneWeights[3];
//
//             if (totalWeight <= 0.0f) { continue; }
//
//             v.BoneWeights = std::array{
//                 boneWeights[0] / totalWeight,
//                 boneWeights[1] / totalWeight,
//                 boneWeights[2] / totalWeight,
//                 boneWeights[3] / totalWeight
//             };
//         }
//
//         ParseSkeleton(dstNode.Bone, pScene->mRootNode, boneInfo);
//     }
//
// }
//
// const Material ModelLoader::ParseMaterial(const aiMaterial* pMaterial, const std::string& filePath)
// {
//     // テクスチャのディレクトリパスを取得
//     std::string dirPath = utl::file::GetDirFromPath(filePath);
//
//     Material material = {};
//
//     // マテリアルの名前を取得
//     {
//         aiString name;
//
//         if (pMaterial->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
//         {
//             material.Name = name.C_Str();
//         }
//     }
//
//     // Diffuseテクスチャの取得
//     {
//         aiString path;
//
//         if (pMaterial->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &path) == AI_SUCCESS)
//         {
//             auto fileDir = std::string(path.C_Str());
//             material.spBaseColorTex = AssetManager::Instance().GetTexture(dirPath + fileDir);
//
//             if (!material.spBaseColorTex)
//             {
//                 FNENG_ASSERT_ERROR("Diffuseテクスチャのロードに失敗");
//                 return Material();
//             }
//         }
//     }
//
//     // DiffuseColorの取得
//     {
//         aiColor4D diffuse;
//
//         if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
//         {
//             material.BaseColor.x = diffuse.r;
//             material.BaseColor.y = diffuse.g;
//             material.BaseColor.z = diffuse.b;
//             material.BaseColor.w = diffuse.a;
//         }
//     }
//
//     // MetallicRoughnessテクスチャの取得
//     {
//         aiString path;
//
//         if (pMaterial->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &path) == AI_SUCCESS ||
//             pMaterial->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &path) == AI_SUCCESS)
//         {
//             auto fileDir = std::string(path.C_Str());
//
//             material.spMetallicRoughnessTex = AssetManager::Instance().GetTexture(dirPath + fileDir);
//
//             if (!material.spMetallicRoughnessTex)
//             {
//                 FNENG_ASSERT_ERROR("MetallicRoughnessテクスチャのロードに失敗");
//                 return Material();
//             }
//         }
//     }
//
//     // Metallicを取得
//     {
//         float metallic = 0.0f;
//
//         if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
//         {
//             material.Metallic = metallic;
//         }
//     }
//
//     // Roughness
//     {
//         float roughness = 0.0f;
//
//         if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
//         {
//             material.Roughness = roughness;
//         }
//     }
//
//     //
//     // テクスチャの取得
//     {
//         aiString path;
//
//         if (pMaterial->GetTexture(AI_MATKEY_EMISSIVE_TEXTURE, &path) == AI_SUCCESS)
//         {
//             auto fileDir = std::string(path.C_Str());
//
//             material.spEmissiveTex = AssetManager::Instance().GetTexture(dirPath + fileDir);
//
//             if (!material.spEmissiveTex)
//             {
//                 FNENG_ASSERT_ERROR("MetallicRoughnessテクスチャのロードに失敗");
//                 return Material();
//             }
//         }
//     }
//
//     // Emissiveの取得
//     {
//         aiColor3D emissive;
//
//         if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
//         {
//             material.Emissive.x = emissive.r;
//             material.Emissive.y = emissive.g;
//             material.Emissive.z = emissive.b;
//         }
//     }
//
//     // 法線テクスチャの取得
//     {
//         aiString path;
//
//         if (pMaterial->GetTexture(AI_MATKEY_NORMAL_TEXTURE, &path) == AI_SUCCESS)
//         {
//             auto fileDir = std::string(path.C_Str());
//
//             material.spNormalTex = AssetManager::Instance().GetTexture(dirPath + fileDir);
//
//             if (!material.spNormalTex)
//             {
//                 FNENG_ASSERT_ERROR("Normalテクスチャのロードに失敗");
//                 return Material();
//             }
//         }
//     }
//
//     return material;
// }
//
// bool ModelLoader::ParseSkeleton(ModelData::Node::BoneData& DstBone, const aiNode* node,
//     std::unordered_map<std::string, std::pair<int, Math::Matrix>>& boneInfoMap)
// {
//     // もし実際にノードがボーンであれば...
//     if(boneInfoMap.find(node->mName.C_Str()) != boneInfoMap.end())
//     {
//         DstBone.Name = node->mName.C_Str();
//         DstBone.Index = boneInfoMap[DstBone.Name].first;
//         DstBone.OffsetMatrix = boneInfoMap[DstBone.Name].second;
//
//         // ボーンの子ノードを再帰的に処理
//         for(UINT i = 0; i < node->mNumChildren; ++i)
//         {
//             ModelData::Node::BoneData childBone;
//             ParseSkeleton(childBone, node->mChildren[i], boneInfoMap);
//             DstBone.Children.push_back(childBone);
//         }
//
//         return true;
//     }
//     // 子ボーンが見つかったら...
//     else
//     {
//         for(UINT i = 0; i < node->mNumChildren; ++i)
//         {
//             // ボーンが見つかったら関数を抜けていく
//             if(ParseSkeleton(DstBone, node->mChildren[i], boneInfoMap))
//             {
//                 return true;
//             }
//         }
//     }
//
//     return false;
// }
