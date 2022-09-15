#include "BossAction.h"

#include <d3dcompiler.h>
#include <DirectXTex.h>
#include <fstream>
#include<sstream>
#include<string>
#include<vector>
#pragma comment(lib, "d3dcompiler.lib")
using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
XMFLOAT3 BossAction::StartTop = {0,-30,650};

XMFLOAT3 BossAction::StartRightBottom{ 30,-70,800 };
XMFLOAT3 BossAction::StartLeftBottom{ -30,-70,700 };
void BossAction::appearance(XMFLOAT3& bossobj, XMFLOAT3& bossobj2, XMFLOAT3& bossscale, XMFLOAT3& bossscale2)
{

	if (set == 0) {
		bossobj = { -30,-500,800 };
		bossscale = { 0,0,0 };
		bossobj2 = { -30,-500,800 };
		bossscale2 = { 0,0,0 };
		set = 1;
	}
	if (set == 1) {
		if (bossscale.x <= 550) {
			bossscale.x+=5;
			bossscale.y+=5;
			bossscale.z+=5;

			bossscale2.x += 5;
			bossscale2.y += 5;
			bossscale2.z += 5;
		}
	}
}

void BossAction::circleattack(XMFLOAT3 *obj, XMFLOAT3* objscale, XMFLOAT3* hp, int* alive, XMFLOAT3* rot)
{
	//XMConvertToRadians(90.0f);//Ž©‹@‚Ì‰ñ“]
	//playerangle++;
	angle += XMConvertToRadians(15.0f);//Ž©‹@‚Ì‰ñ“]
	
	if (circleset == 0) {
		for (int i = 0; i < 20; i++) {
			rot[i] = { 0,180,0 };
			obj[i].z = 700;
			objscale[i] = { 5,5,5 };
			hp[i] = { 1.5,0.5,1 };
			alive[i] = 1;
		}
		circleset = 1;
	}
	//radius+=0.5f;
	if (circleset == 1) {
		for (int i = 0; i < 5; i++) {
			obj[i].x = 20 + cosf((float)(angle * i * 5) * PI / 180) * radius;
			obj[i].y = -20 + sinf((float)(angle * i * 5) * PI / 180) * radius;
			obj[i].z -= 0.7;
			if (obj[i].z <= 300) {
				setbul = 1;
			}

		}
		if (setbul == 1) {
			for (int i = 10; i < 15; i++) {
				obj[i].x = -10 + cosf((float)(angle * i) * PI / 180) * radius;
				obj[i].y = -50 + sinf((float)(angle * i) * PI / 180) * radius;
				obj[i].z -= 0.7;
				if (obj[i].z <= 300) {
					setbul = 2;
				}
			}
		}
		if (setbul == 2) {
			for (int i = 15; i < 20; i++) {
				obj[i].x = 20 + cosf((float)(angle * i) * PI / 180) * radius;
				obj[i].y = 20 + sinf((float)(angle * i) * PI / 180) * radius;
				obj[i].z -=0.7;

			}
		}
	}
}

