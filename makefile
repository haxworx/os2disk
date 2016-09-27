TARGET = os2disk
SRC_DIR=src

PKGS=elementary openssl

LIBS = 

ifeq ($(OS),Windows_NT)
	OS := WINDOWS=1
else
	OS := UNIX=1
endif

FLAGS =  -g -ggdb3 $(shell pkg-config --libs --cflags $(PKGS))
INCLUDES = $(shell pkg-config --cflags $(PKGS))

OBJECTS = disks.o ui.o core.o main.o

$(TARGET) : $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) $(FLAGS) -o $@

main.o: $(SRC_DIR)/main.c
	$(CC) $(INCLUDES) -c $(SRC_DIR)/main.c -o $@

core.o: $(SRC_DIR)/core.c
	$(CC) $(INCLUDES) -c $(SRC_DIR)/core.c -o $@

disks.o:$(SRC_DIR)/disk.c
	$(CC) $(INCLUDES) -c $(SRC_DIR)/disk.c -o $@

ui.o: $(SRC_DIR)/ui.c
	$(CC) $(INCLUDES) -c $(SRC_DIR)/ui.c -o $@

clean:
	-rm $(OBJECTS) $(TARGET)
