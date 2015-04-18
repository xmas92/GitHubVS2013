#include "Unit.h"
#include "Swarm.h"
#include "Utility.h"
#include <iostream>
#include <numeric>


#define MINSTEPPERCENTAGE 1

# define M_PI           3.14159265358979323846f  /* pi */

int CLUSTERLENGHT = 50;

Unit::~Unit()
{
}

void Unit::Seed() 
{
	m_seed = true;
	m_state = s_Seed;
	m_localized = true;
	m_coord = m_realCoord;
}

void Unit::SetPos(FCoord pos)
{
	m_realCoord = pos;
}

void Unit::TryMove(float angle, int precent)
{
	FCoord newPos = m_realCoord;
	float xDelta = MINSTEPPERCENTAGE / 100.f * m_Swarm->Scale() * cosf(angle);
	float yDelta = MINSTEPPERCENTAGE / 100.f * m_Swarm->Scale() * sinf(angle);
	for (int step = 0; step < precent; step += MINSTEPPERCENTAGE)
	{
		newPos = Add(newPos, yDelta, xDelta);
		if (m_Swarm->TryCollision(m_id, newPos))
			break;
		m_realCoord = newPos;
	}
}
std::pair<bool, int> Unit::TryMoveStopOnExit2(float angle, int precent)
{
	FCoord newPos = m_realCoord;
	float xDelta = MINSTEPPERCENTAGE / 100.f * m_Swarm->Scale() * cosf(angle);
	float yDelta = MINSTEPPERCENTAGE / 100.f * m_Swarm->Scale() * sinf(angle);
	for (int step = 0; step < precent; step += MINSTEPPERCENTAGE)
	{
		bool isInside = m_Swarm->IsInside(newPos);
		newPos = Add(newPos, yDelta, xDelta);
		bool willBeInside = m_Swarm->IsInside(newPos);
		if (m_Swarm->TryCollision(m_id, newPos))
			return std::make_pair(false, step);
		if (isInside && !willBeInside) {
			return std::make_pair(true, step + MINSTEPPERCENTAGE);
		}
		m_realCoord = newPos;
	}
	return std::make_pair(false, precent);
}
bool Unit::TryMoveStopOnExit(float angle, int precent)
{
	FCoord newPos = m_realCoord;
	float xDelta = MINSTEPPERCENTAGE / 100.f * m_Swarm->Scale() * cosf(angle);
	float yDelta = MINSTEPPERCENTAGE / 100.f * m_Swarm->Scale() * sinf(angle);
	for (int step = 0; step < precent; step += MINSTEPPERCENTAGE)
	{
		bool isInside = m_Swarm->IsInside(newPos);
		newPos = Add(newPos, yDelta, xDelta);
		bool willBeInside = m_Swarm->IsInside(newPos);
		if (m_Swarm->TryCollision(m_id, newPos))
			return false;
		if (isInside && !willBeInside) {
			return true;
		}
		m_realCoord = newPos;
	}
	return false;
}

