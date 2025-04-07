#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"

Material::Material( std::string const& fileName, Renderer* renderer )
{
	Load( fileName, renderer );
}

Material::Material()
{

}

Material::~Material()
{

}

void Material::Load( std::string const& fileName, Renderer* renderer )
{
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( fileName.c_str() );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, Stringf( "Error! Load Xml Document %s error", fileName.c_str() ) );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "Material" ), "Error! Root name of material should be Material!" );
	m_color = ParseXmlAttribute( *root, "color", Rgba8( 255, 255, 255, 255 ) );
	
	std::string vertexType = ParseXmlAttribute( *root, "vertexType", "Vertex_PCUTBN" );
	if (vertexType == "Vertex_PCUTBN") {
		m_vertexType = VertexType::PCUTBN;
	}
	else if (vertexType == "Vertex_PCU") {
		m_vertexType = VertexType::PCU;
	}

	std::string diffuseTexture = ParseXmlAttribute( *root, "diffuseTexture", "None" );
	if (diffuseTexture == "None") {
		m_diffuseTxeture = nullptr;
	}
	else {
		m_diffuseTxeture = renderer->CreateOrGetTextureFromFile( diffuseTexture.c_str() );
	}

	std::string normalTexture = ParseXmlAttribute( *root, "normalTexture", "None" );
	if (normalTexture == "None") {
		m_normalTexture = nullptr;
	}
	else {
		m_normalTexture = renderer->CreateOrGetTextureFromFile( normalTexture.c_str() );
	}

	std::string specGlossEmitTexture = ParseXmlAttribute( *root, "specGlossEmitTexture", "None" );
	if (specGlossEmitTexture == "None") {
		m_specGlossEmitTexture = nullptr;
	}
	else {
		m_specGlossEmitTexture = renderer->CreateOrGetTextureFromFile( specGlossEmitTexture.c_str() );
	}

	std::string shaders = ParseXmlAttribute( *root, "shader", "None" );
	if (shaders == "None") {
		m_shader = nullptr;
	}
	else {
		m_shader = renderer->CreateShader( shaders.c_str(), m_vertexType );
	}
}
