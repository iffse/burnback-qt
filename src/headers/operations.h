#pragma once

void reorder();
void regenerateMeshData();
void setBoundaryConditions();
void setAlpha();
void setMetric();
void setduVarriable();
void setFlux();
void boundary();
void setdt();

void eulerExplicit();

double getError();

void setqbnd();
void setBurningArea();

void mainLoop();

