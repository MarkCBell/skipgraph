cimport cython
from cpython cimport array
import array

cdef array.array long_array_template = array.array('l', [])

cdef class Graph:
    cdef array.array graph
    cdef long num_vertices
    cdef array.array starts
    
    cdef long[:] vertices
    cdef array.array breaks
    
    def __init__(self, graph):
        cdef array.array starts
        self.graph = array.array('l', [e for v in graph for e in v])
        self.num_vertices = len(graph)
        starts = array.array('l', [0])
        for v in graph:
            starts.append(starts[-1] + len(v))
        self.starts = starts
    
    @cython.boundscheck(False)  # Deactivate bounds checking
    @cython.wraparound(False)   # Deactivate negative indexing.
    cdef DFT(self, long start):
        cdef long head = 0
        cdef long tail = 1
        cdef long current, adjacent
        cdef long degree
        cdef long current_start
        cdef long i
        cdef long[:] seen = array.clone(long_array_template, self.num_vertices, True)
        cdef long last_break = tail
        cdef long[:] vertices = array.clone(long_array_template, self.num_vertices, True)
        cdef array.array breaks = array.array('l', [last_break])
        
        vertices[0] = start
        seen[start] = True
        
        for head in range(self.num_vertices):
            if head == tail:  # Disconnected
                raise ValueError('Graph is disconnected')
            if head == last_break:
                breaks.append(tail)
                last_break = tail
            current = vertices[head]
            current_start = self.starts.data.as_longs[current]
            degree = self.starts.data.as_longs[current+1] - current_start
            for i in range(current_start, current_start + degree):
                adjacent = self.graph.data.as_longs[i]
                if not seen[adjacent]:
                    seen[adjacent] = True
                    vertices[tail] = adjacent
                    tail += 1
        
        self.vertices = vertices
        self.breaks = breaks
    
    @cython.boundscheck(False)  # Deactivate bounds checking
    @cython.wraparound(False)   # Deactivate negative indexing.
    def diameter(self):
        cdef long start, j
        cdef long max_eccentricity = 0, eccentricity
        cdef long[:] done = array.clone(long_array_template, self.num_vertices, True)
        cdef long[:] vertices
        cdef long[:] breaks
        cdef long num_reset

        for start in range(self.num_vertices):
            if done[start]:
                continue
            
            self.DFT(start)
            vertices = self.vertices
            breaks = self.breaks
            eccentricity = len(breaks) - 1
            if eccentricity >= max_eccentricity:
                max_eccentricity = eccentricity
            else:
                num_reset = breaks[max_eccentricity - eccentricity]
                for i in range(num_reset):
                    done[vertices[i]] = True
        
        return max_eccentricity
    
    @cython.boundscheck(False)  # Deactivate bounds checking
    @cython.wraparound(False)   # Deactivate negative indexing.
    def radius(self):
        cdef long start, i
        cdef long min_eccentricity = self.num_vertices, eccentricity
        cdef long[:] done = array.clone(long_array_template, self.num_vertices, True)
        cdef long[:] vertices
        cdef long[:] breaks
        cdef long num_reset

        for start in range(self.num_vertices):
            if done[start]:
                continue
            
            self.DFT(start)
            vertices = self.vertices
            breaks = self.breaks
            eccentricity = len(breaks) - 1
            if eccentricity <= min_eccentricity:
                min_eccentricity = eccentricity
            else:
                num_reset = breaks[eccentricity - min_eccentricity]
                for i in range(num_reset):
                    done[vertices[i]] = True
        
        return min_eccentricity

