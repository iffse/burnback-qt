#pragma once

void reorder();
void regenerateMeshData();
void setBoundaryConditions();
void setAngles();
void setMetric();
void setduVarriable();
void setFlux();
void boundaryFlux();

void eulerExplicit();

double getError();

void setBoundary();
void setBurningArea();

void mainLoop();
