cpp_OBJ=$(wildcard *.cpp)
o_OBJ=$(patsubst %.cpp,%.o,$(cpp_OBJ))
shimd: $(o_OBJ)
	g++ -o shimd $^ -pthread -w -std=c++11
%.o: %.cpp all_func.h data_def.h
	g++ -o $@ -c $< -pthread -w -std=c++11
.PHONY: clean
clean:
	rm -rf *.o shimd