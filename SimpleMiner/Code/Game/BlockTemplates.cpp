#include "Game/BlockTemplates.hpp"
#include "Game/Block.hpp"

void BlockTemplate::SetUpBlockTemplates()
{
	/*unsigned char airType = BlockDefinition::GetDefinitionIndexByName("air");
	unsigned char grassType = BlockDefinition::GetDefinitionIndexByName( "grass" );
	unsigned char stoneType = BlockDefinition::GetDefinitionIndexByName( "stone" );
	unsigned char dirtType = BlockDefinition::GetDefinitionIndexByName( "dirt" );
	unsigned char waterType = BlockDefinition::GetDefinitionIndexByName( "water" );
	unsigned char coalType = BlockDefinition::GetDefinitionIndexByName( "coal" );
	unsigned char ironType = BlockDefinition::GetDefinitionIndexByName( "iron" );
	unsigned char goldType = BlockDefinition::GetDefinitionIndexByName( "gold" );
	unsigned char diamondType = BlockDefinition::GetDefinitionIndexByName( "diamond" );
	unsigned char sandType = BlockDefinition::GetDefinitionIndexByName( "sand" );
	unsigned char iceType = BlockDefinition::GetDefinitionIndexByName( "ice" );
	unsigned char beachType = BlockDefinition::GetDefinitionIndexByName( "beach" );*/
	unsigned char oakLogType = BlockDefinition::GetDefinitionIndexByName( "oak_log" );
	unsigned char oakLeafType = BlockDefinition::GetDefinitionIndexByName( "oak_leaf" );
	unsigned char spruceLogType = BlockDefinition::GetDefinitionIndexByName( "spruce_log" );
	unsigned char spruceLeafType = BlockDefinition::GetDefinitionIndexByName( "spruce_leaf" );
	unsigned char cactusType = BlockDefinition::GetDefinitionIndexByName( "cactus" );

	BlockTemplate oakTemplate;
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 0 ), oakLogType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 1 ), oakLogType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 2 ), oakLogType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 3 ), oakLogType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 4 ), oakLogType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 5 ), oakLogType ) );

	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 2 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 2 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 2 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 2 ), oakLeafType ) );

	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, -1, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 1, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 1, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, -1, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -2, 0, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 2, 0, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 2, 3 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -2, 3 ), oakLeafType ) );

	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, -1, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 1, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 1, 4 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, -1, 4 ), oakLeafType ) );

	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 5 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 5 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 5 ), oakLeafType ) );
	oakTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 5 ), oakLeafType ) );

	s_templates["oak"] = oakTemplate;

	BlockTemplate spruceTemplate;
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 0 ), spruceLogType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 1 ), spruceLogType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 2 ), spruceLogType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 3 ), spruceLogType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 4 ), spruceLogType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 5 ), spruceLogType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 6 ), spruceLogType ) );
	
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 2 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 2 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 2 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 2 ), spruceLeafType ) );
	
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, -1, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 1, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 1, 3 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, -1, 3 ), spruceLeafType ) );
	
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, -1, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 1, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 1, 4 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, -1, 4 ), spruceLeafType ) );
	
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, -1, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 1, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 1, 5 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, -1, 5 ), spruceLeafType ) );

	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( -1, 0, 6 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 1, 0, 6 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 1, 6 ), spruceLeafType ) );
	spruceTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, -1, 6 ), spruceLeafType ) );

	s_templates["spruce"] = spruceTemplate;

	BlockTemplate cactusTemplate;
	cactusTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 0 ), cactusType ) );
	cactusTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 1 ), cactusType ) );
	cactusTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 2 ), cactusType ) );
	cactusTemplate.m_entries.push_back( BlockTemplateEntry( IntVec3( 0, 0, 3 ), cactusType ) );

	s_templates["cactus"] = cactusTemplate;
}

std::map<std::string, BlockTemplate> BlockTemplate::s_templates;

BlockTemplate const& BlockTemplate::GetBlockTemplate( std::string const& name )
{
	auto iter = s_templates.find( name );
	if(iter != s_templates.end()){
		return iter->second;
	}
	ERROR_AND_DIE( Stringf( "Cannot find a block template named %s", name.c_str() ) );
}

BlockTemplateEntry::BlockTemplateEntry( IntVec3 const& relativePos, unsigned char blockType )
	:m_relativePos(relativePos)
	,m_type(blockType)
{

}
