all: cache.cpp
	g++ --std=c++11 cache.cpp -o cache
clean: cache
	rm cache
