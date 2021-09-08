#include "adapter_python_interface.h"
#include <stdlib.h>
#include <string.h>

struct pyin_callable {
    const char *function_name;
    PyObject *module;
    PyObject *arguments;
    PyObject *keywords;
    PyObject *ret_value;
};

void pyin_finish(PyObject **module, PyObject **controller)
{
    Py_CLEAR(*controller);
    *controller = NULL;

    Py_CLEAR(*module);
    *module = NULL;

    Py_Finalize();
}

void pyin_start(const char *pypath)
{
    setenv("PYTHONPATH", pypath, 1);
    Py_Initialize();
    // Pode ser interessante desabilitar o tratamento de sinais
    // que o python coloca, dai executar a linha abaixo ao inves
    //Py_InitializeEx(0);
}

PyObject *pyin_get_module(const char *module)
{
    PyObject *filename = NULL;
    PyObject *ret = NULL;

    filename = PyUnicode_FromString(module);
    if (!filename) exit(__LINE__);

    ret = PyImport_Import(filename);
    Py_DECREF(filename);

    if (ret == NULL) {
        PyErr_Print();
        fprintf(stderr, "failed to load module name \"%s\"\n", module);
        exit(__LINE__);
    }

    return ret;
}

static int pyin_call_internal(struct pyin_callable *pyca)
{
    PyObject *fnc = NULL;

    fnc = PyObject_GetAttrString(pyca->module, pyca->function_name);

    if (!fnc || !PyCallable_Check(fnc)) {
        Py_CLEAR(fnc);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr,
                "failed to load function \"%s\" or is not callable\n",
                pyca->function_name);
        return -2;
    }

    pyca->ret_value = PyObject_Call(fnc,
                              pyca->arguments, // p/ NULL enviar tupla vazia
                              pyca->keywords); // pode ser NULL
    Py_CLEAR(fnc);

    if (pyca->ret_value == NULL) {
        // nunca eh retornado NULL, valor sem retorno eh Py_None
        // nao ha log aqui pq esta dentro de cada funcao
        return -1;
    }

    return 0;
}

static void get_bytes_from_python(PyObject *new_value,
                                  char **ab,
                                  size_t *len)
{
    size_t sz = PyBytes_Size(new_value);
    char *s = PyBytes_AsString(new_value);
    *ab = (char *) malloc(sz);
    memcpy(*ab, s, sz);
    *len = sz;
}

PyObject *pyin_create_controller(PyObject *module, const char *suffix,
                                 const char *db_path)
{
    // Essa classe possui um from ... import ... no modulo python
    // a alocacao eh passada sempre como primeiro parametro nas
    // funcoes que sao chamadas!
    struct pyin_callable pyca = {
        "ProxyController",
        module, NULL, NULL, NULL
    };

    pyca.arguments = Py_BuildValue("(y)", suffix);
    if (pyca.arguments == NULL) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "cant build argument list\n");
        exit(__LINE__);
        return NULL;
    }

    pyca.keywords = Py_BuildValue("{ss}", "database_path", db_path);
    if (pyca.keywords == NULL) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "cant build keyword argument list\n");
        exit(__LINE__);
        return NULL;
    }


    if (pyin_call_internal(&pyca))
    {
        Py_CLEAR(pyca.arguments);
        Py_CLEAR(pyca.keywords);
        Py_CLEAR(pyca.ret_value);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr,
                "cant call ProxyController on python module\n");
        exit(__LINE__);
        return NULL;
    }

    Py_CLEAR(pyca.arguments);
    Py_CLEAR(pyca.keywords);

    return pyca.ret_value;
}

