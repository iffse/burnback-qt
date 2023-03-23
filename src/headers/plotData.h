#pragma once
#include <vector>
#include <QVariant>

namespace plotData
{
void generateData();
std::vector<double> isocolourData(double value, uint shiftX = 0, uint shiftY = 0, double scale = 1);
std::vector<double> contourData(uint shiftX = 0, uint shiftY = 0, double scale = 1);
QVariant burningAreaData();
}
