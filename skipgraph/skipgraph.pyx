cimport cython
from cpython cimport array
import array

cdef array.array long_array_template = array.array('l', [])

cdef class Graph:
    cdef array.array graph
    cdef long num_vertices
    cdef array.array starts
    
    cdef array.array _vertices
    cdef array.array _breaks
    cdef long _eccentricity
    
    def __init__(self, graph):
        self.graph = array.array('l', (e for v in graph for e in v))
        self.num_vertices = len(graph)
        self.starts = array.array('l', b'\0')
        for v in graph:
            self.starts.append(self.starts[-1] + len(v))
    
    cdef DFT(self, long start):
        cdef long head = 0
        cdef long tail = 1
        cdef long current, adjacent
        cdef long i
        cdef array.array seen = array.clone(long_array_template, self.num_vertices, True)
        cdef long last_break = tail
        
        self._vertices = array.clone(long_array_template, self.num_vertices, True)
        self._breaks = array.array('l', [tail])
        self._eccentricity = 0
        
        self._vertices.data.as_longs[0] = start
        seen.data.as_longs[start] = True
        
        for head in range(self.num_vertices):
            if head == tail:  # Disconnected
                raise ValueError('Graph is disconnected')
            if head == last_break:
                self._breaks.append(tail)
                self._eccentricity += 1
                last_break = tail
            current = self._vertices.data.as_longs[head]
            for i in range(self.starts.data.as_longs[current], self.starts.data.as_longs[current+1]):
                adjacent = self.graph.data.as_longs[i]
                if not seen.data.as_longs[adjacent]:
                    seen.data.as_longs[adjacent] = True
                    self._vertices.data.as_longs[tail] = adjacent
                    tail += 1
    
    def diameter(self):
        cdef long start, i, num_reset
        cdef long max_eccentricity = 0
        cdef array.array done = array.clone(long_array_template, self.num_vertices, True)

        for start in range(self.num_vertices):
            if done.data.as_longs[start]:
                continue
            
            self.DFT(start)
            if self._eccentricity >= max_eccentricity:
                max_eccentricity = self._eccentricity
            else:
                num_reset = self._breaks.data.as_longs[max_eccentricity - self._eccentricity]
                for i in range(num_reset):
                    done.data.as_longs[self._vertices.data.as_longs[i]] = True
        
        return max_eccentricity
    
    def radius(self):
        cdef long start, i, num_reset
        cdef long min_eccentricity = self.num_vertices
        cdef array.array done = array.clone(long_array_template, self.num_vertices, True)

        for start in range(self.num_vertices):
            if done.data.as_longs[start]:
                continue
            
            _ = self.DFT(start)
            if self._eccentricity <= min_eccentricity:
                min_eccentricity = self._eccentricity
            else:
                num_reset = self._breaks.data.as_longs[self._eccentricity - min_eccentricity]
                for i in range(num_reset):
                    done.data.as_longs[self._vertices.data.as_longs[i]] = True
        
        return min_eccentricity

