#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <vector>
#include <string>
#include <mutex>

static DebugRenderConfig drs_debugRenderConfig;
static bool drs_debugRenderIsVisible = true;
static BitmapFont* drs_debugRenderBitmapFont = nullptr;
std::mutex drs_mutex;

enum class DRS_DebugRenderObjectType {
	Point,
	Line,
	WireCylinder,
	WireSphere,
	Arrow,
	WorldText,
	BillboardText,
	ScreenText,
	Message,
};

struct DRS_DebugRenderObject {
	DRS_DebugRenderObjectType m_type;
	Vec3 m_startPos;
	Vec3 m_endPos;
	float m_radius;
	float m_liveSeconds = 0.f;
	float m_duration;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	std::string m_text;
	float m_textHeight;
	Vec2 m_alignment;
	Mat44 m_transform;
	DebugRenderMode m_mode;
	bool m_isWired = false;
	std::vector<Vertex_PCU> m_verts;
};

struct DRS_ScreenMessageObject {
	float m_liveSeconds = 0.f;
	float m_duration;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	std::string m_text;
};

static std::vector<DRS_DebugRenderObject*> drs_debugRenderObjects;
static std::vector<DRS_ScreenMessageObject*> drs_debugRenderMessages;

void DebugRenderSystemStartup( DebugRenderConfig const& config )
{
	drs_debugRenderConfig.m_fontName = config.m_fontName;
	drs_debugRenderConfig.m_renderer = config.m_renderer;
	drs_debugRenderBitmapFont = config.m_renderer->CreateOrGetBitmapFontFromFile( (std::string( "Data/Fonts/" ) + config.m_fontName).c_str() );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_DebugRenderClear", Command_DebugRenderClear );
	g_theEventSystem->SubscribeEventCallbackFunction( "Command_DebugRenderToggle", Command_DebugRenderToggle );
}

void DebugRenderSystemShutdown()
{
	DebugRenderClear();
}

void DebugRenderSetVisible()
{
	drs_mutex.lock();
	drs_debugRenderIsVisible = true;
	drs_mutex.unlock();
}

void DebugRenderSetHidden()
{
	drs_mutex.lock();
	drs_debugRenderIsVisible = false;
	drs_mutex.unlock();
}

void DebugRenderClear()
{
	drs_mutex.lock();
	for (auto& obj : drs_debugRenderObjects) {
		delete obj;
		obj = nullptr;
	}
	for (auto& msg : drs_debugRenderMessages) {
		delete msg;
		msg = nullptr;
	}
	drs_mutex.unlock();
}

void DebugRenderBeginFrame()
{
	float deltaSeconds = Clock::GetSystemClock()->GetDeltaSeconds();
	for (auto& obj : drs_debugRenderObjects) {
		if (obj) {
			obj->m_liveSeconds += deltaSeconds;
			if (obj->m_liveSeconds >= obj->m_duration && obj->m_duration > -1.f) {
				// object out of lifespan
				delete obj;
				obj = nullptr;
			}
		}
	}
	for (int i = 0; i < (int)drs_debugRenderMessages.size(); i++) {
		if (drs_debugRenderMessages[i]) {
			drs_debugRenderMessages[i]->m_liveSeconds += deltaSeconds;
			if (drs_debugRenderMessages[i]->m_liveSeconds >= drs_debugRenderMessages[i]->m_duration && drs_debugRenderMessages[i]->m_duration > -1.f) {
				delete drs_debugRenderMessages[i];
				drs_debugRenderMessages.erase( drs_debugRenderMessages.begin() + i );
				i--;
			}
		}
	}
}

Rgba8 const DebugRenderGetDebugObjectCurColor( DRS_DebugRenderObject* obj, bool isXRayMode=false ) {
	if (obj->m_duration <= 0.f) {
		return obj->m_startColor;
	}
	Rgba8 res = Rgba8::Interpolate( obj->m_startColor, obj->m_endColor, obj->m_liveSeconds / obj->m_duration );
	if (isXRayMode) {
		Rgba8 retRgba8;
		retRgba8.r = res.r + 50 > 255 ? 255 : res.r + 50;
		retRgba8.g = res.g + 50 > 255 ? 255 : res.g + 50;
		retRgba8.b = res.b + 50 > 255 ? 255 : res.b + 50;
		retRgba8.a = res.a - 100 < 0 ? 0 : res.a - 100;
		return Rgba8( res.r + 50, res.g + 50, res.b + 50, res.a - 100 );
	}
	else {
		return res;
	}
}

Rgba8 const DebugRenderGetDebugObjectCurColor( DRS_ScreenMessageObject* msg ) {
	if (msg->m_duration <= 0.f) {
		return msg->m_startColor;
	}
	return Rgba8::Interpolate( msg->m_startColor, msg->m_endColor, msg->m_liveSeconds / msg->m_duration );
}

