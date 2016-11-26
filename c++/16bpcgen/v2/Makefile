.PHONY: all gtags tags test build_test clean

config      := Release
link        := static
enable_tiff := yes
enable_png  := yes
enable_jpeg := yes

#####################
# Directories/Files
#####################
incdir := include

srcdir := src
mains  := $(addprefix $(srcdir)/, 16bpcgen.cpp image_formats.cpp test_patterns.cpp image_processes.cpp colorspace.cpp)
srcs   := $(addprefix $(srcdir)/, Image.cpp Pixel.cpp PatternGenerators.cpp ImageProcesses.cpp PixelConverters.cpp) $(mains)
assdir := assets
assets := $(addprefix $(srcdir)/$(assdir)/, color_matching_functions.tar.gz)
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
	cxxopt := -O0 -g3 -pg -fprofile-arcs -ftest-coverage -fsanitize=address
else ifeq ($(config), Release)
	cxxopt := -O3
endif

ifeq ($(link), static)
	CXXFLAGS := --static
	LDLIBS   := -l$(patsubst $(libdir)/lib%.a,%,  $(libs))
	cxxopt   := $(filter-out -fsanitize=address, $(cxxopt))
else ifeq ($(link), shared)
	CXXFLAGS := -fPIC
	LDLIBS   := -l$(patsubst $(libdir)/lib%.so,%, $(libs))
endif

cxxver   := -std=c++03
CPPFLAGS := -I$(incdir) -DPROGRAM_NAME=\"$(basename $(notdir $(firstword $(mains))))\"
CXXFLAGS += $(cxxver) -Werror -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wstrict-aliasing \
			-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow $(cxxopt)
LDFLAGS  += -L$(libdir)
ifeq ($(enable_tiff), yes)
	CPPFLAGS += -DENABLE_TIFF
	LDLIBS   += -ltiff -llzma
endif
ifeq ($(enable_png), yes)
	CPPFLAGS += -DENABLE_PNG
	LDLIBS   += -lpng
endif
ifeq ($(enable_jpeg), yes)
	CPPFLAGS += -DENABLE_JPEG
	LDLIBS   += -ljpeg
endif
ifeq ($(findstring yes, $(enable_tiff) $(enable_png)), yes)
	LDLIBS   += -lz
endif

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

$(srcdir)/%.txt: $(srcdir)/%.tar.gz
	tar xzvf $<

-include $(deps)
