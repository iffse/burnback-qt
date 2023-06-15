import matplotlib.pyplot as plt
import matplotlib.tri as tri
import numpy as np
import json

filename = './results1.8t0.5.json'
file = open(filename, 'r')
data = json.load(file)
file.close()


results = data['burnbackResults']
duVertex = results['duVertex']
uVertex = results['uVertex']
hflux = results['fluxes'][0]
dflux = results['fluxes'][1]

mesh = data['mesh']
nodes = mesh['nodes']
nodes_x = [node[0] for node in nodes]
nodes_y = [node[1] for node in nodes]
triangles = mesh['triangles']

triangulation = tri.Triangulation(nodes_x, nodes_y, triangles)

figure, axis = plt.subplots()
axis.set_aspect('equal')
tcf = axis.tricontourf(triangulation, uVertex, levels=10)
figure.colorbar(tcf, ticks=tcf.levels)
axis.tricontour(triangulation, uVertex, colors='k', levels=tcf.levels)

plt.show()

# plot triangles (mesh) in cyan
# this operation is very slow and demands a lot of memory. Use only for small meshes
# for triangle in triangles:
# 	x = [nodes_x[triangle[i]] for i in range(3)]
# 	y = [nodes_y[triangle[i]] for i in range(3)]
# 	plt.fill(x, y, edgecolor='cyan', fill=False)

# plot boundaries in red
# for edge in boundaryEdges:
# 	x = [nodes_x[edge[i]] for i in range(2)]
# 	y = [nodes_y[edge[i]] for i in range(2)]
# 	axis.plot(x, y, color='red')


