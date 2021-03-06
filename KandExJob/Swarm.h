#pragma once
#include <vector>
#include <list>
#include <SFML\Graphics.hpp>
#include "Unit.h"
#include "typedefs.h"

#define NUMSEEDS 4
class Swarm
{
	std::vector<Unit> m_units;
	float m_scale;
	int m_numUnits;
	Image m_image;
	FCoords m_centers;
	bool m_drawGradient = false;
	bool m_drawAngle = false;
	bool m_drawShadow = false;
	bool m_drawUnits = true;
	bool m_drawIds = false;
	sf::Font m_font;
	int m_lastDoneStep;
public:
	Swarm(Image, int);
	~Swarm();

	void Step();
	void Draw(sf::RenderWindow&);
	void GradientToggle() {
		m_drawGradient = !m_drawGradient;
	};
	void AngleToggle() {
		m_drawAngle = !m_drawAngle;
	};
	void ShadowToggle() {
		m_drawShadow = !m_drawShadow;
	};
	void UnitsToggle() {
		m_drawUnits = !m_drawUnits;
	};
	void IdsToggle() {
		m_drawIds = !m_drawIds;
	};

	float Scale() { return m_scale; }
	void Reset();
	bool TryCollision(int, FCoord);
	bool TryCollision(int, FCoord, int);

	bool IsInside(FCoord);

	bool MoveLock(int);

	void UnitDone(int step) { m_lastDoneStep = step; }
	int StepDone() { return m_lastDoneStep; }

	float Signal(int, int);


	void GradientUpdate(int, int);
	void GradientReset(int, ResetPkg, bool);

	std::vector<ProximityPkg> GetProximityUnits(int);
private:
	void CalculateScale();
	void CreateUnits();
	void SetupSeeds();
	void SetupUnits();
	int width();
	int height();
};

