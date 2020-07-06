CC=arm-none-linux-gnueabi-gcc
LD=arm-none-linux-gnueabi-gcc
CFLAGS += -Wall
# LDFLAGS +=

DIST=tuart
OBJS=$(DIST).o

all: $(DIST)

$(DIST):$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@


clean:
	rm -f $(DIST) $(OBJS)
