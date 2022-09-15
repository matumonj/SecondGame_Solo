#include "GameScene.h"

void GameScene::SpriteCreate()
{
	Sprite::LoadTexture(1, L"Resources/life.png");
	Sprite::LoadTexture(2, L"Resources/tyuta_C.png");
	Sprite::LoadTexture(3, L"Resources/tyuta_B.png");
	Sprite::LoadTexture(4, L"Resources/tyuta_D.png");
	Sprite::LoadTexture(5, L"Resources/tyuta_F.png");
	Sprite::LoadTexture(6, L"Resources/feedwhite.png");
	Sprite::LoadTexture(7, L"Resources/title.png");
	Sprite::LoadTexture(8, L"Resources/clear.png");
	Sprite::LoadTexture(9, L"Resources/lose.png");
	Sprite::LoadTexture(10, L"Resources/rule.png");
	Sprite::LoadTexture(11, L"Resources/damage.png");
	Sprite::LoadTexture(12, L"Resources/bosshp.png");
	// 背景スプライト生成
	spriteBG = Sprite::Create(1, { 0.0f,20.0f });
	BackGround = Sprite::Create(2, { 0,0 });
	BackGround2 = Sprite::Create(3, { 0,0 });
	BackGround3 = Sprite::Create(4, { 0,0 });
	Feed = Sprite::Create(5, { 0,0 });
	Feedw = Sprite::Create(6, { 0,0 });
	title = Sprite::Create(7, { 0,0 });
	clear = Sprite::Create(8, { 0,0 });
	lose = Sprite::Create(9, { 0,0 });
	BossHP = Sprite::Create(12, { 20,0 });
	rule= Sprite::Create(10, { 0,0 });
	damagev = Sprite::Create(11, { 0.0f,0.0f });
}

void GameScene::ModelCreate()
{
	player = new Object3d();
	lazer = new Object3d();
	Boss = new Object3d();
	Bossbotm = new Object3d();
	baction = new BossAction();
	model = Model::LoadFromObj("bullet");
	model2 = Model::LoadFromObj("lazer");;
	model3 = Model::LoadFromObj("subenemy");
	model4 = Model::LoadFromObj("subenemy");
	model5 = Model::LoadFromObj("BossHead");
	model6 = Model::LoadFromObj("BossReg");


	player = Object3d::Create();
	player->SetModel(model);

	lazer = Object3d::Create();
	lazer->SetModel(model2);

	Boss = Object3d::Create();
	Boss->SetModel(model5);

	Bossbotm = Object3d::Create();
	Bossbotm->SetModel(model6);

	for (int i = 0; i < BBmax; i++) {
		BossBullet[i] = new Object3d();
		BossBullet[i] = Object3d::Create();
		BossBullet[i]->SetModel(model4);
	}
	for (int i = 0; i < Bmax; i++) {
		object3d2[i] = new Object3d();
		object3d2[i] = Object3d::Create();
		object3d2[i]->SetModel(model);
	}
	for (int i = 0; i < Emax; i++) {
		enemy[i] = new Object3d();
		enemy[i] = Object3d::Create();
		enemy[i]->SetModel(model3);
	}

	for (int i = 0; i < Emax; i++) {
		EnemyHP[i] = new Texture();
		EnemyHP[i] = Texture::Create();
	}
	for (int i = 0; i < BBmax; i++) {
		BossBulletHP[i] = new Texture();
		BossBulletHP[i] = Texture::Create();
	}

}
void GameScene::Initialize()
{
	SpriteCreate();

	ModelCreate();
#pragma region 各オブジェクトの座標回転スケールの代入変数

	Player_Pos = player->GetPosition();
	Player_Rot = player->GetRotation();

	Boss_Pos = Boss->GetPosition();
	Boss_Scl = Boss->GetScale();
	Boss_Rot = { 0,180,0 };

	Bossbotm_Pos = Bossbotm->GetPosition();
	Bossbotm_Scl = Bossbotm->GetScale();

	playerHPsize = { 200,60 };
	backposition = 0;
	for (int i = 0; i < BBmax; i++) {
		//BossBullet_Pos[i] = { -30,-50,700 };
		BossBullet_Scl[i] = { 5,5,5 };
		BossBulletHP_Scl[i] = { 1.5,0.5,1 };
	}
	Lazer_Pos = { player->GetPosition().x, player->GetPosition().y + 0.1f, player->GetPosition().z };
	Lazer_Rot = player->GetRotation();
	for (int i = 0; i < Bmax; i++) {
		Bullet_Pos[i] = object3d2[i]->GetPosition();
	}
	for (int i = 0; i < Emax; i++) {
		Enemy_Pos[i] = enemy[i]->GetPosition();
		Enemy_Scl[i] = { 50,50,50 };
	}
	Bosshp_Pos = { 50,20 };
	Bosshp_Scl = { 50,3,1 };

	//射影変換行列の作り
	matprojection = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)window_width / window_height, 0.1f, 1000.0f);

	//ビュー変換行列

	eye = { 0, 0, -10 };
	target = { 0, 0, 0 };
	up = { 0, 1, 0 };
	matview = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

