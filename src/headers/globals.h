#pragma once

#include <QQmlApplicationEngine>

inline QObject* root;

inline uint numberArea;
inline bool axisymmetric;
inline bool resume;

inline double cfl;
inline uint minIter;
inline uint maxIter;
inline double tolerance;
inline double viscosity;

// const auto maxNodes = 25000;
// const auto maxTriangles = 2 * maxNodes;
// const auto maxEdges = 3 * maxNodes;
// const auto maxMeshData = 65 * maxNodes;
// const auto maxBoundaries = 100;

// inline std::array<double, maxMeshData> meshData;
// inline std::array<double, maxNodes> x;
// inline std::array<double, maxNodes> y;
// inline std::array<double, maxBoundaries> connectivityMatrixEdges;
// inline std::array<std::array<double, maxNodes>, 4> ubData;
// inline std::array<double, 4> uInit;

inline std::vector<int> meshData;
inline std::vector<double> x;
inline std::vector<double> y;
inline std::vector<double> connectivityMatrixEdges;
inline std::array<std::vector<double>, 4> uBoundaryData;
inline std::array<double, 4> uInit;

inline int numBoundaries;
inline int numNodes;
inline int numTriangles;
inline int numEdges;
inline int numEdges1; // ? nl1
inline int helper; // ? ihlp

// connectivity matrix
inline std::array<std::vector<double>, 3> connectivityMatrixNodeTriangles;
