#include "Game/Block.hpp"

#define GetSpriteIndexFromCoords(x,y) x + y * 64

std::vector<BlockDefinition> BlockDefinition::s_definitions;

BlockDefinition::BlockDefinition()
{

}

BlockDefinition::BlockDefinition( XmlElement* xmlIter )
{
	UNUSED( xmlIter );
}

void BlockDefinition::SetUpDefinitions()
{
	// hard code now
	s_definitions.resize( 19 );
	//air, water, grass, dirt, stone, coal, iron, gold, diamond
	s_definitions[0].m_name = "air";
	s_definitions[0].m_index = 0;
	s_definitions[0].m_visible = false;
	s_definitions[0].m_opaque = false;
	s_definitions[0].m_solid = false;
	s_definitions[0].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 0, 0 ) ).GetUVs();
	s_definitions[0].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 0, 0 ) ).GetUVs();
	s_definitions[0].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 0, 0 ) ).GetUVs();

	s_definitions[1].m_name = "glowstone";
	s_definitions[1].m_index = 1;
	s_definitions[1].m_visible = true;
	s_definitions[1].m_opaque = true;
	s_definitions[1].m_solid = true;
	s_definitions[1].m_lightStrength = 15;
	s_definitions[1].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 46, 34 ) ).GetUVs();
	s_definitions[1].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 46, 34 ) ).GetUVs();
	s_definitions[1].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 46, 34 ) ).GetUVs();

	s_definitions[2].m_name = "grass";
	s_definitions[2].m_index = 2;
	s_definitions[2].m_visible = true;
	s_definitions[2].m_opaque = true;
	s_definitions[2].m_solid = true;
	s_definitions[2].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 33 ) ).GetUVs();
	s_definitions[2].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 34 ) ).GetUVs();
	s_definitions[2].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 33, 33 ) ).GetUVs();

	s_definitions[3].m_name = "dirt";
	s_definitions[3].m_index = 3;
	s_definitions[3].m_visible = true;
	s_definitions[3].m_opaque = true;
	s_definitions[3].m_solid = true;
	s_definitions[3].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 34 ) ).GetUVs();
	s_definitions[3].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 34 ) ).GetUVs();
	s_definitions[3].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 34 ) ).GetUVs();

	s_definitions[4].m_name = "stone";
	s_definitions[4].m_index = 4;
	s_definitions[4].m_visible = true;
	s_definitions[4].m_opaque = true;
	s_definitions[4].m_solid = true;
	s_definitions[4].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 33, 32 ) ).GetUVs();
	s_definitions[4].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 33, 32 ) ).GetUVs();
	s_definitions[4].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 33, 32 ) ).GetUVs();

	s_definitions[5].m_name = "coal";
	s_definitions[5].m_index = 5;
	s_definitions[5].m_visible = true;
	s_definitions[5].m_opaque = true;
	s_definitions[5].m_solid = true;
	s_definitions[5].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 34 ) ).GetUVs();
	s_definitions[5].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 34 ) ).GetUVs();
	s_definitions[5].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 34 ) ).GetUVs();

	s_definitions[6].m_name = "iron";
	s_definitions[6].m_index = 6;
	s_definitions[6].m_visible = true;
	s_definitions[6].m_opaque = true;
	s_definitions[6].m_solid = true;
	s_definitions[6].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 35 ) ).GetUVs();
	s_definitions[6].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 35 ) ).GetUVs();
	s_definitions[6].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 35 ) ).GetUVs();

	s_definitions[7].m_name = "gold";
	s_definitions[7].m_index = 7;
	s_definitions[7].m_visible = true;
	s_definitions[7].m_opaque = true;
	s_definitions[7].m_solid = true;
	s_definitions[7].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 36 ) ).GetUVs();
	s_definitions[7].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 36 ) ).GetUVs();
	s_definitions[7].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 36 ) ).GetUVs();

	s_definitions[8].m_name = "diamond";
	s_definitions[8].m_index = 8;
	s_definitions[8].m_visible = true;
	s_definitions[8].m_opaque = true;
	s_definitions[8].m_solid = true;
	s_definitions[8].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 37 ) ).GetUVs();
	s_definitions[8].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 37 ) ).GetUVs();
	s_definitions[8].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 63, 37 ) ).GetUVs();

	s_definitions[9].m_name = "cobblestone";
	s_definitions[9].m_index = 9;
	s_definitions[9].m_visible = true;
	s_definitions[9].m_opaque = true;
	s_definitions[9].m_solid = true;
	s_definitions[9].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 32 ) ).GetUVs();
	s_definitions[9].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 32 ) ).GetUVs();
	s_definitions[9].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 32 ) ).GetUVs();

	s_definitions[10].m_name = "water";
	s_definitions[10].m_index = 10;
	s_definitions[10].m_visible = true;
	s_definitions[10].m_opaque = true;
	s_definitions[10].m_solid = true;
	s_definitions[10].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 44 ) ).GetUVs();
	s_definitions[10].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 44 ) ).GetUVs();
	s_definitions[10].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 44 ) ).GetUVs();

	s_definitions[11].m_name = "sand";
	s_definitions[11].m_index = 11;
	s_definitions[11].m_visible = true;
	s_definitions[11].m_opaque = true;
	s_definitions[11].m_solid = true;
	s_definitions[11].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 34 ) ).GetUVs();
	s_definitions[11].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 34 ) ).GetUVs();
	s_definitions[11].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 34 ) ).GetUVs();

	s_definitions[12].m_name = "ice";
	s_definitions[12].m_index = 12;
	s_definitions[12].m_visible = true;
	s_definitions[12].m_opaque = true;
	s_definitions[12].m_solid = true;
	s_definitions[12].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 36, 35 ) ).GetUVs();
	s_definitions[12].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 36, 35 ) ).GetUVs();
	s_definitions[12].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 36, 35 ) ).GetUVs();

	s_definitions[13].m_name = "beach";
	s_definitions[13].m_index = 13;
	s_definitions[13].m_visible = true;
	s_definitions[13].m_opaque = true;
	s_definitions[13].m_solid = true;
	s_definitions[13].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 35, 35 ) ).GetUVs();
	s_definitions[13].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 35, 35 ) ).GetUVs();
	s_definitions[13].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 35, 35 ) ).GetUVs();

	s_definitions[14].m_name = "oak_log";
	s_definitions[14].m_index = 14;
	s_definitions[14].m_visible = true;
	s_definitions[14].m_opaque = true;
	s_definitions[14].m_solid = true;
	s_definitions[14].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 38, 33 ) ).GetUVs();
	s_definitions[14].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 38, 33 ) ).GetUVs();
	s_definitions[14].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 36, 33 ) ).GetUVs();

	s_definitions[15].m_name = "oak_leaf";
	s_definitions[15].m_index = 15;
	s_definitions[15].m_visible = true;
	s_definitions[15].m_opaque = true;
	s_definitions[15].m_solid = true;
	s_definitions[15].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 35 ) ).GetUVs();
	s_definitions[15].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 35 ) ).GetUVs();
	s_definitions[15].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 32, 35 ) ).GetUVs();

	s_definitions[16].m_name = "spruce_log";
	s_definitions[16].m_index = 16;
	s_definitions[16].m_visible = true;
	s_definitions[16].m_opaque = true;
	s_definitions[16].m_solid = true;
	s_definitions[16].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 38, 33 ) ).GetUVs();
	s_definitions[16].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 38, 33 ) ).GetUVs();
	s_definitions[16].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 35, 33 ) ).GetUVs();

	s_definitions[17].m_name = "spruce_leaf";
	s_definitions[17].m_index = 17;
	s_definitions[17].m_visible = true;
	s_definitions[17].m_opaque = true;
	s_definitions[17].m_solid = true;
	s_definitions[17].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 35 ) ).GetUVs();
	s_definitions[17].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 35 ) ).GetUVs();
	s_definitions[17].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 34, 35 ) ).GetUVs();

	s_definitions[18].m_name = "cactus";
	s_definitions[18].m_index = 18;
	s_definitions[18].m_visible = true;
	s_definitions[18].m_opaque = true;
	s_definitions[18].m_solid = true;
	s_definitions[18].m_topUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 38, 36 ) ).GetUVs();
	s_definitions[18].m_bottomUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 39, 36 ) ).GetUVs();
	s_definitions[18].m_sideUV = g_sprite->GetSpriteDef( GetSpriteIndexFromCoords( 37, 36 ) ).GetUVs();
}

BlockDefinition const& BlockDefinition::GetDefinitionByIndex( unsigned char index )
{
   	return s_definitions[(size_t)index];
}

BlockDefinition const& BlockDefinition::GetDefinitionByName( std::string const& name )
{
	for (auto& def : s_definitions) {
		if (def.m_name == name) {
			return def;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find a block definition named %s", name.c_str() ) );
}

unsigned char BlockDefinition::GetDefinitionIndexByName( std::string const& name )
{
	for (auto& def : s_definitions) {
		if (def.m_name == name) {
			return def.m_index;
		}
	}
	ERROR_AND_DIE( Stringf( "Cannot find a block definition named %s", name.c_str() ) );
}

Block::Block( unsigned char typeIndex )
	:m_type( typeIndex )
{

}

Block::Block()
{

}
