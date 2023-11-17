6502 emulator I programmed by following the documentation in **[6502 Introduction](http://www.6502.org/users/obelisk/6502/)**;

I took the necessary files for testing from **[Klaus2m5/6502_65C02_functional_tests](https://github.com/Klaus2m5/6502_65C02_functional_tests)**.

#### TEST EMULATOR

```sh
./test_files/as65 -l -m -w -h0 test_files/6502_functional_test.a65
make test
./bin/test.o
```