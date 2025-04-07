#pragma once
#include "Engine/Core/EngineCommon.hpp"

#ifdef ENGINE_DX12_RENDERER_INTERFACE

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

struct IDXGISwapChain3;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12CommandQueue;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12PipelineState;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;
struct D3D12_VIEWPORT;
struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;

typedef void* HANDLE;

constexpr unsigned int DX12_FrameCount = 2;
constexpr unsigned int DX12_NumOfConstantBuffersPerDraw = 14;
constexpr unsigned int DX12_NumOfDrawCallPerFrame = 1000;
constexpr unsigned int DX12_MaxNumOfTextures = 10000;

class DX12Renderer {
public:
	DX12Renderer( Renderer* baseRenderer, RendererConfig config );
	~DX12Renderer();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen( Rgba8 const& clearColor );
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
	void DrawVertexIndexed( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0 );

	// shaders
	Shader* CreateShader( char const* shaderName, VertexType type = VertexType::PCU );
	Shader* CreateShader( char const* shaderName, char const* shaderSource, VertexType type = VertexType::PCU );
	bool CompileShaderToByteCode( ID3DBlob** out_shaderByteCode, char const* name, char const* source, char const* entryPoint, char const* target );
	void BindShader( Shader* shader );

	// buffers
	VertexBuffer* CreateVertexBuffer( size_t const size, unsigned int stride = sizeof( Vertex_PCU ) );
	void CopyCPUToGPU( void const* data, size_t size, VertexBuffer*& vbo );
	void BindVertexBuffer( VertexBuffer* vbo );
	ConstantBuffer* CreateConstantBuffer( size_t const size );
	void CopyCPUToGPU( void const* data, size_t size, ConstantBuffer*& cbo );
	void BindConstantBuffer( int slot, ConstantBuffer* cbo );
	IndexBuffer* CreateIndexBuffer( size_t const size );
	void CopyCPUToGPU( void const* data, size_t size, IndexBuffer*& ibo );
	void BindIndexBuffer( IndexBuffer* ibo );

	// textures
	Texture* CreateOrGetTextureFromFile( char const* filePath );
	BitmapFont* CreateOrGetBitmapFontFromFile( char const* filePathNoExtension );
	Image* CreateImageFromFile( char const* filePath );
	Texture* CreateTextureFromImage( Image const* image );
	void BindTexture( Texture const* texture, int slot=0 );

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
private:
	Texture* CreateTextureFromFile( char const* filePath );
	void WaitForPreviousFrame();

protected:
	bool m_isFirstFrame = true;
	Renderer* m_baseRenderer = nullptr;
	RendererConfig m_config;

	// Pipeline objects.
	D3D12_VIEWPORT* m_cameraViewport;
	//D3D12_RECT m_scissorRect;
	IDXGISwapChain3* m_swapChain;
	ID3D12Device* m_device;
	ID3D12Resource* m_renderTargets[DX12_FrameCount];
	ID3D12Resource* m_depthStencilBuffer;
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12CommandQueue* m_commandQueue;
	ID3D12RootSignature* m_rootSignature;
	ID3D12DescriptorHeap* m_rtvHeap;
	ID3D12DescriptorHeap* m_constantsDescHeap;
	ID3D12DescriptorHeap* m_dsDescHeap;
	ConstantBuffer* m_constantBuffers[DX12_NumOfConstantBuffersPerDraw * DX12_NumOfDrawCallPerFrame];
	int m_curDrawCallIndex = 0;
	CameraConstants m_curCameraConstants;
	ID3D12PipelineState* m_curPipelineState;
	ID3D12GraphicsCommandList* m_commandList;

	std::vector<VertexBuffer*> m_tempVertexBuffers;
	std::vector<IndexBuffer*> m_tempIndexBuffers;
	unsigned int m_rtvDescriptorSize;
	unsigned int m_scuDescriptorSize;

	// App resources.
	//ID3D12Resource* m_vertexBuffer;
	//D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// shaders
	Shader* m_currentShader = nullptr;
	Shader* m_desiredShader = nullptr;
	Shader* m_defaultShader = nullptr;
	std::map<std::string, Shader*> m_loadedShaders;

	Texture const* m_defaultTexture = nullptr;
	Texture const* m_currentTexture = nullptr;
	std::map<std::string, Texture*> m_loadedTextures;
	std::map<std::string, BitmapFont*> m_loadedFonts;
	ID3D12Resource* m_textureBufferUploadHeap;

	// Synchronization objects.
	unsigned int m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence;
	unsigned long long m_fenceValue;

	// pipeline states
	std::map<int, ID3D12PipelineState*> m_pipelineStatesCollection;
	BlendMode m_currentBlendMode = BlendMode::COUNT;
	BlendMode m_desiredBlendMode = BlendMode::ALPHA;

	SamplerMode m_currentSamplerMode = SamplerMode::COUNT;
	SamplerMode m_desiredSamplerMode = SamplerMode::POINT_CLAMP;

	RasterizerMode m_currentRasterizerMode = RasterizerMode::COUNT;
	RasterizerMode m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;

	DepthMode m_desiredDepthMode = DepthMode::ENABLED;
	DepthMode m_currentDepthMode = DepthMode::COUNT;
};

#endif