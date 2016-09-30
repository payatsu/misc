#!/bin/sh -e

! which doxygen > /dev/null 2>&1 && echo $0: command not found: doxygen >&2 && exit 1
! which dot     > /dev/null 2>&1 && echo $0: command not found: dot     >&2 && exit 1
! which global  > /dev/null 2>&1 && echo $0: command not found: global  >&2 && exit 1

tty=`tty`

while getopts e:i: arg; do
	case ${arg} in
	e)
		exclude="${exclude} ${OPTARG}"
		;;
	i)
		include="${include} ${OPTARG}"
		;;
	esac
done
shift `expr ${OPTIND} - 1`

css=doxygen.css
doxygen -w html /dev/null /dev/null - | sed -e '
s/Roboto/Ricty,&/
s/monospace/Ricty, &/
s/,courier/,Ricty&/
s/Arial/Ricty, &/
s/,Geneva/,Ricty&/
s/Tahoma/Ricty,&/
s/Verdana/Ricty,&/
' > ${css}

doxygen -g - | sed -e '
/^PROJECT_NAME /s/= .\+$/= "{project name}"/
/^OUTPUT_LANGUAGE /s/= English$/= Japanese/
/^EXTRACT_ALL /s/= NO$/= YES/
/^EXTRACT_PRIVATE /s/= NO$/= YES/
/^EXTRACT_PACKAGE /s/= NO$/= YES/
/^EXTRACT_STATIC /s/= NO$/= YES/
/^INPUT /s%=$%= '"$*"'%
/^RECURSIVE /s/= NO$/= YES/
/^EXCLUDE /s/=$/='"${exclude}"'/
/^SOURCE_BROWSER /s/= NO$/= YES/
/^INLINE_SOURCES /s/= NO$/= YES/
/^REFERENCED_BY_RELATION /s/= NO$/= YES/
/^REFERENCES_RELATION /s/= NO$/= YES/
# /^USE_HTAGS /s/= NO$/= YES/
# CLANG_ASSISTED_PARSING
/^HTML_EXTRA_STYLESHEET /s/=$/= '${css}'/
/^HTML_TIMESTAMP /s/= NO$/= YES/
# /^HTML_DYNAMIC_SECTIONS /s/= NO$/= YES/
/^GENERATE_TREEVIEW /s/= NO$/= YES/
/^GENERATE_LATEX /s/= YES$/= NO/
/^HIDE_UNDOC_RELATIONS /s/= YES$/= NO/
/^HAVE_DOT /s/= NO$/= YES/
/^DOT_FONTNAME /s/= [[:alpha:]]\+$/= Ricty/
/^UML_LOOK /s/= NO$/= YES/
/^TEMPLATE_RELATIONS /s/= NO$/= YES/
/^CALL_GRAPH /s/= NO$/= YES/
/^CALLER_GRAPH /s/= NO$/= YES/
/^INTERACTIVE_SVG /s/= NO$/= YES/
/^DOT_MULTI_TARGETS /s/= NO$/= YES/
' | tee ${tty} | doxygen -
