.PHONY: clean all

simulador: main.c map.c io.c cpu.c queue.c process.c helper.c
	gcc -Wall main.c cpu.c process.c queue.c map.c helper.c io.c -o simulador

run: simulador
	./simulador

debug: main.c cpu.c io.c map.c queue.c process.c helper.c
	gcc -g main.c map.c cpu.c queue.c process.c helper.c io.c -o main

clean:
	rm simulador main out*
