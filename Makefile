CC            := gcc
OBJCPY        := objcopy
STRIP         := strip
LDFLAGS       :=
CFLAGS        :=
CFLAGS        := $(CFLAGS) -Wall -Wextra -g # Set build warnings and debugging
CFLAGS        := $(CFLAGS) -std=c99 # The standards to build to.
CFLAGS        := $(CFLAGS) -fno-stack-check -fno-stack-protector -fomit-frame-pointer -ffunction-sections -flto
CFLAGS        := $(CFLAGS) -O3 -MMD
TARGET        := daplink_flasher

DEFINES       :=
TAG           :=

GFILES        :=
GFILES        := $(GFILES) main.o

# What list of base filenames are we to build?
FILES_BASE    := $(basename $(GFILES))

.PHONY: all clean echelon_emu qemu_virt emu emu-debug emu-linux emu-linux-debug emu-opensbi-linux emu-opensbi-linux-debug debug

all: $(TARGET).dynamic $(TARGET).dynamic.strip $(TARGET).static $(TARGET).static.strip

rebuild: clean
	$(MAKE) all

clean:
	rm -f $(TARGET).dynamic $(TARGET).dynamic.strip $(TARGET).static $(TARGET).static.strip
	rm -f $(addsuffix .o,$(FILES_BASE))
	rm -f $(addsuffix .d,$(FILES_BASE))

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(TARGET).dynamic: $(GFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(TARGET).static: $(GFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -static $^ -o $@

%.strip: %
	$(STRIP) -s -x -R .comment $^ -o $@

# Header dependency tracking
-include $(addsuffix .$(TAG).d,$(GFILES))
