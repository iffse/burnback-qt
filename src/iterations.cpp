#include <src/headers/iterations.h>
#include <src/headers/globals.h>
#include <src/headers/operations.h>
#include <src/headers/coeficcientMatrix.h>

#include <tuple>

using namespace std;

namespace Iterations {

void subIteration() {
	setduVarriable();
	setFlux();
	boundaryFlux();
	setdt();
	eulerExplicit();
}

tuple<double, double> mainLoop() {
	double areaP = 0.0;
	double areaG = 0.0;

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		int node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		int node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		int node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;
		double yCharacteristic = (y[node1] + y[node2] + y[node3]) / 3;

		double dArea = area[triangle];

		if (axisymmetric)
			dArea *= yCharacteristic;

		areaG += dArea;
	}

	for (uint area = 0; area < numberArea - 1; ++area)
		areaP += 0.5 * (burningArea[area] + burningArea[area + 1]) * (burningWay[area + 1] - burningWay[area]);

	return {areaG, areaP};
}

}

