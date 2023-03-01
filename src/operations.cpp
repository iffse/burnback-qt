#include <src/headers/operations.h>
#include <src/headers/globals.h>
#include <QDebug>

#include <array>
#include <cmath>

using namespace std;

// node as function of triangle (mcnt)
void regenerateMeshData() {
	for (uint edge = 0; edge < numEdges; ++edge) {
		meshData[numTriangleEdge - 1 + 4 * edge] = connectivityMatrixNodeEdge[0][edge];
		meshData[numTriangleEdge - 1 + 4 * edge + 1] = connectivityMatrixNodeEdge[1][edge];

		meshData[numTriangleEdge - 1 + 4 * edge + 2] = connectivityMatrixTriangleEdge[0][edge];
		meshData[numTriangleEdge - 1 + 4 * edge + 3] = connectivityMatrixTriangleEdge[1][edge];
	}
}

void setBoundaryConditions() {
	// std::map<std::string, int> boundaryConditions = {
	// 	{"inlet", 1},
	// 	{"outlet", 2},
	// 	{"symmetry", 3},
	// 	{"inlet-outlet", 12},
	// 	{"inlet-symmetry", 13},
	// 	{"outlet-symmetry", 23}
	// };
	enum boundaryConditions {
		inlet = 1,
		outlet = 2,
		symmetry = 3,
		inletOutlet = 12,
		inletSymmetry = 13,
		outletSymmetry = 23
	};

	nodeBoundaryConditions = array<vector<int>, 2> {vector<int>(numNodes), vector<int>(numNodes)};

	for (uint edge = 0; edge < numEdges; ++edge) {
		int lcc = 0;
		int ic = 0;
		const int triangle1 = connectivityMatrixTriangleEdge[0][edge];
		const int triangle2 = connectivityMatrixTriangleEdge[1][edge];

		if (triangle1 < 0) {
			ic = - triangle1;
			lcc = connectivityMatrixBoundaryConditions[ic - 1];
		} else if (triangle2 < 0) {
			ic = - triangle2;
			lcc = connectivityMatrixBoundaryConditions[ic - 1];
		}

		if (lcc != 0) {
			int node1 = connectivityMatrixNodeEdge[0][edge] - 1; // -1 because the node numbers start from 1
			int node2 = connectivityMatrixNodeEdge[1][edge] - 1;

			if (nodeBoundaryConditions[0][node1] == 0){
				nodeBoundaryConditions[0][node1] = lcc;
				nodeBoundaryConditions[1][node1] = ic;
			} else if (nodeBoundaryConditions[0][node1] != lcc) {
				switch (nodeBoundaryConditions[0][node1]) {
					case inlet:
						if (lcc == outlet)
							nodeBoundaryConditions[0][node1] = inletOutlet;
						else if (lcc == symmetry)
							nodeBoundaryConditions[0][node1] = inletSymmetry;
					break;

					case outlet:
						if (lcc == inlet)
							nodeBoundaryConditions[0][node1] = inletOutlet;
						else if (lcc == symmetry)
							nodeBoundaryConditions[0][node1] = outletSymmetry;

						nodeBoundaryConditions[1][node1] = ic;
					break;

					case symmetry:
						if (lcc == inlet) {
							nodeBoundaryConditions[0][node1] = inletSymmetry;
							nodeBoundaryConditions[1][node1] = ic;
						} else if (lcc == outlet)
						nodeBoundaryConditions[0][node1] = outletSymmetry;

					break;

					default:
					break;

				}
			}

			if (nodeBoundaryConditions[0][node2] == 0) {
				nodeBoundaryConditions[0][node2] = lcc;
				nodeBoundaryConditions[1][node2] = ic;
			} else if (nodeBoundaryConditions[0][node2] != lcc) {
				switch (nodeBoundaryConditions[0][node2]) {
					case inlet:
						if (lcc == outlet)
							nodeBoundaryConditions[0][node2] = inletOutlet;
						else if (lcc == symmetry)
							nodeBoundaryConditions[0][node2] = inletSymmetry;
					break;

					case outlet:
						if (lcc == inlet)
							nodeBoundaryConditions[0][node2] = inletOutlet;
						else if (lcc == symmetry)
							nodeBoundaryConditions[0][node2] = outletSymmetry;

						nodeBoundaryConditions[1][node2] = ic;
					break;

					case symmetry:
						if (lcc == inlet) {
							nodeBoundaryConditions[0][node2] = inletSymmetry;
							nodeBoundaryConditions[1][node2] = ic;
						} else if (lcc == outlet)
							nodeBoundaryConditions[0][node2] = outletSymmetry;

					break;

					default:
					break;

				}
			}
		}
	}
}

