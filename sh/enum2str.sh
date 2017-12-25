#!/bin/sh

indent='    '

while [ $# -ne 0 ]; do
	case ${1} in
	--header) shift; header=${1}; shift;;
	--source) shift; source=${1}; shift;;
	*) break;;
	esac
done

[ $# -ne 0 ] || exit

: ${header:=`echo ${1} | sed -e 's/.[^.]\+$/_tostr.hpp/'`}
: ${source:=`echo ${1} | sed -e 's/.[^.]\+$/_tostr.cpp/'`}

generate_source()
{
	tmp=`mktemp -p /tmp XXXXXX` || return
	trap 'rm -f ${tmp}' EXIT HUP INT QUIT TERM
	tmpname=`basename ${tmp}`
	sed -e '
		1{ # insert #include directives
			i#include <ostream>
			s%^.*$%#include "'${header}'"\n%
			s%[^"]*include/%%
			p
		}
		/^\(class\|struct\)[[:space:]]\+\([A-Za-z0-9_]\+\).*$/{ # append class name to hold space
			s//\2::/
			H
			x
			s/\n//
			x
		}
		/^namespace[[:space:]]\+\([A-Za-z0-9_]\+\).*$/{ # append namespace name to hold space
			s//\1::/
			H
			x
			s/\n//
			x
		}
		/^enum[[:space:]]\+[A-Za-z0-9_]\+.*$/{ # append pseudo scope name(place holder) to hold space
			H
			x
			s/\nenum[[:space:]]\+\([A-Za-z0-9_]\+\).*$/'${tmpname}'::/
			x
		}
		/^}\(;\|.\+namespace\)/{ # clear hold space in case of class declaration end or namespace end
			x
			s/[^:]\+::$//
			x
		}
		/enum/{ # prepend namespace name/class name to enumeration
			G
			s/\(enum[[:space:]]\+\)\(.*\)\n\(.*\)/\1\3\2/
		}
		/enum/,/}.*;/p # extract enumeration definition
		d' ${1} |
	sed -e '
		/^[[:space:]]*enum[[:space:]]\+\([A-Za-z0-9_:]\+\).*$/{ # extract namespace name/class name scope specification
			h
			x
			s/.\+[[:space:]]\([A-Za-z0-9_:]\+::\).*$/\1/
			x
		}
		/^[[:space:]]*enum[[:space:]]\+\([A-Za-z0-9_:]\+\).*$/{ # start function definition
			s//std::ostream\& operator<<(std::ostream\& os, const \1 item)\n{/
			a\'"${indent}"'switch(item){
		}
		/^[[:space:]]\+\([A-Z0-9_]\+\).*$/{ # list enumeration items, prepend namespace name/class name scope specification
			s//'"${indent}"'case \1: return os << "\1";/
			G
			s/\(case \)\(.\+\)\n\(.\+\)/\1\3\2/
		}
		/^[[:space:]]*}.*;/{ # finish function definition
			s//'"${indent}"'default: return os << "unknown";\n'"${indent}"'}\n}\n/
		}
		/'${tmpname}'::/s/// # remove pseudo scope name(place holder)
		$a// vim: set expandtab shiftwidth=0 tabstop=4 :'
}

generate_header_and_source()
{
	generate_source ${1} | tee ${source} |
	sed -e '
		1{ # insert #include directives
			i#pragma once
			i#include <iosfwd>
			s%^.*$%#include "'${1}'"\n%
			s%[^"]*include/%%
			p
		}
		/^std::ostream&/{ # extract function signatures
			s/$/;/
			p
		}
		${s/^/\n/;p}
		d' > ${header}
}

generate_header_and_source ${1}
