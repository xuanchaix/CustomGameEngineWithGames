#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
class Window;

#define DX_SAFE_RELEASE(dxObject)	\
	if((dxObject) != nullptr){		\
		(dxObject)->Release();		\
		(dxObject) = nullptr;		\
	}								\

#if defined OPAQUE
#undef OPAQUE
#endif // OPAQUE

extern const char* defaultShaderSource;
extern const char* shadowDepthMapShaderSource;
extern const char* blurUpShaderSource;
extern const char* blurDownShaderSource;
extern const char* blurCompositeShaderSource;

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE,
	COUNT,
};

enum class SamplerMode {
	POINT_CLAMP,
	BILINEAR_WRAP,
	BILINEAR_CLAMP,
	COUNT,
};

enum class RasterizerMode {
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	SOLID_CULL_FRONT,
	COUNT,
};

enum class DepthMode {
	DISABLED,
	ENABLED,
	COUNT
};

enum class VertexType {
	PCU,
	PCUTBN,
	PCU_SEPARATED,
	PCUN_SEPARATED,
};

enum class ShadowMode
{
	DISABLE,
	POINT_SHADOW,
	COUNT,
};

enum class BitmapFontType {
	FntType,
	PNGTextureType,
};

struct RendererConfig {
	Window* m_window;
	bool m_emissiveEnabled = false;
};


struct CameraConstants {
	Mat44 m_viewMatrix;
	Mat44 m_projectionMatrix;
};

struct LightConstants {
	Vec3 m_lightPosition;
	float m_ambient;
	Mat44 m_lightViewMatrix;
	Mat44 m_lightProjectionMatrix;
};

struct ModelConstants {
	Mat44 m_modelMatrix;
	float m_modelColorR;
	float m_modelColorG;
	float m_modelColorB;
	float m_modelColorA;
};

struct LightingDebug {
	int m_renderAmbient = true;
	int m_renderDiffuse = true;
	int m_renderSpecular = true;
	int m_renderEmissive = true;
	int m_useDiffuseMap = true;
	int m_useNormalMap = true;
	int m_useSpecularMap = true;
	int m_useGlossinessMap = true;
	int m_useEmissiveMap = true;
	float padding[3];
};

struct DirectionalLightConstants {
	Vec3 m_sunDirection;
	float m_sunIntensity;
	float m_ambientIntensity;
	Vec3 m_worldEyePosition;

	float m_minFalloff = 0.f;
	float m_maxFalloff = 0.1f;
	float m_minFalloffMultiplier = 0.f;
	float m_maxFalloffMultiplier = 1.f;

	LightingDebug m_lightingDebug;
};

struct BlurSample {
	Vec2 m_offset;
	float m_weight;
	float m_padding;
};

constexpr int k_blurMaxSamples = 64;

struct BlurConstants {
	Vec2 m_texelSize;
	float m_lerpT;
	int m_numSamples;
	BlurSample m_samples[k_blurMaxSamples];
};