#include "Input.h"
#include"WinApp.h"
//#pragma comment(lib,"dxguid.lib")
void Input::Initialize(WinApp* winapp)
{
	HRESULT result;
	//winappのインスタンスを記録
	this->winapp = winapp;
	//DirectInputのインスタンス生成
	ComPtr<IDirectInput8>dinput = nullptr;
	result = DirectInput8Create(winapp->GetInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);
	//キーボードデバイ氏生成
	//ComPtr<IDirectInputDevice8>devkeyboard;
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	// マウスデバイスの生成	
	result = dinput->CreateDevice(GUID_SysMouse, &devMouse, NULL);

	//入力データ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);
	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(winapp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	//mouse
	// 入力データ形式のセット
	result = devMouse->SetDataFormat(&c_dfDIMouse2); // 標準形式
	
	// 排他制御レベルのセット
	result = devMouse->SetCooperativeLevel(winapp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

}
void Input::update()
{
	HRESULT result;
	{
		//前回のキー情報を保存
		memcpy(keypre, key, sizeof(key));
		//キーボード情報の取得開始
		result = devkeyboard->Acquire();
		//全キーの入力情報取得
		//BYTE key[256] = {};
		result = devkeyboard->GetDeviceState(sizeof(key), key);
	}
	{// マウス
		result = devMouse->Acquire();	// マウス動作開始

		// 前回の入力を保存
		mouseStatePre = mouseState;

		// マウスの入力
		result = devMouse->GetDeviceState(sizeof(mouseState), &mouseState);
	}
}

bool Input::Pushkey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

//トリガー
bool Input::TriggerKey(BYTE keyNumber)
{
	//前回の結果が０で今回の会話が１のとき
	if (!keypre[keyNumber]&&key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::PushMouseLeft()
{
	// 0でなければ押している
	if (mouseState.rgbButtons[0]) {
		return true;
	}

	// 押していない
	return false;
}

bool Input::PushMouseMiddle()
{
	// 0でなければ押している
	if (mouseState.rgbButtons[2]) {
		return true;
	}

	// 押していない
	return false;
}

bool Input::TriggerMouseLeft()
{
	// 前回が0で、今回が0でなければトリガー
	if (!mouseStatePre.rgbButtons[0] && mouseState.rgbButtons[0]) {
		return true;
	}

	// トリガーでない
	return false;
}

bool Input::TriggerMouseMiddle()
{
	// 前回が0で、今回が0でなければトリガー
	if (!mouseStatePre.rgbButtons[2] && mouseState.rgbButtons[2]) {
		return true;
	}

	// トリガーでない
	return false;
}

Input::MouseMove Input::GetMouseMove()
{
	MouseMove tmp;
	tmp.lX = mouseState.lX;
	tmp.lY = mouseState.lY;
	tmp.lZ = mouseState.lZ;
	return tmp;
}
