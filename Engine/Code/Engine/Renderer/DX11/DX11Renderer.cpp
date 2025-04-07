#include "Engine/Renderer/DX11/DX11Renderer.hpp"

#ifdef ENGINE_DX11_RENDERER_INTERFACE

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
#include "Engine/Core/VertexUtils.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
// D3D11 includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined ENGINE_DEBUG_RENDER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#include "ThirdParty/stb/stb_image.h"

#if defined OPAQUE
#undef OPAQUE
#endif // OPAQUE


static int const k_lightConstantsSlot = 0;
static int const k_dirLightConstantsSlot = 1;
static int const k_cameraConstantsSlot = 2;
static int const k_modelConstantsSlot = 3;
static int const k_blurConstantSlot = 5;

DX11Renderer::DX11Renderer( Renderer* baseRenderer, RendererConfig config )
{
	m_baseRenderer = baseRenderer;
	m_config = config;
}

DX11Renderer::~DX11Renderer()
{

}

void DX11Renderer::StartUp()
{
		// Create debug module
#if defined ENGINE_DEBUG_RENDER
	m_dxgiDebugModule = (void*)::LoadLibraryA( "dxgidebug.dll" );
	if (m_dxgiDebugModule == nullptr) {
		ERROR_AND_DIE( "Could not load dxgidebug.dll." );
	}

	typedef HRESULT( WINAPI* GetDebugModuleCB )(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress( (HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface" ))
		(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr) {
		ERROR_AND_DIE( "Could not load debug module" );
	}
#endif

	// Render startup
	unsigned int deviceFlags = 0;
#if defined ENGINE_DEBUG_RENDER
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)m_config.m_window->GetWindowHandle();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc,
		&m_swapChain, &m_device, nullptr, &m_deviceContext );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create D3D 11 device and swap chain." );
	}

	//m_swapChain->SetFullscreenState( true, nullptr );
	//m_swapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0 );

	// Get back buffer texture
	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&backBuffer );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not get swap chain buffer" );
	}

	hr = m_device->CreateRenderTargetView( backBuffer, NULL, &m_renderTargetView );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create render target view for swap chain buffer." );
	}

	backBuffer->Release();

	m_defaultShader = CreateShader( "Default", defaultShaderSource );
	BindShader( m_defaultShader );

	m_immediateVBO_PCU = CreateVertexBuffer( 24 );
	m_immediateVBO_PCUTBN = CreateVertexBuffer( 60, sizeof( Vertex_PCUTBN ) );
	m_immediateIBO = CreateIndexBuffer( 24 );
	m_cameraCBO = CreateConstantBuffer( sizeof( CameraConstants ) );
	m_modelCBO = CreateConstantBuffer( sizeof( ModelConstants ) );
	m_lightCBO = CreateConstantBuffer( sizeof( LightConstants ) );
	m_directionalLightCBO = CreateConstantBuffer( sizeof( DirectionalLightConstants ) );

	// Set rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_FRONT] );

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create rasterizer state" );
	}

	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK] );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create rasterizer state" );
	}
	m_rasterizerState = m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK];
	m_deviceContext->RSSetState( m_rasterizerState );

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE] );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create rasterizer state" );
	}

	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	hr = m_device->CreateRasterizerState( &rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK] );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create rasterizer state" );
	}

	// Create blend states
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
	blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)(BlendMode::OPAQUE)] );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Create blend state for OPAQUE failed!" );
	}

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)(BlendMode::ALPHA)] );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Create blend state for ALPHA failed!" );
	}

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	hr = m_device->CreateBlendState( &blendDesc, &m_blendStates[(int)(BlendMode::ADDITIVE)] );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Create blend state for ADDITIVE failed!" );
	}

	// Create default texture
	Image whiteImage = Image( IntVec2( 2, 2 ), Rgba8::WHITE );
	Texture* whiteTexture = CreateTextureFromImage( &whiteImage );
	whiteTexture->m_name = "Default";
	m_defaultTexture = whiteTexture;
	m_loadedTextures["Default"] = whiteTexture;
	BindTexture( m_defaultTexture );

	// Create sampler state
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)SamplerMode::POINT_CLAMP] );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Create sampler state for POINT CLAMP failed!" );

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP] );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Create sampler state for BILINEAR WRAP failed!" );

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = m_device->CreateSamplerState( &samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_CLAMP] );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Create sampler state for BILINEAR CLAMP failed!" );

	m_samplerState = m_samplerStates[(int)SamplerMode::POINT_CLAMP];
	m_deviceContext->PSSetSamplers( 0, 1, &m_samplerState );

	// Create depth stencil
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_config.m_window->GetClientDimensions().x;
	textureDesc.Height = m_config.m_window->GetClientDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = 1;

	hr = m_device->CreateTexture2D( &textureDesc, nullptr, &m_depthStencilTexture );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create texture for depth stencil." );
	hr = m_device->CreateDepthStencilView( m_depthStencilTexture, nullptr, &m_depthStencilView );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create depth stencil view." );

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStates[(int)DepthMode::DISABLED] );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create depth stencil state." );

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	//depthStencilDesc.StencilEnable = true;
	hr = m_device->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStates[(int)DepthMode::ENABLED] );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create depth stencil state." );
	m_depthStencilState = m_depthStencilStates[(int)DepthMode::ENABLED];
	m_deviceContext->OMSetDepthStencilState( m_depthStencilState, 0 );

	m_emissiveTexture = CreateRenderTexture( m_config.m_window->GetClientDimensions(), "EmissiveTexture" );
	m_screenTexture = CreateRenderTexture( m_config.m_window->GetClientDimensions() * 8, "ScreenTexture");

	for (int i = 0; i < k_blurDownTextureCount; i++) {
		m_blurDownTextures[i] = CreateRenderTexture(m_config.m_window->GetClientDimensions() / (int)pow(2, i + 1), Stringf("Blur Down Texture %d", i).c_str());
	}

	for (int i = 0; i < k_blurUpTextureCount; i++) {
		m_blurUpTextures[i] = CreateRenderTexture( m_config.m_window->GetClientDimensions() / (int)pow(2, i), Stringf("Blur Up Texture %d", i).c_str());
	}
	

	// create shadow map
	// https://learn.microsoft.com/en-us/windows/uwp/gaming/create-depth-buffer-resource--view--and-sampler-state
	// check if shadow map is supported
	D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT isD3D9ShadowSupported = {};
	m_device->CheckFeatureSupport(
		D3D11_FEATURE_D3D9_SHADOW_SUPPORT,
		&isD3D9ShadowSupported,
		sizeof( D3D11_FEATURE_D3D9_SHADOW_SUPPORT )
	);

	if (isD3D9ShadowSupported.SupportsDepthAsTextureWithLessEqualComparisonFilter)
	{
		// Init shadow map resources
		D3D11_TEXTURE2D_DESC shadowMapDesc = {};
		shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		shadowMapDesc.MipLevels = 1;
		shadowMapDesc.ArraySize = 1;
		shadowMapDesc.SampleDesc.Count = 1;
		shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		shadowMapDesc.Width = m_config.m_window->GetClientDimensions().x;
		shadowMapDesc.Height = m_config.m_window->GetClientDimensions().y;

		hr = m_device->CreateTexture2D( &shadowMapDesc, nullptr, &m_shadowMapTexture );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create shadow map texture." );

		// create shader resource view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		hr = m_device->CreateDepthStencilView( m_shadowMapTexture, &depthStencilViewDesc, &m_shadowDepthView );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create shadow stencil view." );

		hr = m_device->CreateShaderResourceView( m_shadowMapTexture, &shaderResourceViewDesc, &m_shadowResourceView );
		GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create shadow shader resource view." );

		// create comparison sampler
		D3D11_SAMPLER_DESC comparisonSamplerDesc = {};
		comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		comparisonSamplerDesc.BorderColor[0] = 1.0f;
		comparisonSamplerDesc.BorderColor[1] = 1.0f;
		comparisonSamplerDesc.BorderColor[2] = 1.0f;
		comparisonSamplerDesc.BorderColor[3] = 1.0f;
		comparisonSamplerDesc.MinLOD = 0.f;
		comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		comparisonSamplerDesc.MipLODBias = 0.f;
		comparisonSamplerDesc.MaxAnisotropy = 0;
		comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

		// Point filtered shadows can be faster, and may be a good choice when
		// rendering on hardware with lower feature levels. This sample has a
		// UI option to enable/disable filtering so you can see the difference
		// in quality and speed.

		m_device->CreateSamplerState( &comparisonSamplerDesc, &m_comparisonSampler_point );

		// create cull front above (in rasterizer state)

		// create view port
		m_shadowViewport = new D3D11_VIEWPORT();
		m_shadowViewport->TopLeftX = 0.f;
		m_shadowViewport->TopLeftY = 0.f;
		m_shadowViewport->Height = (FLOAT)shadowMapDesc.Height;
		m_shadowViewport->Width = (FLOAT)shadowMapDesc.Width;
		m_shadowViewport->MinDepth = 0.f;
		m_shadowViewport->MaxDepth = 1.f;
	}
	SetModelConstants();

	m_blurCBO = CreateConstantBuffer( sizeof( BlurConstants ) );
}

