#include "Engine/Renderer/DX12/DX12Renderer.hpp"

#ifdef ENGINE_DX12_RENDERER_INTERFACE

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Time.hpp"

#include <initguid.h>
#include <d3d12.h>
#include <d3dcompiler.h>
//#include <dxgi.h>
#include <dxgi1_4.h>
#include "ThirdParty/d3dx12/d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "ThirdParty/stb/stb_image.h"

static int const k_dx12LightConstantsSlot = 0;
static int const k_dx12DirLightConstantsSlot = 1;
static int const k_dx12CameraConstantsSlot = 2;
static int const k_dx12ModelConstantsSlot = 3;

#if defined OPAQUE
#undef OPAQUE
#endif // OPAQUE

DX12Renderer::DX12Renderer( Renderer* baseRenderer, RendererConfig config )
{
	m_baseRenderer = baseRenderer;
	m_config = config;
}

DX12Renderer::~DX12Renderer()
{

}

void DX12Renderer::StartUp()
{

#ifdef ENGINE_DEBUG_RENDER
	// Enable the D3D12 debug layer.
	{

		ID3D12Debug* debugController;
		if (SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugController ) ) ))
		{
			debugController->EnableDebugLayer();
		}
		else {
			ERROR_RECOVERABLE( "Cannot Create D3D12 Debugger!" );
		}
	}
#endif

	IDXGIFactory4* factory;
	HRESULT hr = CreateDXGIFactory1( IID_PPV_ARGS( &factory ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 DXGI factory" );
	
	bool m_useWarpDevice = true;
	if (m_useWarpDevice)
	{
		IDXGIAdapter* warpAdapter;
		hr = factory->EnumWarpAdapter( IID_PPV_ARGS( &warpAdapter ) );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 warp adapter" );
	
		hr = D3D12CreateDevice( warpAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_device ) );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 device" );
	}
	else
	{
		/*IDXGIAdapter1* hardwareAdapter;
		hr = ID3D12Device::GetHardwareAdapter( factory, &hardwareAdapter );
	
		ThrowIfFailed( D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS( &m_device )
		) );*/
	}
	// get the descriptor size
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
	m_scuDescriptorSize = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	
	hr = m_device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_commandQueue ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 command queue!" );
	
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = DX12_FrameCount;
	swapChainDesc.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetWindowHandle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;
	
	hr = factory->CreateSwapChain(
		m_commandQueue,        // Swap chain needs the queue so that it can force a flush on it.
		&swapChainDesc,
		(IDXGISwapChain**)&m_swapChain
	);
	
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 swap chain!" );
	
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = DX12_FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &m_rtvHeap ) );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 descriptor heaps!" );
	
	}
	
	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_rtvHeap->GetCPUDescriptorHandleForHeapStart() );
	
		// Create a RTV for each frame.
		for (UINT n = 0; n < DX12_FrameCount; n++)
		{
			hr = m_swapChain->GetBuffer( n, IID_PPV_ARGS( &m_renderTargets[n] ) );
			GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot get D3D12 render target!" );
			m_device->CreateRenderTargetView( m_renderTargets[n], nullptr, rtvHandle );
			rtvHandle.Offset( 1, m_rtvDescriptorSize );
		}
	}
	
	hr = m_device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_commandAllocator ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create D3D12 Command Allocator!" );


	// Create a root signature.
	
	enum class RootParameterIndex
	{
		ConstantBuffer,
		TextureSRV,
		RootParameterCount
	};

	{
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		// Create root parameters and initialize first (constants)
		CD3DX12_ROOT_PARAMETER rootParameters[(int)RootParameterIndex::RootParameterCount] = {};

		// Root parameter descriptor
		CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};

		CD3DX12_DESCRIPTOR_RANGE constantBufferDescRange( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND );
		rootParameters[(int)RootParameterIndex::ConstantBuffer].InitAsDescriptorTable( 1, &constantBufferDescRange, D3D12_SHADER_VISIBILITY_ALL );


		// Include texture and srv
		CD3DX12_DESCRIPTOR_RANGE textureSRV( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );
		//CD3DX12_DESCRIPTOR_RANGE textureSampler( D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0 );

		rootParameters[(int)RootParameterIndex::TextureSRV].InitAsDescriptorTable( 1, &textureSRV, D3D12_SHADER_VISIBILITY_PIXEL );
		//rootParameters[(int)RootParameterIndex::TextureSampler].InitAsDescriptorTable( 1, &textureSampler, D3D12_SHADER_VISIBILITY_PIXEL );

		// create a static sampler
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// use all parameters
		rsigDesc.Init( static_cast<UINT>(RootParameterIndex::RootParameterCount), rootParameters, 1, &samplerDesc, rootSignatureFlags );

		ID3DBlob* signature;
		ID3DBlob* error;
		hr = D3D12SerializeRootSignature( &rsigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error );
		if (!SUCCEEDED( hr )) {
			if (error != NULL) {
				DebuggerPrintf( (char*)error->GetBufferPointer() );
			}
		}
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Serialize Root Signature!" );

		hr = m_device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS( &m_rootSignature ) );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Create Root Signature!" );
	}

	// Constant buffers: create descriptor heap and resource heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame + DX12_MaxNumOfTextures;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = m_device->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( &m_constantsDescHeap ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create constant buffer description heap!" );
	
	// create a resource heap, descriptor heap, and pointer to cbv for each frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetCPUDescriptorHandleForHeapStart());
	D3D12_HEAP_PROPERTIES properties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( 1024 * 64 );
	//descriptorHandle.Offset( m_scuDescriptorSize );

	for (int j = 0; j < DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame; ++j) {
		m_constantBuffers[j] = new ConstantBuffer( 1024 * 64 );
		hr = m_device->CreateCommittedResource(
			&properties, // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&resourceDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS( &m_constantBuffers[j]->m_dx12ConstantBuffer ) );

		m_constantBuffers[j]->m_constantBufferView->BufferLocation = m_constantBuffers[j]->m_dx12ConstantBuffer->GetGPUVirtualAddress();
		m_constantBuffers[j]->m_constantBufferView->SizeInBytes = 512;    // CB size is required to be 256-byte aligned.
		m_device->CreateConstantBufferView( m_constantBuffers[j]->m_constantBufferView, descriptorHandle );
		descriptorHandle.Offset( m_scuDescriptorSize );
	}
	
	// create a depth stencil descriptor heap so we can get a pointer to the depth stencil buffer
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS( &m_dsDescHeap ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot Create Depth Stencil Descriptor heap!" );

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES defaultProperties( D3D12_HEAP_TYPE_DEFAULT );
	CD3DX12_RESOURCE_DESC depthStencilTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, m_config.m_window->GetClientDimensions().x, m_config.m_window->GetClientDimensions().y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );

	m_device->CreateCommittedResource(
		&defaultProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilTextureDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS( &m_depthStencilBuffer )
	);

	m_device->CreateDepthStencilView( m_depthStencilBuffer, &depthStencilDesc, m_dsDescHeap->GetCPUDescriptorHandleForHeapStart() );

	// Create the command list.
	hr = m_device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, m_curPipelineState, IID_PPV_ARGS( &m_commandList ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Create CommandList!" );

	//ID3D12DescriptorHeap* descriptorHeaps[] = { m_constantsDescHeap, m_textureDescHeap };
	//m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

	// set these before bind texture
	m_commandList->SetGraphicsRootSignature( m_rootSignature );
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_constantsDescHeap };
	m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

	Image whiteImage = Image( IntVec2( 256, 256 ), Rgba8::WHITE );
	Texture* whiteTexture = CreateTextureFromImage( &whiteImage );
	whiteTexture->m_name = "Default";
	m_defaultTexture = whiteTexture;
	m_loadedTextures["Default"] = whiteTexture;
	BindTexture( m_defaultTexture );

	// Create the pipeline state, which includes compiling and loading shaders.
	m_defaultShader = CreateShader( "Default", defaultShaderSource );
	m_desiredShader = m_defaultShader;

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		hr = m_device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_fence ) );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Create Fence!" );
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
		if (m_fenceEvent == nullptr)
		{
			ERROR_AND_DIE( "D3D12 Cannot Create Fence Event!" );
		}

	}

	//SetModelConstants();

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForPreviousFrame();
}

