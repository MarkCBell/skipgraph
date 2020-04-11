from setuptools import setup, find_packages, Extension

cgraph_ext = Extension('cGraph', sources = ['./skipgraph/graph.c'])

setup(
	name = 'skipgraph',
	version = '0.1',
	packages=['skipgraph'],
	package_dir={'skipgraph': 'skipgraph'},
	description = 'A graph library which implements the ball skip algorithm to compute diameters quickly.',
	ext_modules = [cgraph_ext]
	)

