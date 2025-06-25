# Testing for EmotiBit DataParser

## Tests
- `find_unused_typetags.sh` is used to compare the typetags in the sample data and the complete Typtags set to flag typetags not in the parsed data. This is an attempt to highlight typetags that are a part of the EmotiBit messaging system, but not being parsed by the DataParser in the `sample_data`. If a new typetag is unlocked by a firmware/software update, the sample data may need to be updated in such a situation.
- `check_parsed_data` is used ot verify the output of the parser when run on sample data. This output is copared with a pre-existing md5 hash to validate the parser output is correct.
- `parsed_output_format` tests the parsed output files based on changes made to the `parsedDataFormat.json` file.
### Unit tests
- 
