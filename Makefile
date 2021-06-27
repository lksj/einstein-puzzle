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
	CXXFLAGS=-Wall $(OPTIMIZE) $(DEBUG) `sdl-config --cflags` -mwindows -fpermissive
	LNFLAGS=`sdl-config --libs` -lz -lSDL_mixer -lSDL_ttf -lfreetype
	LIBS=`sdl-config --libs`
	EXE=.exe
else
	EXE=
	UNAME_S := $(shell uname -s)
	PREFIX=/usr/local
	PROFILER=#-pg
	LIBS=
	ifeq ($(UNAME_S),Linux)
		CXXFLAGS=-pipe -Wall $(OPTIMIZE) $(DEBUG) `sdl-config --cflags` -DPREFIX=L\"$(PREFIX)\" $(PROFILER)
		LNFLAGS=-pipe `sdl-config --libs` -lz -lSDL_mixer -lSDL_ttf -lfreetype $(PROFILER)
		INSTALL=install
	endif
	ifeq ($(UNAME_S),Darwin)
		CXXFLAGS=-pipe -Wall $(OPTIMIZE) $(DEBUG) -I/Library/Frameworks/SDL.framework/Headers/ -I/Library/Frameworks/SDL_ttf.framework/Headers/ -I/Library/Frameworks/SDL_mixer.framework/Headers/ -DPREFIX=L\"$(PREFIX)\" $(PROFILER)
		LNFLAGS=-pipe -framework Cocoa -framework SDL_ttf -framework SDL -framework SDL_mixer -lSDLmain -lz  $(PROFILER)
	endif
endif

MKRES=mkres$(EXE)

COMPILE=$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

TARGET=einstein

BUILD_DIR=./build/

SOURCES=puzgen.cpp main.cpp screen.cpp resources.cpp utils.cpp game.cpp \
	widgets.cpp iconset.cpp puzzle.cpp rules.cpp \
	verthints.cpp random.cpp horhints.cpp menu.cpp font.cpp \
	storage.cpp tablestorage.cpp regstorage.cpp \
	topscores.cpp opensave.cpp descr.cpp options.cpp messages.cpp \
	formatter.cpp i18n.cpp tokenizer.cpp sound.cpp args.cpp
HEADERS=$(SOURCES:.cpp=.h)
OBJECTS=$(SOURCES:%.cpp=$(BUILD_DIR)%.o)

SHARE_SOURCES=exceptions.cpp unicode.cpp streams.cpp table.cpp buffer.cpp convert.cpp lexal.cpp
SHARE_HEADERS=$(SHARE_SOURCES:.cpp=.h)
SHARE_OBJECTS=$(SHARE_SOURCES:%.cpp=$(BUILD_DIR)%.o)

RES_SOURCES=mkres.cpp compressor.cpp format.cpp msgwriter.cpp msgformatter.cpp args.cpp
RES_HEADERS=$(RES_SOURCES:.cpp=.h)
RES_OBJECTS=$(RES_SOURCES:%.cpp=$(BUILD_DIR)%.o)

ALL_SOURCES=$(RES_SOURCES) $(SHARE_SOURCES) $(SOURCES)
ALL_HEADERS=$(RES_HEADERS) $(SHARE_HEADERS) $(HEADERS)
ALL_OBJECTS=$(RES_OBJECTS) $(SHARE_OBJECTS) $(OBJECTS)
ALL_FILES=$(ALL_SOURCES) $(ALL_HEADERS)

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

%.o : %.cpp
$(BUILD_DIR)%.o: %.cpp $(DEPDIR)/%.d
	$(COMPILE) $< -o $@
	$(POSTCOMPILE)

$(MKRES): $(BUILD_DIR) $(RES_OBJECTS) $(SHARE_OBJECTS)
	$(CXX) $(RES_OBJECTS) $(SHARE_OBJECTS) -lz -o $(MKRES) $(LNFLAGS)

einstein: $(BUILD_DIR) $(OBJECTS) $(SHARE_OBJECTS) einstein.res
	$(CXX) $(OBJECTS) $(SHARE_OBJECTS) $(LIBS) -o einstein $(LNFLAGS)

# shouldn't hurt to delete .exe even when it's not windows
clean:
	$(RM) -rf $(BUILD_DIR) *.exe *.res core* *core $(TARGET) *~

einstein.res: $(MKRES)
	cd res  && ../$(MKRES) --source resources.descr --output ../einstein.res && cd ..

run: einstein
	./einstein

install: $(TARGET)
	$(INSTALL) -s -D $(TARGET) $(PREFIX)/bin/$(TARGET)
	$(INSTALL) -D einstein.res $(PREFIX)/share/einstein/res/einstein.res

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES))))
