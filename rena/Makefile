
SOURCES = src/logger.c             \
          src/config.c             \
          external/libini/libini.c

TESTS = tests/logger_unit_test.c   \
        tests/config_unit_test.c   \
        tests/config_setter_unit_test.c

OBJS = $(SOURCES:.c=.o)
TEXEC = $(TESTS:.c=)

CC=gcc
CFLAGS=-Isrc -Iexternal/libini -Wall
TCFLAGS=-Iexternal/cheat -Itests -I.

.PHONY: rena clean install tests

rena: src/main.o $(OBJS)
	-$(CC) $(CFLAGS) -o src/$@ $^

clean:
	-rm -rf src/main.o $(OBJS) src/rena $(TEXEC)

tests: $(TEXEC)
	@-for filename in $(TEXEC); do echo $${filename}; ./$${filename}; done

install:
	@-echo pending

############################################################################

%.o: %.c %.h
	-$(CC) $(CFLAGS) -c -o $@ $<

%_unit_test: %_unit_test.c $(OBJS)
	-$(CC) $(CFLAGS) $(TCFLAGS) -o $@ $< $(OBJS)
