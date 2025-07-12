# ===/ Vars \===

CC = gcc

CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

SRC = \
	src/codJSON.c

OBJ = \
	build/codJSON.o

TARGET = bin/exec

# ===/ RULES \===

all: clean dirs $(TARGET) #run

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

compile: $(TARGET)

dirs:
	mkdir -p build bin

build/%.o: src/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin

run:
	./$(TARGET)