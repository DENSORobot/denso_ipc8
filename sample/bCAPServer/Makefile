TARGET = bCAPServerSample

CC = g++
LIBS = -L../../library/lib
INCS = -I../../library/include
DEFS = 

SRCS = bCAPServerSample.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBS) -lbCAPServer

.cpp.o:
	$(CC) -c -MMD -MP $< $(INCS) $(DEFS)

clean:
	rm -f $(OBJS) $(DEPS)

run:
	@echo "Run $(TARGET)"
	@LD_LIBRARY_PATH="$$LD_LIBRARY_PATH:$(shell echo $(subst -L,,$(LIBS)) | tr ' ' ':')" ./$(TARGET)
