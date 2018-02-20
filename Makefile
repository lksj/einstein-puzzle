# This file is part of Einstein Puzzle

# Einstein Puzzle
# Copyright (C) 2003-2005  Flowix Games

# Modified 2009-02-09 by Tupone Alfredo <tupone@gentoo.org>
# Modified 2018-02-19 by Jordan Evens <jordan.evens@gmail.com>

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
OPTIMIZE=-O3 #-march=pentium4 -mfpmath=sse -fomit-frame-pointer -funroll-loops
DEBUG=#-ggdb
DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

ifeq ($(OS),Windows_NT)
	CXXFLAGS=-Wall $(OPTIMIZE) $(DEBUG) -Ic:/mingw/include/sdl -mwindows
	LNFLAGS=-lmingw32 -lSDLmain -mwindows
	LIBS=-lmingw32 -lSDLmain -lSDL_ttf -lSDL -lfreetype -lz -lSDL_mixer
else
	UNAME_S := $(shell uname -s)
	PREFIX=/usr/local
	PROFILER=#-pg
	LIBS=
	ifeq ($(UNAME_S),Linux)
		CXXFLAGS=-pipe -Wall $(OPTIMIZE) $(DEBUG) `sdl-config --cflags` -DPREFIX=L\"$(PREFIX)\" $(PROFILER)
		LNFLAGS=-pipe -lSDL_ttf -lfreetype `sdl-config --libs` -lz -lSDL_mixer $(PROFILER)
		INSTALL=install
	endif
	ifeq ($(UNAME_S),Darwin)
		CXXFLAGS=-pipe -Wall $(OPTIMIZE) $(DEBUG) -I/Library/Frameworks/SDL.framework/Headers/ -I/Library/Frameworks/SDL_ttf.framework/Headers/ -I/Library/Frameworks/SDL_mixer.framework/Headers/ -DPREFIX=L\"$(PREFIX)\" $(PROFILER)
		LNFLAGS=-pipe -framework Cocoa -framework SDL_ttf -framework SDL -framework SDL_mixer -lSDLmain -lz  $(PROFILER)
	endif
endif

COMPILE=$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

TARGET=einstein

SOURCES=puzgen.cpp main.cpp screen.cpp resources.cpp utils.cpp game.cpp \
	widgets.cpp iconset.cpp puzzle.cpp rules.cpp \
	verthints.cpp random.cpp horhints.cpp menu.cpp font.cpp \
	conf.cpp storage.cpp tablestorage.cpp regstorage.cpp \
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

%.o : %.cpp
%.o: %.cpp $(DEPDIR)/%.d
	$(COMPILE) $<
	$(POSTCOMPILE)

all: $(TARGET)

mkres: $(RES_OBJECTS) $(SHARE_OBJECTS)
	$(CXX) -lz $(RES_OBJECTS) $(SHARE_OBJECTS) -lz -o mkres

einstein: $(OBJECTS) $(SHARE_OBJECTS) einstein.res
	$(CXX) $(OBJECTS) $(SHARE_OBJECTS) $(LIBS) -o einstein $(LNFLAGS)

# shouldn't hurt to delete .exe even when it's not windows
clean:
	$(RM) $(ALL_OBJECTS) *.exe *.res core* *core $(TARGET) *~

einstein.res: mkres
	cd res  && ../mkres --source resources.descr --output ../einstein.res && cd ..

run: einstein
	./einstein

install: $(TARGET)
	$(INSTALL) -s -D $(TARGET) $(PREFIX)/bin/$(TARGET)
	$(INSTALL) -D einstein.res $(PREFIX)/share/einstein/res/einstein.res

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES))))
