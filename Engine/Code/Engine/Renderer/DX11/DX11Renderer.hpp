#pragma once
#include "Engine/Core/EngineCommon.hpp"

#ifdef ENGINE_DX11_RENDERER_INTERFACE

#include <string>
#include <map>
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RendererUtils.hpp"

struct Rgba8;
class Camera;
struct Vertex_PCU;
class Window;
class Image;
class Texture;
class BitmapFont;
class Shader;
class VertexBuffer;
class ConstantBuffer;
class IndexBuffer;
class Renderer;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;
struct ID3D11DepthStencilView;
struct ID3D11DepthStencilState;
struct D3D11_VIEWPORT;

constexpr int k_blurDownTextureCount = 4;
constexpr int k_blurUpTextureCount = k_blurDownTextureCount;

/// <summary>
/// The DX11 renderer class is hidden in engine and provide DX11 methods for the Renderer
/// Only the renderer owns one instance of DX11Renderer
/// </summary>
class DX11Renderer {
public:
	DX11Renderer( Renderer* baseRenderer, RendererConfig config );
	~DX11Renderer();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen( Rgba8 const& clearColor, Rgba8 const& emissiveColor = Rgba8::BLACK );
	void BeginCamera( Camera const& camera );
	void EndCamera( Camera const& camera );
	void SetModelConstants( Mat44 const& modelMatrix = Mat44(), Rgba8 const& modelColor = Rgba8::WHITE );
	void SetDirectionalLightConstants( DirectionalLightConstants const& dlc );
	void SetLightConstants( Vec3 const& lightPosition, float ambient, Mat44 const& lightViewMatrix, Mat44 const& lightProjectionMatrix );
	void SetCustomConstantBuffer( ConstantBuffer*& cbo, void* data, size_t size, int slot );

	void DrawVertexArray( int numVertexes, const Vertex_PCU* vertexed );
	void DrawVertexArray( std::vector<Vertex_PCU> const& verts );
	void DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts );
	void DrawVertexArray( std::vector<Vertex_PCU> const& verts, std::vector<unsigned int> const& indexes );
	void DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts, std::vector<unsigned int> const& indexes );
	void DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset = 0 );
	void DrawVertexBuffers( int bufferCount, VertexBuffer** vbo, int vertexCount, int vertexOffset );
	void DrawVertexBuffersIndexed( int bufferCount, VertexBuffer** vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0 );
	void DrawVertexIndexed( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0 );
	void RenderEmissive();
	Texture* GetCurScreen() const;

	// shaders
	Shader* CreateShader( char const* shaderName, VertexType type = VertexType::PCU );
	Shader* CreateShader( char const* shaderName, char const* shaderSource, VertexType type = VertexType::PCU );
	bool CompileShaderToByteCode( std::vector<unsigned char>& out_byteCode, char const* name, char const* source, char const* entryPoint, char const* target );
	void BindShader( Shader* shader );

	// buffers
	VertexBuffer* CreateVertexBuffer( size_t const size, unsigned int stride = sizeof( Vertex_PCU ) );
	void CopyCPUToGPU( void const* data, size_t size, VertexBuffer*& vbo, size_t vboOffset = 0 );
	void CopyCPUToGPU( void* data, size_t size, VertexBuffer*& vbo, size_t vboOffset = 0 );
	void BindVertexBuffer( VertexBuffer* vbo );
	ConstantBuffer* CreateConstantBuffer( size_t const size );
	void CopyCPUToGPU( void const* data, size_t size, ConstantBuffer*& cbo );
	void BindConstantBuffer( int slot, ConstantBuffer* cbo );
	IndexBuffer* CreateIndexBuffer( size_t const size );
	void CopyCPUToGPU( void const* data, size_t size, IndexBuffer*& ibo );
	void BindIndexBuffer( IndexBuffer* ibo );

	// textures
	Texture* CreateOrGetTextureFromFile( char const* filePath );
	BitmapFont* CreateOrGetBitmapFontFromFile( char const* filePathNoExtension, BitmapFontType type = BitmapFontType::PNGTextureType );
	Image* CreateImageFromFile( char const* filePath );
	Texture* CreateTextureFromImage( Image const* image );
	Texture* CreateRenderTexture( IntVec2 const& dimensions, char const* name );
	void BindTexture( Texture const* texture, int slot = 0 );

	// blend mode
	void SetBlendMode( BlendMode blendMode );
	void SetStatesIfChanged();

	// sample mode
	void SetSamplerMode( SamplerMode samplerMode );

	// rasterizer mode
	void SetRasterizerMode( RasterizerMode rasterizerMode );

	// Depth Mode
	void SetDepthMode( DepthMode depthMode );

	void SetShadowMode( ShadowMode shadowMode );

	// shadow map
	void RenderShadowMap( std::vector<Vertex_PCUTBN> const& verts );
	void RenderShadowMap( VertexBuffer* vbo, int vertexCount, int vertexOffset );
	void RenderShadowMap( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset );

	AABB2 GetSwapChainSize() const;
	void SetScreenRenderTargetView( Camera const& camera );
	void SetBasicRenderTargetView();
	void ResetScreenRenderTargetView();
private:
	Texture* CreateTextureFromFile( char const* filePath );
	void SetDefaultRenderTargets();
public:
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
protected:
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;

#if defined ENGINE_DEBUG_RENDER
	void* m_dxgiDebug = nullptr;
	void* m_dxgiDebugModule = nullptr;
#endif

	std::map<std::string, Shader*> m_loadedShaders;
	Shader* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;

	VertexBuffer* m_immediateVBO_PCU = nullptr;
	VertexBuffer* m_immediateVBO_PCUTBN = nullptr;
	VertexBuffer* m_fullScreenQuadVBO_PCU = nullptr;
	IndexBuffer* m_immediateIBO = nullptr;
	ConstantBuffer* m_directionalLightCBO = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;
	ConstantBuffer* m_blurCBO = nullptr;

	ID3D11BlendState* m_blendState = nullptr;
	BlendMode m_desiredBlendMode = BlendMode::ALPHA;
	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = {};

	Texture const* m_defaultTexture = nullptr;
	Texture const* m_currentTexture = nullptr;
	std::map<std::string, Texture*> m_loadedTextures;
	std::map<std::string, BitmapFont*> m_loadedFonts;

	ID3D11SamplerState* m_samplerState = nullptr;
	SamplerMode m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};

	ID3D11RasterizerState* m_rasterizerState = nullptr;
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};
	RasterizerMode m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;

	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;
	DepthMode m_desiredDepthMode = DepthMode::ENABLED;

	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilState* m_depthStencilStates[(int)DepthMode::COUNT] = {};

	ShadowMode m_shadowMode = ShadowMode::DISABLE;
	ID3D11Texture2D* m_shadowMapTexture = nullptr;
	ID3D11DepthStencilView* m_shadowDepthView = nullptr;
	ID3D11ShaderResourceView* m_shadowResourceView = nullptr;
	ID3D11SamplerState* m_comparisonSampler_point = nullptr;
	D3D11_VIEWPORT* m_shadowViewport = nullptr;
	D3D11_VIEWPORT* m_cameraViewport = nullptr;

	Texture* m_emissiveTexture = nullptr;
	Texture* m_blurDownTextures[k_blurDownTextureCount];
	Texture* m_blurUpTextures[k_blurUpTextureCount];
	BlurConstants m_blurConstants;

	Texture* m_screenTexture = nullptr;

	Renderer* m_baseRenderer = nullptr;
	RendererConfig m_config;

};

#endif