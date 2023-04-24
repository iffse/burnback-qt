import sys
import json
import meshio

def print_help():
	print('''
			Usage: python mesh_convert.py [options] filename
			Options:
				-f, --format: Output format (pretty), defaults to compact
				-o, --output: Output file name, defaults to input file name with json extension name
				-h, --help: Show this help
''')

output_name = ''
filename = ''
loops = 0

index = 1
while index < len(sys.argv):
	match sys.argv[index]:
		case '-h' | '--help':
			print_help()
			exit()
		case '-f' | '--format':
			format = sys.argv[index + 1]
			index += 2
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


for node in mesh.points:
	data['point'].append([node.tolist(), 0])

print('Reading field conditions...')
boundaries = []
for field in mesh.field_data:
	condition = field.split()
	if condition[0] in ['inlet', 'outlet', 'symmetry']:
		boundaries.append(mesh.field_data[field][0])

	condition_code = mesh.field_data[field][0].item()
	match condition[0]:
		case 'inlet':
			conditions['boundary'].append([condition_code, 'inlet', float(condition[1])])
		case 'outlet':
			conditions['boundary'].append([condition_code, 'outlet'])
		case 'symmetry':
			conditions['boundary'].append([condition_code, 'symmetry', [float(condition[1]), float(condition[2])]])
		case 'recession':
			conditions['recession'].append([condition_code, 'recession', float(condition[1])])

print('Assining conditions to boundaries...')
edges = {}
for cell in mesh.cell_data_dict['gmsh:physical']:
	for entry, condition in enumerate(mesh.cell_data_dict['gmsh:physical'][cell].tolist()):

		if (condition in boundaries) and cell == 'line':
			data['line'][entry].sort()
			edges[tuple(data['line'][entry])] = edges.get(tuple(data['line'][entry]), []) + [-condition]
			continue

		for rec in conditions['recession']:
			if rec[0] == condition and cell == 'triangle':
				for node in data['triangle'][entry]:
					if data['point'][node][1] == 0:
						data['point'][node][1] = condition
				continue

print('Creating edge connectivity')
for entry, triangle in enumerate(data['triangle']):
	nodes = [tuple(sorted([triangle[i], triangle[(i + 1) % 3]])) for i in range(3)]
	for node in nodes:
		edges[node] = edges.get(node, []) + [entry]

data['edge'] = [[list(node), *entries] for node, entries in edges.items()]

# index corrections
print('Correcting indices')
for triangle in data['triangle']:
	triangle = list(map(lambda x: x+1, triangle))

for tetra in data['tetra']:
	tetra = list(map(lambda x: x+1, tetra))

for edge in data['edge']:
	edge[0] = list(map(lambda x: x+1, edge[0]))
	edge[1:] = list(map(lambda x: x+1 if x >= 0 else x, edge[1:]))

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
	output_name = filename.split('.')[0] + '.json'

with open(output_name, 'w') as file:
	if format == 'pretty':
		json.dump(meshOut, file, indent=4)
	else:
		json.dump(meshOut, file)

print('Mesh converted to ' + output_name + ' successfully')



