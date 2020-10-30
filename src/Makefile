.PHONY: clean all

simulador: main.c cpu.c process.c
	gcc -Wall main.c cpu.c process.c -o simulador

run: simulador
	./simulador

debug: main.c cpu.c process.c
	gcc -g main.c cpu.c process.c -o main

clean:
	rm simulador
