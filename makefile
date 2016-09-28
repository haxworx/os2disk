TARGET = os2disk
SRC_DIR=src

PKGS=ecore ecore-con elementary openssl 
# eeze
LIBS = 

ifeq ($(OS),Linux)
	PKGS += eeze
else
endif

CFLAGS= -g -ggdb3
FLAGS =  -g -ggdb3 $(shell pkg-config --libs --cflags $(PKGS))
INCLUDES = $(shell pkg-config --cflags $(PKGS))

OBJECTS = disks.o ui.o core.o main.o

$(TARGET) : $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) $(FLAGS) -o $@

main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/main.c -o $@

core.o: $(SRC_DIR)/core.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/core.c -o $@

disks.o:$(SRC_DIR)/disk.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/disk.c -o $@

ui.o: $(SRC_DIR)/ui.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/ui.c -o $@

clean:
	-rm $(OBJECTS) $(TARGET)
