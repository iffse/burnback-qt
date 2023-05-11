#pragma once
#include <vector>
#include <QVariant>

namespace plotData
{
void generateData();
std::vector<double> isocolourData(double value, int shiftX = 0, int shiftY = 0, double scale = 1);
std::vector<double> contourData(int shiftX = 0, int shiftY = 0, double scale = 1);
QVariant burningAreaData();
}
