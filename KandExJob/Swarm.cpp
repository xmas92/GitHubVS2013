#include "Swarm.h"
#include <cassert>
#include <algorithm>
#include <random>
#include <numeric>
#include <iostream>
#include "Utility.h"
#define LOOP for(;;)
# define M_PI           3.14159265358979323846f  /* pi */


Swarm::Swarm(Image image, int numUnits)
{
	m_image = image;
	m_numUnits = numUnits;
	if (!m_font.loadFromFile("arial.ttf")) {
		std::cerr << "Error: Could not load font \"arial.ttf\"." << std::endl;
	}
	CalculateScale();
	CreateUnits();
	SetupSeeds();
	SetupUnits();
}

void Swarm::Reset() 
{
	m_units.clear();
	CreateUnits();
	SetupSeeds();
	SetupUnits();
}

void Swarm::CalculateScale()
{
	float area;
	area = 0.f;
	for (auto row : m_image)
		for (auto pixel : row)
			if (pixel) 
				area++;
	m_scale = 2.f * std::sqrtf(area / (m_numUnits * M_PI));
}

bool Swarm::MoveLock(int id) {
	for (int i = 0; i < m_units.size(); i++) {
		if (i == id) continue;
		if (m_units[i].IsMoving())
			return true;
	}
	return false;
}

void Swarm::CreateUnits()
{
	for (int i = 0; i < m_numUnits; i++)
		m_units.push_back(Unit(i, this));
}

bool Swarm::IsInside(FCoord coord) 
{
	if (coord.first < 0 || coord.second < 0 || coord.first >= height() || coord.second >= width())
		return false;
	return m_image[std::floorf(coord.first)][std::floorf(coord.second)];
}

void Swarm::SetupSeeds()
{
	assert(m_numUnits > NUMSEEDS);
	int numPlacedSeeds = 0;

	// Find random black square
	float x, y;
	std::vector<int> Xi(width()), Yi(height());
	std::iota(Xi.begin(), Xi.end(), 0);
	std::random_shuffle(Xi.begin(), Xi.end());
	std::iota(Yi.begin(), Yi.end(), 0);
	std::random_shuffle(Yi.begin(), Yi.end());
	for (int xt : Xi) {
		for (int yt : Yi) {
			if (m_image[yt][xt]) {
				x = xt; y = yt;
				break;
			}
		}
	}

	m_units[0].SetPos(FCoord(y, x));
	numPlacedSeeds++;
	int placeUnit = 0;
	float angle = -(M_PI / 100);
	while (numPlacedSeeds < NUMSEEDS) 
	{
		angle += M_PI / 100;
		if (angle >= M_PI * 2)
			placeUnit++;
		if (placeUnit == numPlacedSeeds) {
			std::cerr << "Error: Could only place " << numPlacedSeeds << " out of " << NUMSEEDS << " seeds." << std::endl;
			break;
		}
		auto center = m_units[placeUnit].RealCoord();
		y = center.first + 1.01f * m_scale * std::sinf(angle);
		x = center.second + 1.01f * m_scale * std::cosf(angle);
		if (y < 0 || x < 0 || y >= height() || x >= width())
			continue;
		if (m_image[(int)std::floorf(y)][(int)std::floorf(x)]) {
			if (!TryCollision(numPlacedSeeds, FCoord(y, x), numPlacedSeeds)) {
				m_units[numPlacedSeeds].SetPos(FCoord(y, x));
				numPlacedSeeds++;
				angle += M_PI / 3.f;
			}
		}
	}
	for (int i = 0; i < NUMSEEDS; i++)
		m_units[i].Seed();
}

void Swarm::SetupUnits()
{
	float max = std::max(height(), width());
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-(max / 8.f), max + max / 8.f);
	int numPlacedSeeds = NUMSEEDS;
	while (numPlacedSeeds < m_numUnits) {
		FCoord pos = FCoord(dis(gen), dis(gen));
		if (!TryCollision(numPlacedSeeds, pos, numPlacedSeeds)) {
			m_units[numPlacedSeeds].SetPos(pos);
			numPlacedSeeds++;
		}
	}
}

int Swarm::height() 
{
	return m_image.size();
}

int Swarm::width() 
{
	return height() == 0 ? 0 : m_image.front().size();
}