void setAlpha() {
	sector = vector<double>(numNodes);
	alpha.fill(vector<double>(numEdges));

	for (uint edge = 0; edge < numEdges; ++edge) {
		int node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		int node2 = connectivityMatrixNodeEdge[1][edge] - 1;
		int nodeVertex1 = connectivityMatrixVertexEdge[0][edge] - 1;
		int nodeVertex2 = connectivityMatrixVertexEdge[1][edge] - 1;
		double vertexX1 = x[node2] - x[node1];
		double vertexY1 = y[node2] - y[node1];
		double vertexV1 = sqrt(pow(vertexX1, 2) + pow(vertexY1, 2));

		vertexX1 = vertexX1 / vertexV1;
		vertexY1 = vertexY1 / vertexV1;

		alpha[7 - 1][edge] = vertexX1;
		alpha[8 - 1][edge] = vertexY1;

		if (nodeVertex1 >= 0) {
			double vertexX2 = x[nodeVertex1] - x[node1];
			double vertexY2 = y[nodeVertex1] - y[node1];
			double vertexV2 = sqrt(pow(vertexX2, 2) + pow(vertexY2, 2));
			double vertexX5 = x[nodeVertex1] - x[node2];
			double vertexY5 = y[nodeVertex1] - y[node2];
			double vertexV5 = sqrt(pow(vertexX5, 2) + pow(vertexY5, 2));

			vertexX2 = vertexX2 / vertexV2;
			vertexY2 = vertexY2 / vertexV2;
			vertexX5 = vertexX5 / vertexV5;
			vertexY5 = vertexY5 / vertexV5;

			alpha[1 - 1][edge] = acos(vertexX1 * vertexX2 + vertexY1 * vertexY2);
			alpha[3 - 1][edge] = acos(- vertexX1 * vertexX5 - vertexY1 * vertexY5);
		}

		if (nodeVertex2 >= 0) {
			double vertexX3 = x[nodeVertex2] - x[node1];
			double vertexY3 = y[nodeVertex2] - y[node1];
			double vertexV3 = sqrt(pow(vertexX3, 2) + pow(vertexY3, 2));
			double vertexX4 = x[nodeVertex2] - x[node2];
			double vertexY4 = y[nodeVertex2] - y[node2];
			double vertexV4 = sqrt(pow(vertexX4, 2) + pow(vertexY4, 2));

			vertexX3 = vertexX3 / vertexV3;
			vertexY3 = vertexY3 / vertexV3;
			vertexX4 = vertexX4 / vertexV4;
			vertexY4 = vertexY4 / vertexV4;

			alpha[2 - 1][edge] = acos(vertexX1 * vertexX3 + vertexY1 * vertexY3);
			alpha[4 - 1][edge] = acos(- vertexX1 * vertexX4 - vertexY1 * vertexY4);

		}

		// setting angular sector of each node
		sector[node1] = sector[node1] + alpha[1 - 1][edge] + alpha[2 - 1][edge];
		sector[node2] = sector[node2] + alpha[3 - 1][edge] + alpha[4 - 1][edge];

		// setting the correct values of alpha 1, 2 ,3 and 4 in the boundaries
		if (nodeVertex1 < 0) {
			alpha[1 - 1][edge] = alpha[2 - 1][edge];
			alpha[3 - 1][edge] = alpha[4 - 1][edge];
		}
		if (nodeVertex2 < 0) {
			alpha[2 - 1][edge] = alpha[1 - 1][edge];
			alpha[4 - 1][edge] = alpha[3 - 1][edge];
		}

		// beta and beta prime
		alpha[5 - 1][edge] = tan( 1.0/2 * alpha[1 - 1][edge] ) + tan( 1.0/2 * alpha[2 - 1][edge] );
		alpha[6 - 1][edge] = tan( 1.0/2 * alpha[3 - 1][edge] ) + tan( 1.0/2 * alpha[4 - 1][edge] );

	}

	// udate angular sectors
	for (uint node = 0; node < numNodes; ++node) {
		sector[node] = 1.0/2 * sector[node];
	}
}