void DX11Renderer::BeginFrame()
{
	SetDefaultRenderTargets();
	//SetScreenRenderTargetView();
}

void DX11Renderer::EndFrame()
{
	// Present
	HRESULT hr;
	hr = m_swapChain->Present( 0, 0 );
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		ERROR_AND_DIE( "Device has been lost, application will now terminate." );
	}
}

void DX11Renderer::Shutdown()
{
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

	for (int i = 0; i < k_blurUpTextureCount; i++) {
		delete m_blurUpTextures[i];
	}

	for (int i = 0; i < k_blurDownTextureCount; i++) {
		delete m_blurDownTextures[i];
	}

	delete m_emissiveTexture;
	delete m_screenTexture;

	delete m_immediateVBO_PCU;
	delete m_immediateVBO_PCUTBN;
	delete m_fullScreenQuadVBO_PCU;
	delete m_immediateIBO;
	delete m_blurCBO;
	delete m_directionalLightCBO;
	delete m_cameraCBO;
	delete m_modelCBO;
	delete m_lightCBO;
	delete m_shadowViewport;

	for (int i = 0; i < (int)(BlendMode::COUNT); i++) {
		DX_SAFE_RELEASE( m_blendStates[i] );
	}

	for (int i = 0; i < (int)(SamplerMode::COUNT); i++) {
		DX_SAFE_RELEASE( m_samplerStates[i] );
	}

	for (int i = 0; i < (int)(RasterizerMode::COUNT); i++) {
		DX_SAFE_RELEASE( m_rasterizerStates[i] );
	}

	for (int i = 0; i < (int)(DepthMode::COUNT); i++) {
		DX_SAFE_RELEASE( m_depthStencilStates[i] );
	}

	DX_SAFE_RELEASE( m_comparisonSampler_point );
	DX_SAFE_RELEASE( m_shadowDepthView );
	DX_SAFE_RELEASE( m_shadowResourceView );
	DX_SAFE_RELEASE( m_shadowMapTexture );
	DX_SAFE_RELEASE( m_depthStencilView );
	DX_SAFE_RELEASE( m_depthStencilTexture );
	DX_SAFE_RELEASE( m_renderTargetView );
	DX_SAFE_RELEASE( m_swapChain );
	DX_SAFE_RELEASE( m_deviceContext );
	DX_SAFE_RELEASE( m_device );

	// Report error leaks and release debug module
#if defined ENGINE_DEBUG_RENDER
	( (IDXGIDebug*)m_dxgiDebug )->ReportLiveObjects(
		DXGI_DEBUG_ALL,
		(DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_ALL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
	);

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary( (HMODULE)m_dxgiDebugModule );
	m_dxgiDebugModule = nullptr;
#endif
}

void DX11Renderer::ClearScreen( Rgba8 const& clearColor, Rgba8 const& emissiveColor )
{
	// Clear the screen
	float colorAsFloats[4];
	clearColor.GetAsFloats( colorAsFloats );
	float colorWhite[4];
	emissiveColor.GetAsFloats( colorWhite );
	m_deviceContext->ClearRenderTargetView( m_renderTargetView, colorAsFloats );
	m_deviceContext->ClearRenderTargetView( m_emissiveTexture->m_renderTargetView, colorWhite );
	m_deviceContext->ClearRenderTargetView( m_screenTexture->m_renderTargetView, colorAsFloats );
	for (int i = 0; i < k_blurUpTextureCount; i++) {
		m_deviceContext->ClearRenderTargetView( m_blurUpTextures[i]->m_renderTargetView, colorWhite );
	}
	for (int i = 0; i < k_blurDownTextureCount; i++) {
		m_deviceContext->ClearRenderTargetView( m_blurDownTextures[i]->m_renderTargetView, colorWhite );
	}

	m_deviceContext->ClearDepthStencilView( m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
	m_deviceContext->ClearDepthStencilView( m_shadowDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
}

void DX11Renderer::BeginCamera( Camera const& camera )
{
	CameraConstants cameraConstants;
	cameraConstants.m_projectionMatrix = camera.GetProjectionMatrix();
	cameraConstants.m_viewMatrix = camera.GetViewMatrix();
	CopyCPUToGPU( &cameraConstants, sizeof( cameraConstants ), m_cameraCBO );
	BindConstantBuffer( k_cameraConstantsSlot, m_cameraCBO );
	// Set viewport
	static D3D11_VIEWPORT viewport = {};
	AABB2 const& viewportAABB = camera.m_viewPort;
	viewport.TopLeftX = viewportAABB.m_mins.x;
	viewport.TopLeftY = viewportAABB.m_mins.y;
	//float cameraAspect = (camera.m_cameraBox.m_maxs.x - camera.m_cameraBox.m_mins.x) / (camera.m_cameraBox.m_maxs.y - camera.m_cameraBox.m_mins.y);
	viewport.Width = viewportAABB.m_maxs.x - viewportAABB.m_mins.x;
	viewport.Height = viewportAABB.m_maxs.y - viewportAABB.m_mins.y;
	/*if (viewport.Width > cameraAspect * viewport.Height) {
		float newHeight = viewport.Width / cameraAspect;
		viewport.TopLeftY = viewport.Height - newHeight;
		viewport.Height = newHeight;
	}
	if (viewport.Height * cameraAspect > viewport.Width) {
		viewport.Width = viewport.Height * cameraAspect;
	}*/
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;
	m_cameraViewport = &viewport;
	//m_deviceContext->RSSetViewports( 1, &viewport );
}

void DX11Renderer::EndCamera( Camera const& camera )
{
	UNUSED( camera );
}

void DX11Renderer::SetModelConstants( Mat44 const& modelMatrix /*= Mat44()*/, Rgba8 const& modelColor /*= Rgba8::WHITE */ )
{
	ModelConstants modelConstants;
	modelConstants.m_modelMatrix = modelMatrix;
	modelColor.GetAsFloats( &modelConstants.m_modelColorR );
	CopyCPUToGPU( &modelConstants, sizeof( modelConstants ), m_modelCBO );
	BindConstantBuffer( k_modelConstantsSlot, m_modelCBO );
}

void DX11Renderer::SetDirectionalLightConstants( DirectionalLightConstants const& dlc )
{
	CopyCPUToGPU( &dlc, sizeof( dlc ), m_directionalLightCBO );
	BindConstantBuffer( k_dirLightConstantsSlot, m_directionalLightCBO );

}

void DX11Renderer::SetLightConstants( Vec3 const& lightPosition, float ambient, Mat44 const& lightViewMatrix, Mat44 const& lightProjectionMatrix )
{
	LightConstants lightConstants;
	lightConstants.m_lightPosition = lightPosition;
	lightConstants.m_lightViewMatrix = lightViewMatrix;
	lightConstants.m_lightProjectionMatrix = lightProjectionMatrix;
	lightConstants.m_ambient = ambient;
	CopyCPUToGPU( &lightConstants, sizeof( lightConstants ), m_lightCBO );
	BindConstantBuffer( k_lightConstantsSlot, m_lightCBO );
}

void DX11Renderer::SetCustomConstantBuffer( ConstantBuffer*& cbo, void* data, size_t size, int slot )
{
	CopyCPUToGPU( data, size, cbo );
	BindConstantBuffer( slot, cbo );
}

void DX11Renderer::DrawVertexArray( int numVertexes, const Vertex_PCU* vertexed )
{
	CopyCPUToGPU( vertexed, (size_t)numVertexes * sizeof( Vertex_PCU ), m_immediateVBO_PCU );
	DrawVertexBuffer( m_immediateVBO_PCU, numVertexes, 0 );
}

void DX11Renderer::DrawVertexArray( std::vector<Vertex_PCU> const& verts )
{
	DrawVertexArray( (int)verts.size(), verts.data() );
}

void DX11Renderer::DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts )
{
	CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCUTBN ), m_immediateVBO_PCUTBN );
	DrawVertexBuffer( m_immediateVBO_PCUTBN, (int)verts.size(), 0 );
}

