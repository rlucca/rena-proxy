
SRC_FILES=adapter_base_service.cc		\
	adapter_base_xaction.cc				\
	adapter_header_visitor.cc			\
	adapter_service_modifier.cc			\
	squid_internal_action.cc

PY_FILES=adapter_python_interface.c queue.c

HDR_FILES=adapter_base_service.h		\
	adapter_header_visitor.h			\
	adapter_base_xaction.h				\
	queue.h								\
	squid_internal_action.h


OBJ_FILES=${SRC_FILES:.cc=.o} ${PY_FILES:.c=.o}


CPPFLAGS=-g -Wall -Wwrite-strings -pipe -fPIC `python3.8-config --cflags`
CXXFLAGS=-Woverloaded-virtual
LDFLAGS=-shared -lecap `python3.8-config --ldflags`
LIBRARY_NAME=ecap_adapter_service_modifier.so


.PHONY: help clean compile library
help:
	@-echo Utilize os targets \'clean compile library\' para criar o arquivo: ${LIBRARY_NAME}

clean:
	-rm -rf ${OBJ_FILES} ${LIBRARY_NAME} test_python_adapter

compile: ${OBJ_FILES}
library: ${LIBRARY_NAME}

${LIBRARY_NAME}: ${OBJ_FILES}
	g++ ${LDFLAGS} ${OBJ_FILES} -Wl,-E -Wl,-soname -Wl,${LIBRARY_NAME} -o ${LIBRARY_NAME}


objs:
	echo ${OBJ_FILES}

test_python_adapter: test_python_adapter.c
	gcc -g `python3.8-config --cflags` `python3.8-config --ldflags` -O0 adapter_python_interface.c -lecap test_python_adapter.c -o $@
