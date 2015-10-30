OPTIMIZE=-O6
CFLAGS=-Wall $(OPTIMIZE)
LNFLAGS=-lz

TARGET=mkres
SOURCES=main.cpp compressor.cpp unicode.cpp streams.cpp table.cpp \
	lexal.cpp convert.cpp buffer.cpp format.cpp messages.cpp \
	msgformatter.cpp
HEADERS=compressor.h unicode.h streams.h lexal.h convert.h table.h \
	buffer.h format.h messages.h msgformatter.h
OBJECTS=main.o compressor.o unicode.o streams.o lexal.o table.o \
	convert.o buffer.o format.o messages.o msgformatter.o

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

all: $(TARGET)

depend:
	@makedepend $(SOURCES) 2> /dev/null

$(TARGET): $(OBJECTS)
	$(CXX) $(LNFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

clean: 
	rm -f $(OBJECTS) $(TARGET) core

# DO NOT DELETE THIS LINE -- make depend depends on it.

