#!/bin/bash
files=""
for f in $(git diff --name-only);
do 
    files="$files $f"
done

zip "diff.zip" $files