int pyin_adapt_begin(struct pyin_adapt_text *at,
                     PyObject *controller,
                     const char *uri, int request_service)
{
    struct pyin_callable pyca = {
        "adapt_begin",
        at->module, NULL, NULL, NULL
    };

    at->state = Py_BuildValue("{sOsssisi}",
                          "pc", controller,
                          "uri", uri,
                          "request_service", request_service,
                          "reply_service", !request_service);

    if (at->state == NULL) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to create state for adapt_begin\n");
        return -2;
    }

    pyca.arguments = Py_BuildValue("(O)", at->state);
    if (pyca.arguments == NULL) {
        Py_CLEAR(at->state);
        at->state = NULL;
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to create args for adapt_begin\n");
        return -3;
    }

    if (pyin_call_internal(&pyca)) {
        Py_CLEAR(pyca.arguments);
        Py_CLEAR(at->state);
        at->state = NULL;
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to call adapt_begin\n");
        return -4;
    }

    Py_CLEAR(pyca.arguments);
    Py_CLEAR(pyca.ret_value); // a funcao devolve sempre None
    
    return 0;
}

int pyin_adapt_end(struct pyin_adapt_text *at)
{
    struct pyin_callable pyca = {
        "adapt_end",
        at->module, NULL, NULL, NULL
    };
    PyObject *new_value = NULL;

    pyca.arguments = Py_BuildValue("(O)", at->state);
    if (pyca.arguments == NULL) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to create args for adapt_end\n");
        return -1;
    }

    if (pyin_call_internal(&pyca)) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to call adapt_end\n");
    }

    Py_CLEAR(pyca.arguments);

    // new value is a pointer inside ret
    if (!PyArg_Parse(pyca.ret_value, "O", &new_value))
    {
        Py_CLEAR(pyca.ret_value);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to interpret adapt_end\n");
        return -4;
    }

    if (new_value && new_value != Py_None) {
        get_bytes_from_python(new_value, &at->ab, &at->ab_len);
    }

    Py_CLEAR(pyca.ret_value);
    // nao precisamos mais de estado temporario...
    Py_CLEAR(at->state);
    at->state = NULL;
    return 0;
}

int pyin_adapt_header(struct pyin_adapt_text *at, const char *name)
{
    struct pyin_callable pyca = {
        "adapt_header",
        at->module, NULL, NULL, NULL
    };
    int changed = -1;
    PyObject *new_value = NULL;

    // Se um valor de header for binario ele vira como Base64...
    pyca.arguments = Py_BuildValue("(Oy#y#)", at->state,
                         // 51200 eh 48k que reflete o maior header
                         name, strnlen(name, 51201),
                         at->vb, at->vb_len);
    if (pyca.arguments == NULL) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to create args for adapt_header\n");
        return -1;
    }

    if (pyin_call_internal(&pyca)) {
        Py_CLEAR(pyca.arguments);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to call adapt_header\n");
        return -2;
    }

    Py_CLEAR(pyca.arguments);

    // new value is a pointer inside ret
    if (!PyArg_Parse(pyca.ret_value, "(pO)", &changed, &new_value))
    {
        Py_CLEAR(pyca.ret_value);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr,
                "failed to interpret return data from adapt_header\n");
        return -4;
    }

    if (new_value && new_value != Py_None && changed > 0) {
        get_bytes_from_python(new_value, &at->ab, &at->ab_len);
    }

    Py_CLEAR(pyca.ret_value);
    return changed; // 0 false, 1 true
}

int pyin_adapt_body(struct pyin_adapt_text *at)
{
    struct pyin_callable pyca = {
        "adapt_body",
        at->module, NULL, NULL, NULL
    };
    PyObject *new_value = NULL;

    pyca.arguments = Py_BuildValue("(Oy#)",
                                   at->state, at->vb, at->vb_len);
    if (pyca.arguments == NULL) {
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to create args for adapt_body\n");
        return -1;
    }

    if (pyin_call_internal(&pyca)) {
        Py_CLEAR(pyca.arguments);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to call adapt_body\n");
        return -2;
    }
    Py_CLEAR(pyca.arguments);

    // new value is a pointer inside ret
    if (!PyArg_Parse(pyca.ret_value, "O", &new_value))
    {
        Py_CLEAR(pyca.ret_value);
        PyErr_Print(); PyErr_Clear();
        fprintf(stderr, "failed to interpret adapt_body\n");
        return -4;
    }

    if (new_value && new_value != Py_None) {
        get_bytes_from_python(new_value, &at->ab, &at->ab_len);
    }

    Py_CLEAR(pyca.ret_value);
    return 0;
}
