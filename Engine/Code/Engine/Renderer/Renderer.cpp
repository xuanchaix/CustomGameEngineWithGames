#include "Engine/Renderer/Renderer.hpp"
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
#include "Engine/Renderer/DX11/DX11Renderer.hpp"
#include "Engine/Renderer/DX12/DX12Renderer.hpp"


Renderer::Renderer( RendererConfig const& rConfig )
{
	m_config = rConfig;
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer = new DX11Renderer( this, rConfig );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer = new DX12Renderer( this, rConfig );
#endif
}

Renderer::~Renderer()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	delete m_dx11Renderer;
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	delete m_dx12Renderer;
#endif
}

void Renderer::StartUp()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->StartUp();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->StartUp();
#endif
}

void Renderer::BeginFrame()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BeginFrame();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BeginFrame();
#endif
}

void Renderer::EndFrame()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->EndFrame();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->EndFrame();
#endif
}

void Renderer::Shutdown()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->Shutdown();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->Shutdown();
#endif
}

void Renderer::ClearScreen( Rgba8 const& clearColor, Rgba8 const& emissiveColor )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->ClearScreen( clearColor, emissiveColor );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	UNUSED( emissiveColor );
	m_dx12Renderer->ClearScreen( clearColor );
#endif
}

void Renderer::BeginCamera( Camera const& camera )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BeginCamera( camera );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BeginCamera( camera );
#endif
}

void Renderer::EndCamera(Camera const& camera)
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->EndCamera( camera );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->EndCamera( camera );
#endif
}

void Renderer::SetModelConstants( Mat44 const& modelMatrix /*= Mat44()*/, Rgba8 const& modelColor /*= Rgba8::WHITE */ )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetModelConstants( modelMatrix, modelColor );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetModelConstants( modelMatrix, modelColor );
#endif
}

void Renderer::SetDirectionalLightConstants( DirectionalLightConstants const& dlc )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetDirectionalLightConstants( dlc );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetDirectionalLightConstants( dlc );
#endif
}

void Renderer::SetLightConstants( Vec3 const& lightPosition, float ambient, Mat44 const& lightViewMatrix, Mat44 const& lightProjectionMatrix )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetLightConstants( lightPosition, ambient, lightViewMatrix, lightProjectionMatrix );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetLightConstants( lightPosition, ambient, lightViewMatrix, lightProjectionMatrix );
#endif
}

void Renderer::SetCustomConstantBuffer( ConstantBuffer*& cbo, void* data, size_t size, int slot )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetCustomConstantBuffer( cbo, data, size, slot );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetCustomConstantBuffer( cbo, data, size, slot );
#endif
}

void Renderer::DrawVertexArray( int numVertexes, const Vertex_PCU* vertexed )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexArray( numVertexes, vertexed );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexArray( numVertexes, vertexed );
#endif
}

void Renderer::DrawVertexArray( std::vector<Vertex_PCU> const& verts )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexArray( verts );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexArray( verts );
#endif
}

void Renderer::DrawVertexArray( std::vector<Vertex_PCU> const& verts, std::vector<unsigned int> const& indexes )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexArray( verts, indexes );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexArray( verts, indexes );
#endif
}

void Renderer::DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexArray( verts );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexArray( verts );
#endif
}

void Renderer::DrawVertexArray( std::vector<Vertex_PCUTBN> const& verts, std::vector<unsigned int> const& indexes )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexArray( verts, indexes );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexArray( verts, indexes );
#endif
}

void Renderer::DrawVertexBuffer( VertexBuffer* vbo, int vertexCount, int vertexOffset/*=0 */ )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexBuffer( vbo, vertexCount, vertexOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexBuffer( vbo, vertexCount, vertexOffset );
#endif
}

