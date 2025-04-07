#pragma once
#include "Game/VisualTest.hpp"
#include "Game/3DTestShapeUtils.hpp"

class RayVs3DTest : public VisualTest {
public:
	RayVs3DTest();
	virtual ~RayVs3DTest();
	virtual void StartUp() override;
	virtual void RandomizeTest() override;
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

private:
	void RenderUI() const;
	std::vector<TestShape*> m_shapes;
	Vec3 m_referencePostion;
	Vec3 m_referenceForwardNormal;
	bool m_referencePosLocked = false;
	TestShape* m_rayHitShape = nullptr;
	RayCastResult3D m_thisFrameRayRes;
	TestShape* m_movingObject = nullptr;
};