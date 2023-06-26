#pragma once
typedef unsigned int uint;

void reorder();
void regenerateMeshData();
void setBoundaryConditions();
void setAngles();
void setMetric();
void setduVarriable();
void setFlux();
void computeRecession();
void boundaryFlux();

void eulerExplicit();

double getError();

void setBoundary();
void setBurningArea();

void mainLoop();

void calculateAnisotropicMatrix();
double getMaxRecession();

template <typename T, typename V>
auto matrixMultiplication(const T &matrix1, const V &matrix2) {
	auto matrix = matrix1;
	for (uint i = 0; i < matrix1.size(); ++i) {
		for (uint j = 0; j < matrix2[0].size(); ++j) {
			matrix[i][j] = 0;
			for (uint k = 0; k < matrix1[0].size(); ++k)
				matrix[i][j] += matrix1[i][k] * matrix2[k][j];
		}
	}
	return matrix;
}

template <typename T>
auto matrixTranspose(const T &matrix) {
	auto matrixT = matrix;
	for (uint i = 0; i < matrix.size(); ++i) {
		for (uint j = 0; j < matrix[0].size(); ++j)
			matrixT[i][j] = matrix[j][i];
	}
	return matrixT;
}
