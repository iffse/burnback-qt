#include <src/headers/coeficcientMatrix.h>
#include <src/headers/globals.h>

#include <array>
#include <vector>

using namespace std;

// algorithm from TRIMPACK
namespace ConnectivityMatrix {

void NodeTriangles() {
	connectivityMatrixNodeTriangle = array<vector<int>, 3>({vector<int>(numTriangles), vector<int>(numTriangles), vector<int>(numTriangles)});

	vector<int> meshDataHelper = vector<int>(numTriangles, 0);

	// creation of table
	// loop over edges
	for (uint i = 0; i < numEdges; ++i) {
		int index1 = 0;
		int index2 = 0;
		int node1 = edgeData[0][i];
		int node2 = edgeData[1][i];

		// nt
		int triangle1 = edgeData[2][i];
		int triangle2 = edgeData[3][i];

		if (triangle1 > 0) {
			// check if node already exist in cells of mcnt(x, nodeT1)
			for (int j = 0; j < meshDataHelper[triangle1 - 1]; ++j) {
				if (node1 == connectivityMatrixNodeTriangle[j][triangle1 - 1])
					index1 = 1;
				if (node2 == connectivityMatrixNodeTriangle[j][triangle1 - 1])
					index2 = 1;
				if (index1 == 1 && index2 == 1)
					break;
			}

			if (index1 == 0) {
				int &axis = ++ meshDataHelper[triangle1 - 1];
				connectivityMatrixNodeTriangle[axis - 1][triangle1 - 1] = node1;
			}
			if (index2 == 0) {
				int &axis = ++ meshDataHelper[triangle1 - 1];
				connectivityMatrixNodeTriangle[axis - 1][triangle1 - 1] = node2;
			}
			index1 = 0;
			index2 = 0;
		}

		if (triangle2 > 0) {
			// check if node already exist in cells of mcnt(x, nodeT2)
			for (int j = 0; j < meshDataHelper[triangle2 - 1]; ++j) {
				if (node1 == connectivityMatrixNodeTriangle[j][triangle2 - 1])
					index1 = 1;
				if (node2 == connectivityMatrixNodeTriangle[j][triangle2 - 1])
					index2 = 1;
				if (index1 == 1 && index2 == 1)
					break;
			}

			if (index1 == 0) {
				int &axis = ++ meshDataHelper[triangle2 - 1];
				connectivityMatrixNodeTriangle[axis - 1][triangle2 - 1] = node1;
			}
			if (index2 == 0) {
				int &axis = ++ meshDataHelper[triangle2 - 1];
				connectivityMatrixNodeTriangle[axis - 1][triangle2 - 1] = node2;
			}
		}
	}
}

// node as function of edge (mcnl)
void NodeEdge() {
	connectivityMatrixNodeEdge = array<vector<int>, 2>({vector<int>(numEdges), vector<int>(numEdges)});

	for (uint edge = 0; edge < numEdges; ++edge) {
		connectivityMatrixNodeEdge[0][edge] = edgeData[0][edge];
		connectivityMatrixNodeEdge[1][edge] = edgeData[1][edge];
	}
}

// triangle as function of edge (mctl)
void TriangleEdge() {
	connectivityMatrixTriangleEdge = array<vector<int>, 2>({vector<int>(numEdges), vector<int>(numEdges)});

	for (uint edge = 0; edge < numEdges; ++edge) {
		connectivityMatrixTriangleEdge[0][edge] = edgeData[2][edge];
		connectivityMatrixTriangleEdge[1][edge] = edgeData[3][edge];
	}
}

void createVertexEdge() {
	connectivityMatrixVertexEdge = array<vector<int>, 2>({vector<int>(numEdges), vector<int>(numEdges)});

	for (uint edge = 0; edge < numEdges; ++edge) {
		// Search for two identical nodes of two neighbourhood
		const int &triangle1 = connectivityMatrixTriangleEdge[0][edge];
		const int &triangle2 = connectivityMatrixTriangleEdge[1][edge];
		const int &node1 = connectivityMatrixNodeEdge[0][edge];
		const int &node2 = connectivityMatrixNodeEdge[1][edge];
		int node3;

		if (triangle1 > 0) {
			const int &neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle1 - 1];
			const int &neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle1 - 1];
			const int &neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle1 - 1];

