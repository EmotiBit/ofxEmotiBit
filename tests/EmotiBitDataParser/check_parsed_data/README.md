# Description
- This tests verifies that the DataParser creates expected output for the sample data.
- The parsed data will be verified with an MD5 sum.

## Scripts
- Use `md5_creator.sh` for creating the checksum of the ground-truth parsed files
- `run_test.sh` parses the data and verified the md5sum against ground truth