Swarm::~Swarm()
{
}

bool Swarm::TryCollision(int id, FCoord newCoord)
{
	for (int i = 0; i < m_units.size(); i++) {
		if (id == i) continue;
		if (Distance(m_units[i].RealCoord(), newCoord) < m_scale)
			if (Distance(m_units[i].RealCoord(), newCoord) < Distance(m_units[i].RealCoord(), m_units[id].RealCoord()))
				return true;
	}
	return false;
}

bool Swarm::TryCollision(int id, FCoord newCoord, int end)
{
	for (int i = 0; i < end; i++) {
		if (id == i) continue;
		if (Distance(m_units[i].RealCoord(), newCoord) < m_scale)
			return true;
	}
	return false;
}

void Swarm::Step() 
{
	std::vector<int> I(m_numUnits);
	std::iota(I.begin(), I.end(), 0);
	std::random_shuffle(I.begin(), I.end());
	for (int i : I) {
		m_units[i].Step(this);
	}

}

void Swarm::Draw(sf::RenderWindow & window)
{
	float xMargin, yMargin;
	xMargin = window.getSize().x / 8.f;
	yMargin = window.getSize().y / 8.f;

	float squareSize = fminf((window.getSize().x - xMargin * 2) / width(), (window.getSize().y - yMargin * 2) / height());

	sf::RectangleShape rect = sf::RectangleShape(sf::Vector2f(squareSize, squareSize));
	sf::CircleShape unit = sf::CircleShape(m_scale * squareSize / 2);
	unit.setOrigin(m_scale * squareSize / 2, m_scale * squareSize / 2);
	unit.setFillColor(sf::Color::Transparent);
	unit.setOutlineThickness(2);

	sf::CircleShape angle = sf::CircleShape(2);
	angle.setFillColor(sf::Color::Blue);
	angle.setOrigin(1, 1);
	angle.setOutlineThickness(0);

	window.clear(sf::Color::White);
	for (int y = 0; y < height(); y++) {
		for (int x = 0; x < width(); x++) {
			rect.setPosition(xMargin + x*squareSize, yMargin + y*squareSize);
			if (m_image[y][x])
				rect.setFillColor(sf::Color::Black);
			else 
				rect.setFillColor(sf::Color::White);
			window.draw(rect);
		}
	}

	sf::Text gradientTxt;
	gradientTxt.setFont(m_font);
	gradientTxt.setCharacterSize(m_scale * squareSize);
	gradientTxt.setOrigin(m_scale * squareSize * .30f, m_scale * squareSize *.70f);
	gradientTxt.setColor(sf::Color::Cyan);


	sf::Text idTxt;
	idTxt.setFont(m_font);
	idTxt.setCharacterSize(m_scale * squareSize / 2.f);
	idTxt.setOrigin(m_scale * squareSize * .3f, m_scale * squareSize *.40f);
	idTxt.setColor(sf::Color::Cyan);

	for (Unit u : m_units) {

		if (m_drawGradient) {
			gradientTxt.setString(u.Gradient() > 9 ? "*" : std::to_string(u.Gradient()));
			gradientTxt.setPosition(xMargin + u.RealCoord().second * squareSize , yMargin + u.RealCoord().first* squareSize);
			window.draw(gradientTxt);
		}

		if (m_drawUnits){
			unit.setPosition(xMargin + u.RealCoord().second * squareSize, yMargin + u.RealCoord().first* squareSize);
			if (u.IsSeed())
				unit.setOutlineColor(sf::Color::Red);
			else if (u.IsDone())
				unit.setOutlineColor(sf::Color::Yellow);
			else if (u.IsMoving())
				unit.setOutlineColor(sf::Color::Green);
			else
				unit.setOutlineColor(sf::Color::Blue);
			window.draw(unit);
			if (m_drawIds) {
				idTxt.setString(std::to_string(u.Id()));
				idTxt.setPosition(xMargin + u.RealCoord().second * squareSize, yMargin + u.RealCoord().first* squareSize);
				window.draw(idTxt);
			}
		}

		if (m_drawShadow){
			unit.setPosition(xMargin + u.Coord().second * squareSize, yMargin + u.Coord().first* squareSize);
			if (u.IsSeed())
				unit.setOutlineColor(sf::Color::Red);
			else if (u.IsDone())
				unit.setOutlineColor(sf::Color::Yellow);
			else if (u.IsMoving())
				unit.setOutlineColor(sf::Color::Green);
			else
				unit.setOutlineColor(sf::Color::Blue);
			auto c = unit.getOutlineColor();
			c.a /= 2;
			unit.setOutlineColor(c);
			window.draw(unit);
			if (m_drawIds) {
				idTxt.setString(std::to_string(u.Id()));
				idTxt.setPosition(xMargin + u.Coord().second * squareSize, yMargin + u.Coord().first* squareSize);
				window.draw(idTxt);
			}
		}

		if (m_drawAngle) {
			angle.setPosition(xMargin + (u.RealCoord().second + m_scale / 2.f * std::cosf(u.GetAngle())) * squareSize,
				yMargin + (u.RealCoord().first + m_scale / 2.f * std::sinf(u.GetAngle()))* squareSize);
			window.draw(angle);
		}
	}

	window.display();
}


