#!/bin/bash
rm errors.log
rm taskmaster.log

./taskmaster --log-file errors.log --config-file ./test/errors_tests.yaml 2>/dev/null
sleep 4

# expected results
errors_test_results=(
    "ERROR: no-file-or-dir"
    "ERROR: restart-invalid-return"
    "ERROR: restart-time-elapsed"
    "SUCCESS: restart-time-good"
)

# log
file="errors.log"

# Iterate through array of strings
for i in "${errors_test_results[@]}"
do
    # Check if current string is present in file
    if grep -q "$i" $file; then
        substring=$(echo $i | cut -d ':' -f2)
        echo -e "\033[32m PASS: $substring \033[0m"
    else
        echo -e "\033[31m FAIL: $i in $file \033[0m"
    fi
done
