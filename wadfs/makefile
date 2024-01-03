#daemon makefile

#$ g++ -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 wadfs.cpp -o wadfs -lfuse -L ../libWad -lWad -w

CC = g++
CFLAGS = -Wall -std=c++11 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26
LDFLAGS = -lfuse -L../libWad -lWad -w

TARGET = wadfs
SOURCE = wadfs.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)