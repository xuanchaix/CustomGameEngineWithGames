#include "Game/TranslationUtils.hpp"
#include "Game/SM_BitMapFont.hpp"
#include <fstream>

size_t GetSizeOfFile( std::wstring const& path )
{
	struct _stat fileinfo;
	_wstat( path.c_str(), &fileinfo );
	return fileinfo.st_size;
}

std::wstring FileReadToWString( std::wstring const& filename )
{
	std::wstring buffer;            // stores file contents
	FILE* f;
	_wfopen_s(&f, filename.c_str(), L"rtS, ccs=UTF-8" );

	// Failed to open file
	if (f == NULL)
	{
		// ...handle some error...
		return buffer;
	}

	size_t filesize = GetSizeOfFile( filename );

	// Read entire file contents in to memory
	if (filesize > 0)
	{
		buffer.resize( filesize );
		size_t wchars_read = fread( &(buffer.front()), sizeof( wchar_t ), filesize, f );
		buffer.resize( wchars_read );
		buffer.shrink_to_fit();
	}

	fclose( f );

	return buffer;
}
class TranslationTool {
public:
	TranslationTool();
	TranslationTool( SM_GameLanguage language );
	~TranslationTool();
	std::wstring TranslateUnit( std::string const& str ) const;
	std::wstring TranslateAll( std::string const& str ) const;
	void ParseCSVTranslation( std::wstring const& filePath );

	SM_GameLanguage m_language;
	std::map<std::string, std::wstring> m_translations;
};

TranslationTool::TranslationTool()
{
}

TranslationTool::TranslationTool( SM_GameLanguage language )
	:m_language(language)
{
	std::wstring languageAddition;
	if (m_language == SM_GameLanguage::ZH) {
		languageAddition = L"Zh";
	}
	else if (m_language == SM_GameLanguage::ENGLISH) {
		languageAddition = L"En";
	}
	std::wstring provPath = L"Data/Localisations/" + languageAddition + L"/Provinces.csv";
	std::wstring corePath = L"Data/Localisations/" + languageAddition + L"/Core.csv";
	std::wstring forcePath = L"Data/Localisations/" + languageAddition + L"/Forces.csv";

	ParseCSVTranslation( corePath );
	ParseCSVTranslation( provPath );
	ParseCSVTranslation( forcePath );
}

TranslationTool::~TranslationTool()
{

}

std::wstring TranslationTool::TranslateUnit( std::string const& str ) const
{
	auto const iter = m_translations.find( str );
	if (iter != m_translations.end()) {
		return iter->second;
	}
	return std::wstring( str.begin(), str.end() );
}

std::wstring TranslationTool::TranslateAll( std::string const& str ) const
{
	std::wstring wstr;
	size_t findRes;
	size_t findEndRes;
	size_t firstPos = 0;
	do {
		findRes = str.find_first_of( "$(", firstPos );
		std::string partStr = str.substr( firstPos, findRes - firstPos );
		wstr.append( partStr.begin(), partStr.end() );
		if (findRes != std::wstring::npos) {
			findEndRes = str.find_first_of( ")", findRes );
			wstr.append( TranslateUnit( str.substr( findRes + 2, findEndRes - findRes - 2 ) ) );
			firstPos = findEndRes + 1;
		}
	} while (findRes != std::wstring::npos);
	return wstr;
}

void TranslationTool::ParseCSVTranslation( std::wstring const& filePath )
{
	std::wstring wstr = FileReadToWString( filePath );
	WStrings wstrs;
	SplitStringOnDelimiter( wstrs, wstr, '\n' );
	for (auto& line : wstrs) {
		size_t findRes = line.find_first_of( ',', 0 );
		if (findRes != std::string::npos) {
			std::string key;
			std::wstring wKey = line.substr( 0, findRes );
			std::wstring wValue = line.substr( findRes + 1 );
			std::transform( wKey.begin(), wKey.end(), std::back_inserter( key ), []( wchar_t c ) {
				return (char)c;
				} );
			m_translations[key] = wValue;
		}
	}
}

std::vector<TranslationTool> SMTrans_tools;

std::wstring GetWStringTranslation( std::string placeHolder, SM_GameLanguage language /*= SM_GameLanguage::DEFAULT */ )
{
	if (language == SM_GameLanguage::DEFAULT) {
		language = g_gameLanguage;
	}
	return SMTrans_tools[(int)language].TranslateUnit( placeHolder );
}

void StartUpTranslation( SM_GameLanguage defaultLanguage )
{
	g_gameLanguage = defaultLanguage;
	SMTrans_tools.resize( 10 );
	SMTrans_tools[(int)SM_GameLanguage::ENGLISH] = TranslationTool( SM_GameLanguage::ENGLISH );
	SMTrans_tools[(int)SM_GameLanguage::ZH] = TranslationTool( SM_GameLanguage::ZH );
}

void AddVertsForTextPlaceHolder( std::vector<Vertex_PCU>& verts, AABB2 const& box, float cellHeight, std::string const& text, Rgba8 const& tint /*= Rgba8::WHITE*/, float cellAspect /*= 0.618f*/, Vec2 const& alignment /*= Vec2( .5f, .5f )*/, TextBoxMode mode /*= TextBoxMode::SHRINK_TO_FIT*/, int maxGlyphsToDraw /*= INT_MAX */ )
{
	std::wstring wtext;
	wtext = SMTrans_tools[(int)g_gameLanguage].TranslateAll( text );
	
	if (g_gameLanguage == SM_GameLanguage::ENGLISH) {
		std::string ttext;
		std::transform( wtext.begin(), wtext.end(), std::back_inserter( ttext ), []( wchar_t c ) {
			return (char)c;
			} );
		g_ASCIIFont->AddVertsForTextInBox2D( verts, box, cellHeight, ttext, tint, cellAspect, alignment, mode, maxGlyphsToDraw );
	}
	else if (g_gameLanguage == SM_GameLanguage::ZH) {
		g_chineseFont->AddVertsForTextInBox2D( verts, box, cellHeight * 1.2f, wtext, tint, 0.75f, alignment, mode, maxGlyphsToDraw );
	}
}