void DX11Renderer::DrawVertexArray( std::vector<Vertex_PCU> const& verts, std::vector<unsigned int> const& indexes )
{
	CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_immediateVBO_PCU );
	CopyCPUToGPU( indexes.data(), indexes.size() * sizeof( unsigned int ), m_immediateIBO );
	DrawVertexIndexed( m_immediateVBO_PCU, m_immediateIBO, (int)indexes.size(), 0 );
}

void DX11Renderer::DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts, std::vector<unsigned int> const& indexes )
{
	CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCUTBN ), m_immediateVBO_PCUTBN );
	CopyCPUToGPU( indexes.data(), indexes.size() * sizeof( unsigned int ), m_immediateIBO );
	DrawVertexIndexed( m_immediateVBO_PCUTBN, m_immediateIBO, (int)indexes.size(), 0 );
}

void DX11Renderer::DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0 */ )
{
	//if (m_shadowMode != ShadowMode::DISABLE) {
	//	RenderShadowMap( vbo, vertexCount, vertexOffset );
	//}
	BindVertexBuffer( vbo );
	SetStatesIfChanged();
	m_deviceContext->Draw( vertexCount, vertexOffset );
}

void DX11Renderer::DrawVertexBuffers( int bufferCount, VertexBuffer** vbo, int vertexCount, int vertexOffset )
{
	ID3D11Buffer** buffers = new ID3D11Buffer*[bufferCount];
	UINT* startOffset = new UINT[bufferCount];
	UINT* stride = new UINT[bufferCount];

	for (int i = 0; i < bufferCount; i++) {
		buffers[i] = vbo[i]->m_vertexBuffer;
		startOffset[i] = 0;
		stride[i] = vbo[i]->GetStride();
		if (i == 0) {
			if (vbo[0]->m_isLinePrimitive) {
				m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
			}
			else {
				m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			}
		}
	}
	m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, stride, startOffset );

	delete[] buffers;
	delete[] startOffset;
	delete[] stride;
	
	SetStatesIfChanged();
	m_deviceContext->Draw( vertexCount, vertexOffset );
}

