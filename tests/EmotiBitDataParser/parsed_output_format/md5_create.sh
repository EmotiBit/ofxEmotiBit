#!/bin/bash

sample_data_dir="../sample_data"
output_dir="./expected_output_hash"
file_pattern="*_[a-zA-Z]*.csv"
csv_ext=".csv"
md5_ext=".md5"
if [ -d "$sample_data_dir" ]; then
  for file in $(find $sample_data_dir -type f -name "$file_pattern"); do
    if [ -f "$file" ]; then
      base_filename=${file%$csv_ext}
      base_filename=${base_filename##.*/}
      md5_filename=$base_filename$md5_ext
      #echo $md5_filename
      echo "Creating md5sum for : $file to $md5_filename"
      md5sum $file > $output_dir/$md5_filename
    fi
  done
else
  echo "Error: Directory '$directory' not found."
  exit 1
fi
