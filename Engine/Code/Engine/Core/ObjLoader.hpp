#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"
#include <map>

class ObjLoader {
public:
	static bool Load( std::string const& fileName, std::vector<Vertex_PCUTBN>& out_vertexes, 
		std::vector<unsigned int>& out_indexes, bool& out_hasNormals, bool& out_hasUVs, Mat44 const& transform = Mat44() ) noexcept;

	static bool LoadMaterial( std::string const& path, std::map<std::string, Rgba8>& materialMap ) noexcept;
private:
};