void DX11Renderer::DrawVertexBuffersIndexed( int bufferCount, VertexBuffer** vbo, IndexBuffer* ibo, int indexCount, int indexOffset /*= 0 */ )
{
	ID3D11Buffer** buffers = new ID3D11Buffer * [bufferCount];
	UINT* startOffset = new UINT[bufferCount];
	UINT* stride = new UINT[bufferCount];

	for (int i = 0; i < bufferCount; i++) {
		buffers[i] = vbo[i]->m_vertexBuffer;
		startOffset[i] = 0;
		stride[i] = vbo[i]->GetStride();
		if (i == 0) {
			if (vbo[0]->m_isLinePrimitive) {
				m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
			}
			else {
				m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			}
		}
	}
	m_deviceContext->IASetVertexBuffers( 0, bufferCount, buffers, stride, startOffset );

	delete[] buffers;
	delete[] startOffset;
	delete[] stride;

	BindIndexBuffer( ibo );
	SetStatesIfChanged();
	m_deviceContext->DrawIndexed( indexCount, indexOffset, 0 );
}

void DX11Renderer::DrawVertexIndexed( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset /*= 0 */ )
{
	//if (m_shadowMode != ShadowMode::DISABLE) {
	//	RenderShadowMap( vbo, ibo, indexCount, indexOffset );
	//}
	BindVertexBuffer( vbo );
	BindIndexBuffer( ibo );
	SetStatesIfChanged();
	m_deviceContext->DrawIndexed( indexCount, indexOffset, 0 );
}

void DX11Renderer::RenderEmissive()
{
	std::vector<Vertex_PCU> screenVerts;
	AddVertsForAABB2D( screenVerts, AABB2( Vec2( -1.f, 1.f ), Vec2( 0.f, 0.f ) ), Rgba8::WHITE );

	m_blurConstants.m_numSamples = 13;
	m_blurConstants.m_texelSize = Vec2( 1.f / m_emissiveTexture->GetDimensions().x, 1.f / m_emissiveTexture->GetDimensions().y );
	m_blurConstants.m_samples[0].m_offset = Vec2();
	m_blurConstants.m_samples[0].m_weight = 0.0968f;
	m_blurConstants.m_samples[1].m_offset = Vec2( 1.f, 1.f );
	m_blurConstants.m_samples[1].m_weight = 0.129f;
	m_blurConstants.m_samples[2].m_offset = Vec2( 1.f, -1.f );
	m_blurConstants.m_samples[2].m_weight = 0.129f;
	m_blurConstants.m_samples[3].m_offset = Vec2( -1.f, -1.f );
	m_blurConstants.m_samples[3].m_weight = 0.129f;
	m_blurConstants.m_samples[4].m_offset = Vec2( -1.f, 1.f );
	m_blurConstants.m_samples[4].m_weight = 0.129f;
	m_blurConstants.m_samples[5].m_offset = Vec2( 2.f, 0.f );
	m_blurConstants.m_samples[5].m_weight = 0.0645f;
	m_blurConstants.m_samples[6].m_offset = Vec2( -2.f, 0.f );
	m_blurConstants.m_samples[6].m_weight = 0.0645f;
	m_blurConstants.m_samples[7].m_offset = Vec2( 0.f, 2.f );
	m_blurConstants.m_samples[7].m_weight = 0.0645f;
	m_blurConstants.m_samples[8].m_offset = Vec2( 0.f, -2.f );
	m_blurConstants.m_samples[8].m_weight = 0.0645f;
	m_blurConstants.m_samples[9].m_offset = Vec2( 2.f, 2.f );
	m_blurConstants.m_samples[9].m_weight = 0.0323f;
	m_blurConstants.m_samples[10].m_offset = Vec2( -2.f, 2.f );
	m_blurConstants.m_samples[10].m_weight = 0.0323f;
	m_blurConstants.m_samples[11].m_offset = Vec2( 2.f, -2.f );
	m_blurConstants.m_samples[11].m_weight = 0.0323f;
	m_blurConstants.m_samples[12].m_offset = Vec2( -2.f, -2.f );
	m_blurConstants.m_samples[12].m_weight = 0.0323f;

	SetCustomConstantBuffer( m_blurCBO, &m_blurConstants, sizeof( BlurConstants ), k_blurConstantSlot );
	m_deviceContext->OMSetRenderTargets( 1, &m_blurDownTextures[0]->m_renderTargetView, nullptr );
	
	BindTexture( m_emissiveTexture );
	BindShader( CreateShader( "BlurDown", blurDownShaderSource ) );
	SetModelConstants();
	SetRasterizerMode( RasterizerMode::SOLID_CULL_FRONT );
	SetSamplerMode( SamplerMode::BILINEAR_CLAMP );
	SetBlendMode( BlendMode::OPAQUE );
	SetDepthMode( DepthMode::DISABLED );
	DrawVertexArray( screenVerts );

	for (int i = 1; i < k_blurDownTextureCount; i++) {
		screenVerts.clear();
		AddVertsForAABB2D( screenVerts, AABB2( Vec2( -1.f, 1.f ), Vec2( -1.f + (float)pow( 0.5, i ), 1.f - (float)pow( 0.5, i ) ) ), Rgba8::WHITE );
		m_blurConstants.m_texelSize = Vec2( 1.f / m_blurDownTextures[i - 1]->GetDimensions().x, 1.f / m_blurDownTextures[i - 1]->GetDimensions().y );
		SetCustomConstantBuffer( m_blurCBO, &m_blurConstants, sizeof( BlurConstants ), k_blurConstantSlot );
		m_deviceContext->OMSetRenderTargets( 1, &m_blurDownTextures[i]->m_renderTargetView, nullptr );
		BindTexture( m_blurDownTextures[i - 1] );
		DrawVertexArray( screenVerts );
	}

	// blur up
	screenVerts.clear();
	AddVertsForAABB2D( screenVerts, AABB2( Vec2( -1.f, 1.f ), Vec2( -1.f + (float)pow( 0.5, k_blurUpTextureCount - 2 ), 1.f - (float)pow( 0.5, k_blurUpTextureCount - 2 ) ) ), Rgba8::WHITE );

	m_blurConstants.m_numSamples = 9;
	m_blurConstants.m_lerpT = 0.85f;
	m_blurConstants.m_texelSize = Vec2( 1.f / m_blurDownTextures[k_blurDownTextureCount - 1]->GetDimensions().x, 1.f / m_blurDownTextures[k_blurDownTextureCount - 1]->GetDimensions().y);
	m_blurConstants.m_samples[0].m_offset = Vec2();
	m_blurConstants.m_samples[0].m_weight = 0.25f;
	m_blurConstants.m_samples[1].m_offset = Vec2( 1.f, 0.f );
	m_blurConstants.m_samples[1].m_weight = 0.125f;
	m_blurConstants.m_samples[2].m_offset = Vec2( 0.f, -1.f );
	m_blurConstants.m_samples[2].m_weight = 0.125f;
	m_blurConstants.m_samples[3].m_offset = Vec2( -1.f, 0.f );
	m_blurConstants.m_samples[3].m_weight = 0.125f;
	m_blurConstants.m_samples[4].m_offset = Vec2( 0.f, 1.f );
	m_blurConstants.m_samples[4].m_weight = 0.125f;
	m_blurConstants.m_samples[5].m_offset = Vec2( 1.f, 1.f );
	m_blurConstants.m_samples[5].m_weight = 0.0625f;
	m_blurConstants.m_samples[6].m_offset = Vec2( -1.f, 1.f );
	m_blurConstants.m_samples[6].m_weight = 0.0625f;
	m_blurConstants.m_samples[7].m_offset = Vec2( 1.f, -1.f );
	m_blurConstants.m_samples[7].m_weight = 0.0625f;
	m_blurConstants.m_samples[8].m_offset = Vec2( 1.f, -1.f );
	m_blurConstants.m_samples[8].m_weight = 0.0625f;

	SetCustomConstantBuffer( m_blurCBO, &m_blurConstants, sizeof( BlurConstants ), k_blurConstantSlot );
	m_deviceContext->OMSetRenderTargets( 1, &m_blurUpTextures[k_blurUpTextureCount - 1]->m_renderTargetView, nullptr );

	BindTexture( m_blurDownTextures[k_blurDownTextureCount - 1], 0 );
	BindTexture( m_blurDownTextures[k_blurDownTextureCount - 1], 1 );
	BindShader( CreateShader( "BlurUp", blurUpShaderSource ) );
	DrawVertexArray( screenVerts );

	for (int i = k_blurUpTextureCount - 2; i >= 0 ; i--) {
		screenVerts.clear();
		AddVertsForAABB2D( screenVerts, AABB2( Vec2( -1.f, 1.f ), Vec2( -1.f + (float)pow( 0.5, i - 1 ), 1.f - (float)pow( 0.5, i - 1 ) ) ), Rgba8::WHITE );
		m_blurConstants.m_texelSize = Vec2( 1.f / m_blurUpTextures[i + 1]->GetDimensions().x, 1.f / m_blurUpTextures[i + 1]->GetDimensions().y );
		SetCustomConstantBuffer( m_blurCBO, &m_blurConstants, sizeof( BlurConstants ), k_blurConstantSlot );
		m_deviceContext->OMSetRenderTargets( 1, &m_blurUpTextures[i]->m_renderTargetView, nullptr );
		BindTexture( m_blurDownTextures[i] );
		DrawVertexArray( screenVerts );
	}

	// composite together
	m_deviceContext->OMSetRenderTargets( 1, &m_renderTargetView, m_depthStencilView );
	screenVerts.clear();
	AddVertsForAABB2D( screenVerts, AABB2( Vec2( -1.f, 1.f ), Vec2( 1.f, -1.f ) ), Rgba8::WHITE );
	SetBlendMode( BlendMode::ADDITIVE );
	SetRasterizerMode( RasterizerMode::SOLID_CULL_FRONT );
	BindShader( CreateShader( "BlurComposite", blurCompositeShaderSource ) );
	BindTexture( m_blurUpTextures[0] );
	DrawVertexArray( screenVerts );

	SetDefaultRenderTargets();
}