void DX12Renderer::BeginFrame()
{

	m_curDrawCallIndex = 0;
	if (m_isFirstFrame) {
		m_isFirstFrame = false;

		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		HRESULT hr = m_commandList->Close();
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Close CommandList!" );

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { m_commandList };
		m_commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

		WaitForPreviousFrame();
	}
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	HRESULT hr = m_commandAllocator->Reset();
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Reset Command Allocator!" );

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	hr = m_commandList->Reset( m_commandAllocator, m_curPipelineState );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Reset Command List!" );

	m_commandList->SetGraphicsRootSignature( m_rootSignature );
	// set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_constantsDescHeap };
	m_commandList->SetDescriptorHeaps( _countof( descriptorHeaps ), descriptorHeaps );

	// set the root descriptor table 0 to the constant buffer descriptor heap
	CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetGPUDescriptorHandleForHeapStart(), DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame, m_scuDescriptorSize );
	//UINT descriptorOffset = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	//m_commandList->SetGraphicsRootDescriptorTable( 0, m_constantsDescHeap->GetGPUDescriptorHandleForHeapStart() );

	m_commandList->SetGraphicsRootDescriptorTable( 1, descriptorHandle );
}

void DX12Renderer::EndFrame()
{
	// Indicate that the back buffer will now be used to present.
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT );
	m_commandList->ResourceBarrier( 1, &barrier );

	HRESULT hr = m_commandList->Close();
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Close Command List!" );

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

	// Present the frame.
	m_swapChain->Present( 1, 0 );

	WaitForPreviousFrame();

	// delete all temp vertex and index buffers
	for (int i = 0; i < (int)m_tempVertexBuffers.size(); i++) {
		delete m_tempVertexBuffers[i];
	}
	m_tempVertexBuffers.clear();

	for (int i = 0; i < (int)m_tempIndexBuffers.size(); i++) {
		delete m_tempIndexBuffers[i];
	}
	m_tempIndexBuffers.clear();
}

