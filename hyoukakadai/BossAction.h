#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include<time.h>
#define PI 3.141592654
class BossAction
{
private: // エイリアス
// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
private:
	
private:
	float angle=0;
	float radius = 24;
	int set = 0;
	int Shot_Flag[20];
	int bset = 0;
	int setbul = 0;
	int circleset = 0;
	static XMFLOAT3 StartTop;
	static XMFLOAT3 StartRightBottom;
	static XMFLOAT3 StartLeftBottom;
	static XMFLOAT3 StartRightUp;
	int feedset = 0;
	int motionset = 0;
	int rotset = 0;
	int timer=0;
public:
	void SetShflag(int set) {
		for (int i = 0; i < 20; i++) {
			this->Shot_Flag[i] = set;
		}
	}
	const int max = 10;
	void circleattack(XMFLOAT3 *obj, XMFLOAT3* objscale,XMFLOAT3*hp, int* alive,XMFLOAT3* rot);
	void appearance(XMFLOAT3& bossobj, XMFLOAT3& bossobj2,XMFLOAT3& bossscale, XMFLOAT3& bossscale2);
	void normalattack(XMFLOAT3* obj, int* alive, XMFLOAT3* objscale, XMFLOAT3* hp,XMFLOAT3*rot);
	void Shot_Bullet(XMFLOAT3* obj);
	void Feedin(XMFLOAT3& bossobj, XMFLOAT3& bossobj2, float& feed, float& cameraz, float& target, float& playerz, XMFLOAT3& scl, XMFLOAT3& scl2);
	void motion(XMFLOAT3& obj);
	void Rotmotion(XMFLOAT3& obj);
	void losemotion(XMFLOAT3& obj);
public:
	int getsetbuf() { return setbul; }
	void Setsetbul(int setbul) { this->setbul = setbul; }
	void Setcircleset(int setbul) { this->circleset = setbul; }
	void Setfeed(int feed) { this->feedset = feed; }
	int Getfeedset() { return feedset; }
	void Setrotset(int set) { this->rotset = set; }
	void settimer(int timer) { this->timer = timer; }
};

