#!/bin/bash

# Night executables
night="..\out\build\x64-Debug\night.exe"
# Night source code files for testing
night_files=(
	"Programs\cs50\w1_credit.night"
	"Programs\cs50\w1_mario.night"
	"Programs\cs50\w2_readability.night"
	"Programs\cs50\w2_scrabble.night"
	"Programs\cs50\w2_substitution.night")

# Standard input and output files corresponding to the Night source code files
inputs=(
	"StandardIO\w1_credit_input.txt"
	"StandardIO\w1_mario_input.txt"
	"StandardIO\w2_readability_input.txt"
	"StandardIO\w2_scrabble_input.txt"
	"StandardIO\w2_substitution_input.txt")
expected_outputs=(
	"StandardIO\w1_credit_expected.txt"
	"StandardIO\w1_mario_expected.txt"
	"StandardIO\w2_readability_expected.txt"
	"StandardIO\w2_scrabble_expected.txt"
	"StandardIO\w2_substitution_expected.txt")

for i in "${!night_files[@]}"; do
    # Night code
	night_file="${night_files[$i]}"

	# Standard input file
	input="${inputs[$i]}"
    
	# Expected and actual standard output files
	expected_output="${expected_outputs[$i]}"
    actual_output="actual_output.txt"

    # Run the program and capture the output
    $night $night_file < "$input" > "$actual_output"

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
        echo "Output for does not match the expected output."
        echo "Differences:"
        diff "$actual_output" "$expected_output"
    fi

    echo
done

# Clean up temporary output file
rm "actual_output.txt"