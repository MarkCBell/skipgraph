
from time import time
from cGraph import Graph
from sage.all import graphs

def convert(G):
	''' Convert a Sage graph to a cGraph. '''
	return Graph([G[v] for v in G])

def compare(G):
	H = convert(G)
	
	s = time()
	cGraph_diameter = H.diameter()
	cGraph_time = time() - s
	
	s = time()
	sage_diameter = G.diameter()
	sage_time = time() - s
	
	print('Sage: %0.3fs    cGraph: %0.3fs    (Diameters: %s,%s)' % (sage_time, cGraph_time, sage_diameter, cGraph_diameter))

def gen_graphs():
	# Add other graphs to test here:
	for i in range(10):
		yield graphs.RandomGNP(2001,0.05, seed=1)

def test():
	for G in gen_graphs():
		compare(G)

if __name__ == '__main__':
	test()

