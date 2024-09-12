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

all_tests_passed=0

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

    # Check return value for sanity check
    status=$?
    if [ $status -eq 0 ]; then
        echo "'$night_file' execution succeeded."
    else
        echo "'$night_file' execution failed with exit code $status."
        continue
    fi

    # Remove carriage returns, no CR (fuck windows)
    tr -d '\r' < "$actual_output" > "${actual_output}.nocr"
    tr -d '\r' < "$expected_output" > "${expected_output}.nocr"

    # Compare actual output to expected output
    if diff "${actual_output}.nocr" "${expected_output}.nocr" > /dev/null; then
        echo "Output matches the expected output."
    else
        echo "Output does not match the expected output."
        echo "Differences:"
        diff "${actual_output}.nocr" "${expected_output}.nocr"
        all_tests_passed=1
    fi

    # Remove temporary files
    rm "${actual_output}.nocr"
    rm "${expected_output}.nocr"

    echo
done

# Remove temporary output file
rm "$actual_output"

exit $all_tests_passed
