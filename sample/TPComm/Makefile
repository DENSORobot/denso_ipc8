TARGET = TPCommSample

MOUSE_NUMBER = 0
COM_PORT = 0

CC = gcc
LIBS = -L../../library/lib
INCS = -I../../library/include
DEFS = -DINPUT_MOUSE_NUMBER=$(MOUSE_NUMBER) -DTP_COM_PORT=$(COM_PORT) -DDEBUG=0
#DEFS = -DINPUT_MOUSE_NUMBER=$(MOUSE_NUMBER) -DTP_COM_PORT=$(COM_PORT) -DDEBUG=1

SRCS = TPCommSample.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS) -lTPComm

.c.o:
	$(CC) -c -MMD -MP $< $(INCS) $(DEFS)

clean:
	rm -f $(OBJS) $(DEPS)

run:
	sudo chmod 777 /dev/input/event$(MOUSE_NUMBER)
	sudo chmod 777 /dev/ttyS$(COM_PORT)
	@echo "Run $(TARGET)"
	@LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$(shell echo $(subst -L,,$(LIBS)) | tr ' ' ':')" ./$(TARGET)
