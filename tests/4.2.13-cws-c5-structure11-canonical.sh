#!/bin/sh
#
# Regress canonicalization of dim-5 shared-vertex symmetries on a minimal
# explicit structure-11 example.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND='A=$(mktemp) && B=$(mktemp) && OUT=$(mktemp) && printf "4 1 1 1 1\n" >"${A}" && printf "9 1 2 3 3\n" >"${B}" && ./cws-'${DIM}'d.x -c5 -n3 "${A}" "${B}" "${B}" -s11 >"${OUT}" && printf "total=%s\n" "$(wc -l <"${OUT}")" && printf "unique=%s\n" "$(sort "${OUT}" | uniq | wc -l)" && printf "first=%s\n" "$(head -n 1 "${OUT}")" && printf "last=%s\n" "$(tail -n 1 "${OUT}")" && rm -f "${OUT}" "${A}" "${B}"'
DESCRIPTION="cws-${DIM}d.x -c5 structure 11 canonical shared-prefix symmetry"
EXPECTED=$(cat<<-EOF
total=16
unique=16
first=4 1 1 1 1 0 0 0 0  9 1 2 0 0 3 3 0 0  9 1 2 0 0 0 0 3 3  M:137 28 N:15 10
last=4 1 1 1 1 0 0 0 0  9 3 3 0 0 1 2 0 0  9 3 3 0 0 0 0 1 2  M:297 24 N:15 10
EOF
)
run_test "${SKIP}" "${SKIPREASON}"