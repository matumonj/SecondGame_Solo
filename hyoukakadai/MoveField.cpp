
#include "MoveField.h"
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

/// <summary>
/// 静的メンバ変数の実体
/// </summary>
const float MoveField::radius = 5.0f;				// 底面の半径
const float MoveField::prizmHeight = 8.0f;			// 柱の高さ
ID3D12Device* MoveField::device = nullptr;
UINT MoveField::descriptorHandleIncrementSize = 0;
ID3D12GraphicsCommandList* MoveField::cmdList = nullptr;
ComPtr<ID3D12RootSignature> MoveField::rootsignature;
ComPtr<ID3D12PipelineState> MoveField::pipelinestate;
ComPtr<ID3D12DescriptorHeap> MoveField::descHeap;
ComPtr<ID3D12Resource> MoveField::vertBuff;
ComPtr<ID3D12Resource> MoveField::indexBuff;
ComPtr<ID3D12Resource> MoveField::texbuff;
CD3DX12_CPU_DESCRIPTOR_HANDLE MoveField::cpuDescHandleSRV;
CD3DX12_GPU_DESCRIPTOR_HANDLE MoveField::gpuDescHandleSRV;
XMMATRIX MoveField::matView{};
XMMATRIX MoveField::matProjection{};
XMFLOAT3 MoveField::eye = { 0, 0, -50.0f };
XMFLOAT3 MoveField::target = { 0, 0, 0 };
XMFLOAT3 MoveField::up = { 0, 1, 0 };
D3D12_VERTEX_BUFFER_VIEW MoveField::vbView{};
D3D12_INDEX_BUFFER_VIEW MoveField::ibView{};
MoveField::Material MoveField::material;
float MoveField::alpha = 0.0f;
//MoveField::VertexPosNormalUv MoveField::vertices[vertexCount];
//unsigned short MoveField::indices[planeCount * 3];
std::vector<MoveField::VertexPosNormalUv>MoveField::vertices;
std::vector<unsigned short>MoveField::indices;
bool MoveField::StaticInitialize(ID3D12Device* device, int window_width, int window_height)
{
	// nullptrチェック
	assert(device);

	MoveField::device = device;

	// デスクリプタヒープの初期化
	InitializeDescriptorHeap();

	// カメラ初期化
	InitializeCamera(window_width, window_height);

	// パイプライン初期化
	InitializeGraphicsPipeline();

	// テクスチャ読み込み
	//LoadTexture(directoryPath,material.textureFilename);

	// モデル生成
	//CreateModel();

	return true;
}

void MoveField::PreDraw(ID3D12GraphicsCommandList* cmdList)
{
	// PreDrawとPostDrawがペアで呼ばれていなければエラー
	assert(MoveField::cmdList == nullptr);

	// コマンドリストをセット
	MoveField::cmdList = cmdList;

	// パイプラインステートの設定
	cmdList->SetPipelineState(pipelinestate.Get());
	// ルートシグネチャの設定
	cmdList->SetGraphicsRootSignature(rootsignature.Get());
	// プリミティブ形状を設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MoveField::PostDraw()
{
	// コマンドリストを解除
	MoveField::cmdList = nullptr;
}

MoveField* MoveField::Create()
{
	// 3Dオブジェクトのインスタンスを生成
	MoveField* field = new MoveField();
	if (field == nullptr) {
		return nullptr;
	}

	// 初期化
	if (!field->Initialize()) {
		delete field;
		assert(0);
		return nullptr;
	}
	float scale_val = 5;
	field->scale = { scale_val , scale_val , scale_val };

	return field;
}

void MoveField::SetEye(XMFLOAT3 eye)
{
	MoveField::eye = eye;

	UpdateViewMatrix();
}

void MoveField::SetTarget(XMFLOAT3 target)
{
	MoveField::target = target;

	UpdateViewMatrix();
}

void MoveField::CameraMoveVector(XMFLOAT3 move)
{
	XMFLOAT3 eye_moved = GetEye();
	XMFLOAT3 target_moved = GetTarget();

	eye_moved.x += move.x;
	eye_moved.y += move.y;
	eye_moved.z += move.z;

	target_moved.x += move.x;
	target_moved.y += move.y;
	target_moved.z += move.z;

	SetEye(eye_moved);
	SetTarget(target_moved);
}

bool MoveField::InitializeDescriptorHeap()
{
	HRESULT result = S_FALSE;

	// デスクリプタヒープを生成	
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//シェーダから見えるように
	descHeapDesc.NumDescriptors = 1; // シェーダーリソースビュー1つ
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));//生成
	if (FAILED(result)) {
		assert(0);
		return false;
	}

	// デスクリプタサイズを取得
	descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