#pragma endregion
#pragma region カメラ関係
	d = 20;
	angle = 0;
	v0 = { player->GetPosition().x + 0.1f, player->GetPosition().y + 2.0f,-20.0f,0.0f };
	up2 = { 0,0.3f,0,0 };
	target2.m128_f32[0] = 0;// field->GetPosition().x;//object3ds[0].position.x;
	target2.m128_f32[1] = 0;// field->GetPosition().y + 20;//object3ds[0].position.y;

#pragma endregion
	scene = 0;

	Player_Rot = { 0,-101,0 };
	for (int i = 0; i < Bmax; i++) {
		ShotFlag[i] = 0;
		set[i] = 0;
		speedcs[i] = 2.5;
		bspeed[i] = 2.5;
	}

	for (int i = 0; i < Emax; i++) {
		Enemy_Pos[i].x = rand() % 150 - 100;
		Enemy_Pos[i].y = rand() % 100 - 50;
		Enemy_Pos[i].z = rand() % 450 + 400;
		rpos[i] = rand() % 450 + 400;
	}

	rotb = Player_Rot.y;
	rotb2 = Player_Rot.x;

	bfs = 3.2;
	pscale = { 5,5,5 };
	alpha = 1;
	damageflag = 0;

	for (int i = 0; i < Emax; i++) {
		enemyscl[i] = { 3,1,1 };
		//alpha[i] = 1;
		enemyalive[i] = 1;
		view[i] = 0;
		stf[i] = 0;
	}
	enemydethcount = 0;

	for (int i = 0; i < Emax; i++) {
		ec[i] = { 1,0,0,1 };
	}
	BossScene = 0;

	for (int i = 0; i < BBmax; i++) {
		bossbulalive[i] = 1;
	}
	xsize = 800;
	feed = 0;
	feedw = 0;
	backRot = 0;

	bosscolor = { 1,1,1,1 };
	vol = 0.3;
	pushCount = 0;
	audio = new Audio();
	audio->Initialize();
	audio->LoopWave("Resources/loop100216.wav", vol);
}

