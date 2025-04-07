#include "Game/label.hpp"
#include "Game/City.hpp"
#include "Game/Culture.hpp"
#include "Game/Religion.hpp"
#include "Game/Country.hpp"
#include "Game/Game.hpp"
#include "Game/Continent.hpp"
#include "Game/Region.hpp"

Label::Label( void* owner, LabelType type )
	:m_owner(owner)
	,m_type(type)
{
}

Label::~Label()
{
	delete m_vertexBuffer;
}

void Label::ReCalculateVertexData()
{
	if (m_type == LabelType::City || m_type == LabelType::Town) {
		Town* town = (Town*)m_owner;
		float ySize = m_type == LabelType::City ? 0.4f : 0.25f;
		m_labelText = town->m_name;
		m_middlePos = town->m_position - Vec2( 0.f, ySize * 1.f );
		m_startPos = m_middlePos - Vec2( ySize * 0.618f * (float)m_labelText.size() * 0.5f, 0.f );
		m_endPos = m_middlePos + Vec2( ySize * 0.618f * (float)m_labelText.size() * 0.5f, 0.f );
		m_orientation = 0.f;
		m_color = m_type == LabelType::City ? ((((City*)town)->HasAttribute(CityAttribute::Capital)) ? Rgba8( 160, 0, 0 ) : Rgba8( 50, 50, 50 )) : Rgba8( 0, 0, 0 );
		m_size = Vec2( 10000.f, ySize );
		m_curve.m_startPos = m_startPos;
		m_curve.m_endPos = m_endPos;
		m_curve.m_guidePos1 = m_middlePos;
		m_curve.m_guidePos2 = m_middlePos;
	}
	else if (m_type == LabelType::Culture || m_type == LabelType::Country || m_type == LabelType::Religion 
		|| m_type == LabelType::Continent || m_type == LabelType::Region) {
		if (m_type == LabelType::Culture) {
			((Culture*)m_owner)->GetBoundsPointsForLabel( m_startPos, m_endPos );
			m_labelText = ((Culture*)m_owner)->m_name;
			m_middlePos = 0.5f * (m_startPos + m_endPos);
		}
		else if (m_type == LabelType::Religion) {
			((Religion*)m_owner)->GetBoundsPointsForLabel( m_startPos, m_endPos );
			m_labelText = ((Religion*)m_owner)->m_name;
			m_middlePos = 0.5f * (m_startPos + m_endPos);
		}
		else if (m_type == LabelType::Country) {
			((Country*)m_owner)->GetBoundsPointsForLabel( m_startPos, m_endPos );
			m_labelText = ((Country*)m_owner)->m_name;
			if ((int)((Country*)m_owner)->m_provinces.size() > 0) {
				m_enableRenderCountry = true;
				m_middlePos = Interpolate( ((Country*)m_owner)->GetGeometricCenter(), Interpolate( m_startPos, m_endPos, 0.5f ), 0.5f );
			}
			else {
				m_enableRenderCountry = false;
				//m_middlePos = Interpolate( ((Country*)m_owner)->m_originProv->m_geoCenterPos, Interpolate( m_startPos, m_endPos, 0.5f ), 0.5f );
			}
		}
		else if (m_type == LabelType::Continent) {
			((Continent*)m_owner)->GetBoundsPointsForLabel( m_startPos, m_endPos );
			m_labelText = ((Continent*)m_owner)->m_name;
			m_middlePos = 0.5f * (m_startPos + m_endPos);
		}
		else if (m_type == LabelType::Region) {
			((Region*)m_owner)->GetBoundsPointsForLabel( m_startPos, m_endPos );
			m_labelText = ((Region*)m_owner)->m_name;
			m_middlePos = 0.5f * (m_startPos + m_endPos);
		}
		m_curve.m_startPos = m_startPos;
		m_curve.m_endPos = m_endPos;
		m_curve.m_guidePos1 = m_middlePos;
		m_curve.m_guidePos2 = m_middlePos;
		//float length = GetDistance2D( m_startPos, m_endPos );
		//m_orientation = (endPos - startPos).GetOrientationDegrees();
		m_color = Rgba8( 0, 0, 0 );
		if (m_labelText.size() == 0) {
			m_size = Vec2( 0.f, 0.f );
		}
		else {
			m_size = Vec2( 0.f, m_curve.GetApproximateLength() / (float)m_labelText.size() / 0.618f );
		}
	}
	else if (m_type == LabelType::Mountain || m_type == LabelType::River) {

	}
	else if (m_type == LabelType::Sea) {

	}
	std::vector<Vertex_PCU> verts;
	g_ASCIIFont->AddVertsForCurveText2D( verts, m_curve, m_size.y, m_labelText, Rgba8::WHITE, 0.618f );
	Mat44 modelMatrix = GetModelMatrix();
	TransformVertexArrayXY3D( verts, modelMatrix.GetIBasis2D(), modelMatrix.GetJBasis2D(), modelMatrix.GetTranslation2D() );
	if (!m_vertexBuffer && verts.size() > 0) {
		m_vertexBuffer = g_theRenderer->CreateVertexBuffer( verts.size() * sizeof( Vertex_PCU ), sizeof( Vertex_PCU ) );
	}
	if (m_vertexBuffer) {
		g_theRenderer->CopyCPUToGPU( verts.data(), verts.size() * sizeof( Vertex_PCU ), m_vertexBuffer );
	}

}

