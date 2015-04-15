#include "Utility.h"
# define M_PI           3.14159265358979323846f  /* pi */

float Distance(FCoord first, FCoord second) {
	float y = fabsf(first.first - second.first);
	float x = fabsf(first.second - second.second);
	return sqrtf(x*x + y*y);
}


float Angle(FCoord first, FCoord second) {
	return std::atan2f(first.first - second.first, first.second - second.second) + M_PI;
}

FCoord Add(FCoord first, FCoord second)
{
	return std::make_pair(first.first + second.first, first.second + second.second);
}

FCoord Add(FCoord first, float y, float x)
{
	return std::make_pair(first.first + y, first.second + x);
}

FCoord NewFCoord(float y, float x)
{
	return std::make_pair(y, x);
}

bool PointInPoints(FCoord p, FCoords P)
{
	// Check bounding Box
	float minX = P[0].second;
	float maxX = P[0].second;
	float minY = P[0].first;
	float maxY = P[0].first;
	for (FCoord c : P) {
		minX = std::fminf(minX, c.second);
		maxX = std::fmaxf(maxX, c.second);
		minY = std::fminf(minY, c.first);
		maxY = std::fmaxf(maxY, c.first);
	}
	if (p.first < minX || p.first > maxX || p.second < minY || p.second > maxY)
		return false;
	/*

	// http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
	bool inside = false;
	for ( int i = 0, j = polygon.Length - 1 ; i < polygon.Length ; j = i++ )
	{
	if ( ( polygon[ i ].Y > p.Y ) != ( polygon[ j ].Y > p.Y ) &&
	p.X < ( polygon[ j ].X - polygon[ i ].X ) * ( p.Y - polygon[ i ].Y ) / ( polygon[ j ].Y - polygon[ i ].Y ) + polygon[ i ].X )
	{
	inside = !inside;
	}
	}

	return inside;
	*/
	bool isIn = false;
	for (int i = 0, j = P.size() - 1; i < P.size(); j = i++) 
	{
		if ((P[i].first > p.first) != (P[j].first > p.first) && p.second < (P[j].second - P[i].second) * (p.first - P[i].first) / (P[j].first - P[i].first) + P[i].second)
			isIn = !isIn;
	}
	return isIn;
}