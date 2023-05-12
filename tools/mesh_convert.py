import numpy as np
import sys
import json
import meshio

def print_help():
	print('''
Usage: python mesh_convert.py [options] filename
Options:
	-p, --pretty: Using pretty output
	-o, --output: Output file name, defaults to input file name with json extension name
	-h, --help: Show this help
''')

output_name = ''
filename = ''
loops = 0
pretty = False

index = 1
while index < len(sys.argv):
	match sys.argv[index]:
		case '-h' | '--help':
			print_help()
			exit()
		case '-p' | '--pretty':
			pretty = True
			index += 1
		case '-o' | '--output':
			output_name = sys.argv[index + 1]
			index += 2
		case _:
			filename = sys.argv[index]
			index += 1

if filename == '':
	print('No input file specified. Use -h or --help for help')
	exit()

try:
	mesh = meshio.read(filename)
except:
	print('Error reading file')
	exit()

data = {
	'point': [],
	'line': [],
	'triangle': [],
	'tetra': [],
	'quad': [],
	'edge': [],
}

conditions = {
	'boundary': [],
	'recession': [],
}

print('Reading mesh data...')

for entry in mesh.cells_dict:
	if entry not in data:
		continue
	data[entry]= mesh.cells_dict[entry].tolist()


data['point'] = mesh.points.tolist()

print('Reading field conditions...')
boundaries = []
recessions = {}
for field in mesh.field_data:
	condition = field.split()
	if condition[0] in ['inlet', 'outlet', 'symmetry']:
		boundaries.append(mesh.field_data[field][0])
	elif condition[0] == 'recession':
		recessions[mesh.field_data[field][0]] = float(condition[1])

	condition_code = mesh.field_data[field][0].item()
	match condition[0]:
		case 'inlet':
			conditions['boundary'].append({
				'tag': condition_code,
				'type': 'inlet',
				'value': float(condition[1]),
				'description': condition[2:]
			})
		case 'outlet':
			conditions['boundary'].append({
				'tag': condition_code,
				'type': 'outlet',
				'description': condition[1:]
			})
		case 'symmetry':
			angle = np.deg2rad(float(condition[1]))
			conditions['boundary'].append({
				'tag': condition_code,
				'type': 'symmetry',
				'value': angle,
				'description': condition[2:]
			})

print('Assining conditions to boundaries and recession nodes...')
edges = {}
triangle = {}
for cell in mesh.cell_data_dict['gmsh:physical']:
	for entry, condition in enumerate(mesh.cell_data_dict['gmsh:physical'][cell].tolist()):
		if (condition in boundaries) and cell == 'line':
			data['line'][entry].sort()
			edges[tuple(data['line'][entry])] = edges.get(tuple(data['line'][entry]), []) + [-condition]

		if (condition in recessions) and cell == 'triangle':
			triangle[condition] = triangle.get(condition, []) + [data['triangle'][entry]]

if recessions != {}:
	conditions['recession'] = [1] * len(data['point'])
	for condition in triangle:
		nodes = list(set([node for triangle in triangle[condition] for node in triangle]))
		for node in nodes:
			conditions['recession'][node] = recessions[condition]

print('Creating edge connectivity')
for entry, triangle in enumerate(data['triangle']):
	triangle = [tuple(sorted([triangle[i], triangle[(i + 1) % 3]])) for i in range(3)]
	for node in triangle:
		edges[node] = edges.get(node, []) + [entry]

data['edge'] = [list(node) + [*entries] for node, entries in edges.items()]

# index corrections
print('Correcting indices')
for triangle in data['triangle']:
	triangle = list(map(lambda x: x+1, triangle))

for tetra in data['tetra']:
	tetra = list(map(lambda x: x+1, tetra))

for edge in data['edge']:
	edge[0:2] = list(map(lambda x: x+1, edge[0:2]))
	edge[2:4] = list(map(lambda x: x+1 if x >= 0 else x, edge[2:4]))

meshOut = {
	'metaData': {
		'nodes': len(data['point']),
		'triangles': len(data['triangle']),
		# 'tetrahedra': len(data['tetra']),
		# Euler formula for planar graphs
		# 'edges': len(data['point']) + len(data['triangle']) - len(data['tetra']) + loops - 1
		'edges': len(data['edge']),
		'version': '0.1'
	},
	'mesh': {
		'nodes': data['point'],
		# 'triangles': data['triangle'],
		# 'tetrahedra': data['tetra'],
		'edges': data['edge']
	},
	'conditions': conditions,
}

if output_name == '':
	output_name = filename[:filename.rfind('.')] + '.json'


with open(output_name, 'w') as file:
	if pretty:
		json.dump(meshOut, file, indent=4)
	else:
		json.dump(meshOut, file)

print('Mesh converted to ' + output_name + ' successfully')

