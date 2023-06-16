CFLAGS=-Wall -Wextra -O2

xpointerscreen: xpointerscreen.c
	cc $(CFLAGS) xpointerscreen.c -lX11 -lXi -o xpointerscreen

clean:
	rm -f xpointerscreen
