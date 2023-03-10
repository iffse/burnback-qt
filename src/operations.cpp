#include <src/headers/operations.h>
#include <src/headers/globals.h>
#include <QDebug>

#include <array>
#include <cmath>
#include <complex>
#include <algorithm>

using namespace std;

enum boundaryConditions {
	none = 0,
	inlet = 1,
	outlet = 2,
	symmetry = 3,
	inletOutlet = 12,
	inletSymmetry = 13,
	outletSymmetry = 23,
	symmetry2 = 4,
	source2 = 6,
	free2 = 7
};


void regenerateMeshData() {
	for (uint edge = 0; edge < numEdges; ++edge) {
		meshData[numTriangleEdge - 1 + 4 * edge] = connectivityMatrixNodeEdge[0][edge];
		meshData[numTriangleEdge - 1 + 4 * edge + 1] = connectivityMatrixNodeEdge[1][edge];

		meshData[numTriangleEdge - 1 + 4 * edge + 2] = connectivityMatrixTriangleEdge[0][edge];
		meshData[numTriangleEdge - 1 + 4 * edge + 3] = connectivityMatrixTriangleEdge[1][edge];
	}
}

void setBoundaryConditions() {
	nodeBoundaryConditions = array<vector<int>, 2> {vector<int>(numNodes), vector<int>(numNodes)};

	for (uint edge = 0; edge < numEdges; ++edge) {
		auto condition = 0;
		auto boundary = 0;
		auto &triangle1 = connectivityMatrixTriangleEdge[0][edge];
		auto &triangle2 = connectivityMatrixTriangleEdge[1][edge];

		if (triangle1 < 0) {
			boundary = - triangle1;
			condition = connectivityMatrixBoundaryConditions[boundary - 1];
		} else if (triangle2 < 0) {
			boundary = - triangle2;
			condition = connectivityMatrixBoundaryConditions[boundary - 1];
		}

		if (condition != 0) {
			auto setCondition = [&](int node) {
				switch (nodeBoundaryConditions[0][node]) {
					case none:
						nodeBoundaryConditions[0][node] = condition;
						nodeBoundaryConditions[1][node] = boundary;
					break;

					case inlet: case source2:
						if (condition == outlet || condition == free2)
							nodeBoundaryConditions[0][node] = inletOutlet;
						else if (condition == symmetry || condition == symmetry2)
							nodeBoundaryConditions[0][node] = inletSymmetry;
						break;

					case outlet: case free2:
						if (condition == inlet || condition == source2)
							nodeBoundaryConditions[0][node] = inletOutlet;
						else if (condition == symmetry || condition == symmetry2)
							nodeBoundaryConditions[0][node] = outletSymmetry;

						nodeBoundaryConditions[1][node] = boundary;
						break;

					case symmetry: case symmetry2:
						if (condition == inlet || condition == source2) {
							nodeBoundaryConditions[0][node] = inletSymmetry;
							nodeBoundaryConditions[1][node] = boundary;
						} else if (condition == outlet || condition == free2)
							nodeBoundaryConditions[0][node] = outletSymmetry;
						break;

					default:
						break;

				}
			};

			auto node1 = connectivityMatrixNodeEdge[0][edge] - 1; // -1 because the node numbers start from 1
			auto node2 = connectivityMatrixNodeEdge[1][edge] - 1;

			setCondition(node1);
			setCondition(node2);
		}
	}
}

