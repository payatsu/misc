#!/bin/sh -e

while getopts e: arg; do
	case ${arg} in
	e)
		exclude="${exclude} ${OPTARG}"
		;;
	esac
done
shift `expr ${OPTIND} - 1`

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
# USE_HTAGS
# CLANG_ASSISTED_PARSING
/^HTML_TIMESTAMP /s/= NO$/= YES/
/^HTML_DYNAMIC_SECTIONS /s/= NO$/= YES/
/^GENERATE_TREEVIEW /s/= NO$/= YES/
/^GENERATE_LATEX /s/= YES$/= NO/
# /^INCLUDE_PATH /s/=$/= /
/^HIDE_UNDOC_RELATIONS /s/= YES$/= NO/
/^HAVE_DOT /s/= NO$/= YES/
/^UML_LOOK /s/= NO$/= YES/
/^TEMPLATE_RELATIONS /s/= NO$/= YES/
/^CALL_GRAPH /s/= NO$/= YES/
/^CALLER_GRAPH /s/= NO$/= YES/
/^INTERACTIVE_SVG /s/= NO$/= YES/
/^DOT_MULTI_TARGETS /s/= NO$/= YES/
' | doxygen -
