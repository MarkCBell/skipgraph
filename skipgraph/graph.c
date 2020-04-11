#include <Python.h>
//#include <math.h>
#include "structmember.h"
#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

typedef struct {
    PyObject_HEAD
    long num_vertices;
    long** graph;
    long* degrees;
    long num_edges;
    long* vertices;
    char* seen;
    long* breaks;
    long num_breaks;
    char connected;
    long DFT_count;
} Graph;

static void Graph_dealloc(Graph* self);

static int Graph_init(Graph* self, PyObject* args)
{
    long i, j;
    PyObject* py_graph;
    PyObject* py_row;

    if (! PyArg_ParseTuple(args, "O", &py_graph))
        return -1;

    self->num_vertices = (long) PyList_Size(py_graph);

    self->DFT_count = 0;
    self->graph = calloc(self->num_vertices, sizeof(*(self->graph)));
    self->degrees = malloc(self->num_vertices * sizeof(*(self->degrees)));
    self->vertices = malloc(self->num_vertices * sizeof(*(self->vertices)));
    self->seen = malloc(self->num_vertices);
    self->breaks = malloc(0 * sizeof(*(self->breaks)));
    self->num_breaks = 0;
    if (self->graph == NULL || self->degrees == NULL || self->vertices == NULL || self->seen == NULL)
    {
        self->num_vertices = 0;  // Need to set this to prevent dealloc from clearing rows.
        Graph_dealloc(self);  // Free up all memory.
        PyErr_SetNone(PyExc_MemoryError);
        return -1;
    }

    self->connected = -1;  // Connected status unknown.
    self->num_edges = 0;
    for (i = 0; i < self->num_vertices; i++)
    {
        py_row = PyList_GetItem(py_graph, i);
        self->degrees[i] = (long) PyList_Size(py_row);
        self->num_edges += self->degrees[i];
        self->graph[i] = malloc(self->degrees[i] * sizeof(*(self->graph[i])));
        if (self->graph[i] == NULL)
        {
            Graph_dealloc(self);  // Free up all memory.
            PyErr_SetNone(PyExc_MemoryError);
            return -1;
        }
        for (j = 0; j < self->degrees[i]; j++)
            self->graph[i][j] = PyLong_AsLong(PyList_GetItem(py_row, j));
    }
    self->num_edges = self->num_edges / 2;

    return 0;
}


static void Graph_dealloc(Graph* self)
{
    long i;
    for (i = 0; i < self->num_vertices; i++)
        free(self->graph[i]);
    free(self->graph);
    free(self->vertices);
    free(self->seen);
    free(self->breaks);
}


static PyObject* Graph_str(PyObject* self)
{
    return PyString_FromFormat("Graph on %ld vertices with %ld edges.", ((Graph*) self)->num_vertices, ((Graph*) self)->num_edges);
}

static PyObject* Graph_reduce(Graph* self)
{
    long i, j;
    PyObject* py_graph = NULL;
    PyObject* py_row = NULL;

    py_graph = PyList_New(self->num_vertices);
    for (i = 0; i < self->num_vertices; i++)
    {
        py_row = PyList_New(self->degrees[i]);
        for (j = 0; j < self->degrees[i]; j++)
            PyList_SetItem(py_row, j, PyLong_FromLong(self->graph[i][j]));
        PyList_SetItem(py_graph, i, py_row);
    }
    return Py_BuildValue("(O(O))", PyObject_Type((PyObject*) self), py_graph);
}


