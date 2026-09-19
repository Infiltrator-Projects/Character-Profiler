# Decisions

This file records durable architectural decisions for Character Manager.

## ADR-001 — Author-entered data is canon

**Decision.** Suggestions, prompts and visual tooling may assist authors but never silently overwrite authored facts.

**Rationale.** The application is a story bible, so user-authored continuity is the primary authority.

**Consequence.** Suggested content remains distinguishable from persisted canon and destructive changes require deliberate user action.

## ADR-002 — SwiftData storage and portable archive are separate contracts

**Decision.** SwiftData is the local persistence implementation; Character Profiler archive format is the portable interchange contract.

**Rationale.** Raw database layout should not determine backup portability or long-term restore semantics.

**Consequence.** Schema migration and archive-format evolution are versioned/tested independently.

## ADR-003 — Relationship diagrams are derived views

**Decision.** The persistent source is one relationship graph; family/relationship diagrams are projections, not second databases.

**Rationale.** Duplicate relationship stores inevitably drift.

**Consequence.** Graph invariants are enforced in the model and visual layouts remain disposable/rebuildable.

## ADR-004 — Free-text life timing remains author-controlled

**Decision.** Life-event timing is not forced into a universal machine date.

**Rationale.** Fiction may use ages, eras, relative phrases or invented calendars that cannot be meaningfully normalized.

**Consequence.** Chronology ordering is explicit author order with deterministic normalization.

## ADR-005 — 3D reconstruction is temporary unless persistence is explicitly designed

**Decision.** RealityKit photogrammetry output is temporary presentation state and is outside archive format v1.

**Rationale.** Persisting large derived 3D assets changes storage, migration and portability requirements.

**Consequence.** Future persistence/export requires an explicit schema/archive decision rather than incidental model growth.

## ADR-006 — Save failure rolls back the current unit of work

**Decision.** User-visible persistence failure must rollback pending context mutations.

**Rationale.** Leaving failed edits dirty risks them being committed by an unrelated later save.

**Consequence.** Major save/import paths use rollback-safe persistence semantics.