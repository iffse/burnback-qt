#pragma once

#include <QQmlApplicationEngine>
#include <array>
#include <vector>

inline QObject* root;

inline uint numberArea;
inline bool axisymmetric;
inline bool resume;

inline double cfl;
inline double cflViscous;
inline uint minIter;
inline uint maxIter;
inline double tolerance;

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

inline uint numBoundaries;
inline uint numNodes;
inline uint numTriangles;
inline uint numEdges;
inline uint numTriangleEdge; // ? nl1
inline uint meshDataHelper; // ? ihlp
inline uint numBoundaryEdge; // nbl

// connectivity matrix
inline std::array<std::vector<int>, 3> connectivityMatrixNodeTriangle; // mcnt
inline std::array<std::vector<int>, 2> connectivityMatrixNodeEdge; // mcnl
inline std::array<std::vector<int>, 2> connectivityMatrixTriangleEdge; // mctl ?
inline std::array<std::vector<int>, 2> connectivityMatrixNodeBoundary; // mcnb ?
inline std::array<std::vector<int>, 2> connectivityMatrixVertexEdge; // mcvl ?

inline std::vector<int> connectivityMatrixBoundaryConditions;

inline std::array<std::vector<double>, 8> alpha;
inline std::vector<double> sector;
inline std::vector<double> area;
inline std::vector<double> height;
inline std::array<std::vector<int>, 2> nodeBoundaryConditions; // ncc
inline std::vector<double> dt; // time step
inline std::vector<double> eps; // used in flux calculation
inline std::vector<double> uVertex;
inline std::array<std::vector<double>, 2> duVariable; // variable triangle
inline std::array<std::vector<double>, 2> duVertex; // variable vertex
inline std::array<std::vector<double>, 2> maxDuEdge;
inline std::array<std::vector<double>, 2> flux;

inline std::vector<double> burningArea;
inline std::vector<double> burningWay;

inline double timeTotal;

inline double diffusiveWeight;
inline uint diffusiveMethod;
inline uint numIsocontourLines;
inline uint isocontourSize;
