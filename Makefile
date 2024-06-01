build: main.c
	cc main.c -o bin/todo -Wall -Wextra -pedantic

.PHONY: clean
clean:
	$(RM) todo.bin bin/todo

