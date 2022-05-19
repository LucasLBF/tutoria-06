all: bin

bin: producer_consumer.c
	gcc $< -o $@ -pthread

.PHONY: clean run

clean:
	rm -f bin

run:
	./bin 
