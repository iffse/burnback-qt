#include <src/headers/iterations.h>
#include <src/headers/globals.h>
#include <src/headers/operations.h>
#include <src/headers/coeficcientMatrix.h>>

#include <tuple>

using namespace std;

namespace Iterations {

void subIteration() {
	setduVarriable();
	boundary();
	setdt();
	eulerExplicit();
}

tuple<double, double> mainLoop() {
	setqbnd();
	setBurningArea();

	double areaP = 0.0;
	double areaG = 0.0;

	for (uint i = 0; i < numTriangles; ++i) {
		int node1 = connectivityMatrixNodeTriangle[1 - 1][i] - 1;
		int node2 = connectivityMatrixNodeTriangle[2 - 1][i] - 1;
		int node3 = connectivityMatrixNodeTriangle[3 - 1][i] - 1;
		double yCharacteristic = (y[node1] + y[node2] + y[node3]) / 3;

		double dArea = area[i];

		if (axisymmetric)
			dArea = dArea * yCharacteristic;

		areaG = areaG + dArea;
	}

	for (int i = 0; i < numberArea - 1; ++i) {
		areaP = areaP + 0.5 * (burningArea[i] + burningArea[i+1]) * (burningWay[i+1] - burningWay[i]);
	}

	return {areaG, areaP};
}

}

