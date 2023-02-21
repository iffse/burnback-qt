#include <src/headers/operations.h>
#include <src/headers/globals.h>

#include <array>

using namespace std;

void mainLoop(){

}

// algorithm from TRIMPACK
namespace ConnectivityMatrix {

// node as function of triangle (mcnt)
void NodeTriangles() {
	connectivityMatrixNodeTriangle = array<vector<int>, 3>({vector<int>(numTriangles), vector<int>(numTriangles), vector<int>(numTriangles)});

	const uint meshDataSize = numTriangles + 4 * numEdges + numTriangles - 1;
	if (meshDataSize >= meshData.size())
		throw std::invalid_argument("Invalid mesh data. Connectivity matrix node as function of triangle (mcnt) is out of bounds");

	// helping array
	for (int i = 0; i < numTriangles; ++i) {
		meshData[meshDataHelper + i - 1] = 0;
	}

	// creation of table
	// loop over edges
	for (int i = 0; i < numEdges; ++i) {
		int index1 = 0;
		int index2 = 0;
		int node1 = meshData[numTriangles + 4 * i - 4 - 1];
		int node2 = meshData[numTriangles + 4 * i - 3 - 1];

		// nt
		int nodeT1 = meshData[numTriangles + 4 * i - 2 - 1];
		int nodeT2 = meshData[numTriangles + 4 * i - 1 - 1];

		if (nodeT1 > 0) {
			// check if node already exist in cells of mcnt(x, nodeT1)
			for (int j = 0; j < meshData[meshDataHelper + nodeT1 - 1 - 1]; ++j) {
				if (node1 == connectivityMatrixNodeTriangle[j][nodeT1 - 1]) {
					index1 = 1;
					break;
				}
				if (node2 == connectivityMatrixNodeTriangle[j][nodeT1 - 1]) {
					index2 = 1;
					break;
				}
			}

			if (index1 == 0) {
				meshData[meshDataHelper + nodeT1 - 1 - 1] = meshData[meshDataHelper + nodeT1 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[meshDataHelper + nodeT1 - 1 - 1] - 1][nodeT1 - 1] = node1;
			}
			if (index2 == 0) {
				meshData[meshDataHelper + nodeT1 - 1 - 1] = meshData[meshDataHelper + nodeT1 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[meshDataHelper + nodeT1 - 1 - 1] - 1][nodeT1 - 1] = node2;
			}
			index1 = 0;
			index2 = 0;
		}

		if (nodeT2 > 0) {
			// check if node already exist in cells of mcnt(x, nodeT2)
			for (int j = 0; j < meshData[meshDataHelper + nodeT2 - 1 - 1]; ++j) {
				if (node1 == connectivityMatrixNodeTriangle[j][nodeT2 - 1]) {
					index1 = 1;
					break;
				}
				if (node2 == connectivityMatrixNodeTriangle[j][nodeT2 - 1]) {
					index2 = 1;
					break;
				}
			}

			if (index1 == 0) {
				meshData[meshDataHelper + nodeT2 - 1 - 1] = meshData[meshDataHelper + nodeT2 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[meshDataHelper + nodeT2 - 1 - 1] - 1][nodeT2 - 1] = node1;
			}
			if (index2 == 0) {
				meshData[meshDataHelper + nodeT2 - 1 - 1] = meshData[meshDataHelper + nodeT2 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[meshDataHelper + nodeT2 - 1 - 1] - 1][nodeT2 - 1] = node2;
			}
		}
	}
}

// node as function of edge (mcnl)
void NodeEdge() {
	connectivityMatrixNodeEdge= array<vector<int>, 2>({vector<int>(numEdges), vector<int>(numEdges)});

	for (int i = 0; i < numEdges; ++i) {
		connectivityMatrixNodeEdge[0][i] = meshData[numTriangles + 4 * i - 4 - 1];
		connectivityMatrixNodeEdge[1][i] = meshData[numTriangles + 4 * i - 3 - 1];
	}
}

// triangle as function of edge (mctl)
void TriangleEdge() {
	connectivityMatrixTriangleEdge = array<vector<int>, 2>({vector<int>(numEdges), vector<int>(numEdges)});

	for (int i = 0; i < numEdges; ++i) {
		connectivityMatrixTriangleEdge[0][i] = meshData[numTriangles + 4 * i - 2 - 1];
		connectivityMatrixTriangleEdge[1][i] = meshData[numTriangles + 4 * i - 1 - 1];
	}
}

void createVertexEdge() {
	connectivityMatrixVertexEdge = array<vector<int>, 2>({vector<int>(numEdges), vector<int>(numEdges)});

	for (int i = 0; i < numEdges; ++i) {
		// Search for two identical nodes of two neighbourhood, writtin in the mcvl
		const int triangle1 = connectivityMatrixTriangleEdge[0][i];
		const int triangle2 = connectivityMatrixTriangleEdge[1][i];
		const int node1 = connectivityMatrixNodeEdge[0][i];
		const int node2 = connectivityMatrixNodeEdge[1][i];
		int node3;

		if (triangle1 > 0) {
			// ? nh
			const int neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle1 - 1];
			const int neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle1 - 1];
			const int neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle1 - 1];

