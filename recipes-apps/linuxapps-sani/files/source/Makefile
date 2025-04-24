TARGET = linuxapps-sani

SRCS = $(shell find . -type f -name "*.c")

HEADS = $(shell find ./include -type f -name "*.h")
OBJS = $(SRCS:.c=.o)
DEPS = Makefile.depend

INCLUDES += -I./include -I$(OECORE_TARGET_SYSROOT)/usr/include
CCFLAGS += -O2 -Wall $(INCLUDES)
LDFLAGS += -lm

all: $(TARGET)

$(TARGET): $(OBJS) $(HEADS)
	@echo $(SRCS)
	@echo $(HEADS)
	@echo $(CC)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

run: all
	@./$(TARGET)

.PHONY: depend clean
depend:
	$(CC) $(CCFLAGS) -MM $(SRCS) > $(DEPS)
	@sed -i -E "s/^(.+?).o: ([^ ]+?)\1/\2\1.o: \2\1/g" $(DEPS)

clean:
	$(RM) $(OBJS) $(TARGET)

-include $(DEPS)