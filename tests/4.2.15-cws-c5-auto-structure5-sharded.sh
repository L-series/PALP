#!/bin/sh
#
# Regress dim-5 builtin sharding so the worker outputs are disjoint and their
# union matches the unsharded canonical structure output.
#
. tests/lib/run-test.sh

: ${DIM:=6}

if [ "$DIM" -lt 5 ]; then
	SKIP=true
	SKIPREASON="POLY_Dmax too small for -c5"
fi

COMMAND='FULL=$(mktemp) && S1=$(mktemp) && S2=$(mktemp) && SFULL=$(mktemp) && SS1=$(mktemp) && SS2=$(mktemp) && COMBINED=$(mktemp) && ./cws-'${DIM}'d.x -c5 -s5 >"${FULL}" && ./cws-'${DIM}'d.x -c5 -s5 -j2 -k1 >"${S1}" && ./cws-'${DIM}'d.x -c5 -s5 -j2 -k2 >"${S2}" && sort "${FULL}" >"${SFULL}" && sort "${S1}" >"${SS1}" && sort "${S2}" >"${SS2}" && cat "${SS1}" "${SS2}" | sort | uniq >"${COMBINED}" && printf "full=%s\n" "$(wc -l <"${FULL}")" && printf "shard1=%s\n" "$(wc -l <"${S1}")" && printf "shard2=%s\n" "$(wc -l <"${S2}")" && printf "overlap=%s\n" "$(comm -12 "${SS1}" "${SS2}" | wc -l)" && diff -u "${SFULL}" "${COMBINED}" >/dev/null && printf "identical=yes\n" && rm -f "${FULL}" "${S1}" "${S2}" "${SFULL}" "${SS1}" "${SS2}" "${COMBINED}"'
DESCRIPTION="cws-${DIM}d.x -c5 builtin structure 5 shard partition"
EXPECTED=$(cat<<-EOF
full=285
shard1=190
shard2=95
overlap=0
identical=yes
EOF
)
run_test "${SKIP}" "${SKIPREASON}"