// what is this
double sup(double x1, double y1, double x2, double y2, double x3, double y3) {
	return 1.0/2 * abs(x2 * y3 + x3 * y1 + x1 * y2 - x3 * y2 - x1 * y3 - x2 * y1);
}

void setMetric() {
	area = vector<double>(numTriangles);
	height = vector<double>(numTriangles);

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		int node1 = connectivityMatrixNodeTriangle[1 - 1][triangle] - 1;
		int node2 = connectivityMatrixNodeTriangle[2 - 1][triangle] - 1;
		int node3 = connectivityMatrixNodeTriangle[3 - 1][triangle] - 1;

		double vertexX1 = x[node2] - x[node1];
		double vertexY1 = y[node2] - y[node1];
		double vertexV1 = sqrt(pow(vertexX1, 2) + pow(vertexY1, 2));
		double vertexX2 = x[node3] - x[node1];
		double vertexY2 = y[node3] - y[node1];
		double vertexV2 = sqrt(pow(vertexX2, 2) + pow(vertexY2, 2));
		double vertexX3 = x[node3] - x[node1];
		double vertexY3 = y[node3] - y[node1];
		double vertexV3 = sqrt(pow(vertexX3, 2) + pow(vertexY3, 2));

		area[triangle] = sup( x[node1], y[node1], x[node2], y[node2], x[node3], y[node3] );

		// max of the three vertices
		double vvm;
		vvm = max(max(vertexV1, vertexV2), vertexV3);

		height[triangle] = area[triangle] / vvm;
	}
}

