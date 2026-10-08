# S1 Agent Working Guide

## Scope and project baseline

These instructions apply to the Unreal client project rooted at `E:\Git Fork\S1_UE5.8`.

- Work only in the UE 5.8 copy. Never edit the UE 5.5 original or its backup.
- Engine target: Unreal Engine 5.8, Win64, C++20, `BuildSettingsVersion.V7`, `EngineIncludeOrderVersion.Unreal5_8`.
- The game client connects to the external IOCP game server. The current local default is `127.0.0.1:7777`.
- S1 now has source-only Git (2026-10-08); Content and Unreal packages are intentionally excluded. Follow docs/GitManagement.md. Before binary-asset or difficult-to-reverse work, retain verified partial checkpoints; Git does not back up excluded assets.
- The established baseline has passed a clean `S1Editor Win64 Development` build, Blueprint compilation, two-client PIE login/movement, Windows Development packaging, and packaged-client testing.

## Product goal and collaboration contract

- The project is a YouTube portfolio demonstrating an Unreal Engine 5.8 client connected to the existing external IOCP game server.
- The primary playable scenario is multiplayer combat with players and allied characters fighting server-authoritative monsters.
- Supporting portfolio systems include quests, items, character information, and their database-backed server flows.
- The user owns the product frame: feature intent, presentation direction, asset selection/acquisition, scope, and final acceptance.
- Codex owns implementation by default: investigation, architecture proposals, C++ and Blueprint implementation, asset wiring, protocol/client integration, tests, diagnostics, and documentation.
- Optimize for a clear, demonstrable vertical slice rather than attempting to reproduce all systems or content from `UE5_RPG`.
- Use `E:\Git Fork\UE5_RPG\UE5_RPG` as a read-only content and implementation reference. Never modify it, and never migrate or copy its assets into S1 without explicit user approval.
- Follow `docs/ProjectExecutionGuide.md` for work division, milestone order, decision gates, and escalation rules.

## Architecture boundaries

Keep each responsibility in its existing layer.

### UI

- `S1AuthWidget` and `S1CharacterSelectWidget` collect input and present results.
- UI code must not own sockets, parse Protobuf messages, build packet headers, manipulate the world registry, or perform level travel directly.
- Route user actions through `US1GameInstance` or the appropriate subsystem public API.
- Never log account passwords or include them in MCP tool arguments or test reports.

### Game instance orchestration

- `US1GameInstance` coordinates UI, account, network, level-flow, and world-entity systems.
- Keep it as an orchestrator. Do not move socket loops, packet framing, account persistence, or actor-registry ownership into it.
- Delegate authentication state to `US1AccountSubsystem`, transport state to `US1NetworkSubsystem`, map phases to `US1LevelFlowSubsystem`, and actor lifecycle to `US1WorldEntitySubsystem`.

### Account flow

- `US1AccountSubsystem` owns login, sign-up, character creation/selection, pending authentication state, character summaries, and account-related packet results.
- Account UI authorization checks must remain enforced. Do not add a path that bypasses `LoginFromAuthUI` authorization without an explicit design decision and tests.
- The Unreal client must not access MySQL directly. Database work belongs to the game server.

### Network transport and packet dispatch

- `US1NetworkSubsystem` owns connection state, socket/session lifetime, network ticking, outbound submission, and typed packet delegates.
- `PacketSession`, `RecvWorker`, and `SendWorker` own packet queues and background I/O.
- `ClientPacketHandler` parses inbound Protobuf messages and dispatches them through `US1NetworkSubsystem` notifications.
- Worker threads must not read or mutate UObjects. Deliver work to the game thread through the existing queues, ticker, and subsystem delegates.
- Do not introduce blocking socket or database work on the game thread.
- Validate packet header size, packet ID, payload length, partial reads/writes, disconnects, and malformed input when changing transport code.

### Level flow

