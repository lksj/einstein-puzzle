# This file is part of Einstein Puzzle

# Einstein Puzzle
# Copyright (C) 2003-2005  Flowix Games

# Modified 2009-02-09 by Tupone Alfredo <tupone@gentoo.org>

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


########################################
# 
# build parameters
#
########################################

# installation prefix
PREFIX=/usr/local

########################################
#
# do not modify rest of this file
#
########################################

OPTIMIZE=#-O6 -march=pentium4 -mfpmath=sse -fomit-frame-pointer -funroll-loops
PROFILER=#-pg
DEBUG=#-ggdb
CXXFLAGS=-pipe -Wall $(OPTIMIZE) $(DEBUG) `sdl-config --cflags` -DPREFIX=L\"$(PREFIX)\" $(PROFILER)
LNFLAGS=-pipe -lSDL_ttf -lfreetype `sdl-config --libs` -lz -lSDL_mixer $(PROFILER)
INSTALL=install

TARGET=einstein

SOURCES=puzgen.cpp main.cpp screen.cpp resources.cpp utils.cpp game.cpp \
	widgets.cpp iconset.cpp puzzle.cpp rules.cpp \
	verthints.cpp random.cpp horhints.cpp menu.cpp font.cpp \
	storage.cpp tablestorage.cpp regstorage.cpp \
	topscores.cpp opensave.cpp descr.cpp options.cpp messages.cpp \
	formatter.cpp buffer.cpp unicode.cpp convert.cpp table.cpp \
	i18n.cpp lexal.cpp streams.cpp tokenizer.cpp sound.cpp
OBJECTS=puzgen.o main.o screen.o resources.o utils.o game.o \
	widgets.o iconset.o puzzle.o rules.o verthints.o random.o \
	horhints.o menu.o font.o storage.o options.o \
	tablestorage.o regstorage.o topscores.o opensave.o descr.o \
	messages.o formatter.o buffer.o unicode.o convert.o table.o \
	i18n.o lexal.o streams.o tokenizer.o sound.o
HEADERS=screen.h main.h exceptions.h resources.h utils.h \
	widgets.h iconset.h puzzle.h verthints.h random.h horhints.h \
	font.h storage.h tablestorage.h regstorage.h \
	topscores.h opensave.h game.h descr.h options.h messages.h \
	foramtter.h buffer.h visitor.h unicode.h convert.h table.h \
	i18n.h lexal.h streams.h tokenizer.h sound.h

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LNFLAGS)

clean:
	rm -f $(OBJECTS) core* *core $(TARGET) *~

depend:
	@makedepend $(SOURCES) 2> /dev/null

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	$(INSTALL) -s -D $(TARGET) $(PREFIX)/bin/$(TARGET)
	$(INSTALL) -D einstein.res $(PREFIX)/share/einstein/res/einstein.res
	
# DO NOT DELETE THIS LINE -- make depend depends on it.

