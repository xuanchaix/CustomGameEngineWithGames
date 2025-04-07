#include "Game/Projectile.hpp"

Projectile::Projectile( Vec2 const& pos, ProjectileDefinition const& def )
	:Entity(pos, g_theGame)
	,m_def(def)
{
	m_physicsBounds = AABB2( Vec2( 0.f, 0.f ), m_def.m_bounds );
	m_speed = m_def.m_speed;
	m_maxHealth = 1.f;
	m_health = 1.f;
	m_damage = m_def.m_damage;
	m_entityType = EntityType::Projectile;
}

Projectile::~Projectile()
{

}

void Projectile::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}
	if (m_health <= 0.f) {
		m_isDead = true;
		return;
	}
	m_lifeTime += deltaTime;
	if (m_lifeTime >= m_def.m_lifeSpan) {
		Die();
	}
	m_position += GetForwardNormal() * m_speed * deltaTime;
}

void Projectile::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, m_physicsBounds, m_color );

	g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	g_theRenderer->SetDepthMode( DepthMode::ENABLED );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->BindTexture( m_def.m_texture );
	g_theRenderer->SetModelConstants( GetModelConstants() );
	g_theRenderer->SetShadowMode( ShadowMode::DISABLE );

	g_theRenderer->DrawVertexArray( verts );
}

void Projectile::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}

ProjectileDefinition::ProjectileDefinition()
{

}

ProjectileDefinition::ProjectileDefinition( XmlElement* xmlIter )
{
	m_id = ParseXmlAttribute( *xmlIter, "id", m_id );
	m_name = ParseXmlAttribute( *xmlIter, "name", m_name );
	m_productID = ParseXmlAttribute( *xmlIter, "productID", m_productID );
	m_isPiercing = ParseXmlAttribute( *xmlIter, "isPiercing", m_isPiercing );
	m_isParalyzing = ParseXmlAttribute( *xmlIter, "isParalyzing", m_isParalyzing );
	m_isBurning = ParseXmlAttribute( *xmlIter, "isBurning", m_isBurning );
	m_isGuided = ParseXmlAttribute( *xmlIter, "isGuided", m_isGuided );
	m_isSplashing = ParseXmlAttribute( *xmlIter, "isSplashing", m_isSplashing );
	m_texture = g_theRenderer->CreateOrGetTextureFromFile( ParseXmlAttribute( *xmlIter, "texturePath", "" ).c_str() );
	m_damage = ParseXmlAttribute( *xmlIter, "damage", m_damage );
	m_lifeSpan = ParseXmlAttribute( *xmlIter, "lifeSpan", m_lifeSpan );
	m_speed = ParseXmlAttribute( *xmlIter, "speed", m_speed );
	m_projectileCount = ParseXmlAttribute( *xmlIter, "projectileCount", m_projectileCount );
	m_bounds = ParseXmlAttribute( *xmlIter, "bounds", m_bounds );
	m_color = ParseXmlAttribute( *xmlIter, "tintColor", m_color );
}

void ProjectileDefinition::SetUpProjectileDefinitions()
{
	ProjectileDefinition::s_definitions.reserve( 64 );
	XmlDocument xmlDocument;
	XmlError errorCode = xmlDocument.LoadFile( "Data/Definitions/ProjectileDefinitions.xml" );
	GUARANTEE_OR_DIE( errorCode == tinyxml2::XMLError::XML_SUCCESS, "Error! Load Xml Document ProjectileDefinitions.xml error" );
	XmlElement* root = xmlDocument.FirstChildElement();
	GUARANTEE_OR_DIE( !strcmp( root->Name(), "ProjectileDefinitions" ), "Syntax Error! Name of the root of ProjectileDefinitions.xml should be \"ProjectileDefinitions\" " );
	XmlElement* xmlIter = root->FirstChildElement();
	while (xmlIter != nullptr) {
		GUARANTEE_OR_DIE( !strcmp( xmlIter->Name(), "ProjectileDefinition" ), "Syntax Error! Names of the elements of ProjectileDefinitions.xml should be \"ProjectileDefinition\" " );
		ProjectileDefinition::s_definitions.emplace_back( xmlIter );
		xmlIter = xmlIter->NextSiblingElement();
	}
}

std::vector<ProjectileDefinition> ProjectileDefinition::s_definitions;

ProjectileDefinition const& ProjectileDefinition::GetDefinition( std::string const& name )
{
	for (auto const& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( "Cannot find projectile definition" );
}

ProjectileDefinition const& ProjectileDefinition::GetDefinition( int id )
{
	return s_definitions[id];
}
