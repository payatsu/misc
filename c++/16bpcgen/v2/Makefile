.PHONY: all apps gtags clean binclean imgclean ramp betanuri whitenoise builtins

includedir := include/

srcdir := src/
srcfe  := ${addprefix ${srcdir}, 16bpcgen_fe.cpp}
srcbe  := ${addprefix ${srcdir}, 16bpcgen_be.cpp        Image.cpp Pixel.cpp ImageProcesses.cpp PatternGenerators.cpp}
srccnv := ${addprefix ${srcdir}, 16bpcgen_converter.cpp Image.cpp Pixel.cpp ImageProcesses.cpp PixelConverters.cpp}

depdir := dep/
deps   := ${patsubst ${srcdir}%.cpp, ${depdir}%.d, ${srcfe} ${srcbe} ${srccnv}}

objdir := obj/
objfe  := ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${srcfe}}
objbe  := ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${srcbe}}
objcnv := ${patsubst ${srcdir}%.cpp, ${objdir}%.o, ${srccnv}}

bindir := ${CURDIR}/bin/
binfe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcfe}}}
binbe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcbe}}}
bincnv := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srccnv}}}
apps   := ${binfe} ${binbe} ${bincnv}

imgdir := img/

CXXOPT   := -O3
CPPFLAGS := -I${includedir} -DENABLE_PNG -DENABLE_TIFF
CXXFLAGS := -std=c++03 -pedantic -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual \
			-Wformat=2 -Wwrite-strings -Woverloaded-virtual -Wpointer-arith -Wfloat-equal -Wshadow \
			-Wno-long-long \
			${CXXOPT}
LDLIBS   := -lpng -ltiff

width  := 1920
height := 1080

all: apps ramp betanuri whitenoise builtins

apps: ${apps}

gtags: GTAGS GRTAGS GPATH

GTAGS GRTAGS GPATH:
	gtags -c

clean: binclean imgclean
	${RM} GTAGS GRTAGS GPATH

binclean:
	${RM} -r ${bindir} ${depdir} ${objdir}

imgclean:
	${RM} -r ${imgdir}

ramp: ${binfe} ${binbe}
	mkdir -p ${imgdir}
	cd ${imgdir}; \
	${binfe} width=${width} height=${height} initial=000000000000 increment=002200220022 tread=1 | \
	${binbe} width=${width} height=${height} input=- output=ramp-by-fe

betanuri: ${binbe}
	mkdir -p ${imgdir}
	cd ${imgdir}; \
	$< width=${width} height=${height} input=- output=black < /dev/zero

whitenoise: ${binbe}
	mkdir -p ${imgdir}
	cd ${imgdir}; \
	$< width=${width} height=${height} input=- output=whitenoise < /dev/urandom

builtins: ${binbe}
	mkdir -p ${imgdir}
	cd ${imgdir}; \
	$< width=${width} height=${height} builtins=yes

${binfe}: ${objfe}
	mkdir -p ${dir $@}
	${LINK.cc} $^ -o $@

${binbe}: ${objbe}
	mkdir -p ${dir $@}
	${LINK.cc} $^ ${LDLIBS} -o $@

${bincnv}: ${objcnv}
	mkdir -p ${dir $@}
	${LINK.cc} $^ ${LDLIBS} -o $@

${objdir}%.o: ${srcdir}%.cpp
	mkdir -p ${dir $@}
	${COMPILE.cpp} $< -o $@

${depdir}%.d: ${srcdir}%.cpp
	mkdir -p ${dir $@}
	${CXX} ${CPPFLAGS} -MM -MT '$@ ${patsubst ${depdir}%.d, ${objdir}%.o, $@}' -MF $@ $<

-include ${deps}