void Unit::Step(Swarm * swarm)
{
	m_Swarm = swarm;

	// Reduce Signal
	m_gradientPkgSignal -= 0.1f;
	std::vector<ProximityPkg> proxPkgs;
	ProximityPkg pkg;
	float xAverage = 0, yAverage = 0;
	int highGradient = 0;
	std::pair<bool, int> res;
	std::vector<float> X, Y;
	if (m_step_c - m_Swarm->StepDone() > 200) {
		switch (m_state) {
			case s_Done:
			case s_Localize:
			case s_ShapeLocalize:
			case s_Shape:
				m_state = s_Cluster;
			case s_Seed:
				m_step_c = 0;
			case s_Cluster:
				CLUSTERLENGHT = 25;
			default:
				break;
		}
	}
	switch (m_state)
	{
	case s_Cluster:
		if (m_step_c == CLUSTERLENGHT)
			m_state = s_Localize;
		m_Swarm->GradientUpdate(m_id, Gradient());
		if (RetriveGradientPkg()) {
			TryMove(m_gradientPkg.first, 100);
		}
		break;
	case s_ShapeLocalize:
		// CHEAT FOR TESTING
		m_coord = m_realCoord;
		m_state = s_Shape;
		break;
		// CHEAT END
	case s_Localize:
		// CHEAT FOR TESTING
		m_coord = m_realCoord;
		m_localized = true;
		m_startCoord = m_coord;
		m_startStep = m_step_c;
		if (m_Swarm->IsInside(m_coord)) // Should only happen once
		{
			m_state = s_Done;
			m_Swarm->UnitDone(m_step_c);
		}
		else
			m_state = s_Shape;
		break;
		// CHEAT END
		proxPkgs = m_Swarm->GetProximityUnits(m_id);
		m_localize_try++;
		switch (proxPkgs.size()){
		case 0:
			m_localize_try--; // Not this time
			//std::cerr << "Unit(" << m_id << ") cannot be localized. No proximity unit." << std::endl;
			break;
		case 1:
			if (!m_localized && m_localize_try < Gradient() * 2) {
				break;
			}
			m_localized = true;
			TryMove(proxPkgs[0].second.first, 100);
			m_coord = FCoord(proxPkgs[0].first.first + std::sinf(proxPkgs[0].second.first) * m_Swarm->Scale(),
				proxPkgs[0].first.second + std::cosf(proxPkgs[0].second.first) * m_Swarm->Scale());
			break;
		case 2:
			if (!m_localized && m_localize_try < Gradient()) {
				break;
			}
			// Fallthrough
		default:
			m_localized = true;
			for (int i = 0; i < proxPkgs.size(); i++) {
				for (int j = 0; j < i; j++) {
					FCoord P1, P2;
					float a1, a2; // Angles to known line
					float b1, b2; // Internal Angles
					P1 = proxPkgs[i].first;
					P2 = proxPkgs[j].first;
					a1 = Angle(P1, P2);
					a2 = Angle(P2, P1);
					b1 = (a1 < proxPkgs[i].second.first) ? proxPkgs[i].second.first - a1 : a1 - proxPkgs[i].second.first;
					b2 = (a2 < proxPkgs[j].second.first) ? proxPkgs[j].second.first - a2 : a2 - proxPkgs[j].second.first;
					float l_12 = std::sqrtf((P1.first - P2.first) * (P1.first - P2.first)
						+ (P1.second - P2.second) * (P1.second - P2.second));
					float l_13 = l_12 / std::sinf(M_PI - b1 - b2) * std::sinf(b2);
					float l_23 = l_12 / std::sinf(M_PI - b1 - b2) * std::sinf(b1);
					X.push_back(P1.second + std::cosf(proxPkgs[i].second.first) * l_23);
					Y.push_back(P1.first + std::sinf(proxPkgs[i].second.first) * l_23);
				}
			}
			// Average point
			m_coord = FCoord(std::accumulate(Y.begin(), Y.end(), 0.f) / X.size(), std::accumulate(X.begin(), X.end(), 0.f) / X.size());
			break;
		}
		if (m_localized) {
			if (m_Swarm->IsInside(m_coord)) // Should only happen once
				m_state = s_Done;
			else
				m_state = s_Shape;
		}
		break;
	case s_Shape:
		// CHEAT FOR TESTING
		if (m_step_c - m_startStep > 10 && Distance(m_realCoord, m_startCoord) < m_Swarm->Scale()) {
			m_moving = false;
			m_state = s_Localize;
			break;
		}
		if (m_Swarm->MoveLock(m_id))
			break;
		// CHEAT END
		proxPkgs = m_Swarm->GetProximityUnits(m_id);
		for (auto p : proxPkgs) {
			yAverage += p.first.first;
			xAverage += p.first.second;
			if (p.second.second > highGradient) 
				highGradient = p.second.second;
		}
		yAverage /= proxPkgs.size();
		xAverage /= proxPkgs.size();
		if (highGradient <= Gradient())
			m_moving = true;
		if (!m_moving)
			break; 
		/*res = TryMoveStopOnExit2(Angle(m_realCoord, proxPkgs[0].first), 95);
		if (res.first){
			m_state = s_Done;
			break;
		}*/
		for (int i = 0, moves = 100 - res.second; moves > 0 && i < proxPkgs.size();) {
			TryMove(Angle(proxPkgs[i].first, m_realCoord), 5);
			float angle = Angle(m_realCoord, proxPkgs[i].first) - M_PI / 2.f;
			if (angle < 0) angle += M_PI * 2;
			float a = 0.1f;
			do {
				res = TryMoveStopOnExit2((angle + a < 0) ? angle + a + M_PI * 2 : angle + a, 5);
				a -= .01f;
			} while (res.second == 0 && a > -0.1f);
			if (res.second == 0)
				i++;
			else
			{
				bool before = res.first;
				moves -= res.second;
				res = TryMoveStopOnExit2(Angle(m_realCoord, proxPkgs[i].first), 100);
				if (res.first || (res.second == 0 && before) || (before && m_Swarm->IsInside(m_realCoord))) {
					m_state = s_Done;
					m_Swarm->UnitDone(m_step_c);
					break;
				}
				moves -= res.second;
				proxPkgs = m_Swarm->GetProximityUnits(m_id);
				i = 0;
			}
		}

		/*
		if (TryMoveStopOnExit(0.0, 100)) {
			m_state = s_Done;
			break;
		}
		if (TryMoveStopOnExit(0.0, 100)) {
			m_state = s_Done;
			break;
		}
		*/
		if (m_state != s_Done)
			m_state = s_ShapeLocalize;
		break;
	case s_Seed:
		m_Swarm->GradientUpdate(m_id, Gradient());
		if (m_id == 0) {
			if (m_step_c == CLUSTERLENGHT) {
				GradientReset(ResetPkg(0, 0), m_Swarm);
				//m_Swarm->GradientReset(0, ResetPkg(0, 0), true);
			}
		}
		break;
	case s_Done:
		break;
	default:
		break;
	}
	// Step Counter
	m_step_c++;
}

