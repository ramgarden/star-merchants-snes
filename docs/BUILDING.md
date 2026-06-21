# Building

Prerequisites:
- Windows 11
- MSYS2 UCRT64
- PVSnesLib
- VS Code
- Mesen

## Recommended flow
From the repository root, run the project script in the MSYS2 UCRT64 terminal:
```bash
./scripts/build-and-run.sh
```

## Manual build
If you want to build without launching the emulator:
```bash
cd game
make clean
make
```
