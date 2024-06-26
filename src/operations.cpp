#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include <src/headers/operations.h>
#include <src/headers/globals.h>

#include <array>
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

enum diffusiveMethods {
	TriangleBased,
	EdgeBased
};


void regenerateMeshData() {
	for (uint edge = 0; edge < numEdges; ++edge) {
		edgeData[edge][0] = connectivityMatrixNodeEdge[0][edge];
		edgeData[edge][1] = connectivityMatrixNodeEdge[1][edge];
		edgeData[edge][2] = connectivityMatrixTriangleEdge[0][edge];
		edgeData[edge][3] = connectivityMatrixTriangleEdge[1][edge];
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
			condition = connectivityMatrixBoundaryConditions[boundary];
		} else if (triangle2 < 0) {
			boundary = - triangle2;
			condition = connectivityMatrixBoundaryConditions[boundary];
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

void setAngles() {
	sector = vector<double>(numNodes);
	thetaEdge.fill(vector<double>(numEdges));
	betaEdge.fill(vector<double>(numEdges));
	if (diffusiveMethod == TriangleBased) {
		normalEdge.fill(vector<double>(numEdges));
		directionEdge.fill(vector<double>(0));
	} else {
		normalEdge.fill(vector<double>(0));
		directionEdge.fill(vector<double>(numEdges));
	}

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

		if (diffusiveMethod != TriangleBased) {
			directionEdge[0][edge] = vertexX1;
			directionEdge[1][edge] = vertexY1;
		}

		auto bisection = [](double x1, double y1, double x2, double y2) {
			auto x = (x1 + x2) / 2;
			auto y = (y1 + y2) / 2;
			auto v = abs(complex<double>(x, y));

			return make_pair(x / v, y / v);
		};

		if (nodeVertex1 >= 0) {
			auto vertexX2 = x[nodeVertex1] - x[node1];
			auto vertexY2 = y[nodeVertex1] - y[node1];
			auto vertexV2 = abs(complex<double>(vertexX2, vertexY2));
			auto vertexX5 = x[nodeVertex1] - x[node2];
			auto vertexY5 = y[nodeVertex1] - y[node2];
			auto vertexV5 = abs(complex<double>(vertexX5, vertexY5));

			// calculate the angle between two edges
			// the angle is calculated by the dot product of the two vectors
			// vertexX1 * vertexX2 + vertexY1 * vertexY2 = cos(alpha)

			vertexX2 /= vertexV2;
			vertexY2 /= vertexV2;
			vertexX5 /= vertexV5;
			vertexY5 /= vertexV5;

			thetaEdge[0][edge] = acos(vertexX1 * vertexX2 + vertexY1 * vertexY2);
			thetaEdge[2][edge] = acos(-vertexX1 * vertexX5 - vertexY1 * vertexY5);
			if (diffusiveMethod == TriangleBased) {
				auto normal1 = bisection(vertexX1, vertexY1, vertexX2, vertexY2);
				auto normal2 = bisection(- vertexX1, - vertexY1, vertexX5, vertexY5);

				normalEdge[0][edge] = normal1.first;
				normalEdge[1][edge] = normal1.second;
				normalEdge[4][edge] = normal2.first;
				normalEdge[5][edge] = normal2.second;

			}
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

			thetaEdge[1][edge] = acos(vertexX1 * vertexX3 + vertexY1 * vertexY3);
			thetaEdge[3][edge] = acos(-vertexX1 * vertexX4 - vertexY1 * vertexY4);

			if (diffusiveMethod == TriangleBased) {
				auto normal1 = bisection(vertexX1, vertexY1, vertexX3, vertexY3);
				auto normal2 = bisection(- vertexX1, - vertexY1, vertexX4, vertexY4);

				normalEdge[2][edge] = normal1.first;
				normalEdge[3][edge] = normal1.second;
				normalEdge[6][edge] = normal2.first;
				normalEdge[7][edge] = normal2.second;
			}
		}

		// setting angular sector of each node
		sector[node1] += thetaEdge[0][edge] + thetaEdge[1][edge];
		sector[node2] += thetaEdge[2][edge] + thetaEdge[3][edge];

		// setting the correct values of alpha 1, 2 ,3 and 4 in the boundaries
		// -1 due to the substraction of 1 for indexing
		if (nodeVertex1 < -1) {
			thetaEdge[0][edge] = thetaEdge[1][edge];
			thetaEdge[2][edge] = thetaEdge[3][edge];
		}
		if (nodeVertex2 < -1) {
			thetaEdge[1][edge] = thetaEdge[0][edge];
			thetaEdge[3][edge] = thetaEdge[2][edge];
		}

		if (diffusiveMethod == TriangleBased) {
			betaEdge[0][edge] = sin(thetaEdge[0][edge] / 2);
			betaEdge[1][edge] = sin(thetaEdge[2][edge] / 2);
		} else {
			betaEdge[0][edge] = tan(thetaEdge[0][edge] / 2) + tan(thetaEdge[1][edge] / 2);
			betaEdge[1][edge] = tan(thetaEdge[2][edge] / 2) + tan(thetaEdge[3][edge] / 2);
		}
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

		auto surface = [](double x1, double y1, double x2, double y2, double x3, double y3) {
			return 0.5 * abs(x2 * y3 + x3 * y1 + x1 * y2 - x3 * y2 - x1 * y3 - x2 * y1);
		};
		area[triangle] = surface( x[node1], y[node1], x[node2], y[node2], x[node3], y[node3] );

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

		// reciprocal of the cross product
		auto rr = 1 / (y31 * x21 - y21 * x31);
		duVariable[triangle][0] = rr * (u21 * y31 - u31 * y21);
		duVariable[triangle][1] = rr * (u31 * x21 - u21 * x31);
	}
}

void setFlux() {
	flux[1] = vector<double>(numNodes);
	duVertex = vector<array<double, 2>>(numNodes);

	for (uint edge = 0; edge < numEdges; ++edge) {
		const auto triangle1 = connectivityMatrixTriangleEdge[0][edge] - 1;
		const auto triangle2 = connectivityMatrixTriangleEdge[1][edge] - 1;
		const auto node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		const auto node2 = connectivityMatrixNodeEdge[1][edge] - 1;

		if (triangle1 >= 0) {
			duVertex[node1][0] += thetaEdge[0][edge] * duVariable[triangle1][0];
			duVertex[node1][1] += thetaEdge[0][edge] * duVariable[triangle1][1];
			duVertex[node2][0] += thetaEdge[2][edge] * duVariable[triangle1][0];
			duVertex[node2][1] += thetaEdge[2][edge] * duVariable[triangle1][1];
		}

		if (triangle2 >= 0) {
			duVertex[node1][0] += thetaEdge[1][edge] * duVariable[triangle2][0];
			duVertex[node1][1] += thetaEdge[1][edge] * duVariable[triangle2][1];
			duVertex[node2][0] += thetaEdge[3][edge] * duVariable[triangle2][0];
			duVertex[node2][1] += thetaEdge[3][edge] * duVariable[triangle2][1];
		}
	}

	for (uint node = 0; node < numNodes; ++node) {
		duVertex[node][0] /= sector[node];
		duVertex[node][1] /= sector[node];
		duVertex[node][0] /= 2;
		duVertex[node][1] /= 2;

		const auto &condition = nodeBoundaryConditions[0][node];
		const auto &boundary = nodeBoundaryConditions[1][node];
		if (condition == symmetry || condition == symmetry2 || condition == outletSymmetry) {
			auto duVer = duVertex[node][0] * cos(uBoundaryData[boundary]) + duVertex[node][1] * sin(uBoundaryData[boundary]);
			duVertex[node][0] = duVer * cos(uBoundaryData[boundary]);
			duVertex[node][1] = duVer * sin(uBoundaryData[boundary]);
		}

	}

	for (uint edge = 0; edge < numEdges; ++edge) {
		const auto triangle1 = connectivityMatrixTriangleEdge[0][edge] - 1;
		const auto triangle2 = connectivityMatrixTriangleEdge[1][edge] - 1;
		const auto node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		const auto node2 = connectivityMatrixNodeEdge[1][edge] - 1;

		if (diffusiveMethod == TriangleBased) {
			if (triangle1 >= 0) {
				auto &dux = duVariable[triangle1][0];
				auto &duy = duVariable[triangle1][1];

				auto &normal1x = normalEdge[0][edge];
				auto &normal1y = normalEdge[1][edge];
				auto &normal2x = normalEdge[4][edge];
				auto &normal2y = normalEdge[5][edge];

				flux[1][node1] += ((dux - duVertex[node1][0]) * normal1x + (duy - duVertex[node1][1]) * normal1y) * betaEdge[0][edge];
				flux[1][node2] += ((dux - duVertex[node2][0]) * normal2x + (duy - duVertex[node2][1]) * normal2y) * betaEdge[1][edge];
			}

			if (triangle2 >= 0) {
				auto &dux = duVariable[triangle2][0];
				auto &duy = duVariable[triangle2][1];

				auto &normal1x = normalEdge[2][edge];
				auto &normal1y = normalEdge[3][edge];
				auto &normal2x = normalEdge[6][edge];
				auto &normal2y = normalEdge[7][edge];

				flux[1][node1] += ((dux - duVertex[node1][0]) * normal1x + (duy - duVertex[node1][1]) * normal1y) * betaEdge[0][edge];
				flux[1][node2] += ((dux - duVertex[node2][0]) * normal2x + (duy - duVertex[node2][1]) * normal2y) * betaEdge[1][edge];
			}
			continue;
		}

		// Uj and Uj+1 * nj+1/2
		auto dux = 0.0;
		auto duy = 0.0;

		if (triangle1 >= 0) {
			dux += duVariable[triangle1][0];
			duy += duVariable[triangle1][1];
		}
		if (triangle2 >= 0) {
			dux += duVariable[triangle2][0];
			duy += duVariable[triangle2][1];
		}

		dux /= 2;
		duy /= 2;

		auto dux1 = (dux - duVertex[node1][0]) * directionEdge[0][edge];
		auto duy1 = (duy - duVertex[node1][1]) * directionEdge[1][edge];

		auto dux2 = (dux - duVertex[node2][0]) * directionEdge[0][edge];
		auto duy2 = (duy - duVertex[node2][1]) * directionEdge[1][edge];

		flux[1][node1] += betaEdge[0][edge] * (dux1 + duy1);
		flux[1][node2] -= betaEdge[1][edge] * (dux2 + duy2);
	}

	for (uint node = 0; node < numNodes; ++node) {
		flux[1][node] /= 2 * M_PI;
	}

}

void computeRecession() {
	for (uint node = 0; node < numNodes; ++node) {
		array<array<double, 1>, 2> flowDirection = {{
			{duVertex[node][0]},
			{duVertex[node][1]}
		}};
		auto &matrix = recessionMatrix[node];
		auto effectiveRecession = matrixMultiplication(matrix, flowDirection);
		recession[node] = sqrt(pow(effectiveRecession[0][0], 2) + pow(effectiveRecession[1][0], 2));
	}
}

void boundaryFlux() {
	for (uint node = 0; node < numNodes; ++node) {
		const auto &condition = nodeBoundaryConditions[0][node];

		switch (condition) {
			// not a boundary
			case none: {
				flux[0][node] = 1 - recession[node] * abs(complex<double>(duVertex[node][0], duVertex[node][1]));
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
				flux[0][node] = 1 - recession[node] * abs(complex<double>(duVertex[node][0], duVertex[node][1]));
				break;
			}
			// symmetryBoundaries
			case symmetry: case symmetry2: {
				flux[0][node] = 1 - recession[node] * abs(complex<double>(duVertex[node][0], duVertex[node][1]));
				break;
			}
			case outletSymmetry: {
				flux[0][node] = 1 - recession[node] * abs(complex<double>(duVertex[node][0], duVertex[node][1]));
			}
			default:
				break;

		}
	}
}

void eulerExplicit() {
	const auto &minHeight = *min_element(height.begin(), height.end());
	const auto dtMin = cfl * minHeight / maxRecession;

	for (uint node = 0; node < numNodes; ++node) {
		auto diffWeight = diffusiveWeight * recession[node];
		uVertex[node] += dtMin * (flux[0][node] + diffWeight * flux[1][node]);
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

void setBoundary() {
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
	burningTime = vector<double>(numberArea);
	if (numberArea == 0)
		return;

	auto epsilon = 0.001;
	auto uMin = *min_element(uVertex.begin(), uVertex.end());
	auto uMax = *max_element(uVertex.begin(), uVertex.end());

	uMin += (uMax - uMin) * epsilon;
	uMax -= (uMax - uMin) * epsilon;

	uint orderedNode1;
	uint orderedNode2;
	uint orderedNode3;

	for (uint area = 0; area < numberArea; ++area)
		burningTime[area] = uMin + area * (uMax - uMin) / (numberArea);

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
			auto &uCut = burningTime[area];

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

void calculateAnisotropicMatrix() {
	recessionMatrix = vector<array<array<double, 2>, 2>>(numNodes);
	for (uint node = 0; node < numNodes; ++node) {
		auto &recession = recessionAnisotropic[node];
		auto &recession1 = recession[0];
		auto &recession2 = recession[1];
		auto rotation = recession[2] * M_PI / 180;

		array<array<double, 2>, 2> rotationMatrix = {{
			{cos(rotation), -sin(rotation)},
			{sin(rotation), cos(rotation)}
		}};

		array<array<double, 2>, 2> rec = {{
			{recession1, 0},
			{0, recession2}
		}};

		auto rotationMatrixT = matrixTranspose(rotationMatrix);

		auto _op = matrixMultiplication(rotationMatrixT, rec);
		recessionMatrix[node] = matrixMultiplication(_op, rotationMatrix);
	}
}

double getMaxRecession() {
	double maxRecession = 0;
	if (anisotropic) {
		for (auto &recession : recessionAnisotropic) {
			auto &recession1 = recession[0];
			auto &recession2 = recession[1];

			maxRecession = max(maxRecession, max(recession1, recession2));
		}
	} else {
		maxRecession = *max_element(recession.begin(), recession.end());
	}
	return maxRecession;
}