bool Unit::IsMoving()
{
	switch (m_state)
	{
	case s_Cluster:
		return true;
	case s_Seed:
	case s_Done:
	case s_Localize:
		return false;
	case s_ShapeLocalize:
	case s_Shape:
		return m_moving;
	default:
		return false;
	}
}

int Unit::Gradient() 
{
	switch (m_state)
	{
	case s_Cluster:
		return m_gradientPkg.second + 1;
	case s_Done:
		return 0;
	case s_Seed:
	case s_ShapeLocalize:
	case s_Localize:
	case s_Shape:
		return m_resetPkg.first;
	default:
		return -1;
	}
}

void Unit::GradientReset(ResetPkg gradient, Swarm * swarm) 
{
	if (gradient.second <= m_resetPkg.second || gradient.first <= m_resetPkg.first) {
		m_resetPkg = gradient;
		m_gradient_reset = true;
		swarm->GradientReset(m_id, gradient, true);
	}
}

void Unit::ReciveGradientPkg(GradientPkg pkg, float signal) 
{
	if (!m_gradientPkgFlag) {
		m_gradientPkg = pkg;
		m_gradientPkgSignal = signal;
		m_gradientPkgFlag = true;
		return;
	}
	if (signal > m_gradientPkgSignal) {
		m_gradientPkg = pkg;
		m_gradientPkgSignal = signal;
	}
	else if (signal > m_gradientPkgSignal - 0.20f && pkg.second <= m_gradientPkg.second) {
		m_gradientPkg = pkg;
		m_gradientPkgSignal = signal;
	}
}

bool Unit::RetriveGradientPkg() 
{
	if (m_gradientPkgFlag) {
		return true;
	}
	return false;
}