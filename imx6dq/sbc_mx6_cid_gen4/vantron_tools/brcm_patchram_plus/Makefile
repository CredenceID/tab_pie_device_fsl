CC=arm-none-linux-gnueabi-gcc
LD=arm-none-linux-gnueabi-gcc
CFLAGS += -Wall
# LDFLAGS +=

DIST=brcm_patchram_plus
OBJS=$(DIST).o

all: $(DIST)

$(DIST):$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@


clean:
	rm -f $(DIST) $(OBJS)
