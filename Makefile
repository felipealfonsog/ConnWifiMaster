CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`
SRC_DIR = src
TARGET = connwifimaster

all: $(TARGET)

$(TARGET): $(SRC_DIR)/main.o $(SRC_DIR)/connman.o
	$(CC) -o $@ $^ $(LIBS)

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/connman.h
	$(CC) -c $(SRC_DIR)/main.c $(CFLAGS) -o $(SRC_DIR)/main.o

$(SRC_DIR)/connman.o: $(SRC_DIR)/connman.c $(SRC_DIR)/connman.h
	$(CC) -c $(SRC_DIR)/connman.c $(CFLAGS) -o $(SRC_DIR)/connman.o

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
