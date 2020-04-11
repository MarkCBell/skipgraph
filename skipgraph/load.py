
from cGraph import Graph
from glob import glob

def dot(file_glob):
	G = dict()
	for path in glob(file_glob):
		with open(path) as f:
			for line in f:
				if ' -- ' in line:
					points = line.strip().replace('{','').replace(';','').replace('}','').replace('-','').replace('\n','').replace('  ', ' ')
					head, tail = points[:points.find(' ')], points[points.find(' ')+1:]
					G[head] = tail
	names = sorted(G)
	L = dict((name, index) for index, name in enumerate(names))
	return Graph([[L[x] for x in sorted(set(G[v].split(' ')))] for v in names])

def gr(file_glob):
	G = []
	for path in glob(file_glob):
		with open(path) as f:
			for line in f:
				if line.startswith('p '):
					_, _, n, _ = line.split(' ')
					G = [set() for i in range(int(n))]
				if line.startswith('a '):
					_, a, b, _ = line.strip().split(' ')
					a, b = int(a)-1, int(b)-1
					G[a].add(b)
					G[b].add(a)
	return Graph([list(v) for v in G])