void BossAction::normalattack(XMFLOAT3* obj, int* alive, XMFLOAT3* objscale, XMFLOAT3* hp,XMFLOAT3* rot)
{
	
	//for (int i = 0; i < 5; i++) {
		if (Shot_Flag[0] == 0) {
			obj[0] = StartTop;
			Shot_Flag[0] = 1;
			//break;
		}
		if (Shot_Flag[0] == 1) {
			srand((unsigned int)time(NULL));
			obj[0].z -= 1.05f;
			if (obj[0].z < -50) {
				hp[0] = { 1.5,0.5,1 };
				alive[0] = 1;
				objscale[0] = { 5,5,5 };
				StartTop.z = 700;
				StartTop.y = rand() % -20 - 80;
				StartTop.x = rand() % 150 - 60;
				Shot_Flag[0] = 0;
			}
	
		}
//	}

	//for (int i = 5; i < 10; i++) {
		if (Shot_Flag[1] == 0) {
			obj[1] = StartRightBottom;
			Shot_Flag[1] = 1;
			//break;
		}
		if (Shot_Flag[1] == 1) {
			srand((unsigned int)time(NULL));
			obj[1].z -= 0.9f;
			if (obj[1].z < -50) {
				hp[1] = { 1.5,0.5,1 };
				alive[1] = 1;
				objscale[1] = { 5,5,5 };
				StartRightBottom.z = 700;
				StartRightBottom.y = rand() % 150 - 60;
				StartRightBottom.y = rand() % -20 - 70;
				Shot_Flag[1] = 0;
			}
		
	}

	
		if (Shot_Flag[2] == 0) {
			obj[2] =StartLeftBottom;
			Shot_Flag[2] = 1;
		}
		if (Shot_Flag[2] == 1) {
			srand((unsigned int)time(NULL));
			obj[2].z -= 0.9f;
		if (obj[2].z < -50) {
			hp[2] = { 1.5,0.5,1 };
			alive[2] = 1;
			objscale[2] = { 5,5,5 };
			StartLeftBottom.z = 700;
			StartLeftBottom.y = rand() % -20 - 70;
			Shot_Flag[2] = 0;
		}
		}

		if (Shot_Flag[3] == 0) {
			obj[3] = { -80, -20, 750 };
			Shot_Flag[3] = 1;
		}
		if (Shot_Flag[3] == 1) {
			srand((unsigned int)time(NULL));
			obj[3].z -= 1.05f;
			if (obj[3].z < -50) {
				hp[3] = { 1.5,0.5,1 };
				alive[3] = 1;
				objscale[3] = { 5,5,5 };
				obj[3].z = 700;
				obj[3].x = rand() % -120 - 80;
				obj[3].y = rand() % -20 - 70;
				Shot_Flag[3] = 0;
			}
		}

		if (Shot_Flag[4] == 0) {
			obj[4] = { 80, -20, 720 };
			Shot_Flag[4] = 1;
		}
		if (Shot_Flag[4] == 1) {
			srand((unsigned int)time(NULL));
			obj[4].z -= 0.9f;
			if (obj[4].z < -50) {
				hp[4] = { 1.5,0.5,1 };
				alive[4] = 1;
				objscale[4] = { 5,5,5 };
				obj[4].z = 700;
				obj[4].x = rand() % 80;
				obj[4].y = rand() % 20 - 70;
				Shot_Flag[4] = 0;
			}
		}
		for (int i = 5; i < 20; i++) {
			obj[i] = { 1000,1000,-30 };
		}
		for (int i = 0; i < 20; i++) {
			rot[i].y += i;
		}
}

void BossAction::Shot_Bullet(XMFLOAT3* obj)
{
	
}

void BossAction::Feedin(XMFLOAT3& bossobj, XMFLOAT3& bossobj2, float& feed, float& cameraz, float& target, float& playerz, XMFLOAT3&scl, XMFLOAT3& scl2)
{
	switch (feedset)
	{
	case 0:
		feed += 0.05f;
		if (feed >= 1.5) {
			feedset = 1;
		}
		if (feed > 1) {
			cameraz = target;
		}
		break;

	case 1:
		
			feed -= 0.01f;
			if (feed <= 0) {
				appearance(bossobj, bossobj2,scl, scl2);
	}
			if (scl.x >= 180) {
				feedset = 2;
			}
		break;

	case 2:
		if (cameraz >= playerz) {
			cameraz -= 2.4;
		}
		if (cameraz < playerz)
		{
			feedset = 3;
		}
		break;
	}
}

void BossAction::motion(XMFLOAT3& obj)
{
	if (motionset == 0) {
		obj.y -= 1;
		if (obj.y <= -650) {
			motionset = 1;
		}
	}
	else if (motionset == 1) {
		obj.y += 0.5;
		if (obj.y >= -450) {
			motionset = 0;
		}
	}
}

void BossAction::losemotion(XMFLOAT3& obj)
{
	obj.x -= 2;
}
void BossAction::Rotmotion(XMFLOAT3& obj)
{
	if (rotset == 0) {
		if (obj.y <= 550) {
			obj.y += 7;
		}
		if (obj.y >= 550) {
			timer++;
		}
		if (timer > 80) {
			rotset = 1;
		}
	}
	if (rotset == 1) {
		obj.y -= 7;
		if (obj.y <= 180) {
			obj.y = 180;
			timer = 0;
		}
	}
}