Texture* DX11Renderer::GetCurScreen() const
{
	return m_screenTexture;
}

Shader* DX11Renderer::CreateShader( char const* shaderName, VertexType type /*= VertexType::PCU */ )
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

Shader* DX11Renderer::CreateShader( char const* shaderName, char const* shaderSource, VertexType type /*= VertexType::PCU */ )
{
	std::map<std::string, Shader*>::iterator found = m_loadedShaders.find( shaderName );
	if (found != m_loadedShaders.end()) {
		return found->second;
	}
	ShaderConfig sConfig;
	sConfig.m_name = std::string( shaderName );
	Shader* shader = new Shader( sConfig );

	// Compile vertex shader
	std::vector<unsigned char> vertexShaderByteCode;
	CompileShaderToByteCode( vertexShaderByteCode, "VertexShader", shaderSource, sConfig.m_vertexEntryPoint.c_str(), "vs_5_0" );

	// Create vertex shader
	HRESULT hr = m_device->CreateVertexShader(
		vertexShaderByteCode.data(),
		vertexShaderByteCode.size(),
		NULL, &(shader->m_vertexShader)
	);
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create vertex shader." );
	}


	// Compile pixel shader
	std::vector<unsigned char> pixelShaderByteCode;
	CompileShaderToByteCode( pixelShaderByteCode, "PixelShader", shaderSource, sConfig.m_pixelEntryPoint.c_str(), "ps_5_0" );

	// Create pixel shader
	hr = m_device->CreatePixelShader(
		pixelShaderByteCode.data(),
		pixelShaderByteCode.size(),
		NULL, &(shader->m_pixelShader)
	);
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create pixel shader." );
	}

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[6];
	UINT numElements = 0;
	if (type == VertexType::PCU) {
		inputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[1] = { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		numElements = 3;
	}
	else if (type == VertexType::PCU_SEPARATED) {
		inputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[1] = { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		numElements = 3;
	}
	else if (type == VertexType::PCUTBN) {
		inputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[1] = { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[5] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		numElements = 6;
	}
	else if (type == VertexType::PCUN_SEPARATED) {
		inputElementDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[1] = { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		inputElementDesc[3] = { "NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		numElements = 4;
	}

	hr = m_device->CreateInputLayout(
		inputElementDesc, numElements,
		vertexShaderByteCode.data(),
		vertexShaderByteCode.size(),
		&(shader->m_inputLayoutForVertex)
	);
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create vertex layout." );
	}

	m_loadedShaders[shaderName] = shader;
	return shader;

}

bool DX11Renderer::CompileShaderToByteCode( std::vector<unsigned char>& out_byteCode, char const* name, char const* source, char const* entryPoint, char const* target )
{
	// shader preparation
	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined ENGINE_DEBUG_RENDER
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;

	// Compile shader
	HRESULT hr = D3DCompile(
		source, strlen( source ),
		name, nullptr, nullptr,
		entryPoint, target, shaderFlags, 0,
		&shaderBlob, &errorBlob
	);
	if (SUCCEEDED( hr )) {
		out_byteCode.resize( shaderBlob->GetBufferSize() );
		memcpy(
			out_byteCode.data(),
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize()
		);
		shaderBlob->Release();
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

void DX11Renderer::BindShader( Shader* shader )
{
	if (shader) {
		m_deviceContext->IASetInputLayout( shader->m_inputLayoutForVertex );
		m_deviceContext->VSSetShader( shader->m_vertexShader, nullptr, 0 );
		m_deviceContext->PSSetShader( shader->m_pixelShader, nullptr, 0 );
		m_currentShader = shader;
	}
	else {
		m_deviceContext->IASetInputLayout( m_defaultShader->m_inputLayoutForVertex );
		m_deviceContext->VSSetShader( m_defaultShader->m_vertexShader, nullptr, 0 );
		m_deviceContext->PSSetShader( m_defaultShader->m_pixelShader, nullptr, 0 );
		m_currentShader = nullptr;
	}
}

VertexBuffer* DX11Renderer::CreateVertexBuffer( size_t const size, unsigned int stride /*= sizeof( Vertex_PCU ) */ )
{
	VertexBuffer* vertexBuffer = new VertexBuffer( size, stride );
	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer( &bufferDesc, nullptr, &(vertexBuffer->m_vertexBuffer) );
	if (!SUCCEEDED( hr )) {
		hr = m_device->GetDeviceRemovedReason();
		ERROR_AND_DIE( Stringf("Could not create vertex buffer. Error Code: %d", hr) );
	}
	return vertexBuffer;
}

void DX11Renderer::BindVertexBuffer( VertexBuffer* vbo )
{
	UINT stride = vbo->GetStride();
	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers( 0, 1, &(vbo->m_vertexBuffer), &stride, &startOffset );
	if (vbo->m_isLinePrimitive) {
		m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
	}
	else {
		m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}
}

void DX11Renderer::CopyCPUToGPU( void const* data, size_t size, VertexBuffer*& vbo, size_t vboOffset )
{
	// if vbo is too small, make it larger
	if (vbo->m_size < size) {
		unsigned int stride = vbo->GetStride();
		delete vbo;
		vbo = CreateVertexBuffer( size, stride );
	}
	else if (vboOffset == 0) {
		vbo->m_vertexCount = (int)(size / vbo->GetStride());
	}
	// Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( vbo->m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	void* startAddr = (void*)(vboOffset + (size_t)resource.pData);
	memcpy( startAddr, data, size );
	m_deviceContext->Unmap( vbo->m_vertexBuffer, 0 );
}

void DX11Renderer::CopyCPUToGPU( void const* data, size_t size, ConstantBuffer*& cbo )
{
	// Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, data, size );
	m_deviceContext->Unmap( cbo->m_buffer, 0 );
}

void DX11Renderer::CopyCPUToGPU( void* data, size_t size, VertexBuffer*& vbo, size_t vboOffset /*= 0 */ )
{
	// if vbo is too small, make it larger
	if (vbo->m_size < size) {
		unsigned int stride = vbo->GetStride();
		delete vbo;
		vbo = CreateVertexBuffer( size, stride );
	}
	else if (vboOffset == 0) {
		vbo->m_vertexCount = (int)(size / vbo->GetStride());
	}
	// Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( vbo->m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	void* startAddr = (void*)(vboOffset + (size_t)resource.pData);
	memcpy( startAddr, data, size );
	m_deviceContext->Unmap( vbo->m_vertexBuffer, 0 );
}

IndexBuffer* DX11Renderer::CreateIndexBuffer( size_t const size )
{
	IndexBuffer* indexBuffer = new IndexBuffer( size );
	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer( &bufferDesc, nullptr, &(indexBuffer->m_indexBuffer) );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create vertex buffer." );
	}
	return indexBuffer;
}

void DX11Renderer::CopyCPUToGPU( void const* data, size_t size, IndexBuffer*& ibo )
{
	// if vbo is too small, make it larger
	if (ibo->m_size < size) {
		delete ibo;
		ibo = CreateIndexBuffer( size );
	}
	// Copy vertices
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map( ibo->m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
	memcpy( resource.pData, data, size );
	m_deviceContext->Unmap( ibo->m_indexBuffer, 0 );
}

void DX11Renderer::BindIndexBuffer( IndexBuffer* ibo )
{
	m_deviceContext->IASetIndexBuffer( ibo->m_indexBuffer, DXGI_FORMAT_R32_UINT, 0 );
}

Texture* DX11Renderer::CreateOrGetTextureFromFile( char const* filePath )
{
	// See if we already have this texture previously loaded
	std::map<std::string, Texture*>::iterator/*auto*/ findTexture = m_loadedTextures.find( filePath );
	if (findTexture != m_loadedTextures.end()) {
		return findTexture->second;
	}

	// Never seen this texture before!  Let's load it.
	return CreateTextureFromFile( filePath );
}

BitmapFont* DX11Renderer::CreateOrGetBitmapFontFromFile( char const* filePathNoExtension, BitmapFontType type )
{
	/*constexpr int maxSizeOfString = 400;
//char* filePath = (char*)malloc( maxSizeOfString ); // do not forget to free
char filePath[maxSizeOfString];
strcpy_s( filePath, maxSizeOfString, filePathNoExtension );
strcat_s( filePath, maxSizeOfString, ".png" );*/
	std::string filePath( filePathNoExtension );
	if (type == BitmapFontType::FntType) {
		filePath += ".fnt";
	}
	else if(type == BitmapFontType::PNGTextureType) {
		filePath += ".png";
	}
	std::map<std::string, BitmapFont*>::iterator findFont = m_loadedFonts.find( filePath );
	if (findFont != m_loadedFonts.end()) {
		return findFont->second;
	}
	Texture* bitmapTexture = nullptr;
	BitmapFont* newBitmap;

	if (type == BitmapFontType::PNGTextureType) {
		bitmapTexture = CreateTextureFromFile( filePath.c_str() );
	}
	newBitmap = new BitmapFont( filePath.c_str(), *bitmapTexture, type, m_baseRenderer );
	
	m_loadedFonts[filePath] = newBitmap;
	return newBitmap;
}

Image* DX11Renderer::CreateImageFromFile( char const* filePath )
{
	return new Image( filePath );
}

Texture* DX11Renderer::CreateTextureFromImage( Image const* image )
{
	Texture* newTexture = new Texture();
	newTexture->m_dimensions = image->GetDimensions();
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = image->GetDimensions().x;
	textureDesc.Height = image->GetDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = image->GetRawData();
	textureData.SysMemPitch = 4 * image->GetDimensions().x;

	HRESULT hr = m_device->CreateTexture2D( &textureDesc, &textureData, &(newTexture->m_texture) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), Stringf( "CreateTextureFromImage failed for image file \"%s\"", image->GetImageFilePath().c_str() ) );

	hr = m_device->CreateShaderResourceView( newTexture->m_texture, NULL, &(newTexture->m_shaderResourceView) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), Stringf( "CreateSahderResourceView failed for image file \"%s\"", image->GetImageFilePath().c_str() ) );

	return newTexture;
}

Texture* DX11Renderer::CreateRenderTexture( IntVec2 const& dimensions, char const* name )
{
	Texture* newTexture = new Texture();
	newTexture->m_dimensions = dimensions;
	newTexture->m_name = name;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = dimensions.x;
	textureDesc.Height = dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	HRESULT hr = m_device->CreateTexture2D( &textureDesc, NULL, &(newTexture->m_texture) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), Stringf( "Create Render Texture Failed" ) );

	hr = m_device->CreateShaderResourceView( newTexture->m_texture, NULL, &(newTexture->m_shaderResourceView) );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), Stringf( "Create Shader Resource View failed" ) );

	hr = m_device->CreateRenderTargetView( newTexture->m_texture, NULL, &newTexture->m_renderTargetView );
	GUARANTEE_OR_DIE( SUCCEEDED( hr ), "Could not create render target view." );
	return newTexture;
}

