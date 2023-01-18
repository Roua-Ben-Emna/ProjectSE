all: ourShell

ourShell: ourShell.c
	gcc -o ourShell ourShell.c -lreadline
	
clean:
	- rm ourShell


.PHONY: all ourShell clean





