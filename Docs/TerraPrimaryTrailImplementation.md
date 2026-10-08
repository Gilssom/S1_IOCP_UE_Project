# Terra A-D weapon trail implementation record

Date: 2026-09-29

## Current status update (2026-09-30)

The record below describes the original Cascade implementation and its failed visual trial, not the currently accepted visible Trail. The user subsequently added SwordTrailVFX and authored timed Niagara effects in A-D montages. Live MCP dependency checks on 2026-09-30 confirm all four saved A-D montages reference `/Game/SwordTrailVFX/VFX/NS_Trail_14`; the new Q montage references the same system. The previous Trail 4 selection is not the final saved reference. The user confirmed A-D local/remote Trail visibility in 1-/2-client PIE, then confirmed Q motion/Trail, movement/basic-attack recovery and remote Q visibility in the same environments.

The original `P_Terra_AnimTrail` C++ timer/component path is still source-wired; it has not been removed or proven visible. Narrow cleanup is a proposed follow-up, not completed work. Exact final Niagara notify frames, socket settings, width and lifetime were not audited in this status update. The 2026-09-29 package test below does not certify the subsequently added Niagara/Q content. See `TerraQMontageIntegration.md` and `AstraProgressHandoff_2026-09-30.md`.

## Scope and ownership

`P_Terra_AnimTrail` is client-only swing presentation for Terra's A-D basic attack. It starts from the locally predicted montage for the attacker and from the accepted remote montage for other players. It also appears on a miss. It does not request or determine hit, damage, HP, death, or server timing. The existing `P_Terra_Primary_Impact` remains driven exclusively by positive server damage events.

## Asset and animation inspection

- Selected asset: `/Game/ParagonTerra/FX/Particles/Terra/Abilities/Primary/FX/P_Terra_AnimTrail`, a Cascade `ParticleSystem` with `ParticleModuleTypeDataAnimTrail` recorded in the package. UE loaded the asset and resolved its direct material dependency, `M_Terra_PrimaryAnimTrail`; warmup/delay are zero and auto-deactivation is enabled. The Python asset API protected the emitter array, so emitter-level lifetime and loop values were not independently read.
- Terra's existing skeleton supplies `FX_Trail_R_01` and `FX_Trail_R_02` on `weapon_slide`. Their relative positions are approximately `(0,103.68,-52.86)` and `(0,-9.20,-42.84)` cm, so no socket edit was needed. The mesh reports 29 sockets including these two.
- A-D montages each use a single corresponding `Primary_Melee_*_slow` sequence on `UpperBody`. Their lengths are approximately 1.267, 1.333, 1.133 and 1.333 s. The montages have `SaveAttack` and `ResetCombo` notifies, but no Trail notify. The base sequences have no notify events. Read-only sampled hand/weapon poses at 0.1-second intervals informed the initial swing windows below; visual timing remains to be tuned in PIE.
- Unreal MCP was not exposed in this implementing session. A separate read-only UE Python commandlet completed its inspection script, but exited with environment errors for Zen security-config writing and a second MCP HTTP listener binding to the already open editor's port 8000. These do not prove gameplay failure or a clean commandlet exit. The editor was already open; no binary package was changed or saved.

## Implementation

Only `Source/S1/Combat/S1KnightCombatComponent.h/.cpp` implement the feature. A single particle component per Terra character is lazily created on the skeletal mesh. It uses Unreal's `BeginTrails`/`EndTrails` with `ETrailWidthMode_FromCentre`, the existing two sockets and width scale `1.0`. A-D have editable start/end seconds measured from each montage start:

| Step | Start | End | Status |
|---|---:|---:|---|
| A | 0.23 s | 0.73 s | Provisional, pose-sampled; PIE visual timing pending |
| B | 0.17 s | 0.72 s | Provisional, pose-sampled; PIE visual timing pending |
| C | 0.15 s | 0.65 s | Provisional, pose-sampled; PIE visual timing pending |
| D | 0.27 s | 0.78 s | Provisional, pose-sampled; PIE visual timing pending |

These are component-owned presentation timers rather than montage NotifyStates. This keeps the open imported montage packages untouched and allows explicit cleanup on the already centralized local/remote combo, rejection and teardown paths. The choice should be revisited only if PIE shows timing drift from montage playback speed or a need for additional authored swing phases. `Montage_SetBlendingOutDelegate` also cleans up an externally interrupted montage. An older step's blend-out callback is ignored after a new combo step takes ownership. The component is reused across attacks rather than accumulating new particle components, and is destroyed on actor EndPlay. Normal end, 80%/late combo chaining, rejection, server interruption, death and actor teardown clear both trail timers and stop emission. Local approval responses do not start another trail.

