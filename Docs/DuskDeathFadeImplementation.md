# Dusk existing-material death fade — 2026-10-01

## Implemented scope and visual distinction

Client presentation only. IOCP DEAD/HP/damage/navigation/spawn/removal remain unchanged; Room still sends S_DESPAWN 3 seconds after death, and WorldEntity destroys the actor immediately. Terra A–D/Q/trails/impact and the Dusk 0.7 scale / 28×62 capsule / +90° mesh facing / 61.5cm server center are unchanged.

This uses **existing translucent death fade, not masked erosion dissolve**. No MI_Cave_Dissolve/MF_Dissolve integration and no Paragon graph edits. Translucent lighting/shadows differ from opaque; the user accepted the current death transition in 1-/2-client PIE on 2026-10-01.

## Read-only MCP findings and rendered trial

- Actual skeletal mesh: `/Game/ParagonMinions/Characters/Minions/Dusk_Minions/Meshes/Minion_Lane_Melee_Dusk`, one slot `M00_dusk_melee`, material `MI_Minion_Dusk_Melee`. BP CharacterMesh0 has no material overrides. Armour/weapon are part of this mesh, not separately assigned material slots.
- Parent chain: MI_Minion_Dusk_Melee → M_Minion_Dusk_Armored (MIC) → M_Minion_Dusk_Parent (Opaque / material attributes / skeletal usage).
- Root output: MatLayerBlend_Emissive → MF_CharacterEffects → MF_BrightnessAdjust. MF_CharacterEffects BreakMaterialAttributes receives **MF_DeathFade.FadeToBlack**, NOT its AlphaFade output.
- FadeToBlack passes through input OpacityMask unchanged. Setting a MIC to Masked alone would not activate AlphaFade. AlphaFade separately has DitherTemporalAA feeding OpacityMask.
- FadeToBlack opacity is `inputOpacity * (1 - saturate(2 * FadeOut))`. Thus the meaningful opacity fade range is 0..0.5, not 0..1.
- A new, unsaved S1-only trial MIC inherited the original melee material and overrode blend to Translucent, VertexDisplacePower=0. MCP CaptureAssetImage sphere trial showed the surface at FadeOut=0 and background only at 0.5 and 1. Skeletal preview thumbnail returned blank at both endpoints and was inconclusive; it is NOT skeletal visual proof. Captures: workspace `checkpoints/DuskDeath_20261001/{sphere0,sphere05,sphere1}.png`.
- Trial MIC `MI_S1Dusk_DeathPreview` was deleted after the final duplicate was created. No original package was saved. This was a safe MIC trial, not a live Dusk MID/PIE trial; runtime independence is covered separately by code/tests.

## Files, asset and tunables

- `Source/S1/Game/S1DuskMinion.h/.cpp`: death lifecycle and independent transient MID.
- `Source/S1/World/S1WorldEntitySubsystem.cpp`: initial snapshot calls ApplyInitialServerState, ordinary updates still use ApplyServerState.
- `Source/S1/Game/S1DuskDeathPresentationTests.cpp`: focused synthetic-world automation.
- Only saved binary: `/Game/S1/Monsters/Materials/MI_S1Dusk_DeathFade`, parent original MI_Minion_Dusk_Melee, Translucent blend override, FadeOut=0 and VertexDisplacePower=0. No BP/map/mesh/original material save.
- BP_S1DuskMinion Class Defaults → Dusk / Presentation / Death: DeathFadeDelaySeconds=1.5, DeathFadeDurationSeconds=1.2, DeathFadeMaterial. Runtime clamps completion to at most 2.9 seconds. Changing timing does not change server lifespan.
- Original opaque material stays active before the fade delay. At the delay a per-actor MID replaces only inspected slot0, then FadeOut ramps 0→0.5. At completion the entire actor including label is hidden, not destroyed locally. Unsupported material/slot configuration still hides at the end; it must not be mistaken for a successful fade.
- DEAD starts once; duplicate DEAD/HP does not reset elapsed time. HP alone does not start death. Stale alive state cannot resurrect an object ID. Already-DEAD initial snapshots are hidden/completed immediately because no death age exists in the protocol. Respawn uses a fresh actor and original opaque appearance. EndPlay clears owned MID reference and attack/pending state; there are no deferred timers or detached corpses.

## Safety and validation