void DX12Renderer::Shutdown()
{
	WaitForPreviousFrame();
	CloseHandle( m_fenceEvent );

	for (auto& pair : m_loadedShaders) {
		delete pair.second;
	}
	m_loadedShaders.clear();

	for (auto& pair : m_loadedTextures) {
		delete pair.second;
	}
	m_loadedTextures.clear();

	for (auto& pair : m_loadedFonts) {
		delete pair.second;
	}
	m_loadedFonts.clear();

	DX_SAFE_RELEASE( m_textureBufferUploadHeap );

	for (auto& pair : m_pipelineStatesCollection) {
		DX_SAFE_RELEASE( pair.second );
	}
	m_pipelineStatesCollection.clear();

	for (int j = 0; j < DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame; j++) {
		delete m_constantBuffers[j];
	}

	DX_SAFE_RELEASE( m_dsDescHeap );
	DX_SAFE_RELEASE( m_depthStencilBuffer );
	DX_SAFE_RELEASE( m_constantsDescHeap );

	DX_SAFE_RELEASE( m_swapChain );
	DX_SAFE_RELEASE( m_device );
	for (int i = 0; i < DX12_FrameCount; i++) {
		DX_SAFE_RELEASE( m_renderTargets[i] );
	}
	DX_SAFE_RELEASE( m_commandAllocator );
	DX_SAFE_RELEASE( m_commandQueue );
	DX_SAFE_RELEASE( m_rootSignature );
	DX_SAFE_RELEASE( m_rtvHeap );
	DX_SAFE_RELEASE( m_commandList );

	//DX_SAFE_RELEASE( m_vertexBuffer );
	DX_SAFE_RELEASE( m_fence );

#ifdef ENGINE_DEBUG_RENDER
	//m_device->ReportLiveObjects();
#endif // ENGINE_DEBUG_RENDER

}

void DX12Renderer::ClearScreen( Rgba8 const& clearColor )
{
	// Indicate that the back buffer will be used as a render target.
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET );
	m_commandList->ResourceBarrier( 1, &barrier );

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize );
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle( m_dsDescHeap->GetCPUDescriptorHandleForHeapStart() );
	m_commandList->OMSetRenderTargets( 1, &rtvHandle, FALSE, &dsvHandle );

	float colorAsFloats[4];
	clearColor.GetAsFloats( colorAsFloats );
	m_commandList->ClearRenderTargetView( rtvHandle, colorAsFloats, 0, nullptr );
	m_commandList->ClearDepthStencilView( m_dsDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr );
}

void DX12Renderer::BeginCamera( Camera const& camera )
{
	m_curCameraConstants.m_projectionMatrix = camera.GetProjectionMatrix();
	m_curCameraConstants.m_viewMatrix = camera.GetViewMatrix();

	static D3D12_VIEWPORT viewport = {};
	AABB2 const& viewportAABB = camera.m_viewPort;
	viewport.TopLeftX = viewportAABB.m_mins.x;
	viewport.TopLeftY = viewportAABB.m_mins.y;
	viewport.Width = viewportAABB.m_maxs.x - viewportAABB.m_mins.x;
	viewport.Height = viewportAABB.m_maxs.y - viewportAABB.m_mins.y;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_cameraViewport = &viewport;

	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = (LONG)viewport.Width;
	scissorRect.bottom = (LONG)viewport.Height;
	scissorRect.top = 0;

	// Set necessary state.
	m_commandList->RSSetViewports( 1, m_cameraViewport );
	m_commandList->RSSetScissorRects( 1, &scissorRect );

}

void DX12Renderer::EndCamera( Camera const& camera )
{
	UNUSED( camera );
}

void DX12Renderer::SetModelConstants( Mat44 const& modelMatrix /*= Mat44()*/, Rgba8 const& modelColor /*= Rgba8::WHITE */ )
{
	ModelConstants modelConstants;
	modelConstants.m_modelMatrix = modelMatrix;
	modelColor.GetAsFloats( &modelConstants.m_modelColorR );
	//CopyCPUToGPU( &modelConstants, sizeof( modelConstants ), m_constantBuffers[m_curDrawCallIndex * DX12_NumOfConstantBuffersPerDraw + k_dx12ModelConstantsSlot] );
	UINT8* cbGPUAddress;
	CD3DX12_RANGE readRange( 0, 0 );    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12ModelConstantsSlot]->m_dx12ConstantBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&cbGPUAddress) );
	memcpy( cbGPUAddress, &modelConstants, sizeof( ModelConstants ) );
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12ModelConstantsSlot]->m_dx12ConstantBuffer->Unmap( 0, nullptr );

}

void DX12Renderer::SetDirectionalLightConstants( DirectionalLightConstants const& dlc )
{
	UNUSED( dlc );
}

void DX12Renderer::SetLightConstants( Vec3 const& lightPosition, float ambient, Mat44 const& lightViewMatrix, Mat44 const& lightProjectionMatrix )
{
	UNUSED( lightPosition ); UNUSED( ambient ); UNUSED( lightViewMatrix ); UNUSED( lightProjectionMatrix );
}

void DX12Renderer::SetCustomConstantBuffer( ConstantBuffer*& cbo, void* data, size_t size, int slot )
{
	// we do not need this cbo here
	UNUSED( cbo );

	UINT8* cbGPUAddress;
	CD3DX12_RANGE readRange( 0, 0 );    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + slot]->m_dx12ConstantBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&cbGPUAddress) );
	memcpy( cbGPUAddress, data, size );
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + slot]->m_dx12ConstantBuffer->Unmap( 0, nullptr );
}