void MoveField::InitializeCamera(int window_width, int window_height)
{
	// ビュー行列の生成
	matView = XMMatrixLookAtLH(
		XMLoadFloat3(&eye),
		XMLoadFloat3(&target),
		XMLoadFloat3(&up));

	// 平行投影による射影行列の生成
	//constMap->mat = XMMatrixOrthographicOffCenterLH(
	//	0, window_width,
	//	window_height, 0,
	//	0, 1);
	// 透視投影による射影行列の生成
	matProjection = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		(float)window_width / window_height,
		0.1f, 1000.0f
	);
}

bool MoveField::InitializeGraphicsPipeline()
{
	HRESULT result = S_FALSE;
	ComPtr<ID3DBlob> vsBlob; // 頂点シェーダオブジェクト
	ComPtr<ID3DBlob> psBlob;	// ピクセルシェーダオブジェクト
	ComPtr<ID3DBlob> errorBlob; // エラーオブジェクト

	// 頂点シェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		//L"Resources/shaders/BasicVertexShader.hlsl",	// シェーダファイル名
		L"OBJVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "vs_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&vsBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// ピクセルシェーダの読み込みとコンパイル
	result = D3DCompileFromFile(
		//L"Resources/shaders/BasicPixelShader.hlsl",	// シェーダファイル名
		L"OBJPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // インクルード可能にする
		"main", "ps_5_0",	// エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
		0,
		&psBlob, &errorBlob);
	if (FAILED(result)) {
		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

	// 頂点レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ // xy座標(1行で書いたほうが見やすい)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // 法線ベクトル(1行で書いたほうが見やすい)
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{ // uv座標(1行で書いたほうが見やすい)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// グラフィックスパイプラインの流れを設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());

	// サンプルマスク
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // 標準設定
	// ラスタライザステート
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	// デプスステンシルステート
	gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;	// RBGA全てのチャンネルを描画
	blenddesc.BlendEnable = true;
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	// ブレンドステートの設定
	gpipeline.BlendState.RenderTarget[0] = blenddesc;

	// 深度バッファのフォーマット
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// 頂点レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	// 図形の形状設定（三角形）
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;	// 描画対象は1つ
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 0〜255指定のRGBA
	gpipeline.SampleDesc.Count = 1; // 1ピクセルにつき1回サンプリング

	// デスクリプタレンジ
	CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
	descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 レジスタ

	// ルートパラメータ
	//CD3DX12_ROOT_PARAMETER rootparams[2];
	//rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	//rootparams[1].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);
	CD3DX12_ROOT_PARAMETER rootparams[3];
	rootparams[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootparams[2].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);

	// スタティックサンプラー
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	// ルートシグネチャの設定
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparams), rootparams, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	// バージョン自動判定のシリアライズ
	result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	// ルートシグネチャの生成
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	if (FAILED(result)) {
		return result;
	}

	gpipeline.pRootSignature = rootsignature.Get();

	// グラフィックスパイプラインの生成
	result = device->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pipelinestate));

	if (FAILED(result)) {
		return result;
	}

	return true;
}

