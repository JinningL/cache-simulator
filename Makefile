CC     = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11

SRC = main.c config.c cache.c hierarchy.c policy.c trace.c stats.c utils.c
OBJ = $(SRC:.c=.o)
TARGET = cache_sim

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET) \
	  --l1-size 32768  --l1-assoc 4  \
	  --l2-size 262144 --l2-assoc 8  \
	  --l3-size 8388608 --l3-assoc 16 \
	  --block 64 --policy LRU trace.txt
