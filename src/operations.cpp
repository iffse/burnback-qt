#include "src/headers/operations.h"
#include <src/headers/globals.h>

using namespace std;

void resetGlobals() {
	// meshData.fill(0);
	// x.fill(0);
	// y.fill(0);
	// connectivityMatrixEdges.fill(0);
	meshData = vector<int>(65 * numNodes);
	x = vector<double>(numNodes);
	y = vector<double>(numNodes);
	connectivityMatrixEdges = vector<double>(numBoundaries);
	ubData = array<vector<double>, 4>({vector<double>(numNodes), vector<double>(numNodes), vector<double>(numNodes), vector<double>(numNodes)});
	uInit.fill(0);
}

namespace ConnectivityMatrix {

void Triangles() { // mcnt

}

}

