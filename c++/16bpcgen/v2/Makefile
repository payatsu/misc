.PHONY: all gtags tags test clean

config := Release

#####################
# Directories/Files
#####################
incdir := include

srcdir := src
mains  := ${addprefix ${srcdir}/, 16bpcgen.cpp image_formats.cpp test_patterns.cpp colorspace.cpp}
srcs   := ${addprefix ${srcdir}/, Image.cpp Pixel.cpp PatternGenerators.cpp ImageProcesses.cpp PixelConverters.cpp} ${mains}

depdir := .dep
deps   := ${patsubst ${srcdir}/%.cpp, ${depdir}/%.d, ${srcs}}

objdir := .obj
objs   := ${patsubst ${srcdir}/%.cpp, ${objdir}/%.o, ${srcs}}

libdir := lib
libs   := ${patsubst ${srcdir}/%.cpp, ${libdir}/lib%.a, ${firstword ${mains}}}

bindir := bin
bins   := ${patsubst ${srcdir}/%.cpp,${bindir}/%, ${mains}}

imgdir := img

####################
# Flags
####################
ifeq (${config}, Debug)
CXXOPT   := -O0 -g3 -fprofile-arcs -ftest-coverage -fsanitize=address
else
ifeq (${config}, Release)
CXXOPT   := -O3
endif
endif

CXXVER   := -std=c++03
CPPFLAGS += -I${incdir} -DENABLE_PNG -DENABLE_TIFF -DENABLE_JPEG
CXXFLAGS += ${CXXVER} -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wstrict-aliasing \
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

test: clean
	${MAKE} config=Debug all
	for b in ${bins}; do \
		echo -n "running $${b}... "; \
		$${b}; \
		echo done.; \
	done

clean:
	${RM} GTAGS GRTAGS GPATH
	${RM} tags
	${RM} -r ${depdir} ${objdir} ${libdir} ${bindir} ./*.gcov
	${RM} -r ${imgdir}

${bindir}/%: ${objdir}/%.o ${libs}
	@mkdir -p ${dir $@}
	${LINK.cc} -o $@ $< -l${patsubst ${libdir}/lib%.a,%, ${libs}} ${LDLIBS}

${libs}: ${filter-out ${patsubst ${srcdir}/%.cpp, ${objdir}/%.o, ${mains}}, ${objs}}
	@mkdir -p ${dir $@}
	${AR} ruv $@ $^

${objdir}/%.o: ${srcdir}/%.cpp
	@mkdir -p ${dir $@}
	${COMPILE.cc} -o $@ `readlink -m $<`

${depdir}/%.d: ${srcdir}/%.cpp
	@mkdir -p ${dir $@}
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -MM -MT '$@ ${patsubst ${depdir}/%.d, ${objdir}/%.o, $@}' -MF $@ $<

-include ${deps}
