#!/bin/bash
rm valid.log
rm taskmaster.log

./taskmaster --log-file valid.log --config-file ./test/valid_tests.yaml 2>/dev/null
sleep 4

# expected results
string_array=(
    "SUCCESS: cat"
    "SUCCESS: ls"
    "SUCCESS: sleep"
    "SUCCESS: ls-redirect"
    "ERROR: ls-invalid-chdir"
)

# log
file="valid.log"

# Iterate through array of strings
for i in "${string_array[@]}"
do
    # Check if current string is present in file
    if grep -q "$i" $file; then
        substring=$(echo $i | cut -d ':' -f2)
        echo -e "\033[32m PASS: $substring \033[0m"
    else
        echo -e "\033[31m FAIL: $i in $file \033[0m"
    fi
done

