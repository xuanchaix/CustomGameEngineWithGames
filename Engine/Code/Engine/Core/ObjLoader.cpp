#include "Engine/Core/ObjLoader.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <filesystem>
#include <map>

bool ObjLoader::Load( std::string const& fileName, std::vector<Vertex_PCUTBN>& out_vertexes, std::vector<unsigned int>& out_indexes,
	bool& out_hasNormals, bool& out_hasUVs, Mat44 const& transform /*= Mat44() */ ) noexcept
{
	double loadStartTime = GetCurrentTimeSeconds();
	out_vertexes.clear();
	out_indexes.clear();
	out_hasNormals = false;
	out_hasUVs = false;
	std::string rawObjFile;
	FileReadToString( rawObjFile, fileName );
	Strings objLines;

	DebuggerPrintf( "-------------------------------------\n" );
	DebuggerPrintf( "Loaded .obj file %s\n", fileName.c_str() );
	double startTime = GetCurrentTimeSeconds();
	SplitStringIntoLines( objLines, rawObjFile );

	std::vector<Vec3> vertPositions;
	std::vector<Vec3> normals;
	std::vector<Vec2> textureCoords;
	std::map<std::string, Rgba8> materialMap;
	Rgba8* curRgba8 = nullptr;
	int numOfFaces = 0;
	for (auto& line : objLines) {
		Strings lineElements;
		SplitStringOnDelimiter( lineElements, line, ' ', true );
		if ((int)lineElements.size() > 0) {
			if (lineElements[0] == "v") {
				GUARANTEE_OR_DIE( (int)lineElements.size() == 4, "Vertex should have x y z coordinates" );
				vertPositions.emplace_back( stof( lineElements[1] ), stof( lineElements[2] ), stof( lineElements[3] ) );
			}
			else if (lineElements[0] == "vn") {
				GUARANTEE_OR_DIE( (int)lineElements.size() == 4, "Normal should have x y z coordnates" );
				normals.emplace_back( stof( lineElements[1] ), stof( lineElements[2] ), stof( lineElements[3] ) );
			}
			else if (lineElements[0] == "f") {
				numOfFaces++;
				GUARANTEE_OR_DIE( (int)lineElements.size() >= 4, "Face should have at least 3 vertexes" );
				Vertex_PCUTBN vert0;
				Strings ptn;
				SplitStringOnDelimiter( ptn, lineElements[1], '/');
				vert0.m_position = vertPositions[stoi( ptn[0] ) - 1];
				if ((int)ptn.size() > 1) {
					if (ptn[1] != "") {
						out_hasUVs = true;
						vert0.m_uvTexCoords = textureCoords[stoi( ptn[1] ) - 1];
					}
					if ((int)ptn.size() > 2) {
						if (ptn[2] != "") {
							out_hasNormals = true;
							vert0.m_normal = normals[stoi( ptn[2] ) - 1];
						}
					}
				}
				Vertex_PCUTBN vert1;
				SplitStringOnDelimiter( ptn, lineElements[2], '/' );
				vert1.m_position = vertPositions[stoi( ptn[0] ) - 1];
				if ((int)ptn.size() > 1) {
					if (ptn[1] != "") {
						out_hasUVs = true;
						vert1.m_uvTexCoords = textureCoords[stoi( ptn[1] ) - 1];
					}
					if ((int)ptn.size() > 2) {
						if (ptn[2] != "") {
							out_hasNormals = true;
							vert1.m_normal = normals[stoi( ptn[2] ) - 1];
						}
					}
				}

				Vertex_PCUTBN vert2;
				bool vert1AsFirst = true;
				for (int i = 2; i < (int)lineElements.size() - 1; i++) {
					if (vert1AsFirst) {
						SplitStringOnDelimiter( ptn, lineElements[i + 1], '/' );
						vert2.m_position = vertPositions[stoi( ptn[0] ) - 1];
						if ((int)ptn.size() > 1) {
							if (ptn[1] != "") {
								out_hasUVs = true;
								vert2.m_uvTexCoords = textureCoords[stoi( ptn[1] ) - 1];
							}
							if ((int)ptn.size() > 2) {
								if (ptn[2] != "") {
									out_hasNormals = true;
									vert2.m_normal = normals[stoi( ptn[2] ) - 1];
								}
							}
						}
						if (curRgba8 == nullptr) {
							vert0.m_color = Rgba8::WHITE;
							vert1.m_color = Rgba8::WHITE;
							vert2.m_color = Rgba8::WHITE;
						}
						else {
							vert0.m_color = *curRgba8;
							vert1.m_color = *curRgba8;
							vert2.m_color = *curRgba8;
						}
						out_indexes.push_back( (unsigned int)out_vertexes.size() );
						out_vertexes.push_back( vert0 );
						out_indexes.push_back( (unsigned int)out_vertexes.size() );
						out_vertexes.push_back( vert1 );
						out_indexes.push_back( (unsigned int)out_vertexes.size() );
						out_vertexes.push_back( vert2 );
					}
					else {
						SplitStringOnDelimiter( ptn, lineElements[i + 1], '/' );
						vert1.m_position = vertPositions[stoi( ptn[0] ) - 1];
						if ((int)ptn.size() > 1) {
							if (ptn[1] != "") {
								out_hasUVs = true;
								vert1.m_uvTexCoords = textureCoords[stoi( ptn[1] ) - 1];
							}
							if ((int)ptn.size() > 2) {
								if (ptn[2] != "") {
									out_hasNormals = true;
									vert1.m_normal = normals[stoi( ptn[2] ) - 1];
								}
							}
						}
						if (curRgba8 == nullptr) {
							vert0.m_color = Rgba8::WHITE;
							vert1.m_color = Rgba8::WHITE;
							vert2.m_color = Rgba8::WHITE;
						}
						else {
							vert0.m_color = *curRgba8;
							vert1.m_color = *curRgba8;
							vert2.m_color = *curRgba8;
						}
						out_indexes.push_back( (unsigned int)out_vertexes.size() );
						out_vertexes.push_back( vert0 );
						out_indexes.push_back( (unsigned int)out_vertexes.size() );
						out_vertexes.push_back( vert2 );
						out_indexes.push_back( (unsigned int)out_vertexes.size() );
						out_vertexes.push_back( vert1 );
					}
					vert1AsFirst = !vert1AsFirst;
				}
			}
			else if (lineElements[0] == "vt") {
				textureCoords.emplace_back( stof( lineElements[1] ), stof( lineElements[2] ) );
			}
			else if (lineElements[0] == "mtllib") {
				GUARANTEE_OR_DIE( (int)lineElements.size() == 2, "Fail to load material .mtl library!" );
				char drive[300];
				char dir[300];
				char fname[300];
				char ext[300];
				_splitpath_s( fileName.c_str(), drive, 300, dir, 300, fname, 300, ext, 300 );
				std::string materialPath = dir + lineElements[1];
				bool res = LoadMaterial( materialPath, materialMap );
				if (!res) {
					return false;
				}

			}
			else if (lineElements[0] == "o") {

			}
			else if (lineElements[0] == "usemtl") {
				GUARANTEE_OR_DIE( (int)lineElements.size() == 2, "Fail to use material .mtl!" );
				auto iter = materialMap.find( lineElements[1] );
				if (iter != materialMap.end()) {
					curRgba8 = &iter->second;
				}
			}
			else if (lineElements[0] == "s") {

			}
			else if (lineElements[0] == "g") {

			}
		}
	}

	for (int i = 0; i < (int)out_vertexes.size(); i++) {
		out_vertexes[i].m_position = transform.TransformPosition3D( out_vertexes[i].m_position );
	}

	double endTime = GetCurrentTimeSeconds();

	DebuggerPrintf( "                            positions: %d  uvs: %d  normals: %d  faces: %d\n", vertPositions.size(), textureCoords.size(), normals.size(), numOfFaces );
	DebuggerPrintf( "                            vertexes: %d triangles: %d time: %fs\n", out_vertexes.size(), out_indexes.size() / 3, startTime - loadStartTime );
	DebuggerPrintf( "Created CPU mesh            time: %fs\n", endTime - startTime );

	return true;
}

bool ObjLoader::LoadMaterial( std::string const& path, std::map<std::string, Rgba8>& materialMap ) noexcept
{
	materialMap.clear();
	std::string rawMtlFile;
	FileReadToString( rawMtlFile, path );
	Strings mtlLines;
	SplitStringIntoLines( mtlLines, rawMtlFile );

	std::string mtlName;
	for (auto& line : mtlLines) {
		Strings lineElements;
		SplitStringOnDelimiter( lineElements, line, ' ', true );
		if ((int)lineElements.size() > 0) {
			if (lineElements[0] == "newmtl") {
				mtlName = lineElements[1];
			}
			else if (lineElements[0] == "Kd") {
				materialMap[mtlName] = Rgba8();
				materialMap[mtlName].r = DenormalizeByte( stof( lineElements[1] ) );
				materialMap[mtlName].g = DenormalizeByte( stof( lineElements[2] ) );
				materialMap[mtlName].b = DenormalizeByte( stof( lineElements[3] ) );
			}
		}
	}
	return true;
}
