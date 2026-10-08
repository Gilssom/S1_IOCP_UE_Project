# S1 UE 5.8 Project Execution Guide

## 1. Portfolio objective

Build a compact YouTube portfolio that clearly demonstrates:

- An Unreal Engine 5.8 client communicating with the existing C++ IOCP game server
- A multiplayer combat scenario centered on players and allies versus server-authoritative monsters
- Consistent local/remote character presentation across multiple clients
- Quest, item, and character-information flows backed by the server and database
- Engineering evidence: architecture boundaries, packet/state flow, diagnostic tooling, two-client validation, and a packaged build

Depth and demonstrability take priority over content volume. The target is a coherent vertical slice, not a content-complete RPG comparable to `UE5_RPG`.

## 2. Working relationship

The user defines the frame; Codex performs most implementation.

### User responsibilities

The user should make decisions that are subjective, external, credentialed, licensed, or require direct visual judgment:

- Choose the feature goal, expected player experience, and acceptable MVP boundary
- Choose/acquire Marketplace or external assets and confirm their portfolio usage rights
- Import or migrate assets when acquisition UI, account access, or license confirmation is required
- Select the preferred character, weapon, monster, UI, VFX, and audio direction
- Perform artistic adjustments that need direct judgment when requested: retarget pose review, socket alignment, collision silhouette, montage section timing, material appearance, Niagara look, and camera feel
- Start or reset the external server and provide test accounts without exposing passwords in source, prompts, screenshots, or logs
- Run hands-on play checks when feel or appearance is the acceptance criterion
- Approve destructive/bulk binary-asset changes and create a checkpoint when requested

The user does not need to write routine gameplay code, packet plumbing, Blueprint wiring, data registries, validation tools, or repetitive content setup unless they specifically want to.

### Codex responsibilities

Codex should take implementation ownership after the user supplies the frame:

- Inspect existing source, assets, configuration, references, runtime state, and logs
- Propose the smallest architecture that respects the external-server authority boundary
- Implement C++ modules, subsystems, components, packet dispatch, data structures, and editor tooling
- Create or modify approved Blueprints and wire approved content assets through Unreal MCP
- Build input mappings, presentation registries, AnimBP/Montage/Cue connections, UI logic, and placeholder content where appropriate
- Integrate quest, item, character, and combat client flows with the server protocol
- Update tests, build the affected targets, run focused Blueprint validation, and perform two-client PIE/log verification where possible
- Maintain `CurrentAssetRegistry.md` and other design/contract documentation
- Identify blockers early and give the user one concrete manual action at a time

### Shared decision gates

Codex must request a user decision before proceeding when a choice materially changes:

- Character identity, combat style, camera style, or overall presentation
- Target skeleton or an animation-retarget strategy
- Asset purchase, license acceptance, plugin installation, or broad content migration
- Protocol/database schema shared with the external server
- Destructive, bulk, or difficult-to-reverse binary asset work
- MVP scope or portfolio narrative

For ordinary implementation details inside an approved feature, Codex should make a documented engineering choice and continue.

## 3. Default feature workflow

The user can provide a short frame instead of a full specification:

```text
Feature:
Player-facing result:
Preferred asset/style:
Must include:
Must avoid:
```

Missing noncritical fields do not block work. Codex will inspect the baseline and return:

1. The owning client/server/database layers
2. The exact source files and assets that may change
3. Any user decision, missing asset, plugin, restart, server state, or manual editor step required
4. A narrow implementation and validation plan
5. The implementation, verification evidence, updated asset registry, and remaining risks

## 4. Asset intake workflow

When the user says that a new asset has been added, the message only needs to contain the asset name or approximate folder and intended use. Codex should locate and inspect it.

Preferred message:

```text
Added asset:
Intended use:
Source/license, if relevant:
Preferred character/class:
```

Codex then performs this sequence:

1. Resolve the exact S1 `/Game/...` path and asset class.
2. Inspect hard/soft/package/map/code/config references and dependency closure.
3. For character content, inspect skeleton, physics asset, materials, sockets, AnimBP, animation sequences, montages, root-motion mode, and retarget requirements.
4. Check UE 5.8/plugin/native-parent compatibility and current dirty packages.
5. Register it as `Candidate`, `Blocked`, or `Active` in `CurrentAssetRegistry.md`.
6. State what Codex can wire automatically and the smallest remaining manual visual task.
7. After approval, integrate only the selected assets, validate, save only intended packages, and update status to `Active`.

## 5. Delivery roadmap

### Milestone 0 — Production foundation

- Maintain a recoverable checkpoint strategy because the project currently has no Git repository
- Establish the asset registry, presentation-data conventions, IDs, logging, and repeatable two-client validation
- Confirm the portfolio scenario and first character/monster visual direction

