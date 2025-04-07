#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

// D3D11 includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <d3d12.h>

VertexBuffer::VertexBuffer( size_t size, unsigned int stride )
	:m_size(size)
	,m_stride(stride)
	,m_vertexCount((int)size / stride)
{

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_vertexBufferView = new D3D12_VERTEX_BUFFER_VIEW();
#endif
}

unsigned int VertexBuffer::GetStride() const
{
	return m_stride;
}

size_t VertexBuffer::GetSize() const
{
	return m_size;
}

int VertexBuffer::GetVertexCount() const
{
	return m_vertexCount;
}

VertexBuffer::~VertexBuffer()
{

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_vertexBuffer );
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_dx12VertexBuffer );
	delete m_vertexBufferView;
#endif
}

void VertexBuffer::SetAsLinePrimitive( bool value )
{
	m_isLinePrimitive = value;
}

