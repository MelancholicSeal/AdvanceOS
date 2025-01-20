CFLAGS=-Wall
TARGET=main
SRCS=main.c

all: $(TARGET)

$(TARGET):
	gcc $(CFLAGS) $(SRCS) -o $(TARGET)
clean:
	rm -f $(TARGET)
