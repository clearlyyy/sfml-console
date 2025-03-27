all: compile link

compile:
	g++ -IC:\SFML-2.6.2\include -c main.cpp

link:
	g++ main.o -o main -LC:\SFML-2.6.2\lib -lmingw32 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network && main.exe
