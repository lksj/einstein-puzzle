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
CXXFLAGS=-pipe -Wall $(OPTIMIZE) $(DEBUG) `sdl-config --cflags` -DPREFIX=L\"$(PREFIX)\" $(PROFILER) -fpermissive
LNFLAGS=-pipe -lSDL_ttf -lfreetype `sdl-config --libs` -lz -lSDL_mixer $(PROFILER)
INSTALL=install

TARGET=einstein

SOURCES=puzgen.cpp main.cpp screen.cpp resources.cpp utils.cpp game.cpp \
	widgets.cpp iconset.cpp puzzle.cpp rules.cpp \
	verthints.cpp random.cpp horhints.cpp menu.cpp font.cpp \
	storage.cpp tablestorage.cpp regstorage.cpp \
	topscores.cpp opensave.cpp descr.cpp options.cpp messages.cpp \
	formatter.cpp i18n.cpp tokenizer.cpp sound.cpp
OBJECTS=puzgen.o main.o screen.o resources.o utils.o game.o \
	widgets.o iconset.o puzzle.o rules.o verthints.o random.o \
	horhints.o menu.o font.o storage.o options.o \
	tablestorage.o regstorage.o topscores.o opensave.o descr.o \
	messages.o formatter.o i18n.o tokenizer.o sound.o
HEADERS=screen.h main.h resources.h utils.h \
	widgets.h iconset.h puzzle.h verthints.h random.h horhints.h \
	font.h storage.h tablestorage.h regstorage.h \
	topscores.h opensave.h game.h descr.h options.h messages.h \
	formatter.h visitor.h i18n.h tokenizer.h sound.h

SHARE_SOURCES = unicode.cpp streams.cpp table.cpp buffer.cpp convert.cpp lexal.cpp
SHARE_HEADERS = unicode.h exceptions.h streams.h table.h buffer.h convert.h lexal.h
SHARE_OBJECTS = unicode.o streams.o table.o buffer.o convert.o lexal.o

RES_SOURCES=mkres.cpp compressor.cpp format.cpp msgwriter.cpp msgformatter.cpp
RES_HEADERS=compressor.h format.h msgwriter.h msgformatter.h
RES_OBJECTS=mkres.o compressor.o format.o msgwriter.o msgformatter.o

ALL_SOURCES=$(RES_SOURCES) $(SHARE_SOURCES) $(SOURCES)
ALL_HEADERS=$(RES_HEADERS) $(SHARE_HEADERS) $(HEADERS)
ALL_OBJECTS=$(RES_OBJECTS) $(SHARE_OBJECTS) $(OBJECTS)
ALL_FILES=$(ALL_SOURCES) $(ALL_HEADERS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

all: $(TARGET)

mkres: $(RES_OBJECTS) $(SHARE_OBJECTS)
	$(CXX) -lz $(RES_OBJECTS) $(SHARE_OBJECTS) -lz -o mkres

einstein: $(OBJECTS) $(SHARE_OBJECTS) einstein.res
	$(CXX) $(OBJECTS) $(SHARE_OBJECTS) $(LIBS) -o einstein $(LNFLAGS)

clean:
	$(RM) $(ALL_OBJECTS) *.exe *.res core* *core $(TARGET) *~

einstein.res: mkres
	cd res  && ../mkres --source resources.descr --output ../einstein.res && cd ..

run: einstein
	./einstein

install: $(TARGET)
	$(INSTALL) -s -D $(TARGET) $(PREFIX)/bin/$(TARGET)
	$(INSTALL) -D einstein.res $(PREFIX)/share/einstein/res/einstein.res
	
# DO NOT DELETE THIS LINE -- make depend depends on it.