void GameScene::Update(DirectXCommon* dxCommon, Input* input)
{
	this->input = input;
	this->dxcomn = dxCommon;
	//タイトル
	if (scene == 0) {
		TitleInitialize();
		if (input->TriggerMouseLeft()) {
			scene = 1;
		}
	}
	//説明画面
	if (scene == 1) {
		scenetimer++;
		if (input->TriggerMouseLeft()) {
			if (scenetimer > 30) {
				scene = 2;
			}
		
		}
	}
	//ゲームオーバー
	if (scene == 3) {
		if (input->TriggerMouseLeft()) {
			scene = 0;
		}
	}
	//クリア
	if (scene == 4) {
		if (input->TriggerMouseLeft()) {
			scene = 0;
		}
	}

	//ゲーム画面
	if (scene == 2) {

		target2.m128_f32[0] = Player_Pos.x;// field->GetPosition().x;//object3ds[0].position.x;
		target2.m128_f32[1] = Player_Pos.y;// field->GetPosition().y + 20;//object3ds[0].position.y;

		//マウスの移動量
		Input::MouseMove mouseMove = input->GetMouseMove();
		if (input->PushMouseLeft()) {
			dy = mouseMove.lX;
			dx = mouseMove.lY;
			dz = mouseMove.lZ;

			Player_Rot.y -= -dy * 0.2;
			rotb -= -dy * 0.2;
			Player_Rot.z -= dx * 0.2;

			//移動量の制限
			Player_Pos = { 0,-30,-50 };
			playertarget = Player_Pos.z + 10;
			bosstarget = Boss_Pos.z - 300.5;
			rotb2 = player->GetRotation().z;
			Player_Rot.y = max(Player_Rot.y, -134);
			Player_Rot.y = min(Player_Rot.y, -47);
			rotb = max(rotb, -134);
			rotb = min(rotb, -47);
			Player_Rot.z = min(Player_Rot.z, 42);
			Player_Rot.z = max(Player_Rot.z, -24.9);
			col = new Cllision();
		
			if(Player_Rot.y > -115){
				rotb = Player_Rot.y;
			}

			//弾の処理
			for (int i = 0; i < Bmax; i++) {
				if (ShotFlag[i] == 0) {
					Bullet_Pos[i] = Player_Pos;
					if (i != 0 && Bullet_Pos[i - 1].z > 1 && Bullet_Pos[i].z < 50) {
						ShotFlag[i] = 1;
						audio->PlayWave("Resources/se_mod05.wav", vol);
					}
					ShotFlag[0] = 1;
					break;
				}
			}
		}

		for (int i = 0; i < Bmax; i++) {
			if (ShotFlag[i] == 1) {
				if (set[i] == 0) {
					xspeed[i] = cosf((rotb)*PI / 180) * bfs;
					yspeed[i] = sinf(rotb2 * PI / 180) * bfs;
					set[i] = 1;
				}

				if (set[i] == 1) {
					Bullet_Pos[i].x += xspeed[i];
					Bullet_Pos[i].y += yspeed[i];
					Bullet_Pos[i].z += 2.5;
					if (Bullet_Pos[i].z > 300) {
						ShotFlag[i] = 0;
						set[i] = 0;
						Bullet_Pos[i] = Player_Pos;
					}
				}
			}
		}
		//敵の処理
		view[0] = 1;
		Enemy_Pos[0].z -= 1.0;
		for (int i = 1; i < Emax; i++) {
			if (Enemy_Pos[i - 1].z < 380) {//前にいる敵が一定距離言ったら
				stf[i] = 1;//動き始める
			}
			if (stf[i] == 1) {
				Enemy_Pos[i].z -= 0.9;
				view[i] = 1;//動いたら姿見える
			}
		}
		for (int i = 0; i < Emax; i++) {
			if (enemyalive[i] == 1) {
				if (Enemy_Pos[i].z < -100) {
					Enemy_Pos[i].z = rpos[i];//元に位置に
				}
			}

		}
		//プレイヤーのダメージ処理
		for (int i = 0; i < Emax; i++) {
			if (Enemy_Pos[i].z < -20 && Enemy_Pos[i].z >-30 && enemyalive[i] == 1) {
				playerHPsize.x -= 3.0f;
				damagef = 1;
			}
		}
		for (int i = 0; i < BBmax; i++) {
			if (BossBullet_Pos[i].z < -20 &&BossBullet_Pos[i].z >-30 && bossbulalive[i] == 1) {
				playerHPsize.x -= 3.0f;
				damagef = 1;
			}
		}
		if (damagef == 1) {
			damagev->feed(feedd);
			if (feedd <= 0) {
				damagef = 0;
			}
		}
		//0なったら　フェードイン
		if (playerHPsize.x <= 0) {
			feed += 0.05f;
		}
		if (feed >= 4) {//完全に暗くなったらシーン飛ばす
			scene = 3;
		}

		//当たり判定
		for (int i = 0; i < Bmax; i++) {
			for (int j = 0; j < Emax; j++) {
				if (enemyalive[j] == 1) {
					if (col->ChenkSphere2Sphere({ Bullet_Pos[i].x,Bullet_Pos[i].y - 8,Bullet_Pos[i].z }, Enemy_Pos[j],11)==true) {
						enemyscl[j].x -= 0.05;
					}

					if (enemyscl[j].x <= 0) {
						Enemy_Scl[j].x -= 0.01;
						Enemy_Scl[j].y -= 0.01;
						Enemy_Scl[j].z -= 0.01;
					}
					if (Enemy_Scl[j].x <= 0) {
						enemyalive[j] = 0;
					}
					if (enemyalive[j] == 0) {
						enemydethcount++;
					}

				}

			}
		}
		//ボスの弾
		for (int i = 0; i < Bmax; i++) {
			for (int j = 0; j < BBmax; j++) {
				if (bossbulalive[j] == 1) {
					if (col->ChenkSphere2Sphere({ Bullet_Pos[i].x,Bullet_Pos[i].y - 10,Bullet_Pos[i].z }, BossBullet_Pos[j],11)==true){
						BossBulletHP_Scl[j].x -= 0.05;
					}

					if (BossBulletHP_Scl[j].x <= 0) {
						BossBullet_Scl[j].x -= 0.07;
						BossBullet_Scl[j].y -= 0.07;
						BossBullet_Scl[j].z -= 0.07;
					}
					if (BossBullet_Scl[j].x <= 0) {
						bossbulalive[j] = 0;
					}
				}

			}
		}


		//ボスの処理
		if (enemydethcount >= Emax) {
			BossScene = 1;
		}

		//登場
		if (BossScene == 1) {
			baction->motion(Boss_Pos);
			baction->Feedin(Boss_Pos, Bossbotm_Pos, feed, target2.m128_f32[2], bosstarget, playertarget, Boss_Scl, Bossbotm_Scl);
			//baction->appearance(Boss_Pos, Boss_Scl);
		}

		if (BossScene == 1) {
			for (int i = 0; i < Bmax; i++) {
				if (Bullet_Pos[i].z > 260) {
					xsize -= 0.2f;
					bosscolor = { 1,0,0,1 };
					break;
				} else {
					bosscolor = { 1,1,1,1 };
				}
			}
			if (xsize > 400 && baction->Getfeedset() == 3) {
				baction->normalattack(BossBullet_Pos, bossbulalive, BossBullet_Scl, BossBulletHP_Scl, BossBullet_Rot);
			} else if (xsize <= 400) {
				baction->Rotmotion(Boss_Rot);
				baction->circleattack(BossBullet_Pos, BossBullet_Scl, BossBulletHP_Scl, bossbulalive, BossBullet_Rot);
			}
			if (baction->Getfeedset() == 0 || baction->Getfeedset() == 1 || baction->Getfeedset() == 2) {
				for (int i = 0; i < Bmax; i++) {
					ShotFlag[i] = 0;
					Bullet_Pos[i] = Player_Pos;
					rotb = -101;
					rotb2 = 0;
					set[i] = 0;
				}
			}
		}

		//背景処理
		backposition += 8;
		if (backposition > 720) {
			backposition = -700;
		}
		if (xsize <= 0) {
			baction->losemotion(Boss_Rot);
			feedw += 0.02f;
			if (feedw >= 1) {
				scene = 4;
			}
		}

#pragma endregion
		//カメラ関係の処理
		//行列を作り直す
		rotM = XMMatrixRotationX(XMConvertToRadians(angle));
		XMVECTOR v;
		v = XMVector3TransformNormal(v0, rotM);
		eye2 = target2 + v;
		matview = XMMatrixLookAtLH((eye2), (target2), XMLoadFloat3(&up));

		SetPrm();//パラメータのセット
		objUpdate();
	}
}

