.PHONY: all clean binclean imgclean ramp betanuri builtins

includedir := include/

srcdir := src/
srcfe  := ${addprefix ${srcdir}, 16bpcgen_fe.cpp}
srcbe  := ${addprefix ${srcdir}, 16bpcgen_be.cpp FrameBuffer.cpp PatternGenerator.cpp}

bindir := bin/
binfe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcfe}}}
binbe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcbe}}}
apps   := ${binfe} ${binbe}

CPPFLAGS := -I${includedir} -DENABLE_PNG -DENABLE_TIFF
LDLIBS     := -lpng -ltiff

width  := 1920
height := 1080

all: ${apps}

clean: binclean imgclean

binclean:
	${RM} -r ${dir ${apps}}

imgclean:
	${RM} *.png *.tif

ramp: ${apps}
	${binfe} width=${width} height=${height} initial=000000000000 increment=002200220022 tread=1 | ${binbe} width=${width} height=${height} input=- output=ramp

betanuri: ${binbe}
	$< width=${width} height=${height} input=- output=black < /dev/zero

builtins: ${binbe}
	$< width=${width} height=${height} builtins=yes

${binfe}: ${srcfe}
	mkdir -p ${dir $@}
	${LINK.cc} $^ -o $@

${binbe}: ${srcbe}
	mkdir -p ${dir $@}
	${LINK.cc} $^ ${LDLIBS} -o $@
