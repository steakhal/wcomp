# wcomp
Obfuscating compiler for WHILE

This project serves education purposes only.

## Requirements:
 - git, diff, C compiler (like gcc)
 - [conan](https://docs.conan.io/en/latest/installation.html#install-with-pip-recommended) C++ package manager

## Build:
```bash
git clone git@github.com:steakhal/wcomp.git
cd wcomp
mkdir build
cd build
# Downloads the required nasm and cmake for this project.
conan install .. --build=missing 
source activate_run.sh  # Appends cmake and nasm to your PATH envvar.
cmake ..
make && make test
```

## Example usage:
If you want to parse and transform and dump the resulted CFG, run like this:
```
./bin/wcomp ../test/test_divisor.ok \
  --dump-ast --dump-cfg-dot \
  --flatten-cfg \
  --remap-basic-block-ids=42 \
  --random-remap-basic-blocks-seed=42 \
  --random-basic-block-serialization-seed=42 \
  --xor-encode-constants
```

If you want to generate x32 assembly to the standard output, pass the `--compile` flag as well.