void Swarm::GradientReset(int id, ResetPkg gradient, bool proximity)
{
	if (proximity) 
	{
		for (int i = 0; i < m_numUnits; i++) {
			if (i == id) continue;
			if (Distance(m_units[i].RealCoord(), m_units[id].RealCoord()) < m_scale * 1.5f) {
				m_units[i].GradientReset(ResetPkg(gradient.first + 1, gradient.second + 1), this);
			}
		}
	}
	else
	{
		for (int i = 0; i < m_numUnits; i++) {
			if (i == id) continue;
			if (Signal(i,id) > 0.5f) {
				m_units[i].GradientReset(ResetPkg(gradient.first + 1, gradient.second + 1), this);
			}
		}
	}
}

std::vector<ProximityPkg> Swarm::GetProximityUnits(int id) {
	std::vector<ProximityPkg> ret;
	for (int i = 0; i < m_numUnits; i++) {
		if (i == id) continue;
		if (m_units[i].IsLocalized() && Distance(m_units[i].RealCoord(), m_units[id].RealCoord()) < m_scale * 1.1f) {
				ret.push_back(ProximityPkg(m_units[i].Coord(), GradientPkg(Angle(m_units[i].RealCoord(), m_units[id].RealCoord()), m_units[i].Gradient())));
		}
	}
	return ret;
}


void Swarm::GradientUpdate(int id, int gradient) 
{
	for (int i = 0; i < m_numUnits; i++) {
		if (i == id) continue;
		m_units[i].ReciveGradientPkg(GradientPkg(Angle(m_units[i].RealCoord(), m_units[id].RealCoord()), gradient), Signal(i, id));
	}
}

float Swarm::Signal(int first, int second) 
{
	float signal = 1.0f;
	signal -= Distance(m_units[first].RealCoord(), m_units[second].RealCoord()) / 100.0f;
	if (signal < 0) return 0;
	float angle = Angle(m_units[first].RealCoord(), m_units[second].RealCoord());
	FCoord p1 = FCoord(m_units[first].RealCoord().first + m_scale * .75f * std::sinf(angle + M_PI / 2),
		m_units[first].RealCoord().second + m_scale * .75f  * std::cosf(angle + M_PI / 2));
	FCoord p2 = FCoord(m_units[first].RealCoord().first + m_scale * .75f  * std::sinf(angle - M_PI / 2),
		m_units[first].RealCoord().second + m_scale * .75f  * std::cosf(angle - M_PI / 2));
	FCoord p3 = FCoord(m_units[second].RealCoord().first + m_scale * .75f * std::sinf(angle + M_PI / 2),
		m_units[first].RealCoord().second + m_scale * .75f  * std::cosf(angle + M_PI / 2));
	FCoord p4 = FCoord(m_units[second].RealCoord().first + m_scale * .75f * std::sinf(angle - M_PI / 2),
		m_units[first].RealCoord().second + m_scale * .75f  * std::cosf(angle - M_PI / 2));
	for (int i = 0; i < m_numUnits; i++) {
		if (i == first || i == second) continue;
		if (PointInPoints(m_units[i].RealCoord(), { p1, p2, p3, p4 })) 
			signal /= 2.f;
	}
	return signal;
}