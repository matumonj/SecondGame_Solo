#pragma once
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
#include"Field.h"
#include"MoveField.h"
#include"Player.h"
#include"Object3d.h"
#include"Model.h"
#include"Switch.h"
#include"Texture.h"
#include"BossAction.h"
#include"Cllision.h"
using namespace DirectX;
using namespace Microsoft::WRL;

class GameScene
{
private: // エイリアス
	////円当たり判定
	//float collision(XMFLOAT3 object1, XMFLOAT3& object2) {
	//	float a = object1.x - object2.x;
	//	float b = object1.y - object2.y;
	//	float c = object1.z - object2.z;
	//	float distance = sqrtf(a * a + b * b + c * c);
	//	//int radius = R1;
	//	return distance;

	//}
// Microsoft::WRL::を省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	// DirectX::を省略
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;
	using XMVECTOR = DirectX::XMVECTOR;
private:
	const float window_width = 900;
	const float window_height = 720;

	Input* input = nullptr;
	DirectXCommon* dxcomn = nullptr;
	Sprite* spriteBG = nullptr;
	Sprite* BackGround = nullptr;
	Sprite* BackGround2 = nullptr;
	Sprite* BackGround3 = nullptr;
	Sprite* Feed = nullptr;
	Sprite* Feedw = nullptr;
	Sprite* title = nullptr;
	Sprite* clear = nullptr;
	Sprite* lose = nullptr;
	Sprite* rule = nullptr;
	Sprite* damagev = nullptr;
private:
	static const int Bmax = 19;
	static const int Emax = 11;
	static const int Lmax = 10;
	static const int BBmax = 20;
private:
	Cllision* col = nullptr;
	Object3d* player = nullptr;
	Object3d* lazer = nullptr;
	Object3d* Boss = nullptr;
	Object3d* Bossbotm = nullptr;
	Object3d* object3d2[Bmax] = {nullptr};
	Object3d* enemy[Emax] = {nullptr};
	Object3d* BossBullet[BBmax] = {nullptr};// = new Object3d();

	BossAction* baction = nullptr;
	Model* model;
	Model* model2; 
	Model* model3;
	Model* model4;
	Model* model5;
	Model* model6;
	//
	Texture* EnemyHP[Emax];
	Texture* BossBulletHP[BBmax];
	Sprite* BossHP = nullptr;
	
	XMFLOAT3 Player_Pos;// = player->GetPosition();
	XMFLOAT3 Player_Rot;// = player->GetRotation();

	XMFLOAT3 Bullet_Pos[Bmax];

	XMFLOAT3 Enemy_Pos[Emax];
	XMFLOAT3 Enemy_Scl[Emax];

	XMFLOAT3 Boss_Pos;// = Boss->GetPosition();
	XMFLOAT3 Boss_Scl;// = Boss->GetScale();
	XMFLOAT3 Boss_Rot;// = { 0,180,0 };
	XMFLOAT3 Bossbotm_Pos;// = Bossbotm->GetPosition();
	XMFLOAT3 Bossbotm_Scl;// = Bossbotm->GetScale();
	XMFLOAT3 BossBullet_Pos[BBmax];
	XMFLOAT3 BossBullet_Scl[BBmax];
	XMFLOAT3 BossBulletHP_Scl[BBmax];
	XMFLOAT3 BossBullet_Rot[BBmax];

	XMFLOAT2 playerHPsize = { 200,60 };
	float backposition = 0;

	XMFLOAT3 Lazer_Pos;// = { player->GetPosition().x, player->GetPosition().y + 0.1f, player->GetPosition().z };
	XMFLOAT3 Lazer_Rot;// = player->GetRotation();
	
	XMFLOAT2 Bosshp_Pos = { 50,20 };
	XMFLOAT3 Bosshp_Scl = { 50,3,1 };


#pragma endregion

	//射影変換行列の作り
	XMMATRIX matprojection;// = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)window_width / window_height, 0.1f, 1000.0f);

	//ビュー変換行列
	XMMATRIX matview;
	XMFLOAT3 eye;// (0, 0, -10);
	XMFLOAT3 target;// (0, 0, 0);
	XMFLOAT3 up;// (0, 1, 0);
	//matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

#pragma region カメラ関係
	float d = 20;
	float angle = 0;
	XMVECTOR v0;// = { player->GetPosition().x + 0.1f, player->GetPosition().y + 2.0f,-20.0f,0.0f };
	XMMATRIX rotM;// = XMMatrixIdentity();
	XMVECTOR eye2;
	XMVECTOR target2;// = { field[0]->GetPosition().x,field[0]->GetPosition().y,field[0]->GetPosition().z, 0 };
	XMVECTOR up2 = { 0,0.3f,0,0 };
	
#pragma endregion
	int scene = 0;

	int ShotFlag[Bmax];
	int set[Bmax];
	float xspeed[Bmax], yspeed[Bmax];
	float speedcs[Bmax];
	float bspeed[Bmax];
	
	float rpos[Emax];
	
	float dy;
	float dx;
	float dz;
	float rotb;// = Player_Rot.y;
	float rotb2;// = Player_Rot.x;

	float bfs = 3.2;
	XMFLOAT3 pscale = { 5,5,5 };
	float alpha = 1;
	int damageflag = 0;
	int enemyalive[Emax];
	XMFLOAT3 enemyscl[Emax];
	int view[Emax];
	int stf[Emax];
	
	int enemydethcount = 0;
	XMFLOAT4 ec[Emax];
	
	int BossScene = 0;
	int bossbulalive[BBmax];
	
	float xsize = 800;
	float feed = 0;
	float feedd = 0;
	int damagef = 0;
	float feedw = 0;
	float backRot = 0;
	float playertarget;
	float bosstarget;
	XMFLOAT4 bosscolor = { 1,1,1,1 };
	float vol = 0.3;
	int pushCount = 0;
	Audio* audio =nullptr;
	int scenetimer = 0;
public:
	void Initialize();
	void Update(DirectXCommon* dxCommon, Input* input);
	void Draw(DirectXCommon* dxCommon);

	void TitleDraw(DirectXCommon* dxCommon);
	void GameSceneDraw(DirectXCommon* dxCommon);
	void ClearDraw(DirectXCommon* dxCommon);
	void LoseDraw(DirectXCommon* dxCommon);
	void RuleDraw(DirectXCommon* dxCommon);


	void SpriteCreate();
	void ModelCreate();

	void SetPrm();

	void TitleInitialize();
	void objUpdate();
};