void DX11Renderer::BindTexture( Texture const* texture, int slot )
{
	if (texture)
	{
		m_currentTexture = texture;
		m_deviceContext->PSSetShaderResources( slot, 1, &(m_currentTexture->m_shaderResourceView) );
	}
	else if (texture == nullptr)
	{
		m_currentTexture = m_defaultTexture;
		m_deviceContext->PSSetShaderResources( slot, 1, &(m_currentTexture->m_shaderResourceView) );
	}
}

void DX11Renderer::SetBlendMode( BlendMode blendMode )
{
	m_desiredBlendMode = blendMode;
}

void DX11Renderer::SetStatesIfChanged()
{
	if (m_blendStates[(int)m_desiredBlendMode] != m_blendState) {
		m_blendState = m_blendStates[(int)m_desiredBlendMode];
		float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState( m_blendState, blendFactor, sampleMask );
	}
	if (m_samplerState != m_samplerStates[(int)m_desiredSamplerMode]) {
		m_samplerState = m_samplerStates[(int)m_desiredSamplerMode];
		m_deviceContext->PSSetSamplers( 0, 1, &m_samplerState );
	}
	if (m_rasterizerState != m_rasterizerStates[(int)m_desiredRasterizerMode]) {
		m_rasterizerState = m_rasterizerStates[(int)m_desiredRasterizerMode];
		m_deviceContext->RSSetState( m_rasterizerState );
	}
	if (m_depthStencilState != m_depthStencilStates[(int)m_desiredDepthMode]) {
		m_depthStencilState = m_depthStencilStates[(int)m_desiredDepthMode];
		m_deviceContext->OMSetDepthStencilState( m_depthStencilState, 0 );
	}

	//if (m_shadowMode == ShadowMode::DISABLE) {

	//}
	//else {
		//m_deviceContext->OMSetRenderTargets( 1, &m_renderTargetView, m_depthStencilView );
	//}


	if (m_shadowMode != ShadowMode::DISABLE) {
		//ID3D11ShaderResourceView* const pSRV[1] = { NULL };
		//m_deviceContext->PSSetShaderResources( 1, 1, pSRV );
		m_deviceContext->PSSetSamplers( 1, 1, &m_comparisonSampler_point );
		m_deviceContext->PSSetShaderResources( 1, 1, &m_shadowResourceView );
	}
	m_deviceContext->RSSetViewports( 1, m_cameraViewport );
}