bool MoveField::LoadTexture(const std::string& directoryPath, const std::string& filename)
{
	HRESULT result = S_FALSE;

	// WICテクスチャのロード
	TexMetadata metadata{};
	ScratchImage scratchImg{};

	string filepath = directoryPath + filename;

	wchar_t wfilepath[128];
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, filepath.c_str(), -1, wfilepath, _countof(wfilepath));
	directoryPath + filename;
	/*result = LoadFromWICFile(
		L"Resources/tex1.png", WIC_FLAGS_NONE,
		&metadata, scratchImg);*/
	result = LoadFromWICFile(wfilepath, WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(result)) {
		return result;
	}

	const Image* img = scratchImg.GetImage(0, 0, 0); // 生データ抽出

	// リソース設定
	CD3DX12_RESOURCE_DESC texresDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		(UINT)metadata.height,
		(UINT16)metadata.arraySize,
		(UINT16)metadata.mipLevels
	);

	// テクスチャ用バッファの生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0),
		D3D12_HEAP_FLAG_NONE,
		&texresDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // テクスチャ用指定
		nullptr,
		IID_PPV_ARGS(&texbuff));

	if (FAILED(result)) {
		return result;
	}

	// テクスチャバッファにデータ転送
	result = texbuff->WriteToSubresource(
		0,
		nullptr, // 全領域へコピー
		img->pixels,    // 元データアドレス
		(UINT)img->rowPitch,  // 1ラインサイズ
		(UINT)img->slicePitch // 1枚サイズ
	);
	if (FAILED(result)) {
		return result;
	}

	// シェーダリソースビュー作成
	cpuDescHandleSRV = CD3DX12_CPU_DESCRIPTOR_HANDLE(descHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);
	gpuDescHandleSRV = CD3DX12_GPU_DESCRIPTOR_HANDLE(descHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorHandleIncrementSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // 設定構造体
	D3D12_RESOURCE_DESC resDesc = texbuff->GetDesc();

	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(texbuff.Get(), //ビューと関連付けるバッファ
		&srvDesc, //テクスチャ設定情報
		cpuDescHandleSRV
	);

	return true;
}