The configured FX directory was already in `DirectoriesToAlwaysCook` for the preceding impact effect. This task made no config, protocol, server, Blueprint, montage, socket, particle or material changes.

## Verification and remaining acceptance

- 2026-09-30 user 1-/2-client PIE visual check: no visible Trail on either screen. The matching `S1.log` records `Knight primary trail began` and `ended` for both `BP_MyPlayer_C_0` and `BP_Player_C_0` on A-D, with no `Knight trail skipped` entry. This verifies the presentation path reached `BeginTrails`/`EndTrails`, **not** that Cascade emitted or rendered particles. Added diagnostic logging at trail start/end for registration, emitter count, active particle count, attachment and render/bounds state. Both `S1 Win64 Development` and `S1Editor Win64 Development` rebuilt successfully; the rebuilt editor is open. The asset remains Candidate.
- The follow-up PIE log confirms one registered, active Cascade emitter attached to `CharacterMesh0` with template `P_Terra_AnimTrail`; the sample just before ending contains 5-9 active particles and `RecentlyRendered[1]` with non-zero bounds. This rules out a missing asset, missing socket, or zero-particle emission as the immediate cause, but does not establish that the additive material produces visible pixels. Editor log applied `EffectsQuality@1` and `r.MaterialQualityLevel:2` (Medium) after initial higher startup settings. The Trail material package contains a `MaterialExpressionQualitySwitch`. Computer-use screen capture of the live editor timed out twice, so the material preview itself was not inspected.
- User tested `sg.EffectsQuality 3` and the Trail remained invisible, so the Medium-quality branch alone does not explain it. A temporary source-only `DefaultParticle` override was compiled for a planned comparison, but no valid PIE comparison occurred: agent-launched editors hit Zen data-path permission errors, and computer-use screen capture disrupted the user's Parsec remote control. The agent stopped its editor and computer-use session, confirmed remote control recovered, then removed the override. No particle/material asset package was changed or saved. Material-versus-geometry rendering remains unresolved; do not describe this diagnostic build as a fix.

- `S1 Win64 Development` C++ build compiled and linked successfully after the implementation and blending-out callback changes (`Result: Succeeded`).
- After the user saved and closed the editor and Visual Studio debugger, `S1Editor Win64 Development` compiled and linked successfully (`Result: Succeeded`). The earlier DLL/PDB lock failure is resolved.
- A read-only UE 5.8 Python commandlet using the rebuilt editor module exited with code 0 and reported `Success - 0 error(s), 2 warning(s)`. It loaded both FX systems, all A-D montages, Terra mesh, the two existing trail sockets, and the generated classes of `BP_MyPlayer` and `BP_Player`. The warnings were the known missing-class `Orion_Proto_Retarget` reference and the Model Context Protocol EULA notice. No Blueprint package was saved.
- A fresh Windows Development `BuildCookRun` completed build/cook/stage/package/pak/IoStore/archive with `BUILD SUCCESSFUL` and AutomationTool `ExitCode=0`; 821 packages were considered and 813 entered the IoStore container. The fresh `DevelopmentAssetRegistry.bin` contains `P_Terra_AnimTrail`, `P_Terra_Primary_Impact`, `LobbyMap`, `LoadingMap` and `MainMap`. The archive is `Saved/TrailPackage_20260929/Windows`.
- The archived packaged `S1.exe` mounted the IoStore containers, logged `Game Engine Initialized`, and exited code 0 after an explicit quit under NullRHI. This did not enter MainMap or play the trail. It reproduced the previously known Lobby `InputMode:UIOnly` non-focusable-widget error; this task did not alter UI code.
- Read-only UE asset inspection established the class, direct dependency, existing sockets, montage lengths and notify inventory. It did not render the effect or prove the internal emitter's lifetime.
- 1-/2-client PIE, packaged MainMap combat and user visual acceptance of timing, width, brightness and direction are **not yet completed** for this trail revision. The already accepted monster impact effect must not be counted as trail verification.
- In PIE check a single A, A-D chaining and late inputs, misses, both local and remote attacks, simultaneous players, rejection, death, disconnect and reconnect. The trail should follow the axe only during the swing, never during idle/recovery; no long bridge or duplicate component should remain. On a miss the trail should appear but the monster impact should not. Check that HP and existing impact behavior remain unchanged. Finally inspect logs for missing assets, Blueprint Runtime Error, Ensure, Fatal and unexplained Network Failure.