Exit evidence: clean baseline build, current assets documented, deterministic Lobby → Loading → Main flow, two-client login/spawn/movement trace.

### Milestone 1 — Knight combat vertical slice

- Player/creature combat state and client presentation bridge
- `AbilityId -> PresentationData` and `PlayerClass -> PresentationData`
- Basic melee attack request, server validation/result, Montage/Cue/HUD playback
- One monster with server FSM, damage, death, and respawn
- Both clients observe the same attacker, target, HP, and death result

Exit evidence: a recorded two-client encounter in which server logs and both client views agree.

### Milestone 2 — Ally and party encounter

- Players and server-controlled allied actors share the creature/presentation path
- Ally targeting, HP state, attack events, death/despawn, and reconnect recovery
- One small players-plus-allies versus monsters encounter

Exit evidence: actor roles and authoritative state remain consistent after disconnect/reconnect.

### Milestone 3 — Combat variety

- Archer projectile presentation and Mage cone/heal presentation
- Buff, cooldown, crowd-control, hit reaction, and combat UI
- Reuse the same server IDs and presentation registry instead of class-specific packet branches

Exit evidence: melee, projectile, area, buff, and heal demonstrate distinct server-authoritative rules.

### Milestone 4 — Character, item, and database loop

- Character summary/detail, stats, equipment slots, and persistence
- Item definitions, inventory instances, equip/unequip, rewards, and reconnect persistence
- Server/database ownership; Unreal is a request/presentation client and never accesses MySQL directly

Exit evidence: acquire/equip/reconnect produces the same server-backed character state on both UI and world presentation.

### Milestone 5 — Quest loop

- Quest definitions and per-character progress
- Accept, objective progress, completion, reward, and persistence
- At least one combat quest connected to the existing monster/item loop

Exit evidence: a quest survives reconnect and rewards the authoritative inventory/character state exactly once.

### Milestone 6 — Portfolio polish

- Focused HUD, combat feedback, error/reconnect UX, and controlled VFX/audio
- Architecture diagrams, packet trace, database schema view, test evidence, and failure demonstrations
- Packaged two-client run and concise video capture plan

Exit evidence: reproducible packaged demo with no unexplained Error, Fatal, Ensure, Blueprint Runtime Error, or Network Failure.

## 6. Priority rules

- Finish one end-to-end vertical slice before adding more classes, monsters, maps, or UI screens.
- Prefer reusable data-driven presentation over Blueprint branches keyed to specific classes.
- Prefer in-place combat animation for the first externally authoritative network slice. Introduce root motion only with an explicit synchronization design.
- Use placeholders when they preserve architecture and testability. Replace them when the user supplies approved assets.
- Do not port all of `UE5_RPG`. Reuse narrow patterns and selectively approved content only.
- Database features start from server ownership, stable IDs, idempotency, and reconnect behavior; UI follows after the contract is testable.
- Every portfolio feature needs visible player behavior, server evidence, multiplayer consistency evidence, and a clear failure/recovery case.

## 7. Immediate notification protocol

Codex should notify the user as soon as one of these is required:

- A creative or scope decision with materially different outcomes
- A missing mesh, skeleton, animation, montage, VFX, SFX, icon, or UI asset
- Unknown asset provenance or portfolio usage rights
- Manual retargeting, socket, collision, material, Niagara, montage, or visual-quality review
- Plugin enablement, editor restart, full rebuild, server restart, schema migration, credentials, or test accounts
- A server/protocol/database change outside the currently approved client scope
- A checkpoint before destructive, bulk, redirector, migration, or binary-asset work
- A blocker that prevents meaningful validation

Use this compact format:

```text
필요 사항: <one concrete item>
이유: <why it is needed now>
담당: 사용자 | Codex | 공동
차단 여부: 차단 | 비차단
다음 작업: <the next action after it is supplied>
```

Ask for one actionable decision or manual step at a time. If it is non-blocking, continue implementation with a clearly named placeholder and record the replacement point.

## 8. Next recommended decision

The active Knight combat frame now uses Paragon Terra's A-D primary melee combo and Q charge presentation. E is disabled, and shield guard is deferred until Q is finalized. The detailed contract is `docs/KnightCombatDesign.md`.

The next content/design gate is deliberately narrow:

- Visually approve the exact damage and combo-window frames after Codex authors the first S1 montage pass
- Decide Q distance, duration, steering, stop-on-hit, and multi-target policy before the server movement/collision implementation
- Supply or approve a Terra-compatible in-place dodge animation; dodge remains blocked until then
- Record the Paragon Terra portfolio usage-rights note

The local GAS/input scaffold and A-D/Q presentation are implemented. After an editor restart and local animation check, Codex can implement the shared external-server action messages and two-client synchronization. Guard will be redesigned only after the Q server action model is accepted; E remains disabled.
