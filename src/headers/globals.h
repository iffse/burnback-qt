#pragma once

#include <QQmlApplicationEngine>
#include <array>
#include <vector>
#include <map>

inline QObject* root;
inline bool running = false;

inline uint numberArea;
inline bool axisymmetric;
inline bool resume;

inline double cfl;
inline double cflViscous;
inline uint minIter;
inline uint maxIter;
inline uint currentIter;
inline double tolerance;
inline std::vector<double> errorIter;

inline std::vector<int> meshData;
inline std::vector<double> x;
inline std::vector<double> y;
inline std::vector<double> connectivityMatrixEdges;
inline std::map<int, double> uBoundaryData;
inline std::map<int, QString> boundaryDescriptions;
inline double uInit;

inline uint numNodes;
inline uint numTriangles;
inline uint numEdges;
inline uint numTriangleEdge;
inline std::vector<std::array<int, 4>>edgeData;
inline uint numBoundaryEdge;

// connectivity matrix
inline std::array<std::vector<int>, 3> connectivityMatrixNodeTriangle;
inline std::array<std::vector<int>, 2> connectivityMatrixNodeEdge;
inline std::array<std::vector<int>, 2> connectivityMatrixTriangleEdge;
inline std::array<std::vector<int>, 2> connectivityMatrixNodeBoundary;
inline std::array<std::vector<int>, 2> connectivityMatrixVertexEdge;

inline std::map<int, int> connectivityMatrixBoundaryConditions;

inline std::array<std::vector<double>, 8> alpha;
// angle in each node of a edge
inline std::array<std::vector<double>, 4> thetaEdge;
// contribution of angles in each node of a edge
inline std::array<std::vector<double>, 2> betaEdge;
inline std::array<std::vector<double>, 2> directionEdge;
inline std::vector<double> sector;
inline std::vector<double> area;
inline std::vector<double> height;
inline std::array<std::vector<int>, 2> nodeBoundaryConditions;
inline std::vector<double> uVertex;
inline std::array<std::vector<double>, 2> duVariable; // gradient in the triangle
inline std::array<std::vector<double>, 2> duVertex; // gradient in the vertex
inline std::array<std::vector<double>, 2> maxDuEdge;
inline std::array<std::vector<double>, 2> flux;
inline std::vector<double> recession;

inline std::vector<double> burningArea;
inline std::vector<double> burningWay;

inline double timeTotal;

inline double diffusiveWeight;
inline uint diffusiveMethod;
inline uint numIsocontourLines;
inline uint isocontourSize;
inline uint isocontourColor;
inline std::map<uint, std::vector<double>> boundaryDataDict;
