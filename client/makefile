COMPILER = /home/student/Desktop/souboggio/ex8/buildroot-avr32-v3.0.0_TTK4147/output/staging/usr/bin/avr32-linux-gcc


# program executable name
TARGET = cafebabe
# compiler flags
CFLAGS = -g -Wall -D_GNU_SOURCE 
#-I/usr/realtime/include
# linker flags
LDFLAGS = -g  -pthread -lm
# list of sources
SOURCES = $(shell find -name "*.c")
# default rule, to compile everything
all: $(TARGET)
# define object files
OBJECTS = $(SOURCES:.c=.o)
# link programs
$(TARGET): $(OBJECTS)
		$(COMPILER) -o $@ $^ $(LDFLAGS)
# compile
%.o : %.c
		$(COMPILER) $(CFLAGS) -c -o $@ $<
# cleaning
clean:
		rm -f $(TARGET) $(OBJECTS) 