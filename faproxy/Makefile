SOURCE=src/main.c				\
	src/queue.c					\
	src/service_thread.c		\
	src/database.c				\
	src/database_internal.c		\
	src/tree.c					\
	src/ssl.c					\
	src/http.c					\
	src/request.c				\
	src/globals.c

HEADERS=src/globals.h			\
	src/database.h				\
	src/database_internal.h		\
	src/tree.h					\
	src/ssl.h					\
	src/http.h					\
	src/request.h				\
	src/queue.h

OBJS=${SOURCE:.c=.o}

CFLAGS=-g -Wall -Wwrite-strings -std=gnu99 -Wno-format-extra-args
LDFLAGS=-lpthread -lev -lssl -lcrypto

.PHONY: help compile fa
help:
	-@echo "Utilize os targes clean compile fa"

clean:
	-rm -rf ${OBJS} bin/fa bin/database_lookup_test

compile: ${OBJS} ${HEADERS}

fa: compile
	@gcc ${OBJS} -Wl,-E -o bin/$@ ${LDFLAGS}

database_lookup_test: compile
	@gcc ${LDFLAGS} ${CFLAGS} -Werror tests/database_lookup_test.c src/database.o src/database_internal.o src/globals.o src/tree.o -Wl,-E -o bin/$@ -Isrc

database_lookup_run: database_lookup_test
	./bin/database_lookup_test
