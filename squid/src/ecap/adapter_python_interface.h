#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define PY_SSIZE_T_CLEAN size_t
#include <Python.h>

struct pyin_adapt_text {
    PyObject *module; // onda as coisas estao
    PyObject *state; // dados mantidos em uma requisicao
    const char * const vb;
    const size_t vb_len;
    char *ab;
    size_t ab_len;
};

void pyin_start(const char *pypath);
void pyin_finish(PyObject **module, PyObject **controller);

PyObject *pyin_get_module(const char *module);
PyObject *pyin_create_controller(PyObject *module, const char *suffix,
                                 const char *db_path);

int pyin_adapt_begin(struct pyin_adapt_text *at,
                     PyObject *controller,
                     const char *uri, int request_service);
// valor do header eh o que eh considerado virgem e adaptavel
int pyin_adapt_header(struct pyin_adapt_text *at,
                      const char *header_name);
int pyin_adapt_body(struct pyin_adapt_text *at);
// Responsavel por destruir o state armazenado
int pyin_adapt_end(struct pyin_adapt_text *at);


#ifdef __cplusplus
}
#endif
