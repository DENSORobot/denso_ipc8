TARGET = DriverSample

MODULE_NAME = denso_ipc8
MODULE_LOADED := $(shell lsmod | grep $(MODULE_NAME))

CC = gcc
LIBS = 
INCS = -I../../driver_dkms
DEFS = 

SRCS = DriverSample.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

.c.o:
	$(CC) -c -MMD -MP $< $(INCS) $(DEFS)

clean:
	rm -f $(OBJS) $(DEPS)

run:
	$(if $(MODULE_LOADED),,sudo modprobe $(MODULE_NAME))
	sudo chmod 777 /dev/$(MODULE_NAME)
	./$(TARGET)
