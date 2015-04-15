#pragma once
#include "typedefs.h"

float Distance(FCoord, FCoord);

float Angle(FCoord, FCoord);

bool PointInPoints(FCoord, FCoords);

FCoord Add(FCoord, FCoord);

FCoord Add(FCoord, float, float);

FCoord NewFCoord(float, float);