main: src/main.cpp
	mkdir -p target
	g++ src/main.cpp -o target/ari -std=c++23
run: main
	./target/ari
