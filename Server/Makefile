
Server : Server.o thread.o socket.o socketserver.o
	g++ -o Server Server.o thread.o socket.o socketserver.o -pthread

Server.o : Server.cpp socket.h socketserver.h thread.h
	g++ -c Server.cpp

thread.o : thread.cpp thread.h ThreadSem.h
	g++ -c thread.cpp

socket.o : socket.cpp socket.h
	g++ -c socket.cpp

socketserver.o : socketserver.cpp socket.h socketserver.h
	g++ -c socketserver.cpp

Cleaner : CleanThreadSem.cpp
	g++ -o Cleaner CleanThreadSem.cpp -pthread 