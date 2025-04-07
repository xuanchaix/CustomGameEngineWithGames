#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RendererUtils.hpp"
#include <string>

class Texture;
class Shader;
class Renderer;

class Material {
public:
	Material();
	Material( std::string const& fileName, Renderer* renderer );
	virtual ~Material();
	void Load( std::string const& fileName, Renderer* renderer );
	Rgba8 m_color = Rgba8( 255, 255, 255 );
	Shader* m_shader = nullptr;
	VertexType m_vertexType = VertexType::PCUTBN;
	Texture* m_diffuseTxeture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_specGlossEmitTexture = nullptr;
};