.PHONY: all apps gtags clean binclean imgclean ramp betanuri whitenoise builtins

includedir := include/

srcdir := src/
srcfe  := ${addprefix ${srcdir}, 16bpcgen_fe.cpp}
srcbe  := ${addprefix ${srcdir}, 16bpcgen_be.cpp FrameBuffer.cpp PatternGenerator.cpp}
srccnv := ${addprefix ${srcdir}, 16bpcgen_converter.cpp FrameBuffer.cpp read_img.cpp}

bindir := ${CURDIR}/bin/
binfe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcfe}}}
binbe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcbe}}}
bincnv := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srccnv}}}
apps   := ${binfe} ${binbe} ${bincnv}

CPPFLAGS := -I${includedir} -DENABLE_PNG -DENABLE_TIFF
CXXFLAGS := -Wall -Wextra -Weffc++ -Wcast-align -Wcast-qual -Wformat -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings
LDLIBS   := -lpng -ltiff

width  := 1920
height := 1080

imgdir := img/

all: apps ramp betanuri whitenoise builtins

apps: ${apps}

gtags: GTAGS GRTAGS GPATH

GTAGS GRTAGS GPATH:
	gtags -c

clean: binclean imgclean
	${RM} GTAGS GRTAGS GPATH

binclean:
	${RM} -r ${dir ${apps}}

imgclean:
	${RM} -r ${imgdir}

ramp: ${apps}
	mkdir -p ${imgdir}
	cd ${imgdir}; \
	${binfe} width=${width} height=${height} initial=000000000000 increment=002200220022 tread=1 | ${binbe} width=${width} height=${height} input=- output=ramp-by-fe

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

${binfe}: ${srcfe}
	mkdir -p ${dir $@}
	${LINK.cc} $^ -o $@

${binbe}: ${srcbe}
	mkdir -p ${dir $@}
	${LINK.cc} $^ ${LDLIBS} -o $@

${bincnv}: ${srccnv}
	mkdir -p ${dir $@}
	${LINK.cc} $^ ${LDLIBS} -o $@
