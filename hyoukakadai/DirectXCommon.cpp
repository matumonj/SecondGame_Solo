#include "DirectXCommon.h"
void DirectXCommon::Initialize(WinApp* winapp)
{
	assert(winapp);
	this->winapp = winapp;
	//�f�o�C�X�̐���
	InitializeDevice();
	//�R�}���h�֘A�̏�����
	InitializeCommand();
	//�X���b�v�`�F�[������̏�����
	InitializeSwapchain();

	//InitImgui();
	//�����_�[�^�[�Q�b�g�r���[������̏�����
	InitializeRenderTargetView();
	//�[�x�o�b�t�@�̏�����
	InitializeDepthBuffer();
	//�t�F���X����
	InitializeFence();
}
#pragma region �f�o�C�X�̐���
void DirectXCommon::InitializeDevice() {

	HRESULT result;
	//dxgi�t�@�N�g���[�̐���
	result = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	//�A�_�v�^�̗񋓗p
	std::vector<ComPtr<IDXGIAdapter1>>adapters;

	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	ComPtr<IDXGIAdapter1>tmpAdapter;
	for (int i = 0; dxgiFactory->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	//�O���t�B�b�N�{�[�h�̃A�_�v�^���2
	for (int i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC1 adesc;
		adapters[i]->GetDesc1(&adesc);
		if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
			continue;
		}
		std::wstring strDesc = adesc.Description;
		if (strDesc.find(L"Intel") == std::wstring::npos) {
			tmpAdapter = adapters[i];
			break;
		}
	}

	//�f�o�C�X�̐���
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;
	for (int i = 0; i < _countof(levels); i++) {
		result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (result == S_OK) {
			featureLevel = levels[i];
			break;
		}
	}

}
#pragma endregion

#pragma region �R�}���h�֘A�̏�����
void DirectXCommon::InitializeCommand()
{
	HRESULT result;
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));

	//�R�}���h���X�g�̐���
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList));

	//�R�}���h�L���[�̐���
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));

}
#pragma endregion

#pragma region �X���b�v�`�F�[���̏�����
void DirectXCommon::InitializeSwapchain()
{

	//�X���b�v�`�F�[���̐���
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc{};
	swapchainDesc.Width = 900;
	swapchainDesc.Height = 720;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	dxgiFactory->CreateSwapChainForHwnd(cmdQueue.Get(), winapp->GetHwnd(), &swapchainDesc, nullptr, nullptr, &swapchain1);
	swapchain1.As(&swapchain);
	//swapchain->Present(1, 0);
}
#pragma endregion

#pragma region �����_�[�^�[�Q�b�g����̏�����
void DirectXCommon::InitializeRenderTargetView()
{
	HRESULT result;
	//�ŃX�N���v�^�q�[�v�̐���
	//D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 2;
	dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	//�����_�[�^�[�Q�b�gview�̐���
	backbuffers.resize(2);
	for (int i = 0; i < 2; i++) {
		result = swapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffers[i]));
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
			i, dev->GetDescriptorHandleIncrementSize(heapDesc.Type));
		//D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		//handle.ptr += i * dev->GetDescriptorHandleIncrementSize(heapDesc.Type);
		dev->CreateRenderTargetView(backbuffers[i].Get(), nullptr, handle);
	}
}
#pragma endregion

#pragma region �[�x�o�b�t�@�֘A�ݒ�
void DirectXCommon::InitializeDepthBuffer()
{
	HRESULT result;

	depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT, window_width, window_height, 1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);
	//���\�[�X����
	result = dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0), IID_PPV_ARGS(&depthBuffer));

	//�[�xview�p�ŃX�N���v�^�q�[�v�쐬
	//D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	result = dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

	//�[�x�r���[�쐬
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

}
#pragma endregion

#pragma region �t�F���X����
void DirectXCommon::InitializeFence()
{
	HRESULT result;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
}
#pragma endregion

#pragma region �`��O����
void DirectXCommon::BeginDraw()
{
	//�o�b�N�o�b�t�@�̔ԍ��擾
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
	//ImGui_ImplDX12_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	//ImGui::NewFrame();
	//���\�[�X�o���A���������ݗp��
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backbuffers[bbIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	//�`���w��
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
		bbIndex, dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

	//�[�x�X�e���V���r���[�p�ŃX�N���v�^�q�[�v
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	//��ʃN���A�@�`��F�̎w��
	float clearColor[] = { 1,1,0,1 };
	//�����_�[�^�[�Q�b�g�@�N���A
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	//�[�x�o�b�t�@�@�N���A
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//view�|�[�g�̈�̐ݒ�
	cmdList->RSSetViewports(1, &CD3DX12_VIEWPORT(0.0f, 0.0f, WinApp::window_width, WinApp::window_height));
	//�V�U�[�Z�`�̐ݒ�
	cmdList->RSSetScissorRects(1, &CD3DX12_RECT(0, 0, WinApp::window_width, WinApp::window_height));

}
#pragma endregion

#pragma region �`��㏈��
void DirectXCommon::EndDraw()
{
	//�o�b�N�o�b�t�@�̔ԍ��擾
	UINT bbIndex = swapchain->GetCurrentBackBufferIndex();
	// imgui�`��
	//ImGui::Render();
	//ID3D12DescriptorHeap* ppHeaps[] = {  };
	//cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);

	//���\�[�X�o���A��\���p��
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backbuffers[bbIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	//���߂��N���[�Y
	cmdList->Close();
	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdLists[] = { cmdList };
	cmdQueue->ExecuteCommandLists(1, cmdLists);
	//���s�҂�
	cmdQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal) {
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	//�A���P�[�^�[���N���A
	cmdAllocator->Reset();
	//�R�}���h���X�g�̃��Z�b�g
	cmdList->Reset(cmdAllocator.Get(), nullptr);
	//��ʃt���b�v
	swapchain->Present(1, 0);
}
#pragma endregion
//
//bool DirectXCommon::InitImgui()
//{
//	HRESULT result = S_FALSE;
//
//	// �f�X�N���v�^�q�[�v�𐶐�
//	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
//	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//	heapDesc.NumDescriptors = 1;
//	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//	result = dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&imguiHeap));
//	if (FAILED(result)) {
//		assert(0);
//		return false;
//	}
//
//	// �X���b�v�`�F�[���̏����擾
//	DXGI_SWAP_CHAIN_DESC swcDesc = {};
//	result = swapchain->GetDesc(&swcDesc);
//	ImGui::CreateContext();
//	ImGui_ImplWin32_Init(winapp->GetHwnd());
//	ImGui_ImplDX12_Init(
//		GetDev(),
//		swcDesc.BufferCount,
//		swcDesc.BufferDesc.Format,
//		imguiHeap.Get(),
//		imguiHeap->GetCPUDescriptorHandleForHeapStart(),
//		imguiHeap->GetGPUDescriptorHandleForHeapStart()
//	);
//
//	return true;
//}
void DirectXCommon::ClearDepthBuffer()
{
	// �[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h�����擾
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvH = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	// �[�x�o�b�t�@�̃N���A
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}