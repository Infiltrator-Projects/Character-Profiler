<!-- SPDX-License-Identifier: GPL-3.0-or-later -->

# Character Manager

**Project copyright:** © 1993-2026 Shannon Smith

[![iOS Build](https://github.com/Infiltrator-Projects/Character-Profiler/actions/workflows/ios-build.yml/badge.svg)](https://github.com/Infiltrator-Projects/Character-Profiler/actions/workflows/ios-build.yml)

Character Manager is a native iPhone story-bible and character-development app for authors. It combines flexible character profiles, relationships and family, structured life history, genre-aware development questions, portable backup/restore and a focused appearance workspace.

**Release identity:** [ProjectVersion.xcconfig](ProjectVersion.xcconfig)

**Platform:** iOS 17 or later  
**Licence:** GPL-3.0-or-later

## Engineering ethos

Character Manager treats characters, relationships, history and story structure as durable user-owned data first.

Platform frameworks are used where they provide a strong native contract, but the product's meaning is not outsourced to them. The application owns its archive format, reconstruction rules, validation limits and core workflows so stories remain portable and recoverable even as platform storage or optional visual tools evolve. AI and photogrammetry can extend the workspace; neither is required for the core application to remain useful.

The project favours the strongest justified design rather than novelty for its own sake. Proven interaction and data-modelling ideas remain when they serve authors well, while replacements must improve durability, usability, portability or clarity.

## Capabilities

The application is organised around three levels:

- **Story Library** — story overview, cast/development context and restore entry point.
- **Story workspace** — story summary, development metrics and cast-building workflow.
- **Character workspace** — Profile, Character Guide, People & Relationships, History and Visual Studio.

Character Manager remains local-first. SwiftData storage and Character Profiler archive format v1 retain separate compatibility contracts.

Visual Studio has two deliberately separate paths: Image Playground produces authored 2D reference imagery, while RealityKit photogrammetry can reconstruct three or more photographs into a rotatable USDZ model on supported hardware. The 3D path is for appearance inspection, not scene generation, animation, rigging or filmmaking.

## Architecture

SwiftData owns the local story/character store. Large image payloads use external binary storage. Backup files are application-owned JSON interchange documents rather than raw SwiftData database copies, which keeps archive compatibility separate from SwiftData schema compatibility.

Restore validates the archive, creates fresh local identifiers and then rebuilds relationships from archived reconstruction keys. The same backup can therefore be restored repeatedly without identifier collisions.

Backup processing is bounded: archives are limited to 128 MiB encoded size, 1,000 characters and 25,000 relationships per story, with additional limits on nested collections, text and visual payloads.

The optional RealityKit/Quick Look reconstruction implementation is isolated in `Character3DHeadWorkspaceView.swift`; `CharacterDetailView.swift` remains the character-workspace/navigation shell rather than owning the 3D subsystem.

The app contains no advertising or analytics SDKs. Photos are selected through the system picker, and backup/restore is explicit through the system document UI.

## Build and test

Open `CharacterProfiler.xcodeproj`, select the `CharacterProfiler` scheme and build for an iPhone simulator or connected iPhone. A physical-device build requires an Apple Development team under Signing & Capabilities; no personal Team ID is committed to the repository.

GitHub Actions uses pinned Xcode 16.4 on `macos-15`, prepares an iPhone simulator dynamically, runs the complete unit/UI suite on an optimized simulator build with strict Swift concurrency diagnostics, then compiles the unsigned optimized iPhoneOS build once.

Hosted CI validates the compiled application icon against the canonical source image. Physical-device Image Playground output quality and the quality of a particular photogrammetry reconstruction still require real-device testing.

## Release assets

A numbered release publishes:

| File | Purpose |
| --- | --- |
| `CharacterProfiler-<version>-unsigned.ipa` | Unsigned physical-device iOS application package. |
| `CharacterProfiler-<version>-unsigned.ipa.sha256` | SHA-256 checksum for the IPA. |

The IPA is intentionally unsigned. AltStore/AltServer, Sideloadly or another compatible sideloading tool applies the user's development signature during installation.

## Repository and release policy

This repository uses `main` as its working branch. Development changes are made directly on `main`; the normal project workflow does not depend on PR, feature or release branches.

Every push to `main` runs the complete iOS qualification workflow. An unpublished version in ProjectVersion.xcconfig is release intent across corrective commits; commit subjects have no control-plane meaning.

The publisher consumes the successful run's exact sealed IPA and validates source identity, application version/build metadata and complete remote bytes before and after draft promotion. It performs no compilation. Drafts may be repaired; public tags and assets remain immutable.

Simulator tests, device compilation, source/compiled icon validation and IPA sealing belong to the single iOS Build workflow. Publication promotes those exact qualified bytes.

## Documentation

- [Philosophy](docs/PHILOSOPHY.md) — governing engineering standard.
- [Validation](docs/VALIDATION.md) — canonical automated and environment evidence.

- `docs/PRODUCT_SPEC.md` — product intent and boundaries.
- `docs/FEATURE_STATUS.md` — current implementation and validation status.
- `docs/ROADMAP.md` — completed milestones and future candidates.
- `docs/RELEASE_CHECKLIST.md` — stable-release and physical-device validation gates.
- `docs/ARCHITECTURE.md` — model, graph, archive, migration and subsystem design.
- `CHANGELOG.md` — release history.
- `SECURITY.md` — vulnerability reporting and supported-version policy.

## Limits

Character Manager is a character-development tool. It does not silently turn suggestions into canon and does not attempt to write the novel for the author. Visual Studio is an appearance workspace, not a general-purpose image/video production system.

If Image Playground or RealityKit reconstruction is unavailable, the local profile, Guide, relationship, history and backup workflows remain usable.

## Licence

Copyright © 1993-2026 Shannon Smith; © 2026 Olivia Jezewski.

Character Manager is free software licensed under the GNU General Public License version 3 or, at your option, any later version (`GPL-3.0-or-later`). The complete licence text is included in `LICENSE`.