static int DFT(Graph* self, long start, long max_radius)
{
    // Perform a DFT of this graph and report if all vertices were visited.
    // If not, this could be due to:
    //  - an out-of-memory error, or
    //  - the graph being disconnected.
    // If an out-of-memory error does not occur then the self->connected flag will be set.
    long i, b;
    long head = 0, tail = 1;
    long next_eccentricity_increase = tail;
    long current, next;
    long* new_breaks;

    self->DFT_count++;
    self->num_breaks = 0;
    new_breaks = realloc(self->breaks, (self->num_breaks+1) * sizeof(*(self->breaks)));
    if (! new_breaks) return 0;
    self->breaks = new_breaks;
    self->breaks[self->num_breaks] = tail;

    self->vertices[0] = start;
    memset(self->seen, 0, self->num_vertices * sizeof(*(self->seen)));
    self->seen[start] = 1;

    for (head = 0; head < self->num_vertices; head++)
    {
        if (head == tail)
        {
            self->connected = 0;
            return 0;
        }
        if (head == next_eccentricity_increase)
        {
            (self->num_breaks)++;
            new_breaks = realloc(self->breaks, (self->num_breaks+1) * sizeof(*(self->breaks)));
            if (! new_breaks) return 0;
            self->breaks = new_breaks;
            self->breaks[self->num_breaks] = tail;
            next_eccentricity_increase = tail;
            if (self->num_breaks == max_radius)
                break;
        }
        current = self->vertices[head];
        b = self->degrees[current];
        for (i = 0; i < b; i++)
        {
            next = self->graph[current][i];
            if (self->seen[next] == 0)
            {
                self->seen[next] = 1;
                self->vertices[tail] = next;
                tail++;
            }
        }
    }
    self->connected = 1;

    return 1;
}


static PyObject* Graph_far(Graph* self, PyObject* args)
{
    long start = 0;

    if (! PyArg_ParseTuple(args, "l", &start))
        return NULL;

    if (! DFT(self, start, -1))
    {
        if (self->connected == -1)
            return PyErr_NoMemory();
        else
            return Py_BuildValue("d", INFINITY);
    }

    return Py_BuildValue("l", self->vertices[self->num_vertices-1]);
}


static PyObject* Graph_eccentricity(Graph* self, PyObject* args)
{
    long start = 0;

    if (! PyArg_ParseTuple(args, "l", &start))
        return NULL;

    if (! DFT(self, start, -1))
    {
        if (self->connected == -1)
            return PyErr_NoMemory();
        else
            return Py_BuildValue("d", INFINITY);
    }

    return Py_BuildValue("l", self->num_breaks);
}


static PyObject* Graph_ball(Graph* self, PyObject* args)
{
    long i, b;
    long start = 0, radius = 0;
    PyObject* ball = NULL;

    if (! PyArg_ParseTuple(args, "ll", &start, &radius))
        return NULL;

    if (radius < 0)
        return PyList_New(0);

    if (! DFT(self, start, radius) && self->connected == -1)
    {
        return PyErr_NoMemory();
    }

    if (self->num_breaks < radius)
        radius = self->num_breaks;

    // Package up the ball.
    ball = PyList_New(self->breaks[radius]);
    b = self->breaks[radius];
    for (i = 0; i < b; i++)
        PyList_SetItem(ball, i, PyLong_FromLong(self->vertices[i]));

    return ball;
}


static PyObject* Graph_diameter(Graph* self, PyObject* args)
{
    /*  The key lemma which powers this method and provides the large speedup is:
            |ecc(x) - ecc(y)| <= d(x, y).
        Hence if ecc(v) is large and we have just found that ecc(x) is small then
        we know that any vertex y in B(x, ecc(v) - ecc(x)) must have
            ecc(y) <= ecc(v).
       Thus in searching for a vertex with eccentricity larger than that of v we can
       skip everything in this ball.
    */
    long i, j, b;
    long max_eccentricity = 0;
    long current;
    char* done;

    long mini = 0;
    long maxi = self->num_vertices;
    long blocksize;

    if (! PyArg_ParseTuple(args, "|lll", &mini, &maxi, &max_eccentricity))
        return NULL;

    blocksize = maxi - mini;
    done = calloc(blocksize, sizeof(done));
    // self->DFT_count = 0;  // Should we really be resetting the count automatically?
    if (done == NULL)
        return PyErr_NoMemory();

    for (i = mini; i < maxi; i++)
        if (! done[i - mini])
        {
            // done[i - mini] = 1;  // Not really needed as we will never check back here again.
            if (! DFT(self, i, -1))
            {
                if (self->connected == -1)
                    return PyErr_NoMemory();
                else
                    return Py_BuildValue("d", INFINITY);
            }
            if (self->num_breaks >= max_eccentricity)
                max_eccentricity = self->num_breaks;  // New largest eccentricity found.
            else
            {
                // Ball of vertices that can be skipped.
                b = self->breaks[max_eccentricity - self->num_breaks];
                for (j = 0; j < b; j++)
                {
                    current = self->vertices[j];
                    if (i <= current && current < maxi)  // Used to be mini <= current < maxi here. But we don't need to flag older vertices as seen.
                        done[current - mini] = 1;
                }
            }
        }

    return Py_BuildValue("l", max_eccentricity);
}


