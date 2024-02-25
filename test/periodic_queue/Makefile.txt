CXX=g++
CC=$(CXX)

CFLAGS = -pedantic -Wall -Werror
CFLAGS += -ggdb
LDLIBS = -pthread 
CXXFLAGS = $(CFLAGS) -std=c++17

CPPFLAGS = -DDEBUG
CPPFLAGS += -I$(INCLUDES_DIR)

INCLUDES_DIR = ../../inc

SOURCE = utest.cpp
TARGET = utest

vala: $(TARGET)
	valgrind --leak-check=yes --track-origins=yes ./$(TARGET)

hell: $(TARGET)
	valgrind --tool=helgrind ./$(TARGET) 2> hell.txt
    
all: $(TARGET)

utest : $(SOURCE)

check : $(TARGET) 
	@./$(TARGET) -v

recheck: clean check

clean:
	@$(RM) -f ./$(TARGET) *.o


.PHONY : make clean check recheck all vala hell

make:
	@echo 'Attend a maker faire'
	@${TRUE}
