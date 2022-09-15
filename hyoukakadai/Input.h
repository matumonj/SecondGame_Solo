#pragma once
#include<Windows.h>
#include<wrl.h>
#include<dinput.h>
#include"WinApp.h"
#define DIRECTINPUT_VERSION 0x0800
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
using namespace Microsoft::WRL;


class Input
{
private:
	ComPtr<IDirectInputDevice8> devkeyboard;
	BYTE key[256] = {};
	BYTE keypre[256] = {};


	ComPtr<IDirectInputDevice8> devMouse;
	DIMOUSESTATE2 mouseState = {};
	DIMOUSESTATE2 mouseStatePre = {};

	WinApp* winapp = nullptr;
public:

	//namespace省略
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	struct MouseMove {
		LONG    lX;
		LONG    lY;
		LONG    lZ;
	};
	//初期化
	void Initialize(WinApp*winapp);
	//更新
	void update();
	//キーの押下をチェック
	bool Pushkey(BYTE keyNumber);
	//キーのトリガー判定
	bool TriggerKey(BYTE keyNumber);


	/// <summary>
	/// キーの左ボタン押下をチェック
	/// </summary>
	/// <returns>押されているか</returns>
	bool PushMouseLeft();

	/// <summary>
	/// キーの中ボタン押下をチェック
	/// </summary>
	/// <returns>押されているか</returns>
	bool PushMouseMiddle();

	/// <summary>
	/// キーの左ボタントリガーをチェック
	/// </summary>
	/// <returns>トリガーか</returns>
	bool TriggerMouseLeft();

	/// <summary>
	/// キーの中ボタントリガーをチェック
	/// </summary>
	/// <returns>トリガーか</returns>
	bool TriggerMouseMiddle();

	/// <summary>
	/// マウス移動量を取得
	/// </summary>
	/// <returns>マウス移動量</returns>
	MouseMove GetMouseMove();

};

