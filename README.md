# Retargetable TB (Typed-B) Compiler toolkit

## Contents:

- `tb2c`: TB to C converter, written in C.
- `rtbc`: Retargetable TB Compiler, written in TB.
- `libtb`: TB standard library, written in TB.

## File extensions:

- Headers: `.tbh`
- Source code: `.tbc`

## Bootstrapping (building rtbc with gcc):

```sh
cd libtb
  mkdir -p /usr/local/share/tb/include
  cp -r include/*.tbh /usr/local/share/tb/include/
  cp -r *.tbc /usr/local/share/tb/
cd ..

cd tb2c
  gcc tb2c.c -D TB_INCLUDE="/usr/local/share/tb/include" -D TB_SOURCE="/usr/local/share/tb" -O1 -s -o tb2c
cd ..

cd rtbc
  ../tb2c/tb2c rtbc.tbc -D TB_INCLUDE="/usr/local/share/tb/include" -D TB_SOURCE="/usr/local/share/tb" -o rtbc.c
  gcc rtbc.c -O1 -s -o rtbc
cd ..
```

## Building (with rtbc):

```sh
  
```

## Cross-building for architecture X:

```sh
cd rtbc
  
```
