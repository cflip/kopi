KOPIC=../../kopic

OBJS=run_tests.o example.o vars.o
OUT=run_tests

$(OUT): $(OBJS)
	$(CC) -o $@ $^

%.o: %.kopi
	$(KOPIC) -o $@ $^

%.o: %.c
	$(CC) -o $@ -c $^
