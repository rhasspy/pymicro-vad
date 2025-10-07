// src/micro_vad.cpp
#define PY_SSIZE_T_CLEAN
#ifndef Py_LIMITED_API
// Target Python 3.9 stable ABI (adjust if you set a different floor)
#define Py_LIMITED_API 0x03090000
#endif
#include <Python.h>

#include "micro_vad.h"

// -------------------- per-instance handle ----------------------------
struct VadHandle {
    micro_vad::MicroVad *vad;
};

static const char *CAPSULE_NAME = "micro_vad_cpp.VadHandle";

// -------------------- helpers --------------------
static void vad_capsule_destructor(PyObject *capsule) {
    void *p = PyCapsule_GetPointer(capsule, CAPSULE_NAME);
    if (!p) {
        // Name mismatch or already invalid; do not raise from a destructor.
        PyErr_Clear();
        return;
    }
    auto *h = (VadHandle *)p;

    delete h->vad;
    h->vad = NULL;
    free(h);

    PyErr_Clear(); // ensure no lingering errors from any capsule calls
}

static inline VadHandle *get_handle(PyObject *cap) {
    return (VadHandle *)PyCapsule_GetPointer(cap, CAPSULE_NAME);
}

// -------------------- create_vad() --------------------
static PyObject *mod_create_vad(PyObject *, PyObject *) {
    auto *h = (VadHandle *)malloc(sizeof(VadHandle));
    if (!h) {
        return PyErr_NoMemory();
    }

    h->vad = new micro_vad::MicroVad;

    PyObject *capsule =
        PyCapsule_New((void *)h, CAPSULE_NAME, vad_capsule_destructor);
    if (!capsule) {
        delete h->vad;
        h->vad = NULL;
        free(h);
        return NULL;
    }
    return capsule;
}

// -------------------- process_10ms(vad, audio) --------------------
static PyObject *mod_process_10ms(PyObject *, PyObject *args,
                                  PyObject *kwargs) {
    static const char *kwlist[] = {"vad", "audio", NULL};
    PyObject *cap = NULL;
    const char *data = nullptr;
    Py_ssize_t len = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oy#", (char **)kwlist, &cap,
                                     &data, &len)) {
        return NULL;
    }

    VadHandle *h = get_handle(cap);
    if (!h) {
        // PyCapsule_GetPointer already set an error (name mismatch or NULL)
        return NULL;
    }

    if (len < (Py_ssize_t)micro_vad::BYTES_PER_CHUNK) {
        PyErr_Format(
            PyExc_ValueError,
            "audio length (%zd bytes) < required chunk size (%u bytes)",
            (Py_ssize_t)len, (unsigned)micro_vad::BYTES_PER_CHUNK);
        return NULL;
    }

    int16_t *samples = (int16_t *)data;
    micro_vad::FloatType prob = 0;

    Py_BEGIN_ALLOW_THREADS prob = h->vad->Process10ms(samples);
    Py_END_ALLOW_THREADS

        PyObject *py_prob = PyFloat_FromDouble(static_cast<double>(prob));
    if (!py_prob) {
        return NULL;
    }

    return py_prob;
}

// -------------------- reset_vad(vad) --------------------
static PyObject *mod_reset_vad(PyObject *, PyObject *args) {
    PyObject *cap = NULL;
    if (!PyArg_ParseTuple(args, "O", &cap))
        return NULL;

    VadHandle *h = get_handle(cap);
    if (!h) {
        return NULL;
    }

    h->vad->Reset();

    Py_RETURN_NONE;
}

// -------------------- module boilerplate --------------------
static PyMethodDef module_methods[] = {
    {"create_vad", (PyCFunction)mod_create_vad, METH_NOARGS,
     "create_vad() -> capsule"},
    {"process_10ms", (PyCFunction)mod_process_10ms,
     METH_VARARGS | METH_KEYWORDS, "process_10ms(vad, audio: bytes) -> float"},
    {"reset_vad", (PyCFunction)mod_reset_vad, METH_VARARGS,
     "reset_vad(vad) -> None"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    "micro_vad_cpp",
    "Self-contained voice activity detector",
    -1,
    module_methods,
    NULL,
    NULL,
    NULL,
    NULL};

PyMODINIT_FUNC PyInit_micro_vad_cpp(void) {
    PyObject *m = PyModule_Create(&module_def);
    if (!m)
        return NULL;

#ifdef VERSION_INFO
    PyObject *ver = PyUnicode_FromString(VERSION_INFO);
#else
    PyObject *ver = PyUnicode_FromString("dev");
#endif
    if (ver) {
        PyModule_AddObject(m, "__version__", ver); // steals ref
    }
    return m;
}
