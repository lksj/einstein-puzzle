# This file is part of Einstein Puzzle

# Einstein Puzzle
# Copyright (C) 2003-2005  Flowix Games

# Modified 2018-02-10 by Jordan Evens <jordan.evens@gmail.com>

# Einstein Puzzle is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# Einstein Puzzle is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http:#www.gnu.org/licenses/>.


OPTIMIZE=-O6
CFLAGS=-Wall $(OPTIMIZE)
LNFLAGS=-lz
LIBS=-lz

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

