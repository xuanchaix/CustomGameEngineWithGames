#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>
// basic virtual visual test class
class RandomNumberGenerator;

class VisualTest {
public:
	VisualTest();
	virtual ~VisualTest();
	virtual void StartUp() = 0;
	virtual void RandomizeTest() = 0;
	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render() const = 0;
protected:
	Vec2 const GetRandomOffMapPosition();
protected:
	Camera m_camera2D;
	Camera m_camera3D;
	RandomNumberGenerator* m_randNumGen = nullptr; // random number generator of this class
};