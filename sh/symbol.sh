#!/bin/sh -e

usage()
{
	cat <<-EOF
	[Options]
	    -h
	        show this help.
	    -s symbol
	        specify symbol.

	[Examples]
	    * To show data that symbol "foo" points in bar.elf:
	        \$ `basename ${0}` -s foo bar.elf
	    * To overwrite data that symbol "foo" points in bar.elf with baz.bin:
	        \$ `basename ${0}` -s foo baz.bin bar.elf

	EOF
}

while getopts hs: arg; do
	case ${arg} in
	h)  usage; exit;;
	s)  symbol=${OPTARG};;
	\?) usage >&2; exit 1;;
	esac
done
shift `expr ${OPTIND} - 1`

# $1: ELF file
# $2: symbol name
scan()
{
	symbol_table=`LANG=C readelf -s -W ${1} | grep -e '^   Num:\|\<'${2}'\>$' | grep -e '\<'${2}'\>$' -B 1`
	echo "${symbol_table}" | grep -qe ${2} || { echo Error. Symbol \"${2}\" is not found in \"${1}\". >&2; return 1;}
	symbol_size=`echo "${symbol_table}" | awk 'NR == 2{print $3}'`
	section=`echo "${symbol_table}" | awk 'NR == 2{print $7}'`
	section_header=`LANG=C readelf -S -W ${1} | grep -e '^  \[Nr\]\|^  \[ *'${section}'\]'`
	cat <<-EOF
		Symbol "${2}"'s symbol table entry is as follows:
		${symbol_table}

		Section that includes symbol "${2}" is as follows:
		${section_header}

	EOF
	echo "${section_header}" | grep -qe '\.\<bss\>' && { echo Error. Symbol \"${2}\" is located in section \".bss\", whose entity does not exist in \"${1}\". >&2; return 1;}
	offset=`cat <<EOF | awk '
		NR == 2{sym_mem_offset = strtonum("0x"$2)}
		NR == 4{sec_mem_offset = strtonum("0x"$4); sec_elf_offset = strtonum("0x"$5)}
		END{printf "Symbol \"'${2}'\" is located at %#x = %#x + (%#x - %#x), and occupies '${symbol_size}' bytes, in \"'${1}'\".\n", sec_elf_offset + (sym_mem_offset - sec_mem_offset), sec_elf_offset, sym_mem_offset, sec_mem_offset}'
		${symbol_table}
		${section_header}
	EOF
	`
	symbol_location=`echo "${offset}" | sed -e 's/^.\+located at \(\<0x[[:xdigit:]]\+\>\).\+$/\1/'`
	cat <<-EOF
		${offset}

	EOF
}

# $1: ELF file
dump()
{
	od -Ax -j ${symbol_location} -N ${symbol_size} -tx1z -v ${1} | grep -v '^[[:xdigit:]]\+$'
	echo
}

# $1: output ELF file
# $2: symbol name
# $3: input binary file
overwrite()
{
	echo The hex dump of symbol \"${2}\" in \"${1}\" is as follows:
	dump ${1}
	
	[ `wc -c < ${3}` -le ${symbol_size} ] || {return}
	LANG=C dd if=${3} of=${1} bs=1 count=${symbol_size} seek=`printf '%d' ${symbol_location}` conv=notrunc status=none
	
	echo And it was overwritten just now as follows:
	dump ${1}
}

case $# in
	0);;
	1)
		scan ${1} ${symbol} || exit
		dump ${1};;
	2)
		scan ${2} ${symbol} || exit
		overwrite ${2} ${symbol} ${1};;
	*);;
esac