void DX12Renderer::DrawVertexArray( int numVertexes, const Vertex_PCU* vertexed )
{
	if (numVertexes == 0) {
		return;
	}

	// Create a temp vertex buffer.
	const size_t vertexBufferSize = sizeof( Vertex_PCU ) * numVertexes;

	VertexBuffer* vertBuffer = CreateVertexBuffer( vertexBufferSize, sizeof( Vertex_PCU ) );
	CopyCPUToGPU( vertexed, vertexBufferSize, vertBuffer );
	m_tempVertexBuffers.push_back( vertBuffer );

	DrawVertexBuffer( vertBuffer, numVertexes, 0 );
}

void DX12Renderer::DrawVertexArray( std::vector<Vertex_PCU> const& verts )
{
	DrawVertexArray( (int)verts.size(), verts.data() );
}

void DX12Renderer::DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts )
{
	int numOfVerts = (int)verts.size();
	if (numOfVerts == 0) {
		return;
	}

	// Create a temp vertex buffer.
	const size_t vertexBufferSize = sizeof( Vertex_PCUTBN ) * numOfVerts;

	VertexBuffer* vertBuffer = CreateVertexBuffer( vertexBufferSize, sizeof( Vertex_PCUTBN ) );
	CopyCPUToGPU( verts.data(), vertexBufferSize, vertBuffer );
	m_tempVertexBuffers.push_back( vertBuffer );

	DrawVertexBuffer( vertBuffer, numOfVerts, 0 );
}

void DX12Renderer::DrawVertexArray( std::vector<Vertex_PCU> const& verts, std::vector<unsigned int> const& indexes )
{
	int numOfindexes = (int)indexes.size();
	if (numOfindexes == 0) {
		return;
	}
	int numOfVerts = (int)verts.size();

	// Create a temp vertex buffer.
	const size_t vertexBufferSize = sizeof( Vertex_PCU ) * numOfVerts;

	VertexBuffer* vertBuffer = CreateVertexBuffer( vertexBufferSize, sizeof( Vertex_PCU ) );
	CopyCPUToGPU( verts.data(), vertexBufferSize, vertBuffer );
	m_tempVertexBuffers.push_back( vertBuffer );

	// Create a temp index buffer
	const size_t indexBufferSize = sizeof( int ) * numOfindexes;

	IndexBuffer* indexBuffer = CreateIndexBuffer( indexBufferSize );
	CopyCPUToGPU( indexes.data(), indexBufferSize, indexBuffer );
	m_tempIndexBuffers.push_back( indexBuffer );

	DrawVertexIndexed( vertBuffer, indexBuffer, numOfindexes, 0 );
}

void DX12Renderer::DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts, std::vector<unsigned int> const& indexes )
{
	int numOfindexes = (int)indexes.size();
	if (numOfindexes == 0) {
		return;
	}
	int numOfVerts = (int)verts.size();

	// Create a temp vertex buffer.
	const size_t vertexBufferSize = sizeof( Vertex_PCUTBN ) * numOfVerts;

	VertexBuffer* vertBuffer = CreateVertexBuffer( vertexBufferSize, sizeof( Vertex_PCUTBN ) );
	CopyCPUToGPU( verts.data(), vertexBufferSize, vertBuffer );
	m_tempVertexBuffers.push_back( vertBuffer );

	// Create a temp index buffer
	const size_t indexBufferSize = sizeof( int ) * numOfindexes;

	IndexBuffer* indexBuffer = CreateIndexBuffer( indexBufferSize );
	CopyCPUToGPU( indexes.data(), indexBufferSize, indexBuffer );
	m_tempIndexBuffers.push_back( indexBuffer );

	DrawVertexIndexed( vertBuffer, indexBuffer, numOfindexes, 0 );
}

void DX12Renderer::DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0 */ )
{
	//CopyCPUToGPU( &m_curCameraConstants, sizeof( CameraConstants ), m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12CameraConstantsSlot] );
	GUARANTEE_OR_DIE( m_curDrawCallIndex < DX12_NumOfDrawCallPerFrame, "Cannot afford more draw calls!" );

	// copy camera constants
	UINT8* cbGPUAddress;
	CD3DX12_RANGE readRange( 0, 0 );    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12CameraConstantsSlot]->m_dx12ConstantBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&cbGPUAddress) );
	memcpy( cbGPUAddress, &m_curCameraConstants, sizeof( CameraConstants ) );
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12CameraConstantsSlot]->m_dx12ConstantBuffer->Unmap( 0, nullptr );

	CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetGPUDescriptorHandleForHeapStart(), DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex, m_scuDescriptorSize );
	m_commandList->SetGraphicsRootDescriptorTable( 0, descriptorHandle );
	// Set commands.
	SetStatesIfChanged();
	BindVertexBuffer( vbo );
	m_commandList->DrawInstanced( vertexCount, 1, vertexOffset, 0 );
	++m_curDrawCallIndex;
}