void DebugRenderWorld( Camera const& camera )
{
	drs_mutex.lock();
	if (!drs_debugRenderIsVisible) {
		drs_mutex.unlock();
		return;
	}

	drs_debugRenderConfig.m_renderer->BeginCamera( camera );
	drs_debugRenderConfig.m_renderer->SetSamplerMode( SamplerMode::POINT_CLAMP );
	drs_debugRenderConfig.m_renderer->BindShader( nullptr );
	for (auto& obj : drs_debugRenderObjects) {
		if (obj == nullptr) {
			continue;
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::Point){
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::Line) {
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::WireCylinder) {
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
			if (obj->m_isWired) {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
			}
			else {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			}
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
			}
			drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::WireSphere) {
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
			if (obj->m_isWired) {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
			}
			else {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			}
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
			}
			drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::Arrow) {
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::WorldText) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( obj->m_transform, DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( obj->m_transform, DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::BillboardText) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			if (obj->m_mode == DebugRenderMode::ALWAYS) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_OPPOSING, camera.GetTransformMatrix(), obj->m_startPos ), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
			else if (obj->m_mode == DebugRenderMode::USE_DEPTH) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_OPPOSING, camera.GetTransformMatrix(), obj->m_startPos ), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
	}

	for (auto& obj : drs_debugRenderObjects) {
		if (obj == nullptr) {
			continue;
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::Point) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::Line) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::WireCylinder) {
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			if (obj->m_isWired) {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
			}
			else {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			}
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::WireSphere) {
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			if (obj->m_isWired) {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::WIREFRAME_CULL_BACK );
			}
			else {
				drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			}
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::Arrow) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
			drs_debugRenderConfig.m_renderer->BindTexture( nullptr );
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( Mat44(), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::WorldText) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( obj->m_transform, DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( obj->m_transform, DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::BillboardText) {
			drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_NONE );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			if (obj->m_mode == DebugRenderMode::X_RAY) {
				drs_debugRenderConfig.m_renderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_OPPOSING, camera.GetTransformMatrix(), obj->m_startPos ), DebugRenderGetDebugObjectCurColor( obj, true ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::DISABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
				drs_debugRenderConfig.m_renderer->SetModelConstants( GetBillboardMatrix( BillboardType::FULL_CAMERA_OPPOSING, camera.GetTransformMatrix(), obj->m_startPos ), DebugRenderGetDebugObjectCurColor( obj ) );
				drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::OPAQUE );
				drs_debugRenderConfig.m_renderer->SetDepthMode( DepthMode::ENABLED );
				drs_debugRenderConfig.m_renderer->DrawVertexArray( obj->m_verts );
			}
		}
	}
	drs_debugRenderConfig.m_renderer->EndCamera( camera );
	drs_mutex.unlock();
}

void DebugRenderScreen( Camera const& camera )
{
	drs_mutex.lock();
	if (!drs_debugRenderIsVisible) {
		drs_mutex.unlock();
		return;
	}
	drs_debugRenderConfig.m_renderer->BeginCamera( camera );
	std::vector<Vertex_PCU> verts;
	verts.reserve( 1000 );
	drs_debugRenderConfig.m_renderer->SetRasterizerMode( RasterizerMode::SOLID_CULL_BACK );
	for (auto& obj : drs_debugRenderObjects) {
		if (obj == nullptr) {
			continue;
		}
		else if (obj->m_type == DRS_DebugRenderObjectType::ScreenText) {
			verts.clear();
			drs_debugRenderBitmapFont->AddVertsForTextInBox2D( verts, AABB2( Vec2( obj->m_startPos ), Vec2( obj->m_startPos ) + Vec2( (float)obj->m_text.size() * obj->m_textHeight * 0.618f, obj->m_textHeight ) ), obj->m_textHeight, obj->m_text, DebugRenderGetDebugObjectCurColor( obj ), 0.618f, obj->m_alignment, TextBoxMode::OVERRUN );
			drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			drs_debugRenderConfig.m_renderer->SetModelConstants();
			drs_debugRenderConfig.m_renderer->DrawVertexArray( verts );
		}
	}

	float lineHeight = (camera.m_cameraBox.m_maxs.y - camera.m_cameraBox.m_mins.y) / 40.f;
	float curHeight = camera.m_cameraBox.m_maxs.y - lineHeight;
	for (auto& msg : drs_debugRenderMessages) {
		if (msg == nullptr) {
			continue;
		}
		if (msg->m_duration == -1.f) {
			verts.clear();
			drs_debugRenderBitmapFont->AddVertsForText2D( verts, Vec2( 0.f, curHeight ), lineHeight, msg->m_text, DebugRenderGetDebugObjectCurColor( msg ), 0.618f );
			drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			drs_debugRenderConfig.m_renderer->SetModelConstants();
			drs_debugRenderConfig.m_renderer->DrawVertexArray( verts );
			curHeight -= lineHeight;
		}
	}
	for (auto& msg : drs_debugRenderMessages) {
		if (msg == nullptr) {
			continue;
		}
		if (msg->m_duration != -1.f) {
			verts.clear();
			drs_debugRenderBitmapFont->AddVertsForText2D( verts, Vec2( 0.f, curHeight ), lineHeight, msg->m_text, DebugRenderGetDebugObjectCurColor( msg ), 0.618f );
			drs_debugRenderConfig.m_renderer->SetBlendMode( BlendMode::ALPHA );
			drs_debugRenderConfig.m_renderer->BindTexture( &drs_debugRenderBitmapFont->GetTexture() );
			drs_debugRenderConfig.m_renderer->SetModelConstants();
			drs_debugRenderConfig.m_renderer->DrawVertexArray( verts );
			curHeight -= lineHeight;
		}
	}
	drs_debugRenderConfig.m_renderer->EndCamera( camera );
	drs_mutex.unlock();
}

