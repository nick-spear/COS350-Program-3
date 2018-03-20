# build an executable named myprog from myprog.c
all: mysubmit.c 
	gcc mysubmit.c -o mysubmit

debug: mysubmit.c
	gcc -g mysubmit.c -o mysubmit

clean:
	rm mysubmit
