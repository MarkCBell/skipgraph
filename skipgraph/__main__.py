
import skipgraph
import argparse
from time import time

def partition(G, s):
	old, current, new = set(), set([s]), set()
	while current:
		yield current
		for v in current:
			new.update(G[v])
		old, current, new = current, new - current.union(old), set()
	return

def furthests(G, s):
	for X in partition(G, s):
		pass
	return X

def midpoint(G, a, b):
	P = list(partition(G, a))
	d = min(i for i in range(len(P)) if b in P[i])
	path = [b]
	for i in reversed(range(d)):
		path.append(P[i].intersection(G[path[-1]]).pop())
	
	return path[1 + len(path) // 2]

def sweep(G, s=0, n=2):
	l = 0
	for i in range(n):
		a = furthests(G, s).pop()
		b = furthests(G, a).pop()
		l = max(l, G.eccentricity(b))
		s = midpoint(G, a, b)
	
	return s, l


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Compute graph diameter')
	parser.add_argument('path', nargs='+', help='path to load')
	parser.add_argument('--ball', '-1', dest='algo', action='store_const', const='ball', help='')
	parser.add_argument('--ifub', '-2', dest='algo', action='store_const', const='ifub', help='')
	parser.add_argument('--both', '-3', dest='algo', action='store_const', const='ifub+ball', help='')
	parser.add_argument('--gr', '-g', dest='format', action='store_const', const='gr', help='')
	parser.add_argument('--dot', '-d', dest='format', action='store_const', const='dot', help='')
	args = parser.parse_args()
	
	for path in args.path:
		print('Analysing: %s' % path)
		if args.format == 'gr':
			G = skipgraph.load.gr(path)
		elif args.format == 'dot':
			G = skipgraph.load.dot(path)
		else:
			print('Unknown format')
			exit(1)
		print(G)
		start_time = time()
		if args.algo == 'ball':
			print('Diameter: %d' % G.diameter())
			print('Computed %d of %d eccentricities' % (G.DFT_count, G.num_vertices))
			print('Speedup: %0.2f' % (float(G.num_vertices) / G.DFT_count))
		elif args.algo == 'ifub':
			print('Diameter: %d' % G.diameter_ifub(False, *sweep(G)))
			print('Computed %d of %d eccentricities' % (G.DFT_count, G.num_vertices))
			print('Speedup: %0.2f' % (float(G.num_vertices) / G.DFT_count))
			# print('Diameter: %d' % diameter_IFUB(G, cull_ball=False))
		elif args.algo == 'ifub+ball':
			print('Diameter: %d' % G.diameter_ifub(True, *sweep(G)))
			print('Computed %d of %d eccentricities' % (G.DFT_count, G.num_vertices))
			print('Speedup: %0.2f' % (float(G.num_vertices) / G.DFT_count))
			# print('Diameter: %d' % diameter_IFUB(G, cull_ball=True))
		else:
			print('Unknown algorithm')
			exit(1)
		print('Time: %0.3fs' % (time() - start_time))

