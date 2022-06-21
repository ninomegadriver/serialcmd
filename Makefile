bindump: serialcmd.c
	gcc -Werror serialcmd.c -o serialcmd

clean:
	rm -rf serialcmd
