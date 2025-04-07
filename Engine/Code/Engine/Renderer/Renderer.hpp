#pragma once

#include <string>
#include <map>
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/EngineCommon.hpp"
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
class DX11Renderer;
class DX12Renderer;

// Renderer class is responsible for drawing
// Renderer provides interfaces like DrawVertexArray to draw on screen or BeginCamera to set Camera position
class Renderer {
public:
	Renderer( RendererConfig const& rConfig );
	~Renderer();
	void StartUp();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void ClearScreen ( Rgba8 const& clearColor, Rgba8 const& emissiveColor = Rgba8::BLACK );
	void BeginCamera ( Camera const& camera );
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
	void DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset=0 );
	void DrawVertexIndexed( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0 );
	void DrawVertexBuffers( int bufferCount, VertexBuffer** vbo, int vertexCount, int vertexOffset );
	void DrawVertexBuffersIndexed( int bufferCount, VertexBuffer** vbo, IndexBuffer* ibo, int indexCount, int indexOffset = 0 );
	void RenderEmissive();
	Texture* GetCurScreenAsTexture();
	
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

	void SetBasicRenderTargetView();
	void ResetScreenRenderTargetView();
	void SetScreenRenderTargetView( Camera const& camera );
private:
	//Texture* CreateTextureFromData( char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData );
protected:
	RendererConfig m_config;
public:
	DX11Renderer* m_dx11Renderer = nullptr;
	DX12Renderer* m_dx12Renderer = nullptr;
};