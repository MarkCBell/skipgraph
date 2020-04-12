
import skipgraph
import argparse
from time import time

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Compute graph diameter')
    parser.add_argument('path', nargs='+', help='path to load')
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
        print('Diameter: %d' % G.diameter())
        print('Time: %0.3fs' % (time() - start_time))

