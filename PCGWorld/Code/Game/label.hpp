#pragma once

#include "Game/GameCommon.hpp"

enum class LabelType {
	City, Town, Culture, Religion, Country, River, Mountain, Sea, Continent, Region, None
};

class Label {
public:
	Label(void* owner, LabelType type);
	~Label();

	void ReCalculateVertexData();
	void UpdateColor();
	Mat44 GetModelMatrix() const;
	Mat44 GetOffSetModelMatrix() const;
	void Render(Shader* shader= nullptr, Mat44 const& modelMatrix = Mat44()) const;

	bool m_enableRenderCountry = true;
	bool m_enableRender = true;
	void* m_owner = nullptr;
	LabelType m_type = LabelType::None;
	std::string m_labelText;
	Vec2 m_size = Vec2();
	Vec2 m_middlePos = Vec2();
	Vec2 m_startPos = Vec2();
	Vec2 m_endPos = Vec2();
	float m_orientation = 0.f;
	Rgba8 m_color = Rgba8( 255, 255, 0 );
	VertexBuffer* m_vertexBuffer = nullptr;
	CubicBezierCurve2D m_curve;
};