void  MoveField::LoadMaterial(const std::string& directoryPath, const std::string& filename)
{
	std::ifstream file;
	file.open(directoryPath + filename);
	if (file.fail()) {
		assert(0);
	}
	string line;
	while (getline(file, line)) {
		std::istringstream line_stream(line);
		string key;
		getline(line_stream, key, ' ');
		if (key[0] == '\t') {
			key.erase(key.begin());
		}
		if (key == "newmtl") {
			line_stream >> material.name;
		}
		if (key == "Ka") {
			line_stream >> material.ambient.x;
			line_stream >> material.ambient.y;
			line_stream >> material.ambient.z;
		}
		if (key == "Kd") {
			line_stream >> material.diffuse.x;
			line_stream >> material.diffuse.y;
			line_stream >> material.diffuse.z;
		}
		if (key == "Ks") {
			line_stream >> material.specular.x;
			line_stream >> material.specular.y;
			line_stream >> material.specular.z;
		}
		if (key == "map_Kd") {
			line_stream >> material.textureFilename;
			LoadTexture(directoryPath, material.textureFilename);
		}
	}
	file.close();
}
void MoveField::CreateModel()
{
	HRESULT result = S_FALSE;

	std::ifstream file;
	//objファイルを開く
	//file.open(L"Resources/triangle_tex/triangle_tex.obj");
	const string modelname = "Bullet";
	const string filename = modelname + ".obj";
	const string directoryPath = "Resources/" + modelname + "/";
	file.open(directoryPath + filename);
	if (file.fail()) {
		assert(0);
	}

	vector<XMFLOAT3>positions;//頂点座標
	vector<XMFLOAT3>normals;//法線ベクトル
	vector<XMFLOAT2>texcoords;//テクスチャUV
	//1行ずつ読み込む

	string line;
	while (getline(file, line)) {
		//1行分の文字列をストリームに変換
		std::istringstream line_stream(line);
		//半角SPACE区切りで行の先頭文字列を取得
		string key;
		getline(line_stream, key, ' ');

		//先頭文字がvなら頂点座標
		if (key == "v") {
			//X,Y,Z座標読み込み
			XMFLOAT3 position{};
			line_stream >> position.x;
			line_stream >> position.y;
			line_stream >> position.z;
			//座標データに追加
			positions.emplace_back(position);
			//頂点データに追加
			//VertexPosNormalUv vertex{};
			//vertex.pos = position;
			//vertices.emplace_back(vertex);

		}
		//先頭文字列がvtならテクスチャ
		if (key == "vt") {
			//UV成分読み込み
			XMFLOAT2 texcoord{};
			line_stream >> texcoord.x;
			line_stream >> texcoord.y;
			//v方向反転
			texcoord.y = 1.0f - texcoord.y;
			//テクスチャ座標データに追加
			texcoords.emplace_back(texcoord);
		}
		//先頭文字列がvnなら法線ベクトル
		if (key == "vn") {
			//XYZ成分読み込み
			XMFLOAT3 normal{};
			line_stream >> normal.x;
			line_stream >> normal.y;
			line_stream >> normal.z;
			//法線ベクトルデータに追加
			normals.emplace_back(normal);
		}
		//先頭文字列がmtllibならマテリアル
		if (key == "mtllib")
		{
			//マテリアルのファイル名読み込み
			string filename;
			line_stream >> filename;
			LoadMaterial(directoryPath, filename);
		}
		//先頭文字がfならポリゴン
		if (key == "f") {
			//半角スペース区切りで行の続きを読み込む
			string index_string;
			while (getline(line_stream, index_string, ' ')) {
				//頂点インデックス1行分の文字列をストリームに変換
				std::istringstream index_stream(index_string);
				unsigned short indexPosition, indexNormal, indexTexcoord;
				index_stream >> indexPosition;
				index_stream.seekg(1, ios_base::cur);
				index_stream >> indexTexcoord;
				index_stream.seekg(1, ios_base::cur);
				index_stream >> indexNormal;
				//頂点インデックスに追加]
				VertexPosNormalUv vertex{};
				vertex.pos = positions[indexPosition - 1];
				vertex.normal = normals[indexNormal - 1];
				vertex.uv = texcoords[indexTexcoord - 1];
				vertices.emplace_back(vertex);

				indices.emplace_back((unsigned short)indices.size());
			}
		}
	}
	//ファイル閉じる
	file.close();

	//std::vector<VertexPosNormalUv> realVertices;
	//// 頂点座標の計算（重複あり）
	//{
	//	realVertices.resize((division + 1) * 2);
	//	int index = 0;
	//	float zValue;

	//	// 底面
	//	zValue = prizmHeight / 2.0f;
	//	for (int i = 0; i < division; i++)
	//	{
	//		XMFLOAT3 vertex;
	//		vertex.x = radius * sinf(XM_2PI / division * i);
	//		vertex.y = radius * cosf(XM_2PI / division * i);
	//		vertex.z = zValue;
	//		realVertices[index++].pos = vertex;
	//	}
	//	realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// 底面の中心点
	//	// 天面
	//	zValue = -prizmHeight / 2.0f;
	//	for (int i = 0; i < division; i++)
	//	{
	//		XMFLOAT3 vertex;
	//		vertex.x = radius * sinf(XM_2PI / division * i);
	//		vertex.y = radius * cosf(XM_2PI / division * i);
	//		vertex.z = zValue;
	//		realVertices[index++].pos = vertex;
	//	}
	//	realVertices[index++].pos = XMFLOAT3(0, 0, zValue);	// 天面の中心点
	//}

	//// 頂点座標の計算（重複なし）
	//{
	//	int index = 0;
	//	// 底面
	//	for (int i = 0; i < division; i++)
	//	{
	//		unsigned short index0 = i + 1;
	//		unsigned short index1 = i;
	//		unsigned short index2 = division;

	//		vertices[index++] = realVertices[index0];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index2]; // 底面の中心点
	//	}
	//	// 底面の最後の三角形の1番目のインデックスを0に書き換え
	//	vertices[index - 3] = realVertices[0];

	//	int topStart = division + 1;
	//	// 天面
	//	for (int i = 0; i < division; i++)
	//	{
	//		unsigned short index0 = topStart + i;
	//		unsigned short index1 = topStart + i + 1;
	//		unsigned short index2 = topStart + division;

	//		vertices[index++] = realVertices[index0];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index2]; // 天面の中心点
	//	}
	//	// 天面の最後の三角形の1番目のインデックスを0に書き換え
	//	vertices[index - 2] = realVertices[topStart];

	//	// 側面
	//	for (int i = 0; i < division; i++)
	//	{
	//		unsigned short index0 = i + 1;
	//		unsigned short index1 = topStart + i + 1;
	//		unsigned short index2 = i;
	//		unsigned short index3 = topStart + i;

	//		if (i == division - 1)
	//		{
	//			index0 = 0;
	//			index1 = topStart;
	//		}

	//		vertices[index++] = realVertices[index0];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index2];

	//		vertices[index++] = realVertices[index2];
	//		vertices[index++] = realVertices[index1];
	//		vertices[index++] = realVertices[index3];
	//	}
	//}

	//// 頂点インデックスの設定
	//{
	//	for (int i = 0; i < 3; i++)
	//	{
	//		indices[i] = i;
	//	}
	//}

	//// 法線方向の計算
	//for (int i = 0; i < _countof(indices                                                                  ) / 3; i++)
	//{// 三角形１つごとに計算していく
	//	// 三角形のインデックスを取得
	//	unsigned short index0 = indices[i * 3 + 0];
	//	unsigned short index1 = indices[i * 3 + 1];
	//	unsigned short index2 = indices[i * 3 + 2];
	//	// 三角形を構成する頂点座標をベクトルに代入
	//	XMVECTOR p0 = XMLoadFloat3(&vertices[index0].pos);
	//	XMVECTOR p1 = XMLoadFloat3(&vertices[index1].pos);
	//	XMVECTOR p2 = XMLoadFloat3(&vertices[index2].pos);
	//	// p0→p1ベクトル、p0→p2ベクトルを計算
	//	XMVECTOR v1 = XMVectorSubtract(p1, p0);
	//	XMVECTOR v2 = XMVectorSubtract(p2, p0);
	//	// 外積は両方から垂直なベクトル
	//	XMVECTOR normal = XMVector3Cross(v1, v2);
	//	// 正規化（長さを1にする)
	//	normal = XMVector3Normalize(normal);
	//	// 求めた法線を頂点データに代入
	//	XMStoreFloat3(&vertices[index0].normal, normal);
	//	XMStoreFloat3(&vertices[index1].normal, normal);
	//	XMStoreFloat3(&vertices[index2].normal, normal);
	//}
	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices.size());
	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());
	// 頂点バッファ生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		//&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		&CD3DX12_RESOURCE_DESC::Buffer(sizeVB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	// インデックスバッファ生成
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		//&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
		&CD3DX12_RESOURCE_DESC::Buffer(sizeIB),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));
	if (FAILED(result)) {
		assert(0);
		return;
	}

	// 頂点バッファへのデータ転送
	VertexPosNormalUv* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (SUCCEEDED(result)) {
		//memcpy(vertMap, vertices, sizeof(vertices));
		std::copy(vertices.begin(), vertices.end(), vertMap);
		vertBuff->Unmap(0, nullptr);
	}

	// インデックスバッファへのデータ転送
	unsigned short* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	if (SUCCEEDED(result)) {

		// 全インデックスに対して
		//for (int i = 0; i < _countof(indices); i++)
		//{
			//indexMap[i] = indices[i];	// インデックスをコピー
		//}
		std::copy(indices.begin(), indices.end(), indexMap);
		indexBuff->Unmap(0, nullptr);
	}

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//vbView.SizeInBytes = sizeof(vertices);
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	// インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
	//ibView.SizeInBytes = sizeof(indices);
}

