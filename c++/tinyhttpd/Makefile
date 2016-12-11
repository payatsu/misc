.PHONY: all gtags tags lib test build_test clean

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
mains  := $(addprefix $(srcdir)/, tinyhttpd.cpp)
srcs   := $(addprefix $(srcdir)/, Httpd.cpp Socket.cpp HttpdExt.cpp) $(mains)
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
	libs := $(patsubst %, $(libdir)/lib%.a,  $(notdir $(CURDIR)))
else ifeq ($(link), shared)
	libs := $(patsubst %, $(libdir)/lib%.so, $(notdir $(CURDIR)))
endif

bindir  := bin
bins    := $(patsubst $(srcdir)/%.cpp,$(bindir)/%, $(mains))
tstbins := $(patsubst $(srcdir)/%.cpp, $(bindir)/%, $(tests))

imgdir := img

extdir := ../16bpcgen

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
	override LDLIBS   := $(addprefix -l, $(patsubst $(libdir)/lib%.a,%,  $(libs)) $(notdir $(extdir)))
	override cxxopt   := $(filter-out -fsanitize=address, $(cxxopt))
else ifeq ($(link), shared)
	override CXXFLAGS := -fPIC
	override LDLIBS   := $(addprefix -l, $(patsubst $(libdir)/lib%.so,%, $(libs)) $(notdir $(extdir)))
endif

override cxxver   := -std=c++03
override CPPFLAGS := $(addprefix -I, $(incdir) $(extdir)/include) -DPROGRAM_NAME=\"$(notdir $(CURDIR))\"
override CXXFLAGS += $(cxxver) -Werror -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wstrict-aliasing \
						-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow $(cxxopt)
override LDFLAGS  += $(addprefix -L, $(libdir) $(extdir)/lib)
ifeq ($(enable_tiff), yes)
	override CPPFLAGS += -DENABLE_TIFF
	override LDLIBS   += -ltiff -llzma
endif
ifeq ($(enable_png), yes)
	override CPPFLAGS += -DENABLE_PNG
	override LDLIBS   += -lpng
endif
ifeq ($(enable_jpeg), yes)
	override CPPFLAGS += -DENABLE_JPEG
	override LDLIBS   += -ljpeg
endif
ifeq ($(findstring yes, $(enable_tiff) $(enable_png)), yes)
	override LDLIBS   += -lz
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

lib: $(libs)

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
	$(MAKE) -C $(extdir) lib
	$(LINK.cc) -o $@ $< $(LDLIBS)

$(libs): $(filter-out $(patsubst $(srcdir)/%.cpp, $(objdir)/%.o, $(mains)), $(objs))
	@mkdir -p $(dir $@)
ifeq ($(link), static)
	$(AR) rsuv $@ $^
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