void DebugRenderEndFrame()
{

}

void DebugRenderAddObjectToVector(DRS_DebugRenderObject* objToAdd) {
	for (auto& obj : drs_debugRenderObjects) {
		if (obj == nullptr) {
			obj = objToAdd;
			return;
		}
	}
	drs_debugRenderObjects.push_back( objToAdd );
}

void DebugAddWorldPoint( Vec3 const& pos, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::Point;
	obj->m_startPos = pos;
	obj->m_radius = radius;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	AddVertsForSphere3D( obj->m_verts, pos, radius );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddWorldLine( Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::Line;
	obj->m_startPos = start;
	obj->m_endPos = end;
	obj->m_radius = radius;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	AddVertsForCylinder3D( obj->m_verts, start, end, radius );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddWorldWireCylinder( Vec3 const& base, Vec3 const& top, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, bool isWired, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::WireCylinder;
	obj->m_startPos = base;
	obj->m_endPos = top;
	obj->m_radius = radius;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	obj->m_isWired = isWired;
	AddVertsForCylinder3D( obj->m_verts, base, top, radius );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddWorldWireSphere( Vec3 const& center, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, bool isWired, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::WireSphere;
	obj->m_startPos = center;
	obj->m_radius = radius;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	obj->m_isWired = isWired;
	AddVertsForSphere3D( obj->m_verts, center, radius );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddWorldArrow( Vec3 const& start, Vec3 const& end, float radius, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::Arrow;
	obj->m_startPos = start;
	obj->m_endPos = end;
	obj->m_radius = radius;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	AddVertsForArrow3D( obj->m_verts, start, end, radius, radius * 3.f );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddWorldText( std::string const& text, Mat44 const& transform, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::WorldText;
	obj->m_text = text;
	obj->m_transform = transform;
	obj->m_textHeight = textHeight;
	obj->m_alignment = alignment;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	drs_debugRenderBitmapFont->AddVertsForText3DAtOriginXForward( obj->m_verts, textHeight, text, Rgba8::WHITE, 0.618f, alignment );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddWorldBillboardText( std::string const& text, Vec3 const& origin, float textHeight, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH */ )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::BillboardText;
	obj->m_text = text;
	obj->m_startPos = origin;
	obj->m_textHeight = textHeight;
	obj->m_alignment = alignment;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	obj->m_mode = mode;
	drs_debugRenderBitmapFont->AddVertsForText3DAtOriginXForward( obj->m_verts, textHeight, text, Rgba8::WHITE, 0.618f, alignment );
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddScreenText( std::string const& text, Vec2 const& position, float size, Vec2 const& alignment, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	DRS_DebugRenderObject* obj = new DRS_DebugRenderObject;
	obj->m_type = DRS_DebugRenderObjectType::ScreenText;
	obj->m_startPos = position;
	obj->m_text = text;
	obj->m_textHeight = size;
	obj->m_alignment = alignment;
	obj->m_duration = duration;
	obj->m_startColor = startColor;
	obj->m_endColor = endColor;
	//drs_debugRenderBitmapFont->AddVertsForText2D( obj->m_verts, Vec2( 0.f, curHeight ), lineHeight, msg->m_text, DebugRenderGetDebugObjectCurColor( msg ), 0.618f )
	drs_mutex.lock();
	DebugRenderAddObjectToVector( obj );
	drs_mutex.unlock();
}

void DebugAddMessage( std::string const& text, float duration, Rgba8 const& startColor, Rgba8 const& endColor )
{
	DRS_ScreenMessageObject* msg = new DRS_ScreenMessageObject;
	msg->m_text = text;
	msg->m_duration = duration;
	msg->m_startColor = startColor;
	msg->m_endColor = endColor;
	drs_mutex.lock();
	drs_debugRenderMessages.push_back( msg );
	drs_mutex.unlock();
}

bool Command_DebugRenderClear( EventArgs& args )
{
	UNUSED( args );
	DebugRenderClear();
	return true;
}

bool Command_DebugRenderToggle( EventArgs& args )
{
	UNUSED( args );
	drs_debugRenderIsVisible = !drs_debugRenderIsVisible;
	return true;
}