void MoveField::UpdateViewMatrix()
{
	// ビュー行列の更新
	matView = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
}

bool MoveField::Initialize()
{
	// nullptrチェック
	assert(device);

	HRESULT result;
	//// 定数バッファの生成
	//result = device->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
	//	D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferData) + 0xff) & ~0xff),
	//	D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr,
	//	IID_PPV_ARGS(&constBuff));

	// 定数バッファの生成B0
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB0) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB0));
	// 定数バッファの生成B1
	result = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 	// アップロード可能
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataB1) + 0xff) & ~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffB1));


	return true;
}

void MoveField::Update(XMMATRIX matview, XMMATRIX matprojection, XMFLOAT4 color)
{
	HRESULT result;
	XMMATRIX matScale, matRot, matTrans;

	// スケール、回転、平行移動行列の計算
	matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
	matRot = XMMatrixIdentity();
	matRot *= XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	matRot *= XMMatrixRotationX(XMConvertToRadians(rotation.x));
	matRot *= XMMatrixRotationY(XMConvertToRadians(rotation.y));
	matTrans = XMMatrixTranslation(position.x, position.y, position.z);

	// ワールド行列の合成
	matWorld = XMMatrixIdentity(); // 変形をリセット
	matWorld *= matScale; // ワールド行列にスケーリングを反映
	matWorld *= matRot; // ワールド行列に回転を反映
	matWorld *= matTrans; // ワールド行列に平行移動を反映
	rotation.y = 90;
	//rotation.x = 50;
	// 親オブジェクトがあれば
	if (parent != nullptr) {
		// 親オブジェクトのワールド行列を掛ける
		matWorld *= parent->matWorld;
	}

	// 定数バッファへデータ転送
	ConstBufferDataB0* constMap = nullptr;
	result = constBuffB0->Map(0, nullptr, (void**)&constMap);
	constMap->color = color;
	constMap->mat = matWorld * matview * matprojection;	// 行列の合成
	constBuffB0->Unmap(0, nullptr);

	ConstBufferDataB1* constMap1 = nullptr;
	result = constBuffB1->Map(0, nullptr, (void**)&constMap1);
	constMap1->ambient = material.ambient;
	constMap1->diffuse = material.diffuse;
	constMap1->specular = material.specular;
	constMap1->alpha = material.alpha;
	constBuffB1->Unmap(0, nullptr);
}

