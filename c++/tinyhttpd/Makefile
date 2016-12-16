include ../Makefile.config

srcdir := src
mains  := $(addprefix $(srcdir)/, tinyhttpd.cpp)
srcs   := $(addprefix $(srcdir)/, Httpd.cpp Socket.cpp HttpdExt.cpp) $(mains)
assdir := assets
assets := $(addprefix $(srcdir)/$(assdir)/, )

include ../Makefile.files

extdir := ../16bpcgen

include ../Makefile.flags

include ../Makefile.rules

-include $(deps)
