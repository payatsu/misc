.PHONY: all gtags clean

incdir := include/

srcdir := src/
srcs   := ${addprefix ${srcdir}, 16bpcgen.cpp Image.cpp Pixel.cpp PatternGenerators.cpp ImageProcesses.cpp PixelConverters.cpp}
main   := ${firstword ${srcs}}

depdir := dep/
deps   := ${patsubst ${srcdir}%.cpp, ${depdir}%.d, ${srcs}}

objdir := obj/
objs   := ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${srcs}}

libdir := lib/
libs   := ${patsubst ${srcdir}%.cpp, ${libdir}lib%.a, ${main}}

bindir := bin/
bins   := ${patsubst ${srcdir}%.cpp,${bindir}%, ${main}}

imgdir := img/

CXXVER   := -std=c++03
CXXOPT   := -O3
CPPFLAGS += -I${incdir} -DENABLE_PNG -DENABLE_TIFF -DENABLE_JPEG
CXXFLAGS += ${CXXVER} -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual \
			-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow \
			${CXXOPT}
LDFLAGS  += -L${libdir}
LDLIBS   += -ltiff -lpng -ljpeg # -lz -llzma

all: ${bins}

gtags: GTAGS GRTAGS GPATH

GTAGS GRTAGS GPATH:
	gtags -c

tags:
	ctags ${incdir}*.hpp ${srcdir}*.cpp

clean:
	${RM} GTAGS GRTAGS GPATH
	${RM} tags
	${RM} -r ${depdir} ${objdir} ${libdir} ${bindir}
	${RM} -r ${imgdir}

${bins}: ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${main}} ${libs}
	@mkdir -p ${dir $@}
	${LINK.cc} -o $@ $< -l${patsubst ${libdir}lib%.a,%, ${libs}} -Llib ${LDLIBS}

${libs}: ${filter-out ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${main}}, ${objs}}
	@mkdir -p ${dir $@}
	${AR} ruv $@ $^

${objdir}%.o: ${srcdir}%.cpp
	@mkdir -p ${dir $@}
	${COMPILE.cc} -o $@ `readlink -m $<`

${depdir}%.d: ${srcdir}%.cpp
	@mkdir -p ${dir $@}
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -MM -MT '$@ ${patsubst ${depdir}%.d, ${objdir}%.o, $@}' -MF $@ $<

-include ${deps}
