#pragma once
#include "Game/GameCommon.hpp"

struct BlockTemplateEntry {
	BlockTemplateEntry( IntVec3 const& relativePos, unsigned char blockType );

	IntVec3 m_relativePos;
	unsigned char m_type;
};

class BlockTemplate {
public:
	BlockTemplate() = default;
protected:
	static std::map<std::string, BlockTemplate> s_templates;

public:
	static void SetUpBlockTemplates();
	static BlockTemplate const& GetBlockTemplate( std::string const& name );
	std::vector<BlockTemplateEntry> m_entries;
};