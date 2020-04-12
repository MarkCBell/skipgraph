
from .skipgraph import Graph
from . import load

def chain(length):
    return Graph([[1]] + [[i-1, i+1] for i in range(1, length)] + [[length-1]])

