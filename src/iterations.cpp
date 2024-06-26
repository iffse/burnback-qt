#include <src/headers/iterations.h>
#include <src/headers/globals.h>
#include <src/headers/operations.h>
#include <src/headers/connectivityMatrix.h>

#include <tuple>

using namespace std;

namespace Iterations {

void subIteration() {
	setduVarriable();
	setFlux();
	if (anisotropic)
		computeRecession();
	boundaryFlux();
	eulerExplicit();
}

tuple<double, double> mainLoop() {
	double areaGeometric = 0.0;

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		int node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		int node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		int node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

		double cellArea = area[triangle];

		if (axisymmetric) {
			double yCharacteristic = (y[node1] + y[node2] + y[node3]) / 3;
			cellArea *= yCharacteristic;
		}

		areaGeometric += cellArea;
	}

	if (numberArea == 0)
		return {areaGeometric, 0};

	double areaMDF = 0.0;
	for (uint area = 0; area < numberArea - 1; ++area)
		areaMDF += 0.5 * (burningArea[area] + burningArea[area + 1]) * (burningTime[area + 1] - burningTime[area]);

	return {areaGeometric, areaMDF};
}
}

