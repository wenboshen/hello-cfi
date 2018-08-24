# hello-cfi

## Kernel CFI based on Clang

This project hosts two source files for a kernel proc module.

### Build 
1. Add these two source file to `kernel/drivers/misc`
2. Enable build in `kernel/drivers/misc/Makefile`
```
obj-y				+= hello_cfi.o
obj-y				+= hello_cfi_asm.o
```

### Use
```
cat /proc/hello_cfi 
echo [0-5] > /proc/hello_cfi
```


### Code details
Here not_entry_point function must be written in purely assembly, to avoid compiler generated prologue and epilogue.
Detailed explaination in [].

### Acknowledge
This project is based on [cfi_icall.c](https://github.com/trailofbits/clang-cfi-showcase/blob/master/cfi_icall.c), credit to trailofbits.
