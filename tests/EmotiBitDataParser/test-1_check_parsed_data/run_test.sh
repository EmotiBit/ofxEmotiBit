#!/bin/bash

hash_dir="expected_output_hash"
for file in "$hash_dir"/*; do
  if [ -f "$file" ]; then
    #echo "checking $file"
    md5sum -c $file
    if [ $? -eq 0 ]; then
      echo "MD5 checksum verification successful."
    else
      echo "MD5 checksum verification failed."
      exit 1
    fi
  fi
done
exit 0