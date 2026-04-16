#!/bin/sh
#
# Regress dim-5 selection-order pruning so same-source canonicalization only
# collapses genuine descriptor automorphism orbits.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND='F4=$(mktemp) && F4B=$(mktemp) && F5=$(mktemp) && O1=$(mktemp) && O2=$(mktemp) && printf "4 1 1 1 1\n5 1 1 1 2\n" >"${F4}" && cp "${F4}" "${F4B}" && printf "5 1 1 1 1 1\n" >"${F5}" && ./cws-'${DIM}'d.x -c5 -n3 "${F4}" "${F4}" "${F5}" -s12 | sort -u >"${O1}" && ./cws-'${DIM}'d.x -c5 -n3 "${F4}" "${F4B}" "${F5}" -s12 | sort -u >"${O2}" && printf "same=%s\n" "$(wc -l <"${O1}")" && printf "different=%s\n" "$(wc -l <"${O2}")" && diff -u "${O1}" "${O2}" >/dev/null && printf "identical=yes\n" && rm -f "${F4}" "${F4B}" "${F5}" "${O1}" "${O2}"'
DESCRIPTION="cws-${DIM}d.x -c5 structure 12 same-file dim-5 orbit canonicalization"
EXPECTED=$(cat<<-EOF
same=6
different=6
identical=yes
EOF
)
run_test "${SKIP}" "${SKIPREASON}"