void GameScene::TitleInitialize()
{
	scenetimer=0;
	feed = 0;
	feedd = 0;
	Player_Rot = { 0,-101,0 };
	for (int i = 0; i < Bmax; i++) {
		ShotFlag[i] = 0;
		set[i] = 0;
		speedcs[i] = 2.5;
		Bullet_Pos[i] = Player_Pos;
	}
	for (int i = 0; i < BBmax; i++) {
		BossBullet_Pos[i].z = 800;
	}
	for (int i = 0; i < Emax; i++) {
		Enemy_Pos[i].x = rand() % 150 - 100;
		Enemy_Pos[i].y = rand() % 100 - 50;
		Enemy_Pos[i].z = rand() % 450 + 400;
		rpos[i] = rand() % 450 + 400;
		Enemy_Scl[i] = { 5,5,5 };
		enemy[i]->SetRotation({ 0,180,0 });
	}
	Boss_Scl = { 0,0,0 };
	Boss_Rot = { 0,180,0 };
	Bossbotm_Scl = { 0,0,0 };
	Bossbotm->SetRotation({ 0,180,0 });
	baction->Setsetbul(0);
	baction->Setcircleset(0);
	baction->Setfeed(0);
	feedw = 0;
	bfs = 3.2;
	pscale = { 5,5,5 };
	alpha = 1;
	damageflag = 0;
	for (int i = 0; i < Emax; i++) {
		enemyscl[i] = { 3,1,1 };
		//alpha[i] = 1;
		enemyalive[i] = 1;
		view[i] = 0;
		stf[i] = 0;
	}
	enemydethcount = 0;
	for (int i = 0; i < Emax; i++) {
		ec[i] = { 1,0,0,1 };
	}
	BossScene = 0;
	xsize = 800;
	playerHPsize = { 280,60 };
	Enemy_Pos[0].z = 400;
	baction->SetShflag(0);
	Boss_Pos = { -30,-450,800 };
	view[0] = 1;
	stf[0] = 1;
	for (int i = 0; i < BBmax; i++) {
		//	BossBullet_Pos[i].z =700;
		BossBullet_Scl[i] = { 5,5,5 };

		BossBulletHP_Scl[i] = { 1.5,0.5,1 };
		bossbulalive[i] = 1;
		BossBullet_Rot[i] = { 0,180,0 };
	}
	Player_Pos = { 0,-30,-50 };
	target2.m128_f32[2] = Player_Pos.z + 10;
	playertarget = Player_Pos.z + 10;
	bosstarget = Boss_Pos.z - 300.5;
}