void DX11Renderer::SetSamplerMode( SamplerMode samplerMode )
{
	m_desiredSamplerMode = samplerMode;
}

void DX11Renderer::SetRasterizerMode( RasterizerMode rasterizerMode )
{
	m_desiredRasterizerMode = rasterizerMode;
}

void DX11Renderer::SetDepthMode( DepthMode depthMode )
{
	m_desiredDepthMode = depthMode;
}

void DX11Renderer::SetShadowMode( ShadowMode shadowMode )
{
	m_shadowMode = shadowMode;
}

void DX11Renderer::RenderShadowMap( std::vector<Vertex_PCUTBN> const& verts )
{
	CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCUTBN ), m_immediateVBO_PCUTBN );
	RenderShadowMap( m_immediateVBO_PCUTBN, (int)verts.size(), 0 );
}

void DX11Renderer::RenderShadowMap( VertexBuffer* vbo, int vertexCount, int vertexOffset )
{
	// Render all the objects in the scene that can cast shadows onto themselves or onto other objects.
// Only bind the ID3D11DepthStencilView for output.

	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	m_deviceContext->PSSetShaderResources( 1, 1, pSRV );

	m_deviceContext->OMSetRenderTargets( 0, nullptr, m_shadowDepthView );

	m_deviceContext->OMSetDepthStencilState( m_depthStencilStates[(int)DepthMode::ENABLED], 0 );

	// Note that starting with the second frame, the previous call will display
	// warnings in VS debug output about forcing an unbind of the pixel shader
	// resource. This warning can be safely ignored when using shadow buffers
	// as demonstrated in this sample.

	// Set rendering state.
	m_deviceContext->RSSetState( m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_FRONT] );
	m_deviceContext->RSSetViewports( 1, m_shadowViewport );

	// Each vertex is one instance of the VertexPositionTexNormColor struct.
	UINT stride = vbo->GetStride();
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers( 0, 1, &(vbo->m_vertexBuffer), &stride, &offset );

	//m_deviceContext->IASetIndexBuffer( m_immediateIBO->m_indexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	Shader* shader = CreateShader( "ShadowMap", shadowDepthMapShaderSource, VertexType::PCUTBN );

	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_deviceContext->IASetInputLayout( shader->m_inputLayoutForVertex );

	BindConstantBuffer( k_modelConstantsSlot, m_modelCBO );
	BindConstantBuffer( k_lightConstantsSlot, m_lightCBO );
	// Attach our vertex shader.
	m_deviceContext->VSSetShader( shader->m_vertexShader, nullptr, 0 );

	// In some configurations, it's possible to avoid setting a pixel shader
	// (or set PS to nullptr). Not all drivers are tolerant of this, so to be
	// safe set a minimal shader here.
	//
	// Direct3D will discard output from this shader because the render target
	// view is unbound.
	m_deviceContext->PSSetShader( nullptr, nullptr, 0 );

	// Draw the objects.
	m_deviceContext->Draw( vertexCount, vertexOffset );
	m_deviceContext->RSSetState( m_rasterizerState );
	BindShader( m_currentShader );
}

