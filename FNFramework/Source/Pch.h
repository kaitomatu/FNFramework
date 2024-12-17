#pragma once

//============================================
// プリコンパイル済みヘッダー
// ここに書いたものは初回のみ解析されるため、コンパイルが高速になる
// すべての cpp からインクルードされる必要がある
//============================================

//===============================
// 基本
//===============================
#pragma comment(lib, "winmm.lib")

#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <cassert>

#include <wrl/client.h>
//===============================
// STL
//===============================
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <stack>
#include <list>
#include <iterator>
#include <queue>
#include <algorithm>
#include <memory>
#include <random>
#include <fstream>
#include <sstream>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <fileSystem>
#include <chrono>
#include <source_location>
#include <bitset>
#include <set>
#include <stdint.h>
#include <type_traits>

#define _USE_MATH_DEFINES
#include <math.h>

//===============================
// strconv
//===============================
#include "strconv.h"

//===============================
// nameof
//===============================
#include "nameof.hpp"

//===============================
// DirectX3D12
//===============================
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d12.h>
#include <dxgi1_6.h>

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

// DirectX Tool Kit
#pragma comment(lib, "DirectXTK12.lib")
#include <SimpleMath.h>

// DirectX Tex
#pragma comment(lib, "DirectXTex.lib")
#include <DirectXTex.h>

//===============================
// Audio
//===============================
// XAudio2関連
#pragma comment(lib, "xaudio2.lib")
#include<xaudio2.h>

// マルチメディア関連
#pragma comment ( lib, "winmm.lib" )
#include<mmsystem.h>

//===============================
// ImGui
//===============================

#define IMGUI_DEFINE_MATH_OPERATORS
#include"imgui/imgui.h"
#include"imgui/imgui_impl_win32.h"
#include"imgui/imgui_impl_dx12.h"

//===============================
// Json
//===============================
#include "json.hpp"

//===============================
// 自作プログラム
//===============================
#include "Framework/System/System.h"

#include "DXRHelper/d3dx12.h"
