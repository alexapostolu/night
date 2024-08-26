#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <input_file>"
  exit 1
fi

# Night executable
night="$1"

# Night source code files for testing
night_files=(
	"programs/cs50/w1_credit.night"
	"programs/cs50/w1_mario.night"
	"programs/cs50/w2_readability.night"
	"programs/cs50/w2_scrabble.night"
	"programs/cs50/w2_substitution.night")

# Standard input and output files corresponding to the Night source code files
inputs=(
	"StandardIO/w1_credit_input.txt"
	"StandardIO/w1_mario_input.txt"
	"StandardIO/w2_readability_input.txt"
	"StandardIO/w2_scrabble_input.txt"
	"StandardIO/w2_substitution_input.txt")
expected_outputs=(
	"StandardIO/w1_credit_expected.txt"
	"StandardIO/w1_mario_expected.txt"
	"StandardIO/w2_readability_expected.txt"
	"StandardIO/w2_scrabble_expected.txt"
	"StandardIO/w2_substitution_expected.txt")

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
        echo "'$night_file' execution succeeded (Exit code: $status)."
    else
        echo "'$night_file' execution failed (Exit code: $status)."
    fi

    # Compare actual output to expected output
    if diff "$actual_output" "$expected_output" > /dev/null; then
        echo "Output matches the expected output."
    else
        echo "Output does not match the expected output."
        echo "Differences:"
        diff "$actual_output" "$expected_output"
		all_tests_passed=1
    fi

    echo
done

# Clean up temporary output file
rm "actual_output.txt"

exit $all_tests_passed