- Verified SHA-256 checkpoints of the dirty-current Dusk h/cpp, WorldEntity cpp and original melee MIC were made under workspace `checkpoints/DuskDeath_20261001` before edits.
- Pre-save MCP dirty checks: new trial and final MIC=true; BP_S1DuskMinion, ABP_S1Dusk, MainMap, LobbyMap and original melee MIC=false. This is a scoped dirty check, not an exhaustive global package inventory. Trial was removed, only final MIC saved; final dirty=false and asset reload passed.
- S1Editor Development build/link passed. Initial automation launch found no test because cached build source discovery omitted the new cpp; gather rebuild included it. First actual test exposed overly strict float comparisons in the harness; tolerances/frame sampling were corrected. Final automation and packaging results are recorded below after execution.
- User confirmed current-version 1-/2-client PIE lighting change, natural disappearance of body/armour/weapon and respawn on 2026-10-01. This is user-performed visual acceptance, not an independently replayed or log-audited run. Reconnect during fade/DEAD snapshot, interruption by early despawn, broader combat/navigation regression and packaged two-client play remain unverified in this revision.

## Executed final verification

- `DuskDeath_EditorBuildFinal.log`: S1Editor Development compiled/linked, Result: Succeeded. BuildCookRun also built S1 Development successfully.
- `Saved/Automation/DuskDeath_Final/index.json`: 4 tests succeeded, 0 failed. DuskDeathFade verifies independent MIDs, delay/midpoint/completion, HP-only authority, duplicate DEAD, stale alive update, teardown, DEAD snapshot, fresh respawn. Existing ImpactSettings, KnightChargePresentation and KnightQApproval also passed. One pre-existing Q presentation test warning: missing GameplayCueNotifyPaths, fallback scan /Game; not changed in this task. NullRHI synthetic-world checks are not visual/network acceptance.
- `Saved/Logs/DuskDeath_AssetValidation.log`: separate Python commandlet compiled/reloaded BP_S1DuskMinion/ABP_S1Dusk with generated classes BP_S1DuskMinion_C/ABP_S1Dusk_C; saved death MIC blend=Translucent, FadeOut=0, VertexDisplacePower=0; 0 errors. No assets saved. Fresh process dirty-before/after lists both contained only the existing Terra mesh `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra`; it was NOT saved. This differs from scoped live pre-save checks and does not imply a persisted Terra change.
- Windows Development BuildCookRun build/cook/stage/pak/IoStore/package/archive: `BUILD SUCCESSFUL`, AutomationTool ExitCode=0. Cook summary 0 errors / 1 MCP licensing notice; no shader compile failure. Archive: `Saved/DuskDeathPackage_20261001/Windows`. Cooked DevelopmentAssetRegistry contains MI_S1Dusk_DeathFade and all three required maps.
- Original melee MIC SHA-256 remains identical to checkpoint after validation. Q/server/protocol/DB/original Paragon material graphs were not edited.
- Packaged S1 Development executable launched with NullRHI/unattended and ExecCmds=quit, exited0. Log: `Saved/Logs/DuskDeath_PackagedStartup.log`. This is startup/exit only, NOT rendered death, authenticated combat, or two-client runtime proof. Current-revision 1-/2-client PIE visual acceptance was subsequently reported by the user; packaged two-client play remains pending.
- Packaged startup also reproduced the known, out-of-scope Lobby UI focus error: `InputMode:UIOnly - Attempting to focus Non-Focusable widget SObjectWidget` (line777). Therefore the full startup log is NOT error-free. Missing optional aqProf/VTune/PIX DLL messages are baseline profiling notices. No Fatal/Ensure or Dusk material-load failure was found in this smoke-run log. UI code/settings were not edited.

## User acceptance and remaining manual checks

On 2026-10-01 the user reported completing the new-version 1-/2-client PIE checks for lighting change, natural body/armour/weapon disappearance and respawn. No code or binary asset was changed for this documentation update. The procedure below is retained for reproduction; steps3–4 and packaged two-client play are not covered by that confirmation.

1. Open the rebuilt editor, enter MainMap with the external server, kill Dusk. Death_A should remain visible first, then fade during1.5–2.7s and disappear before server despawn; label must not remain floating. Check for a lighting pop when opaque switches to translucent.
2. Wait for fresh respawn: original opaque appearance, size/facing/ground alignment unchanged. Repeat in two clients, comparing one death on both screens and HP/attack results.
3. Disconnect while fading, reconnect during an already-DEAD snapshot, and confirm no replayed death/stale fade on the next spawn. Test server-driven removal during fade; no actor remnant is retained.
4. Recheck wall pursuit/return and Terra A–D/Q/trails/impact; these were not edited.

If true hole/edge erosion is required instead of this existing opacity fade, the smallest separate proposal is an S1-owned material adaptation wiring the existing MF_DeathFade AlphaFade opacity-mask into the preserved final surface. It is not implemented here, nor is the imported cave material used.
