#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Renderer.hpp"

// D3D11 includes
#include <d3d11.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>


Texture::~Texture()
{
#ifdef ENGINE_DX11_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_texture );
	DX_SAFE_RELEASE( m_shaderResourceView );
	DX_SAFE_RELEASE( m_renderTargetView );
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_dx12Texture );
	DX_SAFE_RELEASE( m_textureBufferUploadHeap );
#endif
}
