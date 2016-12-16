include ../Makefile.config

srcdir := src
mains  := $(addprefix $(srcdir)/, 16bpcgen.cpp image_formats.cpp test_patterns.cpp image_processes.cpp colorspace.cpp)
srcs   := $(addprefix $(srcdir)/, Image.cpp Pixel.cpp PatternGenerators.cpp ImageProcesses.cpp PixelConverters.cpp) $(mains)
assdir := assets
assets := $(addprefix $(srcdir)/$(assdir)/, color_matching_functions.tar.gz)

include ../Makefile.files

include ../Makefile.flags

include ../Makefile.rules

$(srcdir)/%.txt: $(srcdir)/%.tar.gz
	tar xzvf $<

-include $(deps)
