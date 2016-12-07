.PHONY: all gtags tags test build_test clean

config      := Release
link        := static

#####################
# Directories/Files
#####################
incdir := include

srcdir := src
mains  := $(addprefix $(srcdir)/, tinyhttpd.cpp)
srcs   := $(addprefix $(srcdir)/, Httpd.cpp Socket.cpp) $(mains)
tstdir := $(addprefix $(srcdir)/, test)
tests  := $(wildcard $(tstdir)/*.cpp)

depdir := .dep
deps   := $(patsubst $(srcdir)/%.cpp, $(depdir)/%.d, $(srcs))

objdir := .obj
objs   := $(patsubst $(srcdir)/%.cpp, $(objdir)/%.o, $(srcs))

libdir := lib
ifeq ($(link), static)
	libs := $(patsubst $(srcdir)/%.cpp, $(libdir)/lib%.a,  $(firstword $(mains)))
else ifeq ($(link), shared)
	libs := $(patsubst $(srcdir)/%.cpp, $(libdir)/lib%.so, $(firstword $(mains)))
endif

bindir  := bin
bins    := $(patsubst $(srcdir)/%.cpp,$(bindir)/%, $(mains))
tstbins := $(patsubst $(srcdir)/%.cpp, $(bindir)/%, $(tests))

imgdir := img

####################
# Flags
####################
ifeq ($(config), Debug)
	override cxxopt := -O0 -g3 -pg -fprofile-arcs -ftest-coverage -fsanitize=address
else ifeq ($(config), Release)
	override cxxopt := -O3 -s
endif

ifeq ($(link), static)
	override CXXFLAGS := --static
	override LDLIBS   := -l$(patsubst $(libdir)/lib%.a,%,  $(libs))
	override cxxopt   := $(filter-out -fsanitize=address, $(cxxopt))
else ifeq ($(link), shared)
	override CXXFLAGS := -fPIC
	override LDLIBS   := -l$(patsubst $(libdir)/lib%.so,%, $(libs))
endif

override cxxver   := -std=c++03
override CPPFLAGS := -I$(incdir) -DPROGRAM_NAME=\"$(basename $(notdir $(firstword $(mains))))\"
override CXXFLAGS += $(cxxver) -Werror -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wstrict-aliasing \
						-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow $(cxxopt)
override LDFLAGS  += -L$(libdir)

override CPPFLAGS += -I../16bpcgen/include
override LDLIBS   += -L../16bpcgen/lib -l16bpcgen -ltiff -llzma -lpng -lz -ljpeg

####################
# Rules
####################
all: $(bins)

gtags: GTAGS GRTAGS GPATH

GTAGS GRTAGS GPATH:
	gtags -c

tags:
	ctags $(incdir)/*.hpp $(srcdir)/*.cpp

test: build_test
	set -e; for b in $(bins) $(tstbins); do \
		echo -n "running '$${b}' ... "; \
		LD_LIBRARY_PATH=$(libdir):${LD_LIBRARY_PATH} $${b}; \
		echo done.; \
		gcov -bdflmr -o $(objdir) $(srcdir)/`basename $${b}`.cpp; \
	done

build_test: clean
	$(MAKE) config=Debug all $(tstbins)

clean:
	$(RM) GTAGS GRTAGS GPATH
	$(RM) tags
	$(RM) -r $(depdir) $(objdir) $(libdir) $(bindir) ./gmon.out ./*.gcov
	$(RM) -r $(imgdir)

$(bindir)/%: $(objdir)/%.o $(libs)
	@mkdir -p $(dir $@)
	$(LINK.cc) -o $@ $< $(LDLIBS)

$(libs): $(filter-out $(patsubst $(srcdir)/%.cpp, $(objdir)/%.o, $(mains)), $(objs))
	@mkdir -p $(dir $@)
ifeq ($(link), static)
	$(AR) ruv $@ $^
else ifeq ($(link), shared)
	$(LINK.cc) --shared -o $@ $^
endif

$(objdir)/%.o: $(srcdir)/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILE.cc) -o $@ $(realpath $<)

$(depdir)/%.d: $(srcdir)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MT '$@ $(patsubst $(depdir)/%.d, $(objdir)/%.o, $@)' -MF $@ $<

-include $(deps)