			if (node1 == neighbourhood1) {
				node3 = node2 == neighbourhood2 ? neighbourhood3 : neighbourhood2;
			} else if (node1 == neighbourhood2) {
				node3 = node2 == neighbourhood1 ? neighbourhood3 : neighbourhood1;
			} else if (node1 == neighbourhood3) {
				node3 = node2 == neighbourhood1 ? neighbourhood2 : neighbourhood1;
			} else {
				throw std::runtime_error("Error in createVertexEdge(). Node1 is not equal to any of the neighbourhood nodes of triangle1.");
			}
			connectivityMatrixVertexEdge[0][i] = node3;
		} else {
			connectivityMatrixVertexEdge[0][i] = triangle1;
			// search for 2 identical nodes of 2 neighbourhood triangles
			// the adequate third node of the first and only real triangle is written in the mcvl
			const int neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle2 - 1];
			const int neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle2 - 1];
			const int neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle2 - 1];

			if (node1 == neighbourhood1) {
				node3 = node2 == neighbourhood2 ? neighbourhood3 : neighbourhood2;
			} else if (node1 == neighbourhood2) {
				node3 = node2 == neighbourhood1 ? neighbourhood3 : neighbourhood1;
			} else if (node1 == neighbourhood3) {
				node3 = node2 == neighbourhood1 ? neighbourhood2 : neighbourhood1;
			} else {
				throw std::runtime_error("Error in createVertexEdge(). Node1 is not equal to any of the neighbourhood nodes of triangle2.");
			}
			connectivityMatrixVertexEdge[1][i] = node3;
		}
		if (triangle2 > 0) {
			if (triangle1 > 0) {
				const int neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle2 - 1];
				const int neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle2 - 1];
				const int neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle2 - 1];

				if (node1 == neighbourhood1) {
					node3 = node2 == neighbourhood2 ? neighbourhood3 : neighbourhood2;
				} else if (node1 == neighbourhood2) {
					node3 = node2 == neighbourhood1 ? neighbourhood3 : neighbourhood1;
				} else if (node1 == neighbourhood3) {
					node3 = node2 == neighbourhood1 ? neighbourhood2 : neighbourhood1;
				} else {
					throw std::runtime_error("Error in createVertexEdge(). Node1 is not equal to any of the neighbourhood nodes of triangle2.");
				}
				connectivityMatrixVertexEdge[1][i] = node3;
			}
		} else {
			connectivityMatrixVertexEdge[1][i] = triangle2;
		}
	}
}

}

void reorder() {
	for (int i = 0; i < numEdges; ++i) {
		const int triangle1 = connectivityMatrixTriangleEdge[0][i];
		const int triangle2 = connectivityMatrixTriangleEdge[1][i];
		const int node1 = connectivityMatrixNodeEdge[0][i];
		const int node2 = connectivityMatrixNodeEdge[1][i];

		const int nodeVertex1 = connectivityMatrixVertexEdge[0][i];
		const int nodeVertex2 = connectivityMatrixVertexEdge[1][i];

		if (triangle1 > 0) {
			int difference1 = ( x[nodeVertex1 - 1] - x[node1 - 1] ) * ( y[node2 - 1] - y[node1 - 1] );
			int difference2 = ( x[node2 - 1] -  x[node1 - 1] ) * ( y[nodeVertex1 - 1] - y[node1 - 1] );
			if (difference1 > difference2) {
				connectivityMatrixTriangleEdge[0][i] = triangle2;
				connectivityMatrixTriangleEdge[1][i] = triangle1;
				connectivityMatrixVertexEdge[0][i] = nodeVertex2;
				connectivityMatrixVertexEdge[1][i] = nodeVertex1;
			}
		} else {
			int difference1 = ( x[nodeVertex2 - 1] - x[node1 - 1] ) * ( y[node2 - 1] - y[node1 - 1] );
			int difference2 = ( x[node2 - 1] -  x[node1 - 1] ) * ( y[nodeVertex2 - 1] - y[node1 - 1] );

			if (difference1 > difference2) {
				connectivityMatrixTriangleEdge[0][i] = triangle2;
				connectivityMatrixTriangleEdge[1][i] = triangle1;
				connectivityMatrixVertexEdge[0][i] = nodeVertex2;
				connectivityMatrixVertexEdge[1][i] = nodeVertex1;
			}
		}
	}
}

void regenerateMeshData() {
	for (int i = 0; i < numEdges; ++i) {
		meshData[numTriangleEdge + 4 * i - 4 - 1] = connectivityMatrixNodeEdge[0][i];
		meshData[numTriangleEdge + 4 * i - 3 - 1] = connectivityMatrixNodeEdge[1][i];

		meshData[numTriangleEdge + 4 * i - 2 - 1] = connectivityMatrixTriangleEdge[0][i];
		meshData[numTriangleEdge + 4 * i - 1 - 1] = connectivityMatrixTriangleEdge[1][i];
	}
}

void getBoundaryConditions() {
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

	for (int i = 0; i < numEdges; ++i) {
		int lcc = 0;
		int ic = 0;
		const int triangle1 = connectivityMatrixTriangleEdge[0][i];
		const int triangle2 = connectivityMatrixTriangleEdge[1][i];

		if (triangle1 < 0) {
			ic = - triangle1;
			lcc = connectivityMatrixBoundaryConditions[i];
		} else if (triangle2 < 0) {
			ic = - triangle2;
			lcc = connectivityMatrixBoundaryConditions[i];
		}

		if (lcc != 0) {
			int node1 = connectivityMatrixNodeEdge[0][i] - 1; // -1 because the node numbers start from 1
			int node2 = connectivityMatrixNodeEdge[1][i] - 1;

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

void setAlpha();
void setMetric();
void setduVar();
void getFlux();
void boundary();
void setdt();

void eulerExplicit();

void getError();

void setqbnd();
void burningArea();

