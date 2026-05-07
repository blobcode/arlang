main: src/main.cpp
	g++ src/main.cpp -o ari -std=c++23
run: main
	./ari
