CC = gcc
CFLAGS_DEBUG= -Wall -Werror  -DDEBUG -DKILLONERR -g -O0 -I. -ggdb -std=c99  $(OTHER)
CFLAGS_DEPLOY= -g -pg -std=c99 $(OTHER)
DEPS = server.h client.h parser.h data_structures.h db_fileio.h helper.h bit_vector.h config.h
LIBS = -lpthread
OBJDB = server.o worker.o
OBJCLIENT = client.o
OBJCOMMON = helper_load.o helper_insert.o helper_init.o helper_socket.o helper_join.o helper_fetch_command.o helper_aggregation.o helper_math_command.o helper_variable.o helper_file_read.o helper_updates.o helper_math.o helper_file_open.o helper_misc.o parser.o bit_vector.o bptree.o threadpool.o list.o client_helper.o
OBJTEST = unit_test.o unit_test_util.o unit_test_sorted.o unit_test_bptree.o

debug: CFLAGS=$(CFLAGS_DEBUG)
debug: server client tests
optimal: CFLAGS=$(CFLAGS_DEPLOY)
optimal: server client tests

server: $(OBJDB) $(OBJCOMMON)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

client: $(OBJCLIENT) $(OBJCOMMON)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

tests: $(OBJTEST) $(OBJCOMMON)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	@rm -f *.o
	@if [ -a db_soc ] ; \
	then \
     rm db_soc ; \
  fi;
	@if [ -a tests ] ; \
	then \
     rm tests ; \
	fi;
	@if [ -a server ] ; \
	then \
     rm server ; \
	fi;
	@if [ -a client ] ; \
	then \
     rm client ; \
	fi;

nuke:
	rm -f *.data

deploy:
	$(MAKE) clean
	$(MAKE) optimal

rebuild:
	$(MAKE) clean
	$(MAKE) debug
