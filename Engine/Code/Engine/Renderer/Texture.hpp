#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include "Engine/Core/EngineCommon.hpp"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D12Resource;
struct ID3D11RenderTargetView;

class Texture
{
	friend class DX11Renderer; // Only the Renderer can create new Texture objects!
	friend class DX12Renderer;
private:
	Texture(){}; // can't instantiate directly; must ask Renderer to do it for you
	Texture( Texture const& copy ) = delete; // No copying allowed!  This represents GPU memory.
	~Texture();

public:
	IntVec2	GetDimensions() const { return m_dimensions; }
	std::string const& GetImageFilePath() const { return m_name; }

protected:
	std::string	m_name;
	IntVec2	m_dimensions;

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_shaderResourceView = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	ID3D12Resource* m_dx12Texture = nullptr;
	ID3D12Resource* m_textureBufferUploadHeap = nullptr;
	int m_textureIndex = -1;
#endif

	// #ToDo: generalize/replace this for D3D11 support!
	// unsigned int m_openglTextureID = 0xFFFFFFFF;
};