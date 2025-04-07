#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "ThirdParty/stb/stb_image.h"

Image::~Image()
{

}

Image::Image( char const* imageFilePath )
{
	m_imageFilePath = imageFilePath;
	int bytesPerTexel = 0;
	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load( imageFilePath, &m_dimensions.x, &m_dimensions.y, &bytesPerTexel, 0 );

	m_rgbaTexels.reserve( (size_t)m_dimensions.x * m_dimensions.y );
	// Check if the load was successful
	GUARANTEE_OR_DIE( texelData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	GUARANTEE_OR_DIE( bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf( "Create Image From Data failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", imageFilePath, bytesPerTexel ) );

	for (int j = 0; j < m_dimensions.y; j++) {
		for (int i = 0; i < m_dimensions.x; i++) {
			unsigned char* firstOfRgba8 = texelData + ((size_t)j * m_dimensions.x + i) * bytesPerTexel * sizeof( unsigned char );
			if (bytesPerTexel == 3) {
				m_rgbaTexels.emplace_back( firstOfRgba8[0], firstOfRgba8[1], firstOfRgba8[2], (unsigned char)255 );
			}
			else {
				m_rgbaTexels.emplace_back( firstOfRgba8[0], firstOfRgba8[1], firstOfRgba8[2], firstOfRgba8[3] );
			}
		}
	}

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( texelData );
}

Image::Image( IntVec2 const& size, Rgba8 const& color )
	:m_dimensions(size)
{
	m_rgbaTexels.resize( (size_t)size.x * size.y, color );
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 const& Image::GetDimensions() const
{
	return m_dimensions;
}

void const* Image::GetRawData() const
{
	return m_rgbaTexels.data();
}

Rgba8 const& Image::GetTexelColor( IntVec2 const& texelCoords ) const
{
	return m_rgbaTexels[texelCoords.x + (size_t)texelCoords.y * m_dimensions.x];
}

void Image::SetTexelColor( IntVec2 const& texelCoords, Rgba8 const& newColor )
{
	m_rgbaTexels[texelCoords.x + (size_t)texelCoords.y * m_dimensions.x] = newColor;
}
