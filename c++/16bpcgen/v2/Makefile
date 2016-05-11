.PHONY: all gtags

includedir := include/

srcdir := src/
srcs   := ${addprefix ${srcdir}, 16bpcgen.cpp Image.cpp Pixel.cpp PatternGenerators.cpp ImageProcesses.cpp PixelConverters.cpp}

depdir := dep/
deps   := ${patsubst ${srcdir}%.cpp, ${depdir}%.d, ${srcs}}

objdir := obj/
objs   := ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${srcs}}

bindir     := ${CURDIR}/bin/
executable := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcs}}}

imgdir := img/

CXXOPT   := -O3
CPPFLAGS += -I${includedir} -DENABLE_PNG -DENABLE_TIFF
CXXFLAGS += -std=c++03 -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual \
			-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow \
			${CXXOPT}
LDLIBS   += -lpng -ltiff

all: ${executable}

gtags: GTAGS GRTAGS GPATH

GTAGS GRTAGS GPATH:
	gtags -c

clean:
	${RM} GTAGS GRTAGS GPATH
	${RM} -r ${bindir} ${depdir} ${objdir}
	${RM} -r ${imgdir}

${executable}: ${objs}
	mkdir -p ${dir $@}
	${LINK.cc} $^ ${LDLIBS} -o $@

${objdir}%.o: ${srcdir}%.cpp
	mkdir -p ${dir $@}
	${COMPILE.cpp} $< -o $@

${depdir}%.d: ${srcdir}%.cpp
	mkdir -p ${dir $@}
	${CXX} ${CPPFLAGS} -MM -MT '$@ ${patsubst ${depdir}%.d, ${objdir}%.o, $@}' -MF $@ $<

-include ${deps}
