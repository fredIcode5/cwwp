#!/bin/bash

# ===============================
# Filtrage + ordre aval
# ===============================

if [ $# -ne 1 ]; then
    echo "Usage: $0 \"ID_USINE\""
    exit 1
fi

ID="$1"
INPUT="parcours_aval.txt"
# compile le code C
gcc leak.c -o leak

(
    # 1) Stockage
    grep "^-[;]$ID;Storage" >"$INPUT"

    # 2) Distribution principale
    grep "^$ID;.*;Service" >>"$INPUT"

    # 3) Distribution secondaire
    grep "^$ID;Junction.*;Service" >>"$INPUT"

    # 4) Branchement final (usagers)
    grep "^$ID;Service.*;Cust" >>"$INPUT"
)


./leak parcours_aval.txt
