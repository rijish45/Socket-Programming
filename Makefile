TARGETS=ringmaster player

all: $(TARGETS)

ringmaster: ringmaster.c
	gcc -g -Wall -std=gnu99 -o $@ $<

player: player.c
	gcc -g -Wall -std=gnu99 -o $@ $<

clean:
	rm -f $(TARGETS) *~