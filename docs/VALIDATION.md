# Validation

## Evidence model

Validation is layered. Compilation proves source compatibility; automated tests prove their covered contracts; integration and hardware tests prove only the environments actually exercised.

## Automated gates

- .github/workflows/ios-build.yml
- .github/workflows/publish-release.yml

CharacterProfilerTests and CharacterProfilerUITests exercise the application model and user-facing flows; hosted CI tests the optimized simulator build and compiles the device payload once. The same job validates source and compiled icon pixels, packages that app, and seals source/version/build identity and checksums.

## Manual/environment-dependent evidence

Image Playground output, photogrammetry quality, camera/photo workflows and signed physical-device behaviour require real supported Apple hardware and user credentials.

A simulated, fixture-driven or hosted result must not be described as proof of a physical-device, destructive-media or boot-path result.

## Release criterion

The exact release revision must pass its required gates, and generated assets must correspond to that revision. Known unsupported or failing behaviour remains documented as such.

## Regression rule

Reproducible defects should gain permanent regression coverage at the narrowest layer that captures the original failure. Validation documentation should distinguish automatic release blockers from optional, manual or milestone evidence.

## Identity and artifact ownership

ProjectVersion.xcconfig owns marketing/build identity for the Xcode project. Simulator tests retain testability metadata while preserving Release optimisation. The device IPA is built once, its Info.plist is checked against that configuration, and only that run's immutable artifact can reach publication. The publisher checks ZIP/Info.plist identity and hashes, compares staged and public bytes, and performs no Xcode build. The separate duplicate icon-build workflow is retired.
