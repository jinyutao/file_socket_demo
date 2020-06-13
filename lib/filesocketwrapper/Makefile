CC ?= $(TARGET_PREFIX)gcc
CXX ?= $(TARGET_PREFIX)g++
AR ?= $(TARGET_PREFIX)ar

SRCDIR = src/
OBJDIR = obj/
DEPDIR = obj/dep/
BINDIR = ../lib/

CXXSRCS = \
	client.cpp \
	server.cpp \
	send.cpp \
	recv.cpp \
	array_opt.cpp

override CXXFLAGS += \
	$(INCLUDE) \
	-I. \
	-I../include \
	-I./include \
	-D_REENTRANT -std=c++1y -Wall -fPIC -ggdb3
override LDFLAGS += -lpthread -lstdc++ -lrt -ldl -ggdb3

LIBNAME=libfilesocketwrapper
SONAMEVERSION=0

CXXOBJS = $(addprefix $(OBJDIR),$(CXXSRCS:.cpp=.o))
CXXDEPS = $(addprefix $(DEPDIR),$(CXXSRCS:.cpp=.d))

all: $(BINDIR)$(LIBNAME).so

install:
#	to do ...

$(BINDIR)$(LIBNAME).so: $(BINDIR)$(LIBNAME).so.$(SONAMEVERSION)
	ln -f -s $(subst $(BINDIR),,$<) $@

$(BINDIR)$(LIBNAME).so.$(SONAMEVERSION): $(CXXOBJS)
	@if [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi;
	$(CXX) -shared -nostartfiles -Wl,-soname,$(subst $(BINDIR),,$@) -o $@ $^ $(LDFLAGS)

$(CXXOBJS):$(OBJDIR)%.o : $(SRCDIR)%.cpp
	@if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CXXDEPS):$(DEPDIR)%.d : $(SRCDIR)%.cpp
	@if [ ! -d $(DEPDIR) ]; then mkdir -p $(DEPDIR); fi;
	@set -e; rm -f $@; \
	$(CC) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(CXXDEPS)

clean:
	@rm -rf $(OBJDIR) $(BINDIR)$(LIBNAME).*

.PHONY: clean
