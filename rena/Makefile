
SOURCES = src/logger.c             \
          src/config.c             \
          src/rena.c               \
          src/task_runner.c        \
          src/database.c           \
          src/database_reader.c    \
          src/server.c             \
          src/proc.c               \
          src/tree.c               \
          src/clients.c            \
          src/http.c               \
          src/task_manager.c       \
          src/context.c            \
          src/context_full_link.c  \
          src/context_html.c       \
          src/base64.c             \
          src/template.c           \
          external/libini/libini.c \
          external/queue/queue.c

TESTS = tests/logger_unit_test.c   \
        tests/config_unit_test.c   \
        tests/base64_unit_test.c   \
        tests/tree_unit_test.c     \
        tests/database_unit_test.c \
        tests/template_unit_test.c \
        tests/database_reverse_unit_test.c \
        tests/clients_list_unit_test.c \
        tests/context_full_link_unit_test.c \
        tests/config_setter_unit_test.c

OBJS = $(SOURCES:.c=.o)
TEXEC = $(TESTS:.c=)

CC=gcc
CFLAGS=-pthread -Isrc -Iexternal/libini -Iexternal/queue -Wall \
       -Iexternal/valgrind
LDFLAGS=-lcrypto -lssl -lm
TCFLAGS=-Iexternal/cheat -Itests -I.
RCID=$(shell git show -s --format="\"\\\"%h\\\"\"")
RCTI=$(shell git show -s --format="\"\\\"%s\\\"\"")
RCUD=$(shell git show -s --format="\"\\\"%ci\\\"\"")
VERSION=-DRENA_ID=${RCID} -DRENA_TITLE=${RCTI} -DRENA_DATE=${RCUD}

ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0
endif

.PHONY: rena clean install tests

rena: src/main.o $(OBJS)
	-$(CC) $(CFLAGS) -o src/$@ $^ $(LDFLAGS)

clean:
	-rm -rf src/main.o $(OBJS) src/rena $(TEXEC)

tests: $(TEXEC)
	@-for filename in $(TEXEC); do echo $${filename}; ./$${filename}; done

install:
	@-echo pending

############################################################################

src/rena.o: src/rena.c src/rena.h
	-$(CC) $(CFLAGS) $(VERSION) -c -o $@ $<

%.o: %.c %.h
	-$(CC) $(CFLAGS) -c -o $@ $<

%_unit_test: %_unit_test.c $(OBJS)
	-$(CC) $(CFLAGS) $(TCFLAGS) -o $@ $< $(OBJS) $(LDFLAGS)

%clients_list_unit_test: %clients_list_unit_test.c $(OBJS:src/clients.o=)
	-$(CC) $(CFLAGS) $(TCFLAGS) -o $@ $< $(OBJS:src/clients.o=) $(LDFLAGS)
