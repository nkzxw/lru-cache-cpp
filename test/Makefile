BOOST_INCLUDE=-I/remote/users4/pbrunell/test/boost_1_46_1
BOOST_LIBPATH=-L/remote/users4/pbrunell/test/boost_1_46_1/stage/lib
BOOST_LIBS=-lboost_regex

OPTIONS=-g $(BOOST_INCLUDE) $(BOOST_LIBPATH) $(BOOST_LIBS)

all: smaps_test lru_tests lru_comp
	./smaps_test
	./lru_tests
	./lru_comp

smaps.o: smaps.cpp smaps.hpp
	g++ -c -o smaps.o $(OPTIONS) smaps.cpp

smaps_test: smaps_test.cpp smaps.o smaps.hpp
	g++ -o smaps_test $(OPTIONS) smaps_test.cpp smaps.o

lru_tests: lru_tests.cpp ../lru.hpp
	g++ -o lru_tests $(OPTIONS) lru_tests.cpp

lru_comp: lru_comp.cpp smaps.o ../lru.hpp lru_cache.h
	g++ -o lru_comp $(OPTIONS) lru_comp.cpp smaps.o

clean:
	\rm -f smaps.o smaps_test lru_tests lru_comp