void DX12Renderer::DrawVertexIndexed( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset /*= 0 */ )
{
	GUARANTEE_OR_DIE( m_curDrawCallIndex < DX12_NumOfDrawCallPerFrame, "Cannot afford more draw calls!" );

	// copy camera constants
	UINT8* cbGPUAddress;
	CD3DX12_RANGE readRange( 0, 0 );    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12CameraConstantsSlot]->m_dx12ConstantBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&cbGPUAddress) );
	memcpy( cbGPUAddress, &m_curCameraConstants, sizeof( CameraConstants ) );
	m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex + k_dx12CameraConstantsSlot]->m_dx12ConstantBuffer->Unmap( 0, nullptr );

	CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetGPUDescriptorHandleForHeapStart(), DX12_NumOfConstantBuffersPerDraw * m_curDrawCallIndex, m_scuDescriptorSize );
	m_commandList->SetGraphicsRootDescriptorTable( 0, descriptorHandle );

	// Set commands.
	SetStatesIfChanged();
	BindVertexBuffer( vbo );
	BindIndexBuffer( ibo );
	m_commandList->DrawIndexedInstanced( indexCount, 1, indexOffset, 0, 0 );
	++m_curDrawCallIndex;
}

Shader* DX12Renderer::CreateShader( char const* shaderName, VertexType type /*= VertexType::PCU */ )
{
	std::map<std::string, Shader*>::iterator found = m_loadedShaders.find( shaderName );
	if (found != m_loadedShaders.end()) {
		return found->second;
	}
	std::string sourceString;
	std::string filePath( shaderName );
	filePath += ".hlsl";
	/*int size = */FileReadToString( sourceString, filePath );
	//GUARANTEE_OR_DIE( size == sourceString.size(), "Read shader error!" );
	return CreateShader( shaderName, sourceString.c_str(), type );
}

Shader* DX12Renderer::CreateShader( char const* shaderName, char const* shaderSource, VertexType type /*= VertexType::PCU */ )
{
	std::map<std::string, Shader*>::iterator found = m_loadedShaders.find( shaderName );
	if (found != m_loadedShaders.end()) {
		return found->second;
	}
	ShaderConfig sConfig;
	sConfig.m_name = std::string( shaderName );
	Shader* shader = new Shader( sConfig );

	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;

	CompileShaderToByteCode( &vertexShader, "VertexShader", shaderSource, sConfig.m_vertexEntryPoint.c_str(), "vs_5_0" );
	CompileShaderToByteCode( &pixelShader, "PixelShader", shaderSource, sConfig.m_pixelEntryPoint.c_str(), "ps_5_0" );

	// Define the vertex input layout.
	static D3D12_INPUT_ELEMENT_DESC inputElementDescsForPCU[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	static D3D12_INPUT_ELEMENT_DESC inputElementDescsForPCUTBN[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	shader->m_vertexShader = vertexShader;
	shader->m_pixelShader = pixelShader;
	shader->m_inputLayoutForVertex = new D3D12_INPUT_LAYOUT_DESC();
	if (type == VertexType::PCU) {
		shader->m_inputLayoutForVertex->pInputElementDescs = inputElementDescsForPCU;
		shader->m_inputLayoutForVertex->NumElements = _countof( inputElementDescsForPCU );
	}
	else if (type == VertexType::PCUTBN) {
		shader->m_inputLayoutForVertex->pInputElementDescs = inputElementDescsForPCUTBN;
		shader->m_inputLayoutForVertex->NumElements = _countof( inputElementDescsForPCUTBN );
	}

	shader->m_shaderIndex = (int)m_loadedShaders.size();
	GUARANTEE_OR_DIE( shader->m_shaderIndex <= 65535, "Cannot create more than 65535 shaders!" );
	m_loadedShaders[shaderName] = shader;
	return shader;
}

bool DX12Renderer::CompileShaderToByteCode( ID3DBlob** out_shaderByteCode, char const* name, char const* source, char const* entryPoint, char const* target )
{
#if defined ENGINE_DEBUG_RENDER
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif


	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;

	// Compile shader
	HRESULT hr = D3DCompile(
		source, strlen( source ),
		name, nullptr, nullptr,
		entryPoint, target, compileFlags, 0,
		&shaderBlob, &errorBlob
	);
	if (SUCCEEDED( hr )) {
		*out_shaderByteCode = shaderBlob;
		if (errorBlob != NULL) {
			errorBlob->Release();
		}
		return true;
	}
	else {
		if (errorBlob != NULL) {
			DebuggerPrintf( (char*)errorBlob->GetBufferPointer() );
		}
		if (errorBlob != NULL) {
			errorBlob->Release();
		}
		ERROR_AND_DIE( Stringf( "Could not compile %s.", name ) );
	}
}

void DX12Renderer::BindShader( Shader* shader )
{
	if (shader == nullptr) {
		m_desiredShader = m_defaultShader;
	}
	else {
		m_desiredShader = shader;
	}
}

VertexBuffer* DX12Renderer::CreateVertexBuffer( size_t const size, unsigned int stride /*= sizeof( Vertex_PCU ) */ )
{
	// Note: using upload heaps to transfer static data like vert buffers is not
	// recommended. Every time the GPU needs it, the upload heap will be marshalled
	// over. Please read up on Default Heap usage. An upload heap is used here for
	// code simplicity and because there are very few verts to actually transfer.

	VertexBuffer* vertBuffer = new VertexBuffer( size, stride );

	CD3DX12_HEAP_PROPERTIES heapProps( D3D12_HEAP_TYPE_UPLOAD );
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer( size );
	HRESULT hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS( &vertBuffer->m_dx12VertexBuffer ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Create Vertex Buffer!" );

	return vertBuffer;
}

void DX12Renderer::CopyCPUToGPU( void const* data, size_t size, VertexBuffer*& vbo )
{
	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange( 0, 0 );        // We do not intend to read from this resource on the CPU.
	vbo->m_dx12VertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin) );
	memcpy( pVertexDataBegin, data, size );
	vbo->m_dx12VertexBuffer->Unmap( 0, nullptr );

	// Initialize the vertex buffer view.
	vbo->m_vertexBufferView->BufferLocation = vbo->m_dx12VertexBuffer->GetGPUVirtualAddress();
	vbo->m_vertexBufferView->StrideInBytes = sizeof( Vertex_PCU );
	vbo->m_vertexBufferView->SizeInBytes = (UINT)size;
}

void DX12Renderer::CopyCPUToGPU( void const* data, size_t size, ConstantBuffer*& cbo )
{
	DebuggerPrintf( "Warning! Cannot copy CPU to GPU for constant buffer in dx12 interface! Use set custom constants instead!" );
	UNUSED( data ); UNUSED( size ); UNUSED( cbo );
	/*UINT8* cbGPUAddress;
	CD3DX12_RANGE readRange( 0, 0 );    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
	cbo->m_dx12ConstantBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&cbGPUAddress) );
	memcpy( cbGPUAddress, &data, size );
	cbo->m_dx12ConstantBuffer->Unmap( 0, nullptr );*/
}

void DX12Renderer::CopyCPUToGPU( void const* data, size_t size, IndexBuffer*& ibo )
{
	// Copy the index data to the index buffer.
	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange( 0, 0 );        // We do not intend to read from this resource on the CPU.
	ibo->m_dx12IndexBuffer->Map( 0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin) );
	memcpy( pIndexDataBegin, data, size );
	ibo->m_dx12IndexBuffer->Unmap( 0, nullptr );

	// Initialize the index buffer view.
	ibo->m_indexBufferView->BufferLocation = ibo->m_dx12IndexBuffer->GetGPUVirtualAddress();
	ibo->m_indexBufferView->Format = DXGI_FORMAT_R32_UINT;
	ibo->m_indexBufferView->SizeInBytes = (UINT)size;
}

