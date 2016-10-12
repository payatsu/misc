#!/bin/sh -e

help()
{
	cat <<EOF
[NAME]
	`basename $0` - auto-generate documents powered by doxygen/graphviz

[SYNOPSIS]
	`basename $0` [-C src_root] [-f Makefile] [-e exclude_dir] [-s] [dir...]

[DESCRIPTION]
	-c
		enable clang assisted parsing.

	-C dir
		same as 'make -C dir'.

	-D name[=var]
		same as 'cc -D name[=var]'.

	-e name
		exclude name from search directory.
		name is file name or directory name.

	-f makefile
		same as 'make -f makefile'.

	-i
		Specify image format.

	-I dir
		same as 'cc -I dir'.

	-s
		omit detailed description within Doxyfile.
EOF
}

init()
{
	! which doxygen > /dev/null 2>&1 && echo $0: command not found: doxygen >&2 && exit 1
	! which dot     > /dev/null 2>&1 && echo $0: command not found: dot     >&2 && exit 1
	! which global  > /dev/null 2>&1 && echo $0: command not found: global  >&2 && exit 1
	css_file=my_style.css
	make_dir=.
	makefile=Makefile
	clang_assisted_parsing=NO
	img_fmt=svg
	tty=`tty`
	doxyfile=Doxyfile
}

generate_css()
{
	doxygen -w html /dev/null /dev/null - | sed -e '
	s/Roboto/Ricty,&/
	s/monospace/Ricty, &/
	s/,courier/,Ricty&/
	s/Arial/Ricty, &/
	s/,Geneva/,Ricty&/
	s/Tahoma/Ricty,&/
	s/Verdana/Ricty,&/
	' > ${css_file}
}

get_macros_from_Makefile()
{
	[ -f ${make_dir}/${makefile} ] &&
		make -C ${make_dir} -f ${makefile} -n -p | grep -oe '-D *\w\+\(=\w\+\)\?' \
			| sort | uniq | sed -e 's/^-D//;H;${x;y/\n/ /;p};d'
}

generate_doc()
{
	doxygen ${s_opt} -g - | sed -e '
	/^PROJECT_NAME /s/= .\+$/= "{project name}"/
	/^OUTPUT_LANGUAGE /s/= English$/= Japanese/
	/^EXTRACT_ALL /s/= NO$/= YES/
	/^EXTRACT_PRIVATE /s/= NO$/= YES/
	/^EXTRACT_PACKAGE /s/= NO$/= YES/
	/^EXTRACT_STATIC /s/= NO$/= YES/
	/^INPUT /s%=$%= '"$*"'%
	/^RECURSIVE /s/= NO$/= YES/
#	/^EXCLUDE /s%=$%='"${exclude}"'%
	/^EXCLUDE_PATTERNS /s%=$%= *.d '"`echo ${exclude} | sed -e 's%\<.\+\>%*/&/*%g'`"'%
	/^SOURCE_BROWSER /s/= NO$/= YES/
	/^INLINE_SOURCES /s/= NO$/= YES/
	/^REFERENCED_BY_RELATION /s/= NO$/= YES/
	/^REFERENCES_RELATION /s/= NO$/= YES/
#	/^USE_HTAGS /s/= NO$/= YES/
#	/^CLANG_ASSISTED_PARSING /s/= NO$/= '${clang_assisted_parsing}'/
#	/^CLANG_OPTIONS /s%=$%= -stdlib=libc++%
	/^HTML_EXTRA_STYLESHEET /s%=$%= '${css_file}'%
	/^HTML_TIMESTAMP /s/= NO$/= YES/
#	/^HTML_DYNAMIC_SECTIONS /s/= NO$/= YES/
	/^GENERATE_TREEVIEW /s/= NO$/= YES/
	/^GENERATE_LATEX /s/= YES$/= NO/
	/^INCLUDE_PATH /s%=$%='"${include}"'%
	/^PREDEFINED /s/=$/='"${macro_definitions}`get_macros_from_Makefile`"'/
	/^HIDE_UNDOC_RELATIONS /s/= YES$/= NO/
	/^HAVE_DOT /s/= NO$/= YES/
	/^DOT_FONTNAME /s/= [[:alpha:]]\+$/= Ricty/
	/^UML_LOOK /s/= NO$/= YES/
	/^TEMPLATE_RELATIONS /s/= NO$/= YES/
	/^CALL_GRAPH /s/= NO$/= YES/
	/^CALLER_GRAPH /s/= NO$/= YES/
	/^DOT_IMAGE_FORMAT /s/= png$/= '${img_fmt}'/
	/^INTERACTIVE_SVG /s/= NO$/= YES/
	/^DOT_MULTI_TARGETS /s/= NO$/= YES/
	' | tee ${tty} | tee ${doxyfile} | doxygen -
}

init

while getopts cC:D:e:f:hi:I:s arg; do
	case ${arg} in
	c)  clang_assisted_parsing=YES;;
	C)  make_dir=${OPTARG};;
	D)  macro_definitions="${macro_definitions} ${OPTARG}";;
	e)  exclude="${exclude} ${OPTARG}";;
	f)  makefile=${OPTARG};;
	h)  help; exit 0;;
	i)  img_fmt=${OPTARG};;
	I)  include="${include} ${OPTARG}";;
	s)  s_opt=-s;;
	\?) help >&2; exit 1;;
	esac
done
shift `expr ${OPTIND} - 1`

generate_css
generate_doc $*
