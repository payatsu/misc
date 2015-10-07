#!/usr/bin/make -f

INCLUDE := include/
SRCDIR := src/
SRCS := number-game-questioner.cpp number-game-solver.cpp number-game-connector.cpp
SRCS := ${addprefix ${SRCDIR}, ${SRCS}}
OBJDIR := obj/
OBJS := ${addprefix ${OBJDIR}, ${patsubst %.cpp, %.o, ${SRCS}}}
DEPDIR := dep/
DEPS := ${addprefix ${DEPDIR}, ${patsubst %.cpp, %.d, ${SRCS}}}
BINDIR := bin/
BINS := questioner solver connector
BINS := ${addprefix ${BINDIR}, ${BINS}}

CPPFLAGS := -I ${INCLUDE}
LDFLAGS := -lreadline
CXX11FLAG := -std=c++11
OPTIMIZEFLAGS := -O3
WARNFLAGS := -Wall -Wextra -Wcast-qual -Wcast-align -Wfloat-equal -Woverloaded-virtual -Wshadow -Wpointer-arith -Wwrite-strings -Wformat=2 -Weffc++
CXXFLAGS := ${CXX11FLAG} ${OPTIMIZEFLAGS} ${WARNFLAGS}

.PHONY : all clean
all : ${BINS}
${BINDIR}% : ${OBJDIR}${SRCDIR}number-game-%.o
	@mkdir -p ${dir $@}
	@${LINK.cpp} $^ -o $@
${OBJDIR}%.o : %.cpp
	@mkdir -p ${dir $@}
	@${COMPILE.cpp} $< -o $@
${DEPDIR}%.d : %.cpp
	@mkdir -p ${dir $@}
	@${CXX} ${CXX11FLAG} ${CPPFLAGS} -MM -MT '$@ ${patsubst %.d, %.o, ${subst ${DEPDIR}, ${OBJDIR}, $@}}' -MF $@ $<
clean :
	@${RM} -r ${BINDIR} ${OBJDIR} ${DEPDIR}
-include ${DEPS}
