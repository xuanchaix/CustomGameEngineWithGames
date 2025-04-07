#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

// D3D11 includes
#include <d3d11.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>

IndexBuffer::IndexBuffer( size_t size )
	:m_size( size )
{
	m_indexCount = int( size / sizeof( int ) );
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_indexBufferView = new D3D12_INDEX_BUFFER_VIEW();
#endif
}

size_t IndexBuffer::GetSize() const
{
	return m_size;
}

int IndexBuffer::GetIndexCount() const
{
	return m_indexCount;
}

IndexBuffer::~IndexBuffer()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_indexBuffer );
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_dx12IndexBuffer );
	delete m_indexBufferView;
#endif
}