void Renderer::DrawVertexIndexed( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexIndexed( vbo, ibo, indexCount, indexOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->DrawVertexIndexed( vbo, ibo, indexCount, indexOffset );
#endif
}

void Renderer::DrawVertexBuffers( int bufferCount, VertexBuffer** vbo, int vertexCount, int vertexOffset )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexBuffers( bufferCount, vbo, vertexCount, vertexOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	UNUSED( bufferCount ); UNUSED( vbo ); UNUSED( vertexOffset ); UNUSED( vertexCount );
	ERROR_AND_DIE( "Currently not support draw multiple buffer in dx12" );
#endif
}

void Renderer::DrawVertexBuffersIndexed( int bufferCount, VertexBuffer** vbo, IndexBuffer* ibo, int indexCount, int indexOffset /*= 0 */ )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->DrawVertexBuffersIndexed( bufferCount, vbo, ibo, indexCount, indexOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	UNUSED( bufferCount ); UNUSED( vbo ); UNUSED( indexCount ); UNUSED( indexOffset ); UNUSED( ibo );
	ERROR_AND_DIE( "Currently not support draw multiple buffer in dx12" );
#endif
}

void Renderer::RenderEmissive()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->RenderEmissive();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	ERROR_AND_DIE( "Currently not support render emissive in dx12" );
#endif

}

Texture* Renderer::GetCurScreenAsTexture()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->GetCurScreen();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	ERROR_AND_DIE( "Currently not this function in dx12" );
#endif

}

Shader* Renderer::CreateShader( char const* shaderName, char const* shaderSource, VertexType type )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateShader( shaderName, shaderSource, type );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateShader( shaderName, shaderSource, type );
#endif
}

Shader* Renderer::CreateShader( char const* shaderName, VertexType type )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateShader( shaderName, type );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateShader( shaderName, type );
#endif
}

bool Renderer::CompileShaderToByteCode( std::vector<unsigned char>& out_byteCode, char const* name, char const* source, 
										char const* entryPoint, char const* target )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CompileShaderToByteCode( out_byteCode, name, source, entryPoint, target );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	UNUSED( out_byteCode ); UNUSED( name ); UNUSED( source ); UNUSED( entryPoint ); UNUSED( target );
	ERROR_AND_DIE( "Do not call this function from renderer in dx12 renderer mode!" );
//	return false;
	//return m_dx12Renderer->CompileShaderToByteCode( out_byteCode, name, source, entryPoint, target );
#endif
}

void Renderer::BindShader( Shader* shader )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BindShader( shader );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BindShader( shader );
#endif
}

VertexBuffer* Renderer::CreateVertexBuffer( size_t const size, unsigned int stride )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateVertexBuffer( size, stride );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateVertexBuffer( size, stride );
#endif
}

void Renderer::CopyCPUToGPU( void const* data, size_t size, VertexBuffer*& vbo, size_t vboOffset )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->CopyCPUToGPU( data, size, vbo, vboOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->CopyCPUToGPU( data, size, vbo );
#endif
}

void Renderer::CopyCPUToGPU( void* data, size_t size, VertexBuffer*& vbo, size_t vboOffset /*= 0 */ )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->CopyCPUToGPU( data, size, vbo, vboOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	//m_dx12Renderer->CopyCPUToGPU( data, size, vbo );
#endif
}

void Renderer::BindVertexBuffer( VertexBuffer* vbo )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BindVertexBuffer( vbo );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BindVertexBuffer( vbo );
#endif
}

IndexBuffer* Renderer::CreateIndexBuffer( size_t const size )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateIndexBuffer( size );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateIndexBuffer( size );
#endif
}

void Renderer::CopyCPUToGPU( void const* data, size_t size, IndexBuffer*& ibo )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->CopyCPUToGPU( data, size, ibo );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->CopyCPUToGPU( data, size, ibo );
#endif
}

void Renderer::BindIndexBuffer( IndexBuffer* ibo )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BindIndexBuffer( ibo );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BindIndexBuffer( ibo );
#endif
}

ConstantBuffer* Renderer::CreateConstantBuffer( size_t const size )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateConstantBuffer( size );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateConstantBuffer( size );
#endif
}

void Renderer::CopyCPUToGPU( void const* data, size_t size, ConstantBuffer*& cbo )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->CopyCPUToGPU( data, size, cbo );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->CopyCPUToGPU( data, size, cbo );
#endif
}