void setAlpha() {
	sector = vector<double>(numNodes);
	alpha.fill(vector<double>(numEdges));

	for (uint edge = 0; edge < numEdges; ++edge) {
		auto node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		auto node2 = connectivityMatrixNodeEdge[1][edge] - 1;
		auto nodeVertex1 = connectivityMatrixVertexEdge[0][edge] - 1;
		auto nodeVertex2 = connectivityMatrixVertexEdge[1][edge] - 1;
		auto vertexX1 = x[node2] - x[node1];
		auto vertexY1 = y[node2] - y[node1];

		auto vertexV1 = abs(complex<double>(vertexX1, vertexY1));

		vertexX1 /= vertexV1;
		vertexY1 /= vertexV1;

		alpha[6][edge] = vertexX1;
		alpha[7][edge] = vertexY1;

		if (nodeVertex1 >= 0) {
			auto vertexX2 = x[nodeVertex1] - x[node1];
			auto vertexY2 = y[nodeVertex1] - y[node1];
			auto vertexV2 = abs(complex<double>(vertexX2, vertexY2));
			auto vertexX5 = x[nodeVertex1] - x[node2];
			auto vertexY5 = y[nodeVertex1] - y[node2];
			auto vertexV5 = abs(complex<double>(vertexX5, vertexY5));

			vertexX2 /= vertexV2;
			vertexY2 /= vertexV2;
			vertexX5 /= vertexV5;
			vertexY5 /= vertexV5;

			alpha[0][edge] = acos(vertexX1 * vertexX2 + vertexY1 * vertexY2);
			alpha[2][edge] = acos(-vertexX1 * vertexX5 - vertexY1 * vertexY5);
		}

		if (nodeVertex2 >= 0) {
			auto vertexX3 = x[nodeVertex2] - x[node1];
			auto vertexY3 = y[nodeVertex2] - y[node1];
			auto vertexV3 = abs(complex<double>(vertexX3, vertexY3));
			auto vertexX4 = x[nodeVertex2] - x[node2];
			auto vertexY4 = y[nodeVertex2] - y[node2];
			auto vertexV4 = abs(complex<double>(vertexX4, vertexY4));

			vertexX3 /= vertexV3;
			vertexY3 /= vertexV3;
			vertexX4 /= vertexV4;
			vertexY4 /= vertexV4;

			alpha[1][edge] = acos(vertexX1 * vertexX3 + vertexY1 * vertexY3);
			alpha[3][edge] = acos(-vertexX1 * vertexX4 - vertexY1 * vertexY4);
		}

		// setting angular sector of each node
		sector[node1] += alpha[0][edge] + alpha[1][edge];
		sector[node2] += alpha[2][edge] + alpha[3][edge];

		// setting the correct values of alpha 1, 2 ,3 and 4 in the boundaries
		// -1 due to the substraction of 1 for indexing
		if (nodeVertex1 < -1) {
			alpha[0][edge] = alpha[1][edge];
			alpha[2][edge] = alpha[3][edge];
		}
		if (nodeVertex2 < -1) {
			alpha[1][edge] = alpha[0][edge];
			alpha[3][edge] = alpha[2][edge];
		}

		// beta and beta prime
		alpha[4][edge] = tan( 0.5 * alpha[0][edge] ) + tan( 0.5 * alpha[1][edge] );
		alpha[5][edge] = tan( 0.5 * alpha[2][edge] ) + tan( 0.5 * alpha[3][edge] );

	}

	// update angular sectors
	for_each(sector.begin(), sector.end(), [](double& sector) { sector /= 2; });
}

void setMetric() {
	area = vector<double>(numTriangles);
	height = vector<double>(numTriangles);

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		auto node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		auto node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		auto node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

		auto vertexX1 = x[node2] - x[node1];
		auto vertexY1 = y[node2] - y[node1];
		auto vertexX2 = x[node2] - x[node3];
		auto vertexY2 = y[node2] - y[node3];
		auto vertexX3 = x[node3] - x[node1];
		auto vertexY3 = y[node3] - y[node1];

		auto vertexV1 = abs(complex<double>(vertexX1, vertexY1));
		auto vertexV2 = abs(complex<double>(vertexX2, vertexY2));
		auto vertexV3 = abs(complex<double>(vertexX3, vertexY3));

		auto sup = [](double x1, double y1, double x2, double y2, double x3, double y3) {
			return 0.5 * abs(x2 * y3 + x3 * y1 + x1 * y2 - x3 * y2 - x1 * y3 - x2 * y1);
		};
		area[triangle] = sup( x[node1], y[node1], x[node2], y[node2], x[node3], y[node3] );

		// max of the three vertices
		auto vertexVMax = max(max(vertexV1, vertexV2), vertexV3);

		height[triangle] = area[triangle] / vertexVMax;
	}
}

