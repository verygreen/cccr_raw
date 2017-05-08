all: teslaraw

teslaraw: teslaraw.c
	gcc -Wall teslaraw.c -o teslaraw
