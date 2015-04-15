#pragma once
#include "typedefs.h"

class Swarm;

class Unit
{
	enum State {
		s_Cluster,
		s_Localize,
		s_Shape,
		s_Done,
		s_Seed,
	};
	FCoord m_coord;
	FCoord m_realCoord;
	int m_id;
	Swarm * m_Swarm;
	bool m_seed = false;
	int m_step_c = 0;
	int m_localize_try = 0;
	ResetPkg m_resetPkg = ResetPkg(0, std::numeric_limits<int>::max());
	bool m_gradient_reset = false;
	State m_state = s_Cluster;

	GradientPkg m_gradientPkg;
	float m_gradientPkgSignal = 0;
	bool m_gradientPkgFlag = false;
	bool m_localized = false;

public:
	Unit(int id, Swarm * swarm) : m_id(id), m_Swarm(swarm) {};
	~Unit();

	void SetPos(FCoord);
	void Seed();
	void TryMove(float, int);

	void GradientReset(ResetPkg, Swarm *);

	void ReciveGradientPkg(GradientPkg, float);

	FCoord RealCoord() { return m_realCoord; }
	FCoord Coord() { return m_coord; }
	int Gradient();
	float GetAngle(){ return m_gradientPkg.first; }
	bool IsSeed() { return m_seed; }
	bool IsDone() { return m_state == s_Done; }
	bool IsLocalized() { return m_localized; }

	int Id() { return m_id; }
	
	void Step(Swarm * swarm);

private:
	bool RetriveGradientPkg();
};