void setduVarriable() {
	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		int node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		int node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		int node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

		double u21 = uVertex[node2] - uVertex[node1];
		double u31 = uVertex[node3] - uVertex[node1];

		double x21 = x[node2] - x[node1];
		double x31 = x[node3] - x[node1];
		double y21 = y[node2] - y[node1];
		double y31 = y[node3] - y[node1];

		double rr = 1.0 / (y31 * x21 - y21 * x31);
		duVariable[0][triangle] = rr * (u21 * y31 - u31 * y21);
		duVariable[1][triangle] = rr * (u31 * x21 - u21 * x31);
	}
}
void setFlux() {
	flux[1] = vector<double>(numNodes, 0);
	duVertex.fill(vector<double>(numNodes, 0));
	eps = vector<double>(numNodes, 0);

	for (uint edge = 0; edge < numEdges; ++edge) {
		int triangle1 = connectivityMatrixTriangleEdge[0][edge] - 1;
		int triangle2 = connectivityMatrixTriangleEdge[1][edge] - 1;
		int node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		int node2 = connectivityMatrixNodeEdge[1][edge] - 1;
		
		double dux = 0.0;
		double duy = 0.0;

		if (triangle1 >= 0) {
			duVertex[0][node1] += alpha[0][edge] * duVariable[0][triangle1];
			duVertex[1][node1] += alpha[0][edge] * duVariable[1][triangle1];
			duVertex[0][node2] += alpha[2][edge] * duVariable[0][triangle1];
			duVertex[1][node2] += alpha[2][edge] * duVariable[1][triangle1];

			dux += duVariable[0][triangle1];
			duy += duVariable[1][triangle1];

			double duMod = 1 + sqrt(pow(duVariable[0][triangle1], 2) + pow(duVariable[1][triangle1], 2));
			if (duMod > eps[node1])
				eps[node1] = duMod;
			// Why not node2 ?
			if (duMod > eps[node1])
				eps[node2] = duMod;
		}

		if (triangle2 >= 0) {
			duVertex[0][node1] += alpha[1][edge] * duVariable[0][triangle2];
			duVertex[1][node1] += alpha[1][edge] * duVariable[1][triangle2];
			duVertex[0][node2] += alpha[3][edge] * duVariable[0][triangle2];
			duVertex[1][node2] += alpha[3][edge] * duVariable[1][triangle2];

			dux += duVariable[0][triangle2];
			duy += duVariable[1][triangle2];
			// why not triangle2 ?
			double duMod = 1 + sqrt(pow(duVariable[0][triangle1], 2) + pow(duVariable[1][triangle1], 2));

			if (duMod > eps[node1])
				eps[node1] = duMod;
			if (duMod > eps[node1])
				eps[node2] = duMod;

		}

		// why du of both triangle?
		dux *= 1.0/2 * alpha[6][edge];
		duy *= 1.0/2 * alpha[7][edge];

		flux[1][node1] += alpha[4][edge] * (dux + duy);
		flux[1][node2] -= alpha[5][edge] * (dux + duy);
	}

	for (uint i = 0; i < numNodes; ++i) {
		eps[i] = eps[i] / M_PI;
		flux[1][i] *= eps[i];
		duVertex[0][i] *= 0.5;
		duVertex[1][i] *= 0.5;
	}

}
void boundary() {
	for (uint i = 0; i < numNodes; ++i) {
		int boundary = nodeBoundaryConditions[0][i];
		int condition = nodeBoundaryConditions[1][i];

		duVertex[0][i] = duVertex[0][i] / sector[i];
		duVertex[1][i] = duVertex[1][i] / sector[i];

		if (boundary == 0) {
			flux[0][i] = 1 - sqrt( pow(duVertex[0][i], 2) + pow(duVertex[1][i], 2) );
		} else {
			if (boundary == 1 || boundary == 12 || boundary == 13) {
		  		// source boundary
				flux[0][i] = 0.0;
				flux[1][0] = 0.0;
			}

			if (boundary == 2) {
	  			// free boundary
				flux[0][i] = 1 - sqrt( pow(duVertex[0][i], 2) + pow(duVertex[1][i], 2) );
			}

			if (boundary == 3 || boundary == 23) {
				// symmetry boundary
				double ubNorm = sqrt( pow(uBoundaryData[1 - 1][condition], 2) + pow(uBoundaryData[2 - 1][condition], 2));
				double duVer = duVertex[0][i] * uBoundaryData[1 - 1][condition] + duVertex[1][i] * uBoundaryData[1][condition];
				duVertex[0][i] = duVer * uBoundaryData[0][condition] / ubNorm;
				duVertex[1][i] = duVer * uBoundaryData[1][condition] / ubNorm;
				flux[0][i] = 1 - sqrt( pow(duVertex[0][i], 2) + pow(duVertex[1][i], 2) );
				flux[1][i] = 2 * flux[0][i];
			}
		}
	}
}
void setdt() {
	for (uint i = 0; i < numNodes; ++i) {
		dt[i] = 1.570796 * cfl * height[i] / eps[i];
	}
}

void eulerExplicit() {
	double dtmin = dt[0];

	for (uint i = 0; i < numNodes; ++i) {
		if (dtmin > dt[i]) 
			dtmin = dt[i];
	}

	for (uint i = 0; i < numNodes; ++i)
		uVertex[i] = uVertex[i] + dtmin * (flux[0][i] + elipch * flux[1][i]);
	
	timeTotal = timeTotal + dtmin;
}

double getError() {
	double error = 0.0;
	for (uint i = 0; i < numNodes; ++i)
		error = error + pow(flux[0][i], 2);

	error = sqrt(error / numNodes);
	return error;
}

