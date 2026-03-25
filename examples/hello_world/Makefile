CC     = gcc
CFLAGS = -I_parcel

stdout_FLAG =
null_FLAG    = -DOUTPUT_NULL

.PHONY: run clean

run: hello_stdout hello_null
	@echo "stdout implementation:"
	./hello_stdout
	@echo "null implementation (output is suppressed):"
	./hello_null
	@echo "(no output — correct)"

hello_%: output.c output/%.c main.c
	$(CC) $(CFLAGS) $($*_FLAG) $^ -o $@

clean:
	rm -f hello_stdout hello_null
