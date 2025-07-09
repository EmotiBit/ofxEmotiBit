# Description
- Test to validate effect of changing the `parsedDataFormat.json`.
- The test temporarily changes the `parsedDataFormat.json` file by toggling `TL` column to `false`.
- The md5sum of the resulting parsed sample data is compared against the ground-truth hash.
- The `parsedDataFormat.json` holds the modified version of the output format.

