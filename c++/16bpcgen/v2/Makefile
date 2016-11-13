.PHONY: all gtags tags test build_test clean

config := Release
link   := static

#####################
# Directories/Files
#####################
incdir := include

srcdir := src
mains  := ${addprefix ${srcdir}/, 16bpcgen.cpp image_formats.cpp test_patterns.cpp image_processes.cpp colorspace.cpp}
srcs   := ${addprefix ${srcdir}/, Image.cpp Pixel.cpp PatternGenerators.cpp ImageProcesses.cpp PixelConverters.cpp} ${mains}

depdir := .dep
deps   := ${patsubst ${srcdir}/%.cpp, ${depdir}/%.d, ${srcs}}

objdir := .obj
objs   := ${patsubst ${srcdir}/%.cpp, ${objdir}/%.o, ${srcs}}

libdir := lib
ifeq (${link}, static)
	libs := ${patsubst ${srcdir}/%.cpp, ${libdir}/lib%.a,  ${firstword ${mains}}}
else
ifeq (${link}, shared)
	libs := ${patsubst ${srcdir}/%.cpp, ${libdir}/lib%.so, ${firstword ${mains}}}
endif
endif

bindir := bin
bins   := ${patsubst ${srcdir}/%.cpp,${bindir}/%, ${mains}}

imgdir := img

####################
# Flags
####################
ifeq (${config}, Debug)
	CXXOPT := -O0 -g3 -pg -fprofile-arcs -ftest-coverage -fsanitize=address
else
ifeq (${config}, Release)
	CXXOPT := -O3
endif
endif

ifeq (${link}, static)
	LDLIBS   := -l${patsubst ${libdir}/lib%.a,%,  ${libs}}
else
ifeq (${link}, shared)
	LDLIBS   := -l${patsubst ${libdir}/lib%.so,%, ${libs}}
	CXXFLAGS := -fPIC
endif
endif

CXXVER   := -std=c++03
CPPFLAGS += -I${incdir} -DENABLE_PNG -DENABLE_TIFF -DENABLE_JPEG
CXXFLAGS += ${CXXVER} -Werror -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wstrict-aliasing \
			-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow \
			${CXXOPT}
LDFLAGS  += -L${libdir}
LDLIBS   += -ltiff -lpng -ljpeg # -lz -llzma

####################
# Rules
####################
all: ${bins}

gtags: GTAGS GRTAGS GPATH

GTAGS GRTAGS GPATH:
	gtags -c

tags:
	ctags ${incdir}/*.hpp ${srcdir}/*.cpp

test: build_test
	for b in ${bins}; do \
		echo -n "running $${b}... "; \
		LD_LIBRARY_PATH=${libdir} $${b}; \
		echo done.; \
		gcov -bdflmr -o ${objdir} ${srcdir}/`basename $${b}`.cpp; \
	done

build_test: clean
	${MAKE} config=Debug all

clean:
	${RM} GTAGS GRTAGS GPATH
	${RM} tags
	${RM} -r ${depdir} ${objdir} ${libdir} ${bindir} ./gmon.out ./*.gcov
	${RM} -r ${imgdir}

${bindir}/%: ${objdir}/%.o ${libs}
	@mkdir -p ${dir $@}
	${LINK.cc} -o $@ $< ${LDLIBS}

${libs}: ${filter-out ${patsubst ${srcdir}/%.cpp, ${objdir}/%.o, ${mains}}, ${objs}}
	@mkdir -p ${dir $@}
ifeq (${link}, static)
	${AR} ruv $@ $^
else
ifeq (${link}, shared)
	${LINK.cc} --shared -o $@ $^
endif
endif

${objdir}/%.o: ${srcdir}/%.cpp
	@mkdir -p ${dir $@}
	${COMPILE.cc} -o $@ `readlink -m $<`

${depdir}/%.d: ${srcdir}/%.cpp
	@mkdir -p ${dir $@}
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -MM -MT '$@ ${patsubst ${depdir}/%.d, ${objdir}/%.o, $@}' -MF $@ $<

-include ${deps}