			if (node1 == neighbourhood1) {
				node3 = node2 == neighbourhood2 ? neighbourhood3 : neighbourhood2;
			} else if (node1 == neighbourhood2) {
				node3 = node2 == neighbourhood1 ? neighbourhood3 : neighbourhood1;
			} else if (node1 == neighbourhood3) {
				node3 = node2 == neighbourhood1 ? neighbourhood2 : neighbourhood1;
			} else {
				throw std::runtime_error("Error in createVertexEdge(). Node1 is not equal to any of the neighbourhood nodes of triangle1.");
			}
			connectivityMatrixVertexEdge[0][edge] = node3;
		} else {
			connectivityMatrixVertexEdge[0][edge] = triangle1;
			// search for 2 identical nodes of 2 neighbourhood triangles
			// the adequate third node of the first and only real triangle is written in the mcvl
			const int &neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle2 - 1];
			const int &neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle2 - 1];
			const int &neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle2 - 1];

			if (node1 == neighbourhood1) {
				node3 = node2 == neighbourhood2 ? neighbourhood3 : neighbourhood2;
			} else if (node1 == neighbourhood2) {
				node3 = node2 == neighbourhood1 ? neighbourhood3 : neighbourhood1;
			} else if (node1 == neighbourhood3) {
				node3 = node2 == neighbourhood1 ? neighbourhood2 : neighbourhood1;
			} else {
				throw std::runtime_error("Error in createVertexEdge(). Node1 is not equal to any of the neighbourhood nodes of triangle2.");
			}
			connectivityMatrixVertexEdge[1][edge] = node3;
		}
		if (triangle2 > 0) {
			if (triangle1 > 0) {
				const int &neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle2 - 1];
				const int &neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle2 - 1];
				const int &neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle2 - 1];

				if (node1 == neighbourhood1) {
					node3 = node2 == neighbourhood2 ? neighbourhood3 : neighbourhood2;
				} else if (node1 == neighbourhood2) {
					node3 = node2 == neighbourhood1 ? neighbourhood3 : neighbourhood1;
				} else if (node1 == neighbourhood3) {
					node3 = node2 == neighbourhood1 ? neighbourhood2 : neighbourhood1;
				} else {
					throw std::runtime_error("Error in createVertexEdge(). Node1 is not equal to any of the neighbourhood nodes of triangle2.");
				}
				connectivityMatrixVertexEdge[1][edge] = node3;
			}
		} else {
			connectivityMatrixVertexEdge[1][edge] = triangle2;
		}
	}
}

void reorder() {
	for (uint edge = 0; edge < numEdges; ++edge) {
		const int triangle1 = connectivityMatrixTriangleEdge[0][edge] - 1;
		const int triangle2 = connectivityMatrixTriangleEdge[1][edge] - 1;
		const int node1 = connectivityMatrixNodeEdge[0][edge] - 1;
		const int node2 = connectivityMatrixNodeEdge[1][edge] - 1;

		const int nodeVertex1 = connectivityMatrixVertexEdge[0][edge] - 1;
		const int nodeVertex2 = connectivityMatrixVertexEdge[1][edge] - 1;

		if (triangle1 >= 0) {
			int difference1 = ( x[nodeVertex1] - x[node1] ) * ( y[node2] - y[node1] );
			int difference2 = ( x[node2] - x[node1] ) * ( y[nodeVertex1] - y[node1] );
			if (difference1 > difference2) {
				connectivityMatrixTriangleEdge[0][edge] = triangle2 + 1; // naming correction
				connectivityMatrixTriangleEdge[1][edge] = triangle1 + 1;
				connectivityMatrixVertexEdge[0][edge] = nodeVertex2 + 1;
				connectivityMatrixVertexEdge[1][edge] = nodeVertex1 + 1;
			}
		} else {
			int difference1 = ( x[nodeVertex2] - x[node1] ) * ( y[node2] - y[node1] );
			int difference2 = ( x[node2] -  x[node1] ) * ( y[nodeVertex2] - y[node1] );

			if (difference1 < difference2) {
				connectivityMatrixTriangleEdge[0][edge] = triangle2 + 1;
				connectivityMatrixTriangleEdge[1][edge] = triangle1 + 1;
				connectivityMatrixVertexEdge[0][edge] = nodeVertex2 + 1;
				connectivityMatrixVertexEdge[1][edge] = nodeVertex1 + 1;
			}
		}
	}
}

}