static PyObject* Graph_diameter_ifub(Graph* self, PyObject* args)
{
    long i, j;
    long m = 0;
    long start = 0;
    long r, e;
    long* P = NULL;
    long* P_b = NULL;
    char* done = NULL;
    char cull = 0;

    if (! PyArg_ParseTuple(args, "|lll", &cull, &start, &m))
        return NULL;

    // self->DFT_count = 0;  // Should we really be resetting the count automatically?
    if (! DFT(self, start, -1))
    {
        if (self->connected == -1)
            return PyErr_NoMemory();
        else
            return Py_BuildValue("d", INFINITY);
    }
    done = calloc(cull ? self->num_vertices : 1, sizeof(done));
    P = malloc(self->num_vertices * sizeof(*(self->vertices)));
    P_b = malloc((self->num_breaks+1) * sizeof(*(self->breaks)));
    if (done == NULL || P == NULL || P_b == NULL)
    {
        free(done);
        free(P);
        free(P_b);
        return PyErr_NoMemory();
    }
    memcpy(P, self->vertices, self->num_vertices * sizeof(*(self->vertices)));
    memcpy(P_b, self->breaks, (self->num_breaks+1) * sizeof(*(self->breaks)));

    for (r = self->num_breaks-1; r >= 0; r--)
    {
        printf("r: %ld lower: %ld upper: %ld\n", r, m, 2*r);
        if (m > 2*r) break;
        for(i = P_b[r]; i < P_b[r+1]; i++)
            if (!(cull && done[P[i]]))
            {
                if (! DFT(self, P[i], -1))
                {
                    free(done);
                    free(P);
                    free(P_b);
                    return PyErr_NoMemory();
                }
                e = self->num_breaks;
                if (e > m)
                {
                    m = e;
                    if (m > 2*r) break;
                }
                else if (cull)
                    for (j = 0; j < self->breaks[m - e]; j++)
                        done[self->vertices[j]] = 1;
            }
    }

    free(done);
    free(P);
    free(P_b);
    return Py_BuildValue("l", m);
}


static PyObject* Graph_radius(Graph* self, PyObject* args)
{
    long i, j, b;
    long min_eccentricity = self->num_vertices;
    long current;
    char* done;

    long mini = 0;
    long maxi = self->num_vertices;
    long blocksize;

    if (! PyArg_ParseTuple(args, "|lll", &mini, &maxi, &min_eccentricity))
        return NULL;

    blocksize = maxi - mini;
    done = calloc(blocksize, sizeof(done));
    if (done == NULL)
        return PyErr_NoMemory();

    for (i = mini; i < maxi; i++)
        if (! done[i - mini])
        {
            // done[i - mini] = 1;  // Not really needed as we will never check back here again.
            if (! DFT(self, i, -1))
            {
                if (self->connected == -1)
                    return PyErr_NoMemory();
                else
                    return Py_BuildValue("d", INFINITY);
            }
            if (self->num_breaks <= min_eccentricity)
                min_eccentricity = self->num_breaks;
            else
            {
                b = self->breaks[self->num_breaks - min_eccentricity];
                for (j = 0; j < b; j++)
                {
                    current = self->vertices[j];
                    if (i <= current && current < maxi)  // Used to be mini <= current < maxi here. But we don't need to flag older vertices as seen.
                        done[current - mini] = 1;
                }
            }
        }

    return Py_BuildValue("l", min_eccentricity);
}


static Py_ssize_t Graph_len(PyObject* self)
{
    return (Py_ssize_t) ((Graph*) self)->num_vertices;
}


