.PHONY: all apps clean binclean imgclean ramp betanuri whitenoise builtins

includedir := include/

srcdir := src/
srcfe  := ${addprefix ${srcdir}, 16bpcgen_fe.cpp}
srcbe  := ${addprefix ${srcdir}, 16bpcgen_be.cpp FrameBuffer.cpp PatternGenerator.cpp}

bindir := ${CURDIR}/bin/
binfe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcfe}}}
binbe  := ${patsubst ${srcdir}%.cpp,${bindir}%, ${firstword ${srcbe}}}
apps   := ${binfe} ${binbe}

CPPFLAGS := -I${includedir} -DENABLE_PNG -DENABLE_TIFF
LDLIBS   := -lpng -ltiff

width  := 1920
height := 1080

imgdir := img/

all: apps ramp betanuri whitenoise builtins

apps: ${apps}

clean: binclean imgclean

binclean:
	${RM} -r ${dir ${apps}}

imgclean:
	${RM} -r ${imgdir}

ramp: ${apps}
	mkdir -p ${imgdir}
	cd ${imgdir}; \
	${binfe} width=${width} height=${height} initial=000000000000 increment=002200220022 tread=1 | ${binbe} width=${width} height=${height} input=- output=ramp-by-fe

# 	cd ${imgdir}; \
# 	for i in `seq ${height}`; do \
# 		for j in `seq ${width}`; do \
# 			printf \\u`printf %04x $$i`; \
# 		done; \
# 	done | ${binbe} width=${width} height=${height} input=- output=ramp-by-shell

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
