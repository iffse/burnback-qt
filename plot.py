import matplotlib.pyplot as plt
import matplotlib.tri as tri
import numpy as np

filename = 'plotData.txt'
# filename = 'graf.plot'
file = open(filename, 'r')

numTriangles = int(file.readline())
triangles = []
for i in range(numTriangles):
	line = file.readline()
	triangles.append([int(x) - 1 for x in line.split()])

numNodes = int(file.readline())
nodes_x = []
nodes_y = []
u_vertex = []
du_vertex_x = []
du_vertex_y = []
for i in range(numNodes):
	line = file.readline()
	line = line.split()
	nodes_x.append(float(line[0]))
	nodes_y.append(float(line[1]))
	u_vertex.append(float(line[2]))
	du_vertex_x.append(float(line[3]))
	du_vertex_y.append(float(line[4]))

numBoundaryEdges = int(file.readline())
boundaryEdges = []
for i in range(numBoundaryEdges):
	line = file.readline()
	boundaryEdges.append([int(x) - 1 for x in line.split()])

file.close()

triangulation = tri.Triangulation(nodes_x, nodes_y, triangles)

figure, axis = plt.subplots()
axis.set_aspect('equal')
tcf = axis.tricontourf(triangulation, u_vertex, levels=10)
figure.colorbar(tcf, ticks=tcf.levels)
axis.tricontour(triangulation, u_vertex, colors='k', levels=tcf.levels)

# plot triangles (mesh) in cyan
# this operation is very slow and demands a lot of memory. Use only for small meshes
# for triangle in triangles:
# 	x = [nodes_x[triangle[i]] for i in range(3)]
# 	y = [nodes_y[triangle[i]] for i in range(3)]
# 	plt.fill(x, y, edgecolor='cyan', fill=False)

# plot boundaries in red
for edge in boundaryEdges:
	x = [nodes_x[edge[i]] for i in range(2)]
	y = [nodes_y[edge[i]] for i in range(2)]
	axis.plot(x, y, color='red')

plt.show()


