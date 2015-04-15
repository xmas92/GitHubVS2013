#pragma once
#include <vector>

typedef std::vector<std::vector<bool>> Image;
typedef std::pair<int, int> Coord;
typedef std::vector<Coord> Coords;
typedef std::pair<float, float> FCoord;
typedef std::vector<FCoord> FCoords;
typedef std::pair<int, int> ResetPkg;
typedef std::pair<float, int> GradientPkg;
typedef std::pair<FCoord, GradientPkg> ProximityPkg;