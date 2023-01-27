#!/bin/bash
rm valid.log
rm taskmaster.log

./taskmaster --log-file valid.log --config-file ./test/valid_tests.yaml 2>/dev/null
sleep 4

# expected results
valid_test_results=(
    "SUCCESS: cat"
    "SUCCESS: ls"
    "SUCCESS: sleep"
    "SUCCESS: ls-redirect"
    "ERROR: ls-invalid-chdir"
    "SUCCESS: ls-umask-norights"
)

# log
file="valid.log"

# Iterate through array of strings
for i in "${valid_test_results[@]}"
do
    # Check if current string is present in file
    if grep -q "$i" $file; then
        substring=$(echo $i | cut -d ':' -f2)
        echo -e "\033[32m PASS: $substring \033[0m"
    else
        echo -e "\033[31m FAIL: $i in $file \033[0m"
    fi
done

# make sure the umask was applied to the target and to the target only
# bsd stat
#umask_permission_string=$(stat test/ls_invalid_umask_file | cut -d ' ' -f 3)
#original_permission_string=$(stat test/cat_file | cut -d ' ' -f 3)
# gnu stat
umask_permission_string=$(stat -c '%A' test/ls_invalid_umask_file)
original_permission_string=$(stat -c '%A' test/cat_file)

if echo $umask_permission_string | grep -q '\-rw\-\-\-\-\-\-\-'; then
    echo -e "\033[32m PASS:  umask permissions: umask(044)"
else
    echo -e "\033[31m FAIL:  umask permissions: umask(044)\033[0m"
fi

if echo $original_permission_string | grep -q '\-rw\-r\-\-r\-\-'; then
    echo -e "\033[32m PASS:  umask permissions: original(022)"
else
    echo -e "\033[31m FAIL:  umask permissions: original(022)\033[0m"
fi
