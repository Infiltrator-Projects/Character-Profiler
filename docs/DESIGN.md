# Design

## First-principles position

Character Manager starts with the behaviour the project must own. Standards, platform frameworks and mature implementations are evidence and mechanisms, not specifications to copy blindly or semantic dependencies that may redefine the product later.

## Goals

- keep author work local-first and recoverable
- make archives portable independently of raw SwiftData storage
- keep core writing workflows useful without AI
- bound large/nested input and destructive actions explicitly

## Non-goals

The visual workspace is not intended to become a filmmaking, animation or game engine, and optional platform AI is not a required dependency for basic authoring.

## Dependency and language policy

Prefer first-party C/C++ for portable/native implementation where it fits the problem. Use platform-native language/frameworks at genuine platform boundaries. Dependencies are accepted when their documented contract is stronger than reimplementation, but project-owned behaviour stays explicit and testable.

## Failure semantics

Unknown, unavailable, unsupported and invalid are distinct states. The project prefers a clear refusal to guessed success. Mutating or destructive operations require stronger preconditions and post-verification than read-only operations.

## Decision quality

A design change should state the problem, alternatives, evidence, trade-offs and validation method. Newness alone is not a benefit. Proven mechanisms remain when they are the strongest justified choice.