void DX11Renderer::RenderShadowMap( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset )
{
	// Render all the objects in the scene that can cast shadows onto themselves or onto other objects.
// Only bind the ID3D11DepthStencilView for output.
	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	m_deviceContext->PSSetShaderResources( 1, 1, pSRV );

	m_deviceContext->OMSetRenderTargets( 0, nullptr, m_shadowDepthView );

	m_deviceContext->OMSetDepthStencilState( m_depthStencilStates[(int)DepthMode::ENABLED], 0 );
	// Note that starting with the second frame, the previous call will display
	// warnings in VS debug output about forcing an unbind of the pixel shader
	// resource. This warning can be safely ignored when using shadow buffers
	// as demonstrated in this sample.

	// Set rendering state.
	m_deviceContext->RSSetState( m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_FRONT] );
	m_deviceContext->RSSetViewports( 1, m_shadowViewport );

	// Each vertex is one instance of the VertexPositionTexNormColor struct.
	UINT stride = vbo->GetStride();
	UINT offset = 0;
	m_deviceContext->IASetVertexBuffers( 0, 1, &(vbo->m_vertexBuffer), &stride, &offset );
	m_deviceContext->IASetIndexBuffer( ibo->m_indexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	Shader* shader = CreateShader( "ShadowMap", shadowDepthMapShaderSource, VertexType::PCUTBN );

	m_deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_deviceContext->IASetInputLayout( shader->m_inputLayoutForVertex );

	BindConstantBuffer( k_modelConstantsSlot, m_modelCBO );
	BindConstantBuffer( k_lightConstantsSlot, m_lightCBO );
	// Attach our vertex shader.
	m_deviceContext->VSSetShader( shader->m_vertexShader, nullptr, 0 );

	// In some configurations, it's possible to avoid setting a pixel shader
	// (or set PS to nullptr). Not all drivers are tolerant of this, so to be
	// safe set a minimal shader here.
	//
	// Direct3D will discard output from this shader because the render target
	// view is unbound.
	m_deviceContext->PSSetShader( nullptr, nullptr, 0 );

	// Draw the objects.
	m_deviceContext->DrawIndexed( indexCount, indexOffset, 0 );
	m_deviceContext->RSSetState( m_rasterizerState );
	BindShader( m_currentShader );
}

AABB2 DX11Renderer::GetSwapChainSize() const
{
	DXGI_SWAP_CHAIN_DESC desc;
	m_swapChain->GetDesc( &desc );
	AABB2 size( Vec2(), Vec2( (float)desc.BufferDesc.Width, (float)desc.BufferDesc.Height ) );
	return size;
}

void DX11Renderer::SetScreenRenderTargetView( Camera const& camera )
{
	UNUSED( camera );
	ID3D11RenderTargetView* renderTargetViews[] = { m_screenTexture->m_renderTargetView };
	m_deviceContext->OMSetRenderTargets( 1, renderTargetViews, nullptr );
	m_cameraViewport->Width *= 8.f;
	m_cameraViewport->Height *= 8.f;
}

void DX11Renderer::SetBasicRenderTargetView()
{
	m_deviceContext->OMSetRenderTargets( 1, &m_renderTargetView, m_depthStencilView );
}

void DX11Renderer::ResetScreenRenderTargetView()
{
	m_cameraViewport->Width /= 8.f;
	m_cameraViewport->Height /= 8.f;
}

Texture* DX11Renderer::CreateTextureFromFile( char const* filePath )
{
	Image image = Image( filePath );
	Texture* newTexture = CreateTextureFromImage( &image );
	newTexture->m_name = filePath;
	m_loadedTextures[std::string( filePath )] = newTexture;
	return newTexture;
	/*
	IntVec2 dimensions = IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load( filePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "Failed to load image \"%s\"", filePath ) );

	Texture* newTexture = CreateTextureFromData( filePath, dimensions, bytesPerTexel, texelData );

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( texelData );

	m_loadedTextures[std::string(filePath)] = newTexture;
	return newTexture;*/
}

void DX11Renderer::SetDefaultRenderTargets()
{
	ID3D11RenderTargetView* renderTargetViews[] = { m_renderTargetView, m_emissiveTexture->m_renderTargetView };
	m_deviceContext->OMSetRenderTargets( 2, renderTargetViews, m_depthStencilView );
}

ConstantBuffer* DX11Renderer::CreateConstantBuffer( size_t const size )
{
	ConstantBuffer* constantBuffer = new ConstantBuffer( size );
	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (UINT)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer( &bufferDesc, nullptr, &(constantBuffer->m_buffer) );
	if (!SUCCEEDED( hr )) {
		ERROR_AND_DIE( "Could not create constant buffer." );
	}
	return constantBuffer;
}

void DX11Renderer::BindConstantBuffer( int slot, ConstantBuffer* cbo )
{
	m_deviceContext->VSSetConstantBuffers( slot, 1, &(cbo->m_buffer) );
	m_deviceContext->PSSetConstantBuffers( slot, 1, &(cbo->m_buffer) );
}


#endif