void Renderer::BindConstantBuffer( int slot, ConstantBuffer* cbo )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BindConstantBuffer( slot, cbo );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BindConstantBuffer( slot, cbo );
#endif
}


Texture* Renderer::CreateOrGetTextureFromFile( char const* filePath )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateOrGetTextureFromFile( filePath );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateOrGetTextureFromFile( filePath );
#endif
}

Image* Renderer::CreateImageFromFile( char const* filePath )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateImageFromFile( filePath );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateImageFromFile( filePath );
#endif
}

Texture* Renderer::CreateTextureFromImage( Image const* image )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateTextureFromImage( image );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateTextureFromImage( image );
#endif
}

Texture* Renderer::CreateRenderTexture( IntVec2 const& dimensions, char const* name )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateRenderTexture( dimensions, name );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	UNUSED( dimensions ); UNUSED( name );
	return nullptr;
	//return m_dx12Renderer->CreateTextureFromImage( image );
#endif
}

BitmapFont* Renderer::CreateOrGetBitmapFontFromFile( char const* filePathNoExtension, BitmapFontType type )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->CreateOrGetBitmapFontFromFile( filePathNoExtension, type );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->CreateOrGetBitmapFontFromFile( filePathNoExtension );
#endif
}

void Renderer::RenderShadowMap( VertexBuffer* vbo, int vertexCount, int vertexOffset )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->RenderShadowMap( vbo, vertexCount, vertexOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->RenderShadowMap( vbo, vertexCount, vertexOffset );
#endif
}

void Renderer::RenderShadowMap( VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int indexOffset )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->RenderShadowMap( vbo, ibo, indexCount, indexOffset );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->RenderShadowMap( vbo, ibo, indexCount, indexOffset );
#endif
}

void Renderer::RenderShadowMap( std::vector<Vertex_PCUTBN> const& verts )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->RenderShadowMap( verts );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->RenderShadowMap( verts );
#endif
}

AABB2 Renderer::GetSwapChainSize() const
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->GetSwapChainSize();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	return m_dx12Renderer->GetSwapChainSize();
#endif
}

void Renderer::SetBasicRenderTargetView()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->SetBasicRenderTargetView();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
#endif
}

void Renderer::ResetScreenRenderTargetView()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->ResetScreenRenderTargetView();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
#endif
}

void Renderer::SetScreenRenderTargetView(Camera const& camera)
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	return m_dx11Renderer->SetScreenRenderTargetView(camera);
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
#endif
}

/*
Texture* Renderer::CreateTextureFromData( char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData )
{
	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "CreateTextureFromData failed for \"%s\" - texelData was null!", name ) );
	GUARANTEE_OR_DIE( bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf( "CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel ) );
	GUARANTEE_OR_DIE( dimensions.x > 0 && dimensions.y > 0, Stringf( "CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y ) );

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;


	return newTexture;
}
*/

void Renderer::BindTexture( Texture const* texture, int slot )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->BindTexture( texture, slot );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->BindTexture( texture, slot );
#endif
}

void Renderer::SetBlendMode( BlendMode blendMode )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetBlendMode( blendMode );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetBlendMode( blendMode );
#endif
}

void Renderer::SetStatesIfChanged()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetStatesIfChanged();
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetStatesIfChanged();
#endif
}

void Renderer::SetSamplerMode( SamplerMode samplerMode )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetSamplerMode( samplerMode );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetSamplerMode( samplerMode );
#endif
}

void Renderer::SetRasterizerMode( RasterizerMode rasterizerMode )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetRasterizerMode( rasterizerMode );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetRasterizerMode( rasterizerMode );
#endif
}

void Renderer::SetDepthMode( DepthMode depthMode )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetDepthMode( depthMode );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetDepthMode( depthMode );
#endif
}

void Renderer::SetShadowMode( ShadowMode shadowMode )
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	m_dx11Renderer->SetShadowMode( shadowMode );
#endif
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_dx12Renderer->SetShadowMode( shadowMode );
#endif
}

