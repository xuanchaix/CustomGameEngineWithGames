#pragma once
#include <string>
#include "Engine/Core/EngineCommon.hpp"

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct D3D12_INPUT_LAYOUT_DESC;
struct D3D12_INPUT_ELEMENT_DESC;

struct ShaderConfig {
	std::string m_name;
	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";
};

class Shader {
	friend class DX11Renderer;
	friend class DX12Renderer;
public:
	Shader( ShaderConfig const& config );
	Shader( Shader const& copy ) = delete;
	~Shader();

	std::string const& GetName() const;

protected:
	ShaderConfig m_config;

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	// dx11 stuffs
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayoutForVertex = nullptr;
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	// dx12 stuffs
	int m_shaderIndex = -1;
	ID3DBlob* m_vertexShader = nullptr;
	ID3DBlob* m_pixelShader = nullptr;
	D3D12_INPUT_LAYOUT_DESC* m_inputLayoutForVertex = nullptr;
#endif
};