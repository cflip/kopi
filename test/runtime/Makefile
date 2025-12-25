KOPIC=../../kopic

OBJS=run_tests.o arith.o functions.o vars.o
OUT=run_tests

$(OUT): $(OBJS)
	$(CC) -o $@ $^

%.o: %.kopi
	$(KOPIC) -o $@ $^

%.o: %.c
	$(CC) -o $@ -c $^
