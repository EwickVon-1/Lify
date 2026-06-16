#!/usr/bin/env bash
# File: runSuite.sh 

# This script is invoked in the following.
# ./runSuite.sh suite-file program
# where suite-file represents a list of test stems and the argument program 
# is the name of the program to be tested.

# The runSuite.sh script runs program on each test in the test suite (as specified by suite-file)
# and reports on any tests whose output does not match the expected output.

if [ "$#" -ne 2 ]; then
    echo "Usage: ./runSuite.sh suite-file program" >&2
    exit 1
fi


# Get all the stems in suite-file
suite_file=$1
program=$2

for stem in $(cat "$suite_file"); do
    expect_file="${stem}.expect"

    # Check for expect files (required)
    if [ ! -r "$expect_file" ]; then
        echo "Error: Expect file '$expect_file' missing or not readable" >&2
        exit 2
    fi

    # Check for in_file and args_file (optional files)
    in_file="${stem}.in"
    args_file="${stem}.args"

    if [ -r "$args_file" ]; then
        args=$(cat "$args_file")
    else
        args=""
    fi

    # Create temp file to store output
    tempfile=$(mktemp)

    if [ -r "$in_file" ]; then
        "$program" $args < "$in_file" > "$tempfile"
    else
        "$program" $args > "$tempfile"
    fi

    # Check for differences with the expect file
    diff "$tempfile" "$expect_file" > /dev/null

    if [ $? -ne 0]; then
        echo "Test Failed: $stem"
        echo "Args:"
        [ -r "$args_file" ] && cat "$args_file"
        echo "Input:"
        [ -r "$in_file" ] && cat "$in_file"
        echo "Expected:"
        cat "$expect_file"
        echo "Actual:"
        cat "$tempfile"
    fi

    rm $tempfile
done
