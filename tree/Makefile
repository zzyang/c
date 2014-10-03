all:test
 
 CPPFLAGS+=-std=c++0x -Wall -pedantic
 CPPFLAGS+=-g -O0
 CPPFLAGS+=-isystem ~/tools/gnu/boost/
  
# CPPFLAGS+=-fopenmp
# CPPFLAGS+=-march=native
  #  
# LDFLAGS+=-L ~/custom/boost/stage/lib/ -Wl,-rpath,/home/sehe/custom/boost/stage/lib
# LDFLAGS+=-lboost_system -lboost_regex -lboost_thread -lpthread -lboost_iostreams -lboost_serialization
#  
%.o: %.cpp ast.hpp
	$(CXX) $(CPPFLAGS) $< -c -o $@
	 
test: main.o parser.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