void DX12Renderer::BindIndexBuffer( IndexBuffer* ibo )
{
	//m_commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_commandList->IASetIndexBuffer( ibo->m_indexBufferView );
}

Texture* DX12Renderer::CreateOrGetTextureFromFile( char const* filePath )
{	
	// See if we already have this texture previously loaded
	std::map<std::string, Texture*>::iterator/*auto*/ findTexture = m_loadedTextures.find( filePath );
	if (findTexture != m_loadedTextures.end()) {
		return findTexture->second;
	}

	// Never seen this texture before!  Let's load it.
	return CreateTextureFromFile( filePath );
}

BitmapFont* DX12Renderer::CreateOrGetBitmapFontFromFile( char const* filePathNoExtension )
{	/*constexpr int maxSizeOfString = 400;
	//char* filePath = (char*)malloc( maxSizeOfString ); // do not forget to free
	char filePath[maxSizeOfString];
	strcpy_s( filePath, maxSizeOfString, filePathNoExtension );
	strcat_s( filePath, maxSizeOfString, ".png" );*/
	std::string filePath(filePathNoExtension);
	if (filePath == "Data/Fonts/Alvin'sStupidChineseFont") {
		filePath += ".fnt";
	}
	else {
		filePath += ".png";
	}
	std::map<std::string, BitmapFont*>::iterator findFont = m_loadedFonts.find( filePath );
	if (findFont != m_loadedFonts.end()) {
		return findFont->second;
	}
	Texture* bitmapTexture;
	BitmapFont* newBitmap;
	if (filePath == "Data/Fonts/Alvin'sStupidChineseFont.fnt") {
		bitmapTexture = CreateTextureFromFile( (std::string( filePathNoExtension ) + ".png").c_str() );
		newBitmap = new BitmapFont( filePath.c_str(), *bitmapTexture, IntVec2( 200, 150 ) );
	}
	else {
		bitmapTexture = CreateTextureFromFile( filePath.c_str() );
		newBitmap = new BitmapFont( filePath.c_str(), *bitmapTexture );
	}
	m_loadedFonts[filePath] = newBitmap;
	return newBitmap;
}

Image* DX12Renderer::CreateImageFromFile( char const* filePath )
{
	return new Image( filePath );
}

