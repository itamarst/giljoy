#include <stdio.h>
#include <stdlib.h>

#include <Python.h>
#include <pythread.h>


static PyMethodDef GiljoyMethods[] = {
    {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
init_giljoy(void)
{
    (void) Py_InitModule("_giljoy", GiljoyMethods);
}
