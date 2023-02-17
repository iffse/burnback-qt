#pragma once

#include <QQmlApplicationEngine>

inline QObject* root;

inline int numberArea;
inline bool axisymmetric;
inline bool resume;

inline double cfl;
inline int minIter;
inline int maxIter;
inline double tolerance;
inline double viscosity;

const auto maxNodes = 25000;
const auto maxTriangles = 2 * maxNodes;
const auto maxEdges = 3 * maxNodes;
const auto maxMeshData = 65 * maxNodes;
const auto maxBoundaries = 100;

inline double meshData[maxMeshData];
inline double x[maxNodes];
inline double y[maxNodes];
inline double connectivityMatrixEdges[maxBoundaries];
inline double ubData[4][maxNodes];
inline double uInit[4];
inline int numBoundaries;