void setqbnd() {
	connectivityMatrixNodeBoundary = array<vector<int>, 2>({vector<int>(numTriangles), vector<int>(numTriangles)});

	numBoundarySides = 0;
	for (uint i = 0; i < numEdges; ++i) {
		int triangle1 = connectivityMatrixTriangleEdge[0][i];
		int triangle2 = connectivityMatrixTriangleEdge[1][i];

		if (triangle1 < 0 || triangle2 < 0) {
			numBoundarySides = numBoundarySides + 1;
			connectivityMatrixNodeBoundary[0][numBoundarySides] = connectivityMatrixNodeEdge[0][numBoundarySides];
			connectivityMatrixNodeBoundary[1][numBoundarySides] = connectivityMatrixNodeEdge[1][numBoundarySides];
		}
	}
}

void setBurningArea() {
	burningArea = vector<double>(numberArea);
	burningWay = vector<double>(numberArea);
	double epsilon = 0.001;
	double uMin = uVertex[0];
	double uMax = uVertex[0];

	double orderedNode1;
	double orderedNode2;
	double orderedNode3;

	for (uint i = 0; i < numNodes; ++i) {
		if (uMin > uVertex[i])
			uMin = uVertex[i];
		if (uMax < uVertex[i])
			uMax = uVertex[i];
	}

	uMin = uMin + (uMax - uMin) * epsilon;
	uMax = uMax - (uMax - uMin) * epsilon;

	for (uint area = 0; area < numberArea; ++area) {
		burningArea[area] = 0.0;
		double uCut = uMin + (area-1) * (uMax - uMin) / numberArea;
		burningWay[area] = uCut;
	}

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		double node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		double node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		double node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

		if (uVertex[node1] > uVertex[node2]) {
			if (uVertex[node1] > uVertex[node3]) {
				orderedNode1 = node1;
				if (uVertex[node2] > uVertex[node3]) {
					orderedNode2 = node2;
					orderedNode3 = node3;
				} else {
					orderedNode2 = node3;
					orderedNode3 = node2;
				}
			} else {
				orderedNode1 = node3;
				orderedNode2 = node1;
				orderedNode3 = node2;
			}
		} else {
			if (uVertex[node1] > uVertex[node3]) {
				orderedNode1 = node2;
				orderedNode2 = node1;
				orderedNode3 = node3;
			} else {
				orderedNode3 = node1;
				if (uVertex[node2] > uVertex[node3]) {
					orderedNode1 = node2;
					orderedNode2 = node3;
				} else {
					orderedNode1 = node3;
					orderedNode2 = node2;
				}
			}
		}

		for (uint area = 0; area < numberArea; ++area) {
			double uCut = burningWay[area];

			if (uCut < uVertex[node1] && uCut > uVertex[node3]) {
				double factor;
				double xCut1;
				double xCut2;
				double yCut1;
				double yCut2;

				if (uCut > uVertex[orderedNode2]) {
					factor = (uCut - uVertex[orderedNode2]) / (uVertex[orderedNode1] - uVertex[orderedNode2]);
					xCut1 = x[orderedNode2] + (x[orderedNode1] - x[orderedNode2]) * factor;
					yCut1 = y[orderedNode2] + (y[orderedNode1] - y[orderedNode2]) * factor;
				} else {
					factor = (uCut - uVertex[orderedNode3]) / (uVertex[orderedNode2] - uVertex[orderedNode3]);
					xCut1 = x[orderedNode3] + (x[orderedNode2] - x[orderedNode3]) * factor;
					yCut1 = y[orderedNode3] + (y[orderedNode2] - y[orderedNode3]) * factor;
				}
				factor = (uCut - uVertex[orderedNode3]) / (uVertex[orderedNode1] - uVertex[orderedNode3]);
				xCut2 = x[orderedNode3] + (x[orderedNode1] - x[orderedNode3]) * factor;
				yCut2 = y[orderedNode3] + (y[orderedNode1] - y[orderedNode3]) * factor;
				double distance = sqrt(pow(xCut1 - xCut2, 2) + pow(yCut1 - yCut2, 2));

				if (axisymmetric)
					distance = distance * 1.0/2 * (yCut1 + yCut2);

				burningArea[area] = burningArea[area] + distance;
			}
		}
	}
	


}
