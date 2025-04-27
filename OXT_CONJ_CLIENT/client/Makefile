CC=g++
CFLAGS=-I. -I./blake3/
CONFIG=-std=c++17 -O3 -msse2 -msse -msse4 -mssse3 -march=native -maes -lpthread -lgmpxx -lgmp -lhiredis -lredis++ -pthread -Wl,-rpath,/usr/local/lib,./blake3/libblake3.so

all: sse_setup_client sse_search_client

sse_setup_client: aes.cpp rawdatautil.cpp ecc_x25519.cpp bloom_filter.cpp ./blake3/blake_hash.cpp mainwindow_client.cpp sse_setup_client.cpp
	$(CC) -o sse_setup_client aes.cpp rawdatautil.cpp ecc_x25519.cpp bloom_filter.cpp ./blake3/blake_hash.cpp mainwindow_client.cpp sse_setup_client.cpp $(CONFIG)

sse_search_client: aes.cpp rawdatautil.cpp ecc_x25519.cpp bloom_filter.cpp ./blake3/blake_hash.cpp mainwindow_client.cpp sse_search_client.cpp
	$(CC) -o sse_search_client aes.cpp rawdatautil.cpp ecc_x25519.cpp bloom_filter.cpp ./blake3/blake_hash.cpp mainwindow_client.cpp sse_search_client.cpp $(CONFIG)

.PHONEY: clean clean_all

clean:
	rm -rf *.o *.gch sse_setup_client sse_search_client

clean_all:
	rm -rf *.o *.gch sse_setup_client sse_search_client eidxdb.csv bloom_filter.dat
	@redis-cli flushall
	@redis-cli save
