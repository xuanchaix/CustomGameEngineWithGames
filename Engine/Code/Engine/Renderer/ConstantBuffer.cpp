#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

// D3D11 includes
#include <d3d11.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>

ConstantBuffer::ConstantBuffer( size_t size )
	:m_size(size)
{
#ifdef ENGINE_DX12_RENDERER_INTERFACE
	m_constantBufferView = new D3D12_CONSTANT_BUFFER_VIEW_DESC();
#endif
}

ConstantBuffer::~ConstantBuffer()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_buffer );
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_dx12ConstantBuffer );
	delete m_constantBufferView;
#endif
}