- `US1LevelFlowSubsystem` owns the `Lobby -> Loading -> Main` phase transition, readiness state, and pending spawn data received before the main world is ready.
- `S1LobbyGameMode` starts the lobby/auth/connect flow.
- `S1LoadingGameMode` controls the loading hold and opens the pending main level.
- `S1MainGameMode` completes main-level entry.
- Do not bypass the loading phase or spawn actors into a world that is not ready.

### World entities

- `US1WorldEntitySubsystem` owns local/remote player registration, spawn, movement application, despawn, and registry reset for the current `UWorld`.
- Preserve the distinction between the local player and other players.
- Handle duplicate spawn, movement-before-spawn, despawn-before-spawn, world teardown, and reconnect cases explicitly.
- `BP_MyPlayer` is the current main-map default pawn. The obsolete `BP_ThirdPersonCharacter` was intentionally removed and must not be recreated or referenced.

### Protocol files

- Treat `*.pb.h` and `*.pb.cc` as generated files. Do not edit them manually.
- For protocol changes, locate and update the canonical `.proto` source, regenerate both server and client outputs, and verify compatibility on both sides.
- Register new server packets in the correct handler table, expose a typed network event, and let the owning subsystem consume it.

## Maps and assets

- Required maps are `/Game/Maps/LobbyMap`, `/Game/Maps/LoadingMap`, and `/Game/Maps/MainMap`.
- These maps must remain in `MapsToCook`.
- `LobbyMap` is both the editor startup map and game default map.
- `DefaultGraphicsRHI` must have one effective `DefaultGraphicsRHI_DX12` declaration.
- Before deleting, renaming, moving, or reparenting an asset, check hard, package, soft, map, code, and configuration references.
- Save only assets intentionally changed by the current task. Do not use broad Save All, bulk resave, redirector fix-up, Blueprint recompile, or content migration unless the task requires it.
- Report every dirty package before saving it.

### Asset inventory policy

- `docs/CurrentAssetRegistry.md` is the canonical inventory of project content assets relevant to active gameplay and portfolio flows.
- Whenever the user reports that an asset was added, inspect its exact content path, asset class, dependencies, skeleton or parent where applicable, referencers, compatibility, and dirty state before wiring it into gameplay.
- Update `docs/CurrentAssetRegistry.md` in the same task after an asset is inspected or its usage changes.
- Keep `Active`, `Candidate`, `Planned`, `Blocked`, and `Deprecated` states distinct. An asset is `Active` only after its real project reference and intended runtime use are verified.
- Record source/provenance and license notes when the information is available. Ask the user when provenance or usage rights are unclear and the asset is intended for the published portfolio.
- Do not infer that a newly imported asset should replace an existing asset. Ask for the intended role if the choice changes character identity, animation skeleton, combat style, UI direction, or migration scope.
- Record retired assets instead of silently removing their history. The removed `BP_ThirdPersonCharacter` remains deprecated and must not be recreated.
- Candidate assets in `UE5_RPG` are references, not S1 inventory entries, until they are explicitly approved and migrated into an S1 content path.

## Codex and Unreal MCP workflow

Use Codex local file and shell tools for source analysis, C++ edits, diffs, builds, and generated-file checks. Use Unreal MCP for live editor state, maps, actors, assets, Blueprint state, PIE, Slate/UMG inspection, automation, and logs.

For every nontrivial task:

1. Read the relevant source and asset relationships.
2. State the owning architecture layer and expected behavior.
3. List the exact files and assets that may change.
4. Perform read-only MCP inspection before editor mutations.
5. Make one coherent change at a time.
6. Verify the narrow change before broad regression testing.
7. Report what changed, why, validation evidence, and remaining limitations.

When required information, content, or manual editor work blocks safe progress, notify the user immediately using the compact format defined in `docs/ProjectExecutionGuide.md`. Continue all non-blocked work with explicit placeholders rather than waiting unnecessarily.

MCP calls run serially on the Unreal game thread. Do not issue overlapping tool calls. Use the discovery sequence only as needed:

1. `list_toolsets`
2. `describe_toolset` for the relevant toolset
3. `call_tool`

