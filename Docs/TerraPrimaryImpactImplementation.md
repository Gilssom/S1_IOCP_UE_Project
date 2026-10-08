# Terra primary-to-Dusk impact presentation

Date: 2026-09-29

## Scope and authority

The external IOCP server still decides attack acceptance, hit, damage, HP and death. This change adds client-only Cascade impact presentation for a **positive** `S_DAMAGE_RESULT` from an approved Terra primary action to a currently spawned Dusk. A–D share one effect. Clicks, AnimNotify, rejected actions, misses and HP snapshots do not trigger it. No server, Protobuf, DB, Paragon package, map or Dusk animation was edited.

## Existing packet contract used

- The requesting client gets accepted `S_COMBAT_ACTION_RESULT` with `action_type`, `source_object_id` and `action_instance_id`. The server sends this before scheduling the hit.
- Other clients get accepted `S_COMBAT_ACTION` with the same identifiers before the later damage result.
- `S_DAMAGE_RESULT` contains action, source, target, positive damage and a unique room event sequence. The event sequence, not action ID alone, distinguishes future multi-target/multi-hit damage events.
- `US1WorldEntitySubsystem` caches at most 64 approved action mappings for eight seconds and up to 256 played impact event sequences. It checks the action's source against the damage source, that the source player and Dusk target still exist, and that a configured action type has a particle. Missing or late action metadata is omitted rather than guessed as Terra. World/registry teardown, explicit disconnect and network disconnection clear both caches. Despawn removes mappings owned by that source. Damage HP application remains on its existing path even if the presentation is suppressed.

This uses the existing server order on one TCP stream and does not add out-of-order damage buffering. If action metadata is absent at damage dispatch (for example during level transition), the impact is intentionally skipped; no stale later replay occurs. The level flow continues to apply pending HP without converting snapshots into impacts.

## Configured effect and lifetime

- Project Settings class: `US1CombatImpactSettings` (`DefaultGame.ini`), one rule for wire `CombatActionType=1` (`COMBAT_ACTION_KNIGHT_PRIMARY`). Additional action types can map to other particles without changing Dusk logic.
- Particle: `/Game/ParagonTerra/FX/Particles/Terra/Abilities/Primary/FX/P_Terra_Primary_Impact`.
- Initial location: target actor/capsule center plus local `(0,0,30)` cm, intended as a Dusk torso starting point. It is not an exact weapon contact point.
- Facing: horizontal source-to-target direction; rotation offset `(0,0,0)`. Both basis and offset are configurable.
- Uniform scale: `0.7`, matched to the Dusk visual scale and accepted in the user's 2026-09-29 two-client PIE appearance check.
- World-space spawn, automatic particle destruction, plus a 2.5-second forced cleanup cap. It is never attached to the moving/deleting monster. A lethal confirmed hit still spawns at the current target location before later despawn; a packet for an already absent target does not create an actor or effect.
- The specific particle folder is included in `DirectoriesToAlwaysCook`; its material, mesh and vector-field dependencies are reached through the particle asset.

## Asset inspection

The selected package loaded as `ParticleSystem` in a read-only headless UE 5.8 inspection. System warmup and delay were zero and auto-deactivation was true. The asset registry exposed eight direct dependencies and a 32-package `/Game` dependency closure with no missing assets. The imported emitter array is protected in the available Python API, so internal emitter loop count, lifetime and visual look were not verified. The separate inspection process reported `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra` dirty; it was not saved. Unreal MCP was not exposed to this implementing task.

## Files and verification state

Changed: `Source/S1/Combat/S1CombatImpactSettings.h`, `Source/S1/Combat/S1CombatImpactSettingsTests.cpp`, `Source/S1/World/S1WorldEntitySubsystem.h/.cpp`, `Source/S1/S1GameInstance.cpp`, `Source/S1/S1.Build.cs`, `Config/DefaultGame.ini`, this document, and `docs/CurrentAssetRegistry.md`. No binary content package was saved.

- `S1 Win64 Development`: compile and link succeeded, including a final rebuild after cache and logging changes.
- `S1Editor Win64 Development`: initial affected C++ compilation succeeded, but link failed while Unreal Editor held the DLL and Visual Studio debugger held its PDB. After the user closed both applications, the final build compiled and linked successfully (`BUILD_EXIT=0`).
- `S1.Combat.ImpactSettings` automation test: 1 test found and completed with `Result={Success}` in the 2026-09-29 editor log. It verifies the configured action type, exact soft asset path, particle load, 30 cm offset, scale 0.7 and 2.5-second lifetime cap.
- Windows Development `BuildCookRun` with build/cook/stage/package/pak/IoStore/archive completed with `BUILD SUCCESSFUL` and AutomationTool `ExitCode=0`; 821 packages considered, 813 cooked and 8 skipped by platform. The cooked DevelopmentAssetRegistry contains the exact `P_Terra_Primary_Impact` path. The archived package is under `Saved/ImpactPackage_20260929` and contains the S1 IoStore containers.
- Archived packaged `S1.exe` started under NullRHI and logged `Game Engine Initialized`, then exited with code 0 after an explicit quit command. Startup logged game-server connection failures because port 7777 was unavailable and the known `InputMode:UIOnly` non-focusable-widget error. This startup check did not enter MainMap or spawn the effect.
- The existing Debug GameServer was subsequently started as PID 17900. It remained alive, accepted a local TCP connection on port 7777, and logged a successful account selection. The connection probe also produced the expected short disconnect log.
- In 2026-09-29 two-client MainMap PIE, the user confirmed that each A-D hit showed one impact on each screen, misses and wall-blocked attacks showed none, and size, brightness and facing were appropriate. This is user-observed visual acceptance, not a per-event particle count independently proven by logs.
- The current `Saved/Logs/S1.log` recorded two successful `Combat impact particle loaded` lines for the exact configured asset path. The GameServer impact-session log recorded accepted A-D actions and positive Knight-hit damage. A focused search found no `Combat impact particle unavailable`, Blueprint Runtime Error, Ensure, Fatal or Network Failure entry in that client log. These logs corroborate loading and server damage, but do not establish the visual count or negative cases on their own.
- Imported emitter loops, packaged MainMap combat, forced rejection/duplicate/delayed cases and disconnect/reconnect while an impact is pending remain unverified. Do not reuse earlier combat/Nav test success as evidence for these cases. The older `Orion_Proto_Retarget` missing Rig warning remained during cook; no particle dependency was reported missing.

### Focused play check and remaining cases

Completed by user in two-client MainMap PIE: A-D hits each showed one effect on both screens; misses and wall-blocked attempts showed none; size, brightness and facing were accepted. Still check forced server rejection, two players attacking close together, disconnect/reconnect during pending damage, effects following a moving corpse, duplicate flashes and effects persisting beyond 2.5 seconds. Packaged MainMap combat was not run.
