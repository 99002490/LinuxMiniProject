all: httpServerFunctions.o httpServer.o
        gcc -pthread -o httpServer httpServerFunctions.o httpServer.o

httpServerFunction.o:
        gcc httpServerFunctions.c -pthread -c -o httpServerFunctions.o

httpServer.o:
        gcc httpServer.c -pthread -c -o httpServer.o

run:
        ./httpServer

clean:
        rm -rf httpServerFunctions.o httpServer.o httpServer
