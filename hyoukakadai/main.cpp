
#include<Windows.h>
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<vector>
#include<string>
#include<DirectXMath.h>
#include<d3dcompiler.h>
#include<dinput.h>
#include<dinput.h>

#include<DirectXTex.h>
#include<wrl.h>

//まっつんホーム汚すぎ
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"d3d12.lib")

#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include"Audio.h"
#include"Sprite.h"
#include"Input.h"
#include"WinApp.h"
#include"DirectXCommon.h"
#include"Player.h"
#include"Object3d.h"
#include"Model.h"
#include"Texture.h"
#include"GameScene.h"
#pragma endregion

using namespace DirectX;
using namespace Microsoft::WRL;

static ID3D12Device* g_pd3dDevice = NULL;
static int const NUM_FRAMES_IN_FLIGHT = 3;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#pragma region DirectXの初期化
	const int window_width = 900;
	const int window_height = 720;

	WinApp* winapp = nullptr;
	//初期化
	winapp = new WinApp();
	winapp->Createwindow();

	//でバッグレイヤーのやつ
#ifdef _DEBUG
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif

#pragma region 入力と共通部分のポインタ置き場
	//DirectXの共通部分
	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winapp);
	
	// キーボードなどの入力処理
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winapp);

	Audio* audio = nullptr;
	audio = new Audio();
	
#pragma endregion

#pragma region 静的初期化
	Sprite::StaticInitialize(dxCommon->GetDev(), window_width, window_height);
	Texture::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height);
	Object3d::StaticInitialize(dxCommon->GetDev(), WinApp::window_width, WinApp::window_height);
	GameScene* gamescene = new GameScene();

	gamescene->Initialize();
#pragma endregion
	while (true) {
		//ウィンドウメッセージ処理
		if (winapp->Processmassage()) {
			break;
		}
		input->update();

		gamescene->Update(dxCommon, input);
		
		dxCommon->BeginDraw();//描画コマンドの上らへんに
		gamescene->Draw(dxCommon);
		dxCommon->EndDraw();//ループl内の末尾らへんに
	}
	delete audio;
	delete input;
	delete dxCommon;
	delete winapp;
	delete gamescene;
	return 0;
}