Texture* DX12Renderer::CreateTextureFromImage( Image const* image )
{
	Texture* newTexture = new Texture();
	newTexture->m_dimensions = image->GetDimensions();
	newTexture->m_textureIndex = (int)m_loadedTextures.size();

	GUARANTEE_OR_DIE( newTexture->m_textureIndex < DX12_MaxNumOfTextures, Stringf( "Cannot create more than %d textures!", DX12_MaxNumOfTextures ) );

	// now describe the texture with the information we have obtained from the image
	D3D12_RESOURCE_DESC resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	resourceDescription.Width = newTexture->m_dimensions.x; // width of the texture
	resourceDescription.Height = newTexture->m_dimensions.y; // height of the texture
	resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	resourceDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // This is the dxgi format of the image (format of the pixels)
	resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags
	
	D3D12_HEAP_PROPERTIES properties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT );

	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	HRESULT hr = m_device->CreateCommittedResource(
		&properties, // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&resourceDescription, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS( &newTexture->m_dx12Texture ) );

	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Cannot create texture committed resource!" );

	// upload the texture to GPU
	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = image->GetRawData(); // pointer to our image data
	textureData.RowPitch = image->GetDimensions().x * 32 / 8; // size of all our triangle vertex data
	textureData.SlicePitch = image->GetDimensions().x * image->GetDimensions().y; // also the size of our triangle vertex data

	GUARANTEE_OR_DIE( newTexture->m_dx12Texture != nullptr, "Cannot create texture in dx12!" );

	// Create upload heap

	// aligned by 256, hard code 256 * 256 * 256 here
	UINT64 textureUploadBufferSize = 256 * 65536;

	properties = CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD );
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( textureUploadBufferSize );
	// now we create an upload heap to upload our texture to the GPU
	hr = m_device->CreateCommittedResource(
		&properties, // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&resourceDesc, // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS( &newTexture->m_textureBufferUploadHeap ) );

	// Now we copy the upload buffer contents to the default heap
	UpdateSubresources( m_commandList, newTexture->m_dx12Texture, newTexture->m_textureBufferUploadHeap, 0, 0, 1, &textureData );

	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( newTexture->m_dx12Texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
	m_commandList->ResourceBarrier( 1, &barrier );


	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetCPUDescriptorHandleForHeapStart(), DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame + newTexture->m_textureIndex, m_scuDescriptorSize );
	// Create Shader Resource View for Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView( newTexture->m_dx12Texture, &srvDesc, descriptorHandle );
	
	return newTexture;
}

void DX12Renderer::BindTexture( Texture const* texture, int slot )
{
	UNUSED( slot );
	if (texture == nullptr) {
		CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetGPUDescriptorHandleForHeapStart(), DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame + m_defaultTexture->m_textureIndex, m_scuDescriptorSize );
		m_commandList->SetGraphicsRootDescriptorTable( 1, descriptorHandle );
	}
	else {
		CD3DX12_GPU_DESCRIPTOR_HANDLE descriptorHandle( m_constantsDescHeap->GetGPUDescriptorHandleForHeapStart(), DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame + texture->m_textureIndex, m_scuDescriptorSize );
		m_commandList->SetGraphicsRootDescriptorTable( 1, descriptorHandle );
	}
	m_currentTexture = texture;
}

void DX12Renderer::SetBlendMode( BlendMode blendMode )
{
	m_desiredBlendMode = blendMode;
}

