TARGET = bCAPClientSample

CC = gcc
LIBS = -L../../library/lib
INCS = -I../../library/include
DEFS = 

SRCS = bCAPClientSample.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS) -lbCAPClient

.c.o:
	$(CC) -c -MMD -MP $< $(INCS) $(DEFS)

clean:
	rm -f $(OBJS) $(DEPS)

run:
	@echo "Run $(TARGET)"
	@LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$(shell echo $(subst -L,,$(LIBS)) | tr ' ' ':')" ./$(TARGET)
