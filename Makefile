clean:
	rm -f p.out cs.out ft.out
all : peer centralserver filetransfer

peer : peer.c
	gcc -o p.out peer.c -w -fno-stack-protector
centralserver : centralServer.c
	gcc -o cs.out centralServer.c -w -fno-stack-protector
filetransfer : filetransfer.c
	gcc -o ft.out filetransfer.c -w -fno-stack-protector
