# App icon tooling

`icon_tools.cpp` provides two native C++17 commands without requiring Python.

Build on Linux or macOS with `c++ -std=c++17 -O2 .github/scripts/icon_tools.cpp -lz -o icon-tools`.

- `icon-tools generate AppIcon-1024.png` reconstructs the canonical 1024×1024 RGB source icon. The exact canonical PNG Git object is `9ab34644b8b0d5f951a3e47651abb2687803f05c`.
- `icon-tools validate source.png compiled.png [max-mean-error]` decodes ordinary PNG and Apple's CgBI PNG format, sampling RGB differences and limiting unexpected white pixels.

The iOS Build workflow regenerates the original icon, confirms identical PNG bytes with `cmp`, and validates Apple's compiled icon before sealing the unsigned IPA. The committed source icon and the iOS app itself are unchanged.
