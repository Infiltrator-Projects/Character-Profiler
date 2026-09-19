# Validation

## Evidence model

Validation is layered. Compilation proves source compatibility; automated tests prove their covered contracts; integration and hardware tests prove only the environments actually exercised.

## Automated gates

- .github/workflows/ios-build.yml
- .github/workflows/icon-integrity.yml
- .github/workflows/publish-release.yml

CharacterProfilerTests and CharacterProfilerUITests exercise the application model and user-facing flows; hosted CI performs simulator testing and optimized simulator/device compilation.

## Manual/environment-dependent evidence

Image Playground output, photogrammetry quality, camera/photo workflows and signed physical-device behaviour require real supported Apple hardware and user credentials.

A simulated, fixture-driven or hosted result must not be described as proof of a physical-device, destructive-media or boot-path result.

## Release criterion

The exact release revision must pass its required gates, and generated assets must correspond to that revision. Known unsupported or failing behaviour remains documented as such.

## Regression rule

Reproducible defects should gain permanent regression coverage at the narrowest layer that captures the original failure. Validation documentation should distinguish automatic release blockers from optional, manual or milestone evidence.