void GameScene::SetPrm()
{
#pragma region 各パラメータのセット
	player->SetPosition(Player_Pos);
	player->SetRotation({ Player_Rot.x,Player_Rot.y,Player_Rot.z });
	player->SetScale({ 4,4,4 });

	for (int i = 0; i < Bmax; i++) {
		object3d2[i]->SetScale({ 4,4,4 });
		object3d2[i]->SetPosition(Bullet_Pos[i]);
	}

	Lazer_Pos = { player->GetPosition().x, player->GetPosition().y + 0.2f, player->GetPosition().z };
	Lazer_Rot = player->GetRotation();
	lazer->SetScale({ 100,2,2 });
	lazer->SetPosition(Lazer_Pos);
	lazer->SetRotation(Lazer_Rot);

	for (int i = 0; i < BBmax; i++) {
		BossBullet[i]->SetPosition(BossBullet_Pos[i]);
		BossBullet[i]->SetScale(BossBullet_Scl[i]);
	}
	for (int i = 0; i < Emax; i++) {
		EnemyHP[i]->SetPosition({ Enemy_Pos[i].x,Enemy_Pos[i].y + 25,Enemy_Pos[i].z - 5 });
		EnemyHP[i]->SetScale(enemyscl[i]);
		enemy[i]->SetScale(Enemy_Scl[i]);
		enemy[i]->SetPosition(Enemy_Pos[i]);
	}

	for (int i = 0; i < BBmax; i++) {
		BossBulletHP[i]->SetPosition({ BossBullet_Pos[i].x, BossBullet_Pos[i].y + 25, BossBullet_Pos[i].z });
		BossBulletHP[i]->SetScale(BossBulletHP_Scl[i]);
		BossBullet[i]->SetRotation(BossBullet_Rot[i]);
	}

	Boss->SetPosition(Boss_Pos);
	Boss->SetScale(Boss_Scl);
	Boss->SetRotation(Boss_Rot);
	Bossbotm->SetPosition(Bossbotm_Pos);
	Bossbotm->SetScale(Bossbotm_Scl);
	BossHP->SetPosition(Bosshp_Pos);
	BossHP->SetSize({ xsize,40 });

#pragma endregion

#pragma region パラメータのセット背景スプライト周り
	spriteBG->SetPosition({ 20,620 });
	BackGround3->SetPosition({ 0,backposition });
	spriteBG->SetSize(playerHPsize);
	BackGround->SetSize({ window_width ,window_height });
	BackGround2->SetSize({ window_width ,window_height });
	BackGround3->SetSize({ window_width ,window_height });

	Feed->SetPosition({ 0, 0 });
	Feed->SetSize({ window_width ,window_height });
	Feed->setcolor({ feed,0,0,1 });

	Feedw->SetPosition({ 0, 0 });
	Feedw->SetSize({ window_width ,window_height });
	Feedw->setcolor({ feedw,0,0,1 });

	damagev->SetPosition({ 0, 0 });
	damagev->SetSize({ window_width ,window_height });
	damagev->setcolor({ feedd,0,0,1 });

#pragma endregion
}

