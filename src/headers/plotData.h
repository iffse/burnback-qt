#pragma once
#include <vector>
#include <QVariant>

namespace plotData
{
void generateData();
std::vector<double> isocolourData(double value);
std::vector<double> contourData();
QVariant burningAreaData();
}
