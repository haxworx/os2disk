TARGET = os2disk
SRC_DIR=src

PKGS=elementary 

LIBS = 

ifeq ($(OS),Windows_NT)
	OS := WINDOWS=1
else
	OS := UNIX=1
endif

FLAGS =  -g -ggdb3 $(shell pkg-config --libs --cflags $(PKGS))

OBJECTS = core.o main.o

$(TARGET) : $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) -o $@

main.o: $(SRC_DIR)/main.c
	$(CC) $(FLAGS) -c $(SRC_DIR)/main.c -o $@

core.o: $(SRC_DIR)/core.c
	$(CC) $(FLAGS) -c $(SRC_DIR)/core.c -o $@

clean:
	-rm $(OBJECTS) $(TARGET)