void GameScene::objUpdate()
{
	lazer->Update(matview, matprojection, { 1,1,1,1 });
	player->Update(matview, matprojection, { 1,1,1,0.7 });
	for (int i = 0; i < Bmax; i++) {
		object3d2[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	for (int i = 0; i < Emax; i++) {
		enemy[i]->Update(matview, matprojection, { 0.6,0.6,0.1,1 });
		EnemyHP[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
	Boss->Update(matview, matprojection, bosscolor);
	Bossbotm->Update(matview, matprojection, bosscolor);
	for (int i = 0; i < BBmax; i++) {
		BossBullet[i]->Update(matview, matprojection, { 0.3,1,0.3,1 });
		BossBulletHP[i]->Update(matview, matprojection, { 1,1,1,1 });
	}
}
void GameScene::TitleDraw(DirectXCommon* dxCommon)
{
	if (scene == 0) {
		title->SetPosition({ 0, 0 });
		title->SetSize({ window_width ,window_height });
		//title->setcolor({ 1,0,0,1 });
		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画

		title->Draw();
		// スプライト描画後処理
		Sprite::PostDraw();
	}
}
void GameScene::RuleDraw(DirectXCommon* dxCommon)
{
	if (scene == 1) {
		rule->SetPosition({ 0, 0 });
		rule->SetSize({ window_width ,window_height });
		rule->setcolor({ 1,0,0,1 });
		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画
		rule->Draw();
		dxcomn->ClearDepthBuffer();
	}
}
void GameScene::GameSceneDraw(DirectXCommon* dxCommon)
{
	if (scene == 2) {
		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画
		BackGround2->Draw();
		BackGround3->Draw();
		BackGround->Draw();

		dxcomn->ClearDepthBuffer();
		// スプライト描画後処理
		Sprite::PostDraw();
		//プリティ部形状の設定コマンド
		dxcomn->GetCmdList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		player->PreDraw(dxcomn->GetCmdList());
		player->Draw();
		player->PostDraw();

		Boss->PreDraw(dxcomn->GetCmdList());

		if (BossScene == 1) {
			Boss->Draw();
		}
		Boss->PostDraw();

		Bossbotm->PreDraw(dxcomn->GetCmdList());

		if (BossScene == 1) {
			Bossbotm->Draw();
		}
		Bossbotm->PostDraw();

		for (int i = 0; i < BBmax; i++) {
			BossBullet[i]->PreDraw(dxcomn->GetCmdList());
			if (baction->Getfeedset() == 3) {
				BossBullet[i]->Draw();
			}
			BossBullet[i]->PostDraw();

			BossBulletHP[i]->PreDraw(dxcomn->GetCmdList());
			if (baction->Getfeedset() == 3) {
				BossBulletHP[i]->Draw();
			}
			BossBulletHP[i]->PostDraw();
		}
		for (int i = 0; i < Bmax; i++) {
			object3d2[i]->PreDraw(dxcomn->GetCmdList());
			object3d2[i]->Draw();
			object3d2[i]->PostDraw();

		}
		//たま照準
		lazer->PreDraw(dxcomn->GetCmdList());
		lazer->Draw();
		lazer->PostDraw();

		//敵描画
		for (int i = 0; i < Emax; i++) {
			if (enemyalive[i] == 1) {
				enemy[i]->PreDraw(dxcomn->GetCmdList());
				if (view[i] == 1) {
					enemy[i]->Draw();
				}
				enemy[i]->PostDraw();

				EnemyHP[i]->PreDraw(dxcomn->GetCmdList());
				if (view[i] == 1) {
					EnemyHP[i]->Draw();
				}
				EnemyHP[i]->PostDraw();
			}
		}

		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画
		if (BossScene == 1) {
			BossHP->Draw();
		}
		// スプライト描画後処理
		Sprite::PostDraw();
		// 背景スプライト描画前処理
		if (damageflag != 1) {
			Sprite::PreDraw(dxcomn->GetCmdList());
			// 背景スプライト描画
			spriteBG->Draw();
			// スプライト描画後処理
			Sprite::PostDraw();
		}
		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画
		Feed->Draw();
		Feedw->Draw();
		damagev->Draw();
		// スプライト描画後処理
		Sprite::PostDraw();
	}
}

void GameScene::LoseDraw(DirectXCommon* dxCommon)
{
	if (scene == 3) {
		lose->SetPosition({ 0, 0 });
		lose->SetSize({ window_width ,window_height });
		lose->setcolor({ 1,0,0,1 });
		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画
		lose->Draw();
		// スプライト描画後処理
		Sprite::PostDraw();
	}
}

void GameScene::ClearDraw(DirectXCommon* dxCommon)
{
	if (scene == 4) {
		clear->SetPosition({ 0, 0 });
		clear->SetSize({ window_width ,window_height });
		clear->setcolor({ 1,0,0,1 });
		Sprite::PreDraw(dxcomn->GetCmdList());
		// 背景スプライト描画
		clear->Draw();
		Sprite::PostDraw();
	}
}

void GameScene::Draw(DirectXCommon* dxcoomn)
{
	this->dxcomn = dxcoomn;

	TitleDraw(dxcomn);
	RuleDraw(dxcomn);
	GameSceneDraw(dxcomn);
	ClearDraw(dxcomn);
	LoseDraw(dxcomn);

}