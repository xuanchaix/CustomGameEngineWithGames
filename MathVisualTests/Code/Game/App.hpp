#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Game/GameCommon.hpp"
class VisualTest;

enum class MathVisualTestName {
	POINT_VS_SHAPES, 
	RAY_CAST_VS_DISCS, 
	RAY_CAST_VS_LINE_SEGS,
	RAY_CAST_VS_AABB2S,
	RAY_CAST_VS_3D,
	CURVE_2D,
	Pachinko_Machine_2D,
	NUM };

//-------------8.23.2023 class App--------------

class App {
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsQuitting() const { return m_isQuitting; }
	bool IsDebugMode() const { return m_debugMode; }

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();

	void HandleKey();
	void GoToNextTest();
	void GoToPreviousTest();
	void StartNewTest();

	static bool SetQuitting( EventArgs& args );
	
private:
	VisualTest* m_curVisualTest = nullptr;
	MathVisualTestName m_curTestName = MathVisualTestName::RAY_CAST_VS_3D;
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_pauseAfterUpdate = false;
	bool m_debugMode = false;

	double m_timeStart;
	double m_timeEnd;
};