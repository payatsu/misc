.PHONY: all clean ramp beta builtins

all: 16bpcgen_fe 16bpcgen_be

ramp: all
	./16bpcgen_fe initial=000000000000 increment=002200220022 tread=1 | ./16bpcgen_be

beta: 16bpcgen_be
	cat /dev/zero | ./16bpcge_be

builtins: 16bpcgen_be
	./16bpcgen_be builtins=yes

16bpcgen_fe: 16bpcgen_fe.cpp
	${CXX}

16bpcgen_be: 16bpcgen_be.cpp
	${CXX} -DENABLE_PNG -DENABLE_TIFF $^ -o $@ -lpng -ltiff
