# Architecture

## Purpose

Character Manager is a native iPhone story-bible and character-development application centred on durable author-owned characters, relationships, history and portable archives.

## System decomposition

- SwiftUI application and navigation
- SwiftData local store
- application-owned versioned archive format
- Image Playground integration
- RealityKit/Quick Look appearance workspace
- unit/UI tests and release packaging

## Ownership boundaries

SwiftData is a storage implementation, not the interchange contract. The application owns its archive, reconstruction and validation semantics. Optional AI/photogrammetry extends visual work but does not define core story data.

Dependencies provide mechanisms behind explicit boundaries; they do not replace the project's responsibility for product semantics. Portable/domain rules should remain independent of UI/toolkit or host-specific handles wherever the architecture permits it.

## Source of truth

Code and tests define executable behaviour. This document defines ownership and dependency direction. Specialist documents may define narrower contracts but must remain consistent with this architecture.

## Change discipline

Cross-layer shortcuts need a documented reason. Unsupported states must remain explicit across boundaries. Compatibility or persistent-format changes require an intentional migration/versioning decision rather than accidental behaviour.

## Specialist documentation

- ARCHITECTURE.md
- docs/PRODUCT_SPEC.md
- docs/FEATURE_STATUS.md
- docs/RELEASE_CHECKLIST.md
