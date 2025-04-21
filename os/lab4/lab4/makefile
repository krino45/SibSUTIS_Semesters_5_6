CC=g++

OBJ=obj
APPS=apps

SOURCES = $(wildcard $(APPS)/*.cpp)
EXECUTABLES = $(patsubst $(APPS)/%.cpp, my_%, $(SOURCES))

all: $(EXECUTABLES) mainApp forkApp

build: $(EXECUTABLES) mainApp forkApp

run:
	./lab4

run_fork:
	./lab4_fork

mainApp: 
	$(CC) -o lab4 lab4.cpp

forkApp:
	$(CC) -o lab4_fork lab4_fork.cpp

my_%: $(APPS)/%.cpp
	$(CC) $< -o $(OBJ)/$@

clean:
	rm $(OBJ)/my_* lab4 lab4_fork