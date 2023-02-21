#pragma once

namespace ConnectivityMatrix {
void NodeTriangles();
void NodeEdge();
void TriangleEdge();

void createVertexEdge();
void reorder();
}

void regenerateMeshData();
void getBoundaryConditions();
void setAlpha();
void setMetric();
void setduVar();
void flux();
void boundary();
void setdt();

void eulerExplicit();

void getError();

void setqbnd();
void burningArea();

void mainLoop();