static PyObject* Graph_getitem(PyObject* self, Py_ssize_t key2)
{
    long i;
    long key = (long) key2;
    PyObject* py_row = NULL;

    py_row = PyList_New(((Graph*) self)->degrees[key]);
    for (i = 0; i < ((Graph*) self)->degrees[key]; i++)
        PyList_SetItem(py_row, i, PyLong_FromLong(((Graph*) self)->graph[key][i]));
    return py_row;
}



/* --------------------------------- */

static PySequenceMethods Graph_sequence_methods = {
    &Graph_len,                  /* sq_length */
    0,
    0,
    &Graph_getitem,
};

static PyMemberDef Graph_members[] = {
    {"num_edges", T_INT, offsetof(Graph, num_edges), 1, "number of edges"},
    {"num_vertices", T_INT, offsetof(Graph, num_vertices), 1, "number of vertices"},
    {"DFT_count", T_INT, offsetof(Graph, DFT_count), 1, "number of DFT that have been performed"},
    {NULL}  /* Sentinel */
};

static PyMethodDef Graph_methods[] = {
    {"eccentricity", (PyCFunction)Graph_eccentricity, METH_VARARGS, "Return the eccentricity of the given vertex."},
    {"ball", (PyCFunction)Graph_ball, METH_VARARGS, "Return the *closed* ball about the given vertex."},
    {"diameter", (PyCFunction)Graph_diameter, METH_VARARGS, "Return the diameter of self."},
    {"diameter_ifub", (PyCFunction)Graph_diameter_ifub, METH_VARARGS, "Return the diameter of self using the iFUB algorithm."},
    {"radius", (PyCFunction)Graph_radius, METH_VARARGS, "Return the radius of self."},
    {"__reduce__", (PyCFunction)Graph_reduce, METH_NOARGS, ""},
    //{"__getstate__", (PyCFunction)Graph_getstate, METH_NOARGS, ""},
    //{"__setstate__", (PyCFunction)Graph_setstate, METH_VARARGS, ""},
    {NULL}  /* Sentinel */
};

/* --------------------------------- */

static PyTypeObject cGraph_GraphType = {
#if PY_MAJOR_VERSION < 3
    PyObject_HEAD_INIT(NULL) 0,/* ob_size */
#else
    PyVarObject_HEAD_INIT(NULL, 0)
#endif
    "cGraph.Graph",             /*tp_name*/
    sizeof(Graph),              /*tp_basicsize*/
    0,                          /*tp_itemsize*/
    (destructor) Graph_dealloc, /*tp_dealloc*/
    0,                          /*tp_print*/
    0,                          /*tp_getattr*/
    0,                          /*tp_setattr*/
    0,                          /*tp_compare*/
    0,                          /*tp_repr*/
    0,                          /*tp_as_number*/
    &Graph_sequence_methods,    /*tp_as_sequence*/
    0,                          /*tp_as_mapping*/
    0,                          /*tp_hash */
    0,                          /*tp_call*/
    &Graph_str,                 /*tp_str*/
    0,                          /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Graph objects",            /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    Graph_methods,              /* tp_methods */
    Graph_members,              /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc) Graph_init,      /* tp_init */
    0,                          /* tp_alloc */
    PyType_GenericNew,          /* tp_new */
};

#if PY_MAJOR_VERSION < 3
static PyMethodDef cGraph_methods[] = {
    {NULL}  /* Sentinel */
};

PyMODINIT_FUNC initcGraph(void)
{
    PyObject* m;

    if (PyType_Ready(&cGraph_GraphType) < 0)
        return;

    m = Py_InitModule3("cGraph", cGraph_methods, "Class for testing diameter skip calculations.");

    Py_INCREF(&cGraph_GraphType);
    PyModule_AddObject(m, "Graph", (PyObject*) &cGraph_GraphType);
}
#else
static PyModuleDef cGraphmodule = {
    PyModuleDef_HEAD_INIT,
    "cGraph",
    "Class for testing diameter skip calculations.",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_cGraph(void)
{
    PyObject* m;

    cGraph_GraphType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&cGraph_GraphType) < 0)
        return NULL;

    m = PyModule_Create(&cGraphmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&cGraph_GraphType);
    PyModule_AddObject(m, "Graph", (PyObject*) &cGraph_GraphType);
    return m;
}
#endif

