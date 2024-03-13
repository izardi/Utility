CXX = clang++
TARGET = mywget

CXXFLAGS = -Wall -std= c++2a

SRC = $(wildcard *.cpp)

OBJ = $(patsubst %.cpp, %.o, $(SRC))

include = ./include

$(TARGET) : $(OBJ)
	$(CXX) $^ -o $@

%.o : %.cpp
	$(CXX) $< -c  -I $(include) -o $@

.PHONY:clean

clean:
	rm -rf $(OBJ) $(TARGET)
