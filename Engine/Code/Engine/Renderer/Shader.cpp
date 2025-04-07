#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Renderer.hpp"

// D3D11 includes
#include <d3d11.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi.h>

Shader::Shader( ShaderConfig const& config )
	:m_config(config)
{

}

Shader::~Shader()
{
	//DX_SAFE_RELEASE( m_vertexBuffer );

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_vertexShader );
	DX_SAFE_RELEASE( m_pixelShader );
	DX_SAFE_RELEASE( m_inputLayoutForVertex );
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	DX_SAFE_RELEASE( m_vertexShader );
	DX_SAFE_RELEASE( m_pixelShader );
	delete m_inputLayoutForVertex;
#endif
}

std::string const& Shader::GetName() const
{
	return m_config.m_name;
}

