#pragma once

void reorder();
void regenerateMeshData();
void setBoundaryConditions();
void setAngles();
void setMetric();
void setduVarriable();
void setFlux();
void boundaryFlux();
void setdt();

void eulerExplicit();

double getError();

void setqbnd();
void setBurningArea();

void mainLoop();
