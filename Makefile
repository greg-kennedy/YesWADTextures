CC ?= gcc

CFLAGS ?= -O2
CFLAGS += -Wall -Wextra

LFLAGS = -s

OBJ = utils.o main.o wad.o bsp.o texture.o
OUTPUT = yeswadtextures

all:	$(OUTPUT)

yeswadtextures: $(OBJ)
	$(CC) $(LFLAGS) -o $(OUTPUT) $(OBJ)

clean:
	rm -f *.o $(OUTPUT)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

