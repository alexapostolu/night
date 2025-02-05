#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <night executable>"
  exit 1
fi

# Night executable
night="$1"

# Night source code files for testing
night_files=($(find programs/cs50 programs/math -type f -name "*.night" | sort))

# Standard input and output files corresponding to the Night source code files
inputs=($(find stdio/cs50 stdio/math -type f -name "*_input.txt" | sort))
expected_outputs=($(find stdio/cs50 stdio/math -type f -name "*_expected.txt" | sort))

tests_failed=0

echo -e "\e[36mRunning ${#night_files[@]} tests...\e[0m\n"

for i in "${!night_files[@]}"; do
    # Night code
    night_file="${night_files[$i]}"

    # Standard input file
    input="${inputs[$i]}"
    
    # Expected and actual standard output files
    expected_output="${expected_outputs[$i]}"
    actual_output="actual_output.txt"

    # Run the program and capture the output
    "$night" "$night_file" < "$input" > "$actual_output"
    exit_code=$?

    echo "$night_file"

    # Check return value for sanity check
    if [ $exit_code -ne 0 ]; then
        echo -e "\e[31mFailed. \e[33mCompile error with exit code $exit_code.\e[0m\n"
        cat "$actual_output"
        ((tests_failed++))
        continue
    fi

    # Remove carriage returns (no CR)
    tr -d '\r' < "$actual_output" > "${actual_output}.nocr"
    tr -d '\r' < "$expected_output" > "${expected_output}.nocr"
    
    # Compare actual output to expected output
    if diff "${actual_output}.nocr" "${expected_output}.nocr" > /dev/null; then
        echo -e "\e[32mPassed.\e[0m"
    else
        echo -e "\e[31mFailed. \e[33mOutput does not match the expected output."
        echo -e "Result/Expected:\e[0m"
        diff "${actual_output}.nocr" "${expected_output}.nocr"
        ((tests_failed++))
    fi

    # Remove temporary files
    rm "${actual_output}.nocr"
    rm "${expected_output}.nocr"

    echo
done

if [ $tests_failed -eq 0 ]; then
    echo -e "\e[1;32mAll tests passed.\e[0m"
elif [ $tests_failed -eq 1 ]; then
    echo -e "\e[1;31m${tests_failed} test failed.\e[0m"
else
    echo -e "\e[1;31m${tests_failed} tests failed.\e[0m"
fi

# Remove temporary output file
rm "$actual_output"

exit $tests_failed
