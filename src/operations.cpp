#include <src/headers/operations.h>
#include <src/headers/globals.h>

#include <array>

using namespace std;

// algorithm from TRIMPACK
namespace ConnectivityMatrix {

// node as function of triangle (mcnt)
void NodeTriangles() {
	connectivityMatrixNodeTriangle = array<vector<int>, 3>({vector<int>(numTriangles), vector<int>(numTriangles), vector<int>(numTriangles)});
	
	uint meshDataSize = numTriangles + 4 * numEdges + numTriangles - 1;
	if (meshDataSize >= meshData.size())
		throw std::invalid_argument("Invalid mesh data. Connectivity matrix node as function of triangle (mcnt) is out of bounds");

	// helping array
	for (int i = 0; i < numTriangles; ++i) {
		meshData[helper + i - 1] = 0;
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
			for (int j = 0; j < meshData[helper + nodeT1 - 1 - 1]; ++j) {
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
				meshData[helper + nodeT1 - 1 - 1] = meshData[helper + nodeT1 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[helper + nodeT1 - 1 - 1] - 1][nodeT1 - 1] = node1;
			}
			if (index2 == 0) {
				meshData[helper + nodeT1 - 1 - 1] = meshData[helper + nodeT1 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[helper + nodeT1 - 1 - 1] - 1][nodeT1 - 1] = node2;
			}
			index1 = 0;
			index2 = 0;
		}

		if (nodeT2 > 0) {
			// check if node already exist in cells of mcnt(x, nodeT2) 
			for (int j = 0; j < meshData[helper + nodeT2 - 1 - 1]; ++j) {
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
				meshData[helper + nodeT2 - 1 - 1] = meshData[helper + nodeT2 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[helper + nodeT2 - 1 - 1] - 1][nodeT2 - 1] = node1;
			}
			if (index2 == 0) {
				meshData[helper + nodeT2 - 1 - 1] = meshData[helper + nodeT2 - 1 - 1] + 1;
				connectivityMatrixNodeTriangle[meshData[helper + nodeT2 - 1 - 1] - 1][nodeT2 - 1] = node2;
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
		int triangle1 = connectivityMatrixTriangleEdge[0][i];
		int triangle2 = connectivityMatrixTriangleEdge[1][i];
		int node1 = connectivityMatrixNodeEdge[0][i];
		int node2 = connectivityMatrixNodeEdge[1][i];
		int node3;

		if (triangle1 > 0) {
			// ? nh
			int neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle1 - 1];
			int neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle1 - 1];
			int neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle1 - 1];

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
			int neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle2 - 1];
			int neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle2 - 1];
			int neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle2 - 1];
			
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
				int neighbourhood1 = connectivityMatrixNodeTriangle[0][triangle2 - 1];
				int neighbourhood2 = connectivityMatrixNodeTriangle[1][triangle2 - 1];
				int neighbourhood3 = connectivityMatrixNodeTriangle[2][triangle2 - 1];

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