Prefer these toolsets for routine S1 work:

- `Editor` and `Logs` for session, PIE, and diagnostic state.
- `Actor` and `Scene` for map/world inspection.
- `Asset` and `Object` for dependencies and references.
- `Blueprint` for load, parent, generated-class, and compile checks.
- `UMG` and `Slate UI Automation` for authentication and character-selection UI.
- `Config Settings` for effective settings and duplicate overrides.
- `Automation Test` for repeatable checks.

Do not invoke unrelated Material, Niagara, PCG, GAS, Sequencer, or mesh toolsets unless the requested feature uses them.

## Editing rules

- Inspect before modifying. Do not infer asset references from filenames alone.
- Keep changes scoped to the requested behavior and existing architecture.
- Avoid broad refactors during bug fixes unless the current structure prevents a correct fix.
- Do not modify engine source or engine plugins for a project issue without explicit authorization and a documented reason.
- Do not add runtime dependencies on experimental MCP or editor-only modules to the `S1` game module.
- Put future S1-specific MCP diagnostics in a separate project Editor plugin such as `S1MCPTools`.
- Adding a new reflected `UFUNCTION` or tool schema requires an editor restart; Live Coding is sufficient only for supported implementation-body changes.
- Preserve Windows Development packaging while making editor-tool changes.

## Validation requirements

Choose checks based on the changed layer, then run the integration checks required by the risk.

### C++ changes

Build the affected target. For full editor validation:

```powershell
& "E:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat" S1Editor Win64 Development -Project="E:\Git Fork\S1_UE5.8\S1.uproject" -WaitMutex -NoHotReloadFromIDE
```

Do not claim C++ validation from Live Coding alone when headers, reflected declarations, module dependencies, generated code, or build settings changed.

### Blueprint and asset changes

- Load and compile only affected Blueprints first.
- Require zero K2/compiler errors and resolve new warnings.
- Confirm parent class and generated class.
- Inspect dirty packages and save only approved assets.
- Reopen affected maps or assets after saving when persistence matters.

### Network, account, level, or world changes

Run two-client PIE with the external game server available. Validate:

- Both clients connect independently.
- Two test accounts can log in.
- Lobby, Loading, and Main transitions complete.
- Local and remote players spawn correctly.
- Movement is visible to the other client.
- Despawn occurs on disconnect.
- Reconnect does not duplicate players or retain stale account/world state.
- PIE shutdown produces no Fatal, Ensure, Blueprint Runtime Error, Network Failure, or stale-session failure.

Do not put test-account passwords in source, config committed to version control, logs, screenshots, prompts, or reports.

### Packaging-sensitive changes

- Confirm all three maps remain cooked.
- Run Windows Development Build/Cook/Stage/Package/Archive.
- Require `BUILD SUCCESSFUL` and AutomationTool `ExitCode=0`.
- Launch two packaged clients against the external server and repeat login, transition, spawn, movement, disconnect, and reconnect checks.

## Known baseline messages

Do not misclassify these as S1 module failures without new evidence:

- Missing optional `aqProf`, VTune, WinPix, or PIX profiling components.
- Missing Android, iOS, Linux, Mac, tvOS, or VisionOS SDK/resources when validating Windows only.
- NVIDIA driver/TSR 16-bit VALU limitation.
- Engine/plugin-origin `r.MotionVectorSimulation` render-thread-safety warning.

Track separately:

- UI focus errors caused by focusing a widget that is not focusable while applying `InputModeUIOnly`.
- New occurrences of `TP_ThirdPersonCharacter` or `BP_ThirdPersonCharacter`; neither should appear after the obsolete asset removal.

## Completion standard

A task is complete only when:

- The implementation respects the ownership boundaries above.
- Changed source builds successfully.
- Affected Blueprint/assets load and compile without new errors.
- Required maps load without missing dependencies.
- Relevant PIE or packaged-client behavior is verified.
- New log errors are explained or fixed.
- The final report names changed files/assets, tests run, results, and any unverified risk.