void Label::UpdateColor()
{
	float cameraScale = 0.f;
	if (g_theGame->m_viewMode == MapViewMode::ViewMode2D || g_theGame->m_viewMode == MapViewMode::ViewModeSphere) {
		cameraScale = g_theGame->m_worldCameraScale;
	}
	else if (g_theGame->m_viewMode == MapViewMode::ViewMode3D) {
		cameraScale = g_theGame->m_worldCamera.m_position.z;
	}
	if (m_type == LabelType::Culture || m_type == LabelType::Religion || m_type == LabelType::Continent) {
		constexpr float disappearValue = 40.f;
		if (cameraScale < disappearValue) {
			m_enableRender = false;
		}
		else {
			m_enableRender = true;
			m_color.a = (unsigned char)Interpolate( 0.f, 255.f, SmoothStop2( RangeMapClamped( cameraScale, disappearValue, 100.f, 0.f, 1.f ) ) );
		}
	}
	else if (m_type == LabelType::Country) {
		constexpr float disappearValue = 10.f;
		if (cameraScale < disappearValue || cameraScale > 70.f) {
			m_enableRender = false;
		}
		else {
			m_enableRender = true;
			m_color.a = (unsigned char)Interpolate( 0.f, 255.f, SmoothStop6( RangeMapClamped( cameraScale, disappearValue, 100.f, 0.f, 1.f ) ) );
		}
		if (((Country*)m_owner)->ChangeNameTitle()) {
			m_labelText = ((Country*)m_owner)->m_name;
		}
	}
	else if (m_type == LabelType::City || m_type == LabelType::Town || m_type == LabelType::Region) {
		constexpr float disappearValue = 40.f;
		if (cameraScale > disappearValue) {
			m_enableRender = false;
		}
		else {
			m_enableRender = true;
			m_color.a = (unsigned char)Interpolate( 0.f, 255.f, SmoothStop2( RangeMapClamped( cameraScale, disappearValue, 10.f, 0.f, 1.f ) ) );
		}
	}
}

Mat44 Label::GetModelMatrix() const
{
	Mat44 mat = Mat44::CreateZRotationDegrees( m_orientation );
	return mat;
}

Mat44 Label::GetOffSetModelMatrix() const
{
	Mat44 mat = (m_type == LabelType::City || m_type == LabelType::Town) ? 
		Mat44::CreateTranslation2D( Vec2( 0.015f, -0.015f ) )
		:Mat44::CreateTranslation2D( Vec2( 0.05f, -0.05f ) );
	mat.AppendZRotation( m_orientation );
	return mat;
}

void Label::Render(Shader* shader, Mat44 const& modelMatrix) const
{
	if (!m_enableRender || !m_enableRenderCountry) {
		return;
	}
	if (m_vertexBuffer) {
		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		g_theRenderer->BindShader( shader );
		g_theRenderer->SetModelConstants( modelMatrix, m_color );
		//g_theRenderer->SetModelConstants( Mat44(), Rgba8(255 - m_color.r, 255 - m_color.g, 255 - m_color.b, 255));
		g_theRenderer->DrawVertexBuffer( m_vertexBuffer, m_vertexBuffer->GetVertexCount() );
// 		g_theRenderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
// 		g_theRenderer->SetDepthMode( DepthMode::DISABLED );
// 		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
// 		g_theRenderer->BindShader( shader );
// 		g_theRenderer->SetModelConstants( GetOffSetModelMatrix(), m_color );
// 		g_theRenderer->DrawVertexBuffer( m_vertexBuffer, m_vertexBuffer->GetVertexCount() );
	}
}
