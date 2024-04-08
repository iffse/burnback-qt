import sys
import json
import meshio

def print_help():
	print('''
Usage: python result_convert.py input_file output_file/extension
	input_file: json file generated by burnback export
	output_file: output file, will use the name of input file if only extension is given
''')

filename = ''
output_name = ''

if sys.argv[1] == '-h' or sys.argv[1] == '--help' or len(sys.argv) < 3:
	print_help()
	exit()

filename = sys.argv[1]
output_name = sys.argv[2]
# check if has extension name
if '.' not in output_name:
	# remove extension name
	name = filename.split('.')
	if len(name) > 1:
		name = '.'.join(name[:-1])
	output_name = name + '.' + output_name

print('Reading file ' + filename)
try:
	data = json.load(open(filename))
except:
	print('Error reading file ' + filename)

# convert to meshio
print('Converting to meshio')
mesh = data['mesh']
results = data['burnbackResults']

# correcting index
print('Correcting index')
triangle = [[node - 1 for node in tri] for tri in mesh['triangles']]

point = mesh['nodes']
cells = [
	("triangle", triangle),
]
point_data = {
	"time": results['uVertex'],
	"fluxHamiltonian": results['fluxes'][0],
	"fluxDiffusive": results['fluxes'][1],
}

cell_data = {
	"gradient": [results['gradient']],
}

mesh = meshio.Mesh(
	points=point,
	cells=cells,
	point_data=point_data,
	cell_data=cell_data,
)

print('Writing to ' + output_name)
mesh.write(output_name)

