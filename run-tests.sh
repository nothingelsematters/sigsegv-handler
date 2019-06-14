#!/bin/sh

TESTS=( nullptr left-out-of-bound right-out-of-bound invalid-permissions )

if make tests
then
    for TEST in "${TESTS[@]}"
    do
        echo -e "\nTEST ${TEST}\n"
        ./${TEST}
        echo -e "\n---------------------"
    done
fi
make clean
