.PHONY: all clean ramp beta builtins

width = 1920
height = 1080

all: 16bpcgen_fe 16bpcgen_be

clean:
	${RM} 16bpcgen_fe 16bpcgen_be
	${RM} *.png *.tif

ramp: all
	./16bpcgen_fe width=${width} height=${height} initial=000000000000 increment=002200220022 tread=1 | ./16bpcgen_be width=${width} height=${height} input=- output=ramp

beta: 16bpcgen_be
	./16bpcgen_be width=${width} height=${height} input=- output=black < /dev/zero

builtins: 16bpcgen_be
	./16bpcgen_be width=${width} height=${height} builtins=yes

16bpcgen_fe: 16bpcgen_fe.cpp
	${LINK.cc} $^ -o $@

16bpcgen_be: 16bpcgen_be.cpp
	${LINK.cc} -DENABLE_PNG -DENABLE_TIFF $^ -o $@ -lpng -ltiff