void MoveField::Draw()
{
	// nullptrチェック
	assert(device);
	assert(MoveField::cmdList);

	// 頂点バッファの設定
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// インデックスバッファの設定
	cmdList->IASetIndexBuffer(&ibView);

	// デスクリプタヒープの配列
	ID3D12DescriptorHeap* ppHeaps[] = { descHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// 定数バッファビューをセット
	//cmdList->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(0, constBuffB0->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, constBuffB1->GetGPUVirtualAddress());

	// シェーダリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(2, gpuDescHandleSRV);
	//cmdList->SetGraphicsRootDescriptorTable(1, gpuDescHandleSRV);
	// 描画コマンド
	cmdList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
	//cmdList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);
}


bool MoveField::SPBoxCollision(XMFLOAT3 object1, XMFLOAT3 radius1, XMFLOAT3 object2, XMFLOAT3 radius2)
{
	SwitchVertex Object1;
	SwitchVertex Object2;
	Object1.Right = object1.x + radius1.x;
	Object1.Left = object1.x - radius1.x;
	Object1.Up = object1.y + radius1.y;
	Object1.Down = object1.y - radius1.y;

	Object2.Right = object2.x + radius2.x;
	Object2.Left = object2.x - radius2.x;
	Object2.Down = object2.y - radius2.y;
	Object2.Up = object2.y + radius2.y;
	//xとyはそれぞれ中心座標として計算する
	return Object1.Right > Object2.Left && Object2.Right > Object1.Left && Object1.Up > Object2.Down && Object2.Up > Object1.Down;
}
//総移動量を決めてブロックを動かす関数(座標指定めんどくさいので)
void MoveField::moveX(float& movement, float& movespeed, XMFLOAT3& obj)
{
	//もとの位置と動いた後の位置の２点間の距離
	float len;
	//元の位置をセット
	if (set == FALSE) {
		GetPosition_x.x = obj.x;
		set = TRUE;
	} else if (set == TRUE) {
		//movement分動いたらスピードを逆に
		obj.x += movespeed;
		len = sqrt((GetPosition_x.x - obj.x) * (GetPosition_x.x - obj.x));
		if (len >= movement) {
			movespeed = movespeed * -1;
		}
	}
	SetPosition(obj);
}

//総移動量を決めてブロックを動かす関数(座標指定めんどくさいので)
void MoveField::moveY(float& movement, float& movespeed, XMFLOAT3& obj)
{
	//もとの位置と動いた後の位置の２点間の距離
	float len;
	//元の位置をセット
	if (set == FALSE) {
		GetPosition_y.y = obj.y;
		set = TRUE;
	} else if (set == TRUE) {
		//movement分動いたらスピードを逆に
		obj.y -= movespeed;
		len = sqrt((GetPosition_y.y- obj.y) * (GetPosition_y.y - obj.y));
		if (len >= movement) {
			movespeed = movespeed * -1;
		}
	}
	SetPosition(obj);
}

void MoveField::pushSwitch_moveX(XMFLOAT3 &object1, XMFLOAT3 radius1, XMFLOAT3 &object2, XMFLOAT3 radius2, XMFLOAT3& object3,float&movement, float& speed)
{
	if (SPBoxCollision(object1, radius1, object2, radius2) == TRUE) {
		moveX(movement,speed,object3);
	}
}

void MoveField::pushSwitch_moveY(XMFLOAT3& object1, XMFLOAT3 radius1, XMFLOAT3& object2, XMFLOAT3 radius2, XMFLOAT3& object3, float& movement, float& speed)
{
	if (SPBoxCollision(object1, radius1, object2, radius2) == TRUE) {
		moveY(movement, speed, object3);
	}
}

void MoveField::light_Block(XMFLOAT3& object1, XMFLOAT3 radius1, XMFLOAT3& object2, XMFLOAT3 radius2, XMFLOAT3& object3)
{
	if (SPBoxCollision(object1, radius1, object2, radius2) == TRUE) {
		if (alpha <= 1.0f) {
			alpha += 0.05f;
		}
	}
	else
	{
		if (alpha >= 0.0f) {
			alpha -= 0.05f;
		}
	}
}