static D3D12_BLEND_DESC DX12MakeBlendDescHelper( BlendMode mode ) {
	D3D12_BLEND_DESC blendDesc = {};
	if (mode == BlendMode::ADDITIVE) {
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
		blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
		blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	else if (mode == BlendMode::ALPHA) {
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		//blendDesc.RenderTarget[0].LogicOpEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
		blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
		blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	else if (mode == BlendMode::OPAQUE) {
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
		blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
		blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	return blendDesc;
}

static D3D12_RASTERIZER_DESC DX12MakeRasterizerDescHelper( RasterizerMode mode ) {
	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	if (mode == RasterizerMode::SOLID_CULL_NONE) {
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = true;
	}
	else if (mode == RasterizerMode::SOLID_CULL_BACK) {
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = true;
	}
	else if (mode == RasterizerMode::SOLID_CULL_FRONT) {
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = true;
	}
	else if (mode == RasterizerMode::WIREFRAME_CULL_BACK) {
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = true;
	}
	else if (mode == RasterizerMode::WIREFRAME_CULL_NONE) {
		rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.f;
		rasterizerDesc.SlopeScaledDepthBias = 0.f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = true;
	}
	return rasterizerDesc;
}

static D3D12_SAMPLER_DESC DX12MakeSamplerDescHelper( SamplerMode mode ) {
	D3D12_SAMPLER_DESC samplerDesc = {};
	if (mode == SamplerMode::POINT_CLAMP) {
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	}
	else if (mode == SamplerMode::BILINEAR_WRAP) {
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	}

	return samplerDesc;
}

static D3D12_DEPTH_STENCIL_DESC DX12MakeDepthStencilDescHelper( DepthMode mode ) {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};

	if (mode == DepthMode::DISABLED) {
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	}
	else if (mode == DepthMode::ENABLED) {
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}

	return depthStencilDesc;
}

void DX12Renderer::SetStatesIfChanged()
{
	// 1.find if the states are changed:
	if (m_currentRasterizerMode == m_desiredRasterizerMode && m_currentBlendMode == m_desiredBlendMode
		&& m_currentDepthMode == m_desiredDepthMode && m_currentSamplerMode == m_desiredSamplerMode
		&& m_currentShader == m_desiredShader) {
		m_commandList->SetPipelineState( m_curPipelineState );
		return;
	}

	// need to change pipeline state
	m_currentRasterizerMode = m_desiredRasterizerMode;
	m_currentBlendMode = m_desiredBlendMode;
	m_currentDepthMode = m_desiredDepthMode;
	m_currentSamplerMode = m_desiredSamplerMode;
	m_currentShader = m_desiredShader;

	// 2.find if the pipeline state has been created
	int indexNum = (m_desiredShader->m_shaderIndex << 16) | ((int)m_desiredDepthMode << 12) 
		| ((int)m_desiredSamplerMode << 8) | ((int)m_desiredRasterizerMode << 4) | ((int)m_desiredBlendMode);

	auto iter = m_pipelineStatesCollection.find( indexNum );
	if (iter != m_pipelineStatesCollection.end()) {
		m_commandList->SetPipelineState( iter->second );
		m_curPipelineState = iter->second;
		return;
	}

	// 3.cannot find: create a new one
	ID3D12PipelineState* pipelineState = nullptr;

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = *m_desiredShader->m_inputLayoutForVertex;
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS = { reinterpret_cast<UINT8*>(m_desiredShader->m_vertexShader->GetBufferPointer()), m_desiredShader->m_vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(m_desiredShader->m_pixelShader->GetBufferPointer()), m_desiredShader->m_pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = DX12MakeRasterizerDescHelper( m_desiredRasterizerMode );
	psoDesc.BlendState = DX12MakeBlendDescHelper( m_desiredBlendMode );
	psoDesc.DepthStencilState = DX12MakeDepthStencilDescHelper( m_desiredDepthMode );
	//psoDesc.DepthStencilState.DepthEnable = false;
	//psoDesc.DepthStencilState.StencilEnable = false;
	//psoDesc.SampleDesc = DX12MakeSamplerDescHelper( m_desiredSamplerMode );
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	HRESULT hr = m_device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &pipelineState ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Create Graphics Pipeline State!" );

	m_commandList->SetPipelineState( pipelineState );
	m_pipelineStatesCollection[indexNum] = pipelineState;
	m_curPipelineState = pipelineState;
}

void DX12Renderer::SetSamplerMode( SamplerMode samplerMode )
{
	m_desiredSamplerMode = samplerMode;
}

void DX12Renderer::SetRasterizerMode( RasterizerMode rasterizerMode )
{
	m_desiredRasterizerMode = rasterizerMode;
}

void DX12Renderer::SetDepthMode( DepthMode depthMode )
{
	m_desiredDepthMode = depthMode;
}

void DX12Renderer::SetShadowMode( ShadowMode shadowMode )
{
	if (shadowMode == ShadowMode::DISABLE) {
		return;
	}
	UNUSED( shadowMode );
	ERROR_AND_DIE( "Do not support shadows in dx12 now!" );
}

void DX12Renderer::RenderShadowMap( std::vector<Vertex_PCUTBN> const& verts )
{
	UNUSED( verts );
	//ERROR_AND_DIE( "Do not support shadows in dx12 now!" );
}

void DX12Renderer::RenderShadowMap( VertexBuffer* vbo, int vertexCount, int vertexOffset )
{
	UNUSED( vbo ); UNUSED( vertexCount ); UNUSED( vertexOffset );
	//ERROR_AND_DIE( "Do not support shadows in dx12 now!" );
}

void DX12Renderer::RenderShadowMap( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset )
{
	UNUSED( vbo ); UNUSED( ibo ); UNUSED( indexCount ); UNUSED( indexOffset );
	//ERROR_AND_DIE( "Do not support shadows in dx12 now!" );
}

AABB2 DX12Renderer::GetSwapChainSize() const
{
	return AABB2();
}

Texture* DX12Renderer::CreateTextureFromFile( char const* filePath )
{
	Image image = Image( filePath );
	Texture* newTexture = CreateTextureFromImage( &image );
	newTexture->m_name = filePath;
	m_loadedTextures[std::string( filePath )] = newTexture;
	return newTexture;
}

void DX12Renderer::BindConstantBuffer( int slot, ConstantBuffer* cbo )
{
	UNUSED( slot ); UNUSED( cbo );
	DebuggerPrintf( "Warning! Cannot bind constant buffer in dx12 interface! Use set xxx constants" );
}

IndexBuffer* DX12Renderer::CreateIndexBuffer( size_t const size )
{
	IndexBuffer* indexBuffer = new IndexBuffer( size );

	CD3DX12_HEAP_PROPERTIES heapProps( D3D12_HEAP_TYPE_UPLOAD );
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer( size );
	HRESULT hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS( &indexBuffer->m_dx12IndexBuffer ) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Create Index Buffer!" );

	return indexBuffer;
}

void DX12Renderer::BindVertexBuffer( VertexBuffer* vbo )
{
	m_commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_commandList->IASetVertexBuffers( 0, 1, vbo->m_vertexBufferView );
}

ConstantBuffer* DX12Renderer::CreateConstantBuffer( size_t const size )
{
	UNUSED( size );
	DebuggerPrintf( "Warning! Cannot Create Constant Buffer in dx12 interface. All Constant Buffer is Created at the very beginning!" );
	return nullptr;
}

void DX12Renderer::WaitForPreviousFrame() {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.

	const UINT64 fence = m_fenceValue;
	HRESULT hr = m_commandQueue->Signal( m_fence, fence );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Signal!" );
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		hr = m_fence->SetEventOnCompletion( fence, m_fenceEvent );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "D3D12 Cannot Set Event On Completion!" );
		WaitForSingleObject( m_fenceEvent, INFINITE );
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

}

#endif