void setduVarriable() {
	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		auto node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		auto node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		auto node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

		auto u21 = uVertex[node2] - uVertex[node1];
		auto u31 = uVertex[node3] - uVertex[node1];

		auto x21 = x[node2] - x[node1];
		auto x31 = x[node3] - x[node1];
		auto y21 = y[node2] - y[node1];
		auto y31 = y[node3] - y[node1];

		auto rr = 1 / (y31 * x21 - y21 * x31);
		duVariable[0][triangle] = rr * (u21 * y31 - u31 * y21);
		duVariable[1][triangle] = rr * (u31 * x21 - u21 * x31);
	}
}

void setFlux() {
	flux[1] = vector<double>(numNodes);
	duVertex.fill(vector<double>(numNodes));
	eps = vector<double>(numNodes);

	for (uint edge = 0; edge < numEdges; ++edge) {
		const auto triangle1 = connectivityMatrixTriangleEdge[0][edge] - 1;
		const auto triangle2 = connectivityMatrixTriangleEdge[1][edge] - 1;
		const auto node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		const auto node2 = connectivityMatrixNodeEdge[1][edge] - 1;

		auto dux = 0.0;
		auto duy = 0.0;

		if (triangle1 >= 0) {
			duVertex[0][node1] += alpha[0][edge] * duVariable[0][triangle1];
			duVertex[1][node1] += alpha[0][edge] * duVariable[1][triangle1];
			duVertex[0][node2] += alpha[2][edge] * duVariable[0][triangle1];
			duVertex[1][node2] += alpha[2][edge] * duVariable[1][triangle1];

			dux += duVariable[0][triangle1];
			duy += duVariable[1][triangle1];

			auto duMod = 1 + abs(complex<double>(duVariable[0][triangle1], duVariable[1][triangle1]));
			if (duMod > eps[node1])
				eps[node1] = duMod;
			if (duMod > eps[node2])
				eps[node2] = duMod;
		}

		if (triangle2 >= 0) {
			duVertex[0][node1] += alpha[1][edge] * duVariable[0][triangle2];
			duVertex[1][node1] += alpha[1][edge] * duVariable[1][triangle2];
			duVertex[0][node2] += alpha[3][edge] * duVariable[0][triangle2];
			duVertex[1][node2] += alpha[3][edge] * duVariable[1][triangle2];

			dux += duVariable[0][triangle2];
			duy += duVariable[1][triangle2];

			// ?
			auto duMod = 1 + abs(complex<double>(duVariable[0][triangle1], duVariable[1][triangle1]));
			if (duMod > eps[node1])
				eps[node1] = duMod;
			if (duMod > eps[node2])
				eps[node2] = duMod;

		}

		dux *= 0.5 * alpha[6][edge];
		duy *= 0.5 * alpha[7][edge];

		flux[1][node1] += alpha[4][edge] * (dux + duy);
		flux[1][node2] -= alpha[5][edge] * (dux + duy);
	}

	for (uint node = 0; node < numNodes; ++node) {
		eps[node] /= M_PI;
		flux[1][node] *= eps[node];
		duVertex[0][node] *= 0.5;
		duVertex[1][node] *= 0.5;
	}

}

void boundary() {
	for (uint node = 0; node < numNodes; ++node) {
		const auto &condition = nodeBoundaryConditions[0][node];
		const auto boundary = nodeBoundaryConditions[1][node] - 1;

		duVertex[0][node] /= sector[node];
		duVertex[1][node] /= sector[node];

		switch (condition) {
			// not a boundary
			case none: {
				flux[0][node] = 1 - abs(complex<double>(duVertex[0][node], duVertex[1][node]));
			break;
			}
			// sourceBoundaries
			case inlet: case inletOutlet: case inletSymmetry: case source2: {
				flux[0][node] = 0;
				flux[1][node] = 0;
				break;
			}
			// freeBoundaries
			case outlet: case free2: {
				flux[0][node] = 1 - abs(complex<double>(duVertex[0][node], duVertex[1][node]));
				flux[1][node] *= 2;
				break;
			}
			// symmetryBoundaries
			case symmetry: case outletSymmetry: case symmetry2: {
				auto ubNorm = abs(complex<double>(uBoundaryData[0][boundary], uBoundaryData[1][boundary]));
				auto duVer = duVertex[0][node] * uBoundaryData[0][boundary] + duVertex[1][node] * uBoundaryData[1][boundary];

				duVertex[0][node] = duVer * uBoundaryData[0][boundary] / ubNorm;
				duVertex[1][node] = duVer * uBoundaryData[1][boundary] / ubNorm;
				flux[0][node] = 1 - abs(complex<double>(duVertex[0][node], duVertex[1][node]));
				flux[1][node] *= 2;
				break;
			}
			default:
				break;

		}
	}
}

void setdt() {
	for (uint node = 0; node < numNodes; ++node) {
		dt[node] = 1.570796 * cfl * height[node] / eps[node];
	}
}

void eulerExplicit() {
	const auto &dtMin = *min_element(dt.begin(), dt.end());

	for (uint node = 0; node < numNodes; ++node) {
		uVertex[node] += dtMin * (flux[0][node] + viscosity * flux[1][node]);
		if (uVertex[node] < 0)
			qDebug() << "uVertex < 0";
	}


	timeTotal += dtMin;
}

double getError() {
	auto error = 0.0;
	for (uint node = 0; node < numNodes; ++node)
		error += pow(flux[0][node], 2);

	error = sqrt(error) / numNodes;
	return error;
}

void setqbnd() {
	connectivityMatrixNodeBoundary = array<vector<int>, 2>({vector<int>(numTriangles), vector<int>(numTriangles)});

	numBoundaryEdge = 0;
	for (uint i = 0; i < numEdges; ++i) {
		auto &triangle1 = connectivityMatrixTriangleEdge[0][i];
		auto &triangle2 = connectivityMatrixTriangleEdge[1][i];

		if (triangle1 < 0 || triangle2 < 0) {
			connectivityMatrixNodeBoundary[0][numBoundaryEdge] = connectivityMatrixNodeEdge[0][numBoundaryEdge];
			connectivityMatrixNodeBoundary[1][numBoundaryEdge] = connectivityMatrixNodeEdge[1][numBoundaryEdge];
			++numBoundaryEdge;
		}
	}
}

void setBurningArea() {
	burningArea = vector<double>(numberArea);
	burningWay = vector<double>(numberArea);
	auto epsilon = 0.001;
	auto uMin = *min_element(uVertex.begin(), uVertex.end());
	auto uMax = *max_element(uVertex.begin(), uVertex.end());

	uMin += (uMax - uMin) * epsilon;
	uMax -= (uMax - uMin) * epsilon;

	double orderedNode1;
	double orderedNode2;
	double orderedNode3;

	for (uint area = 0; area < numberArea; ++area) {
		auto uCut = uMin + area * (uMax - uMin) / (numberArea - 1);
		burningWay[area] = uCut;
	}

	for (uint triangle = 0; triangle < numTriangles; ++triangle) {
		auto node1 = connectivityMatrixNodeTriangle[0][triangle] - 1;
		auto node2 = connectivityMatrixNodeTriangle[1][triangle] - 1;
		auto node3 = connectivityMatrixNodeTriangle[2][triangle] - 1;

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
			auto &uCut = burningWay[area];

			if (uCut < uVertex[orderedNode1] && uCut > uVertex[orderedNode3]) {
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

				auto distance = abs(complex<double>(xCut1 - xCut2, yCut1 - yCut2));


				if (axisymmetric)
					distance *= 0.5 * (yCut1 + yCut2);

				burningArea[area] += distance;
			}
		}
	}

}
