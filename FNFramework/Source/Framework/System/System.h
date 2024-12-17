#pragma once

// 簡易アサート表示
#include "Framework/System/Utility/Assert.h"
// Utilityファイル
#include "Framework/System/Utility/Utility.h"
#include "Framework/System/Utility/String.h"
#include "Framework/System/Utility/File.h"
// MathHelper
#include "Framework/System/Math/MathHelper.h"
// 簡易シングルトンクラス
#include "Framework/System/Utility/Singleton.h"
// 簡易ステートマシンクラス
#include "Framework/System/Utility/StateMachine.h"

//======================
// Helper
//======================
// ImGuiHelper
#include "Framework/System/ImGui/ImGuiDevice/ImGuiDevice.h"
// MessageHelper
#include "Framework/System/GraphicsHelper/Message/MessageHelper.h"
// RandomHelper
#include "Framework/System/Utility/RandomHelper.h"

//======================
// 数学関係
//======================
#include "Framework/System/Math/Collision/CollisionData/CollisionData.h"	// 当たり判定基底クラス
#include "Framework/System/Math/Collision/CollisionData/Box.h"				// ボックス
#include "Framework/System/Math/Collision/CollisionData/LineSegment.h"		// 線分
#include "Framework/System/Math/Collision/CollisionData/BoundingBox.h"		// AABB / OBB
#include "Framework/System/Math/Collision/CollisionData/Capsule.h"			// カプセル
#include "Framework/System/Math/Collision/CollisionData/Polygon.h"			// 多角形
#include "Framework/System/Math/Collision/CollisionData/Plane.h"			// 平面
#include "Framework/System/Math/Collision/CollisionData/Sphere.h"			// 球体

// 当たり判定
//#include "Framework/Math/Collision/Collision.h"
#include "Framework/System/Math/Collision/Collider.h"	// 当たり判定

// 時間計測クラス
#include "Framework/System/Math/Timer/Timer.h"

//======================
// 描画関係
//======================// デバイス
#include "Framework/Graphics/GraphicsDevice.h"
// ヒープ
#include "Framework/Graphics/Heap/Heap.h"
#include "Framework/Graphics/Heap/RTVHeap/RTVHeap.h"
#include "Framework/Graphics/Heap/CBVSRVUAVHeap/CBVSRVUAVHeap.h"
#include "Framework/Graphics/Heap/DSVHeap/DSVHeap.h"
// 定数バッファのアロケーター
#include "Framework/Graphics/Buffer/CBufferAllocater/CBufferAllocater.h"
// 定数バッファラッピング
#include "Framework/Graphics/Buffer/CBufferAllocater/CBufferData/Constantbuffer.h"
// 定数バッファデータ
#include "Framework/Graphics/Buffer/CBufferAllocater/CBufferData/CBufferData.h"
// デプスステンシル
#include "Framework/Graphics/Buffer/DepthStencil/DepthStencil.h"
// テクスチャ
#include "Framework/Graphics/Buffer/ShaderResourceTexture/ShaderResourceTexture.h"
// レンダーターゲット
#include "Framework/Graphics/Buffer/RenderTarget/RenderTarget.h"
// メッシュ
#include "Framework/Graphics/Shape/Mesh/Mesh.h"
#include "Framework/Graphics/Shape/Mesh/SpriteMesh.h"
// モデル
#include "Framework/Graphics/Model/ModelData/Model.h"
#include "Framework/Graphics/Model/Animation/Animation.h"

// Shader
#include "Framework/Graphics/Shader/Shader.h"

// カメラ
#include "Application/Object/Camera/Camera.h"

//======================
// Collision
//======================
// 当たり判定
#include "Framework/KDFramework/KdCollision.h"
#include "Framework/KDFramework/KdCollider.h"
#include "Framework/System/Math/Collision/CollisionHelper.h"

//======================
// Manager
//======================

//------------
// Scene
//------------
// コンポーネント基底クラス
#include "Application/Component/BaseComponent.h"
// オブジェクト基底クラス
#include "Application/Object/GameObject.h"
// シーン基底クラス
#include "Application/System/SceneManager/Scene/Scene.h"
// シーンマネージャー
#include "Application/System/SceneManager/SceneManager.h"
//------------
// Renderer
//------------
// 描画クラス
#include "Application/System/Renderer/Renderer.h"

// ガウシアンブラー
#include "Framework/Manager/Shader/PostProcess/GaussianBlur.h"

// シェーダーマネージャー
#include "Framework/Manager/Shader/ShaderManager.h"


//------------
// Asset
//------------
// アセット管理クラス
#include "Framework/Manager/Asset/AssetsManager.h"

//------------
// Audio
//------------
#include "Framework/Audio/AudioDevice.h"
#include "Framework/Audio/SoundData.h"

//======================
// Device
//======================
// マウス
#include "Device/Mouse/Mouse.h"

//======================
// System
//======================
// ウィンドウメッセージ処理
#include "Framework/System/Window/Window.h"
// FPS管理 / 計測
#include "Framework/System/Math/FPSController/FPSController.h"
// デバッグワイヤー
#include "Framework/System/Math/Collision/DebugWire.h"

#include "Framework/System/Utility/ImGuiHelper.h"
