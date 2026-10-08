# S1 Knight Combat Design

Latest multi-monster scope (2026-10-06): the user explicitly approved **all valid in-range targets for both Terra A–D primary attacks and each Q hit**. Per-target damage, timing, facing, input-buffer and approval/recovery rules are unchanged. A primary action applies once per eligible captured object; Q applies once per captured object per HitIndex. Reborn ObjectIds are excluded from older actions. See MultiDuskSpawnImplementation.md for implementation and current validation; previous single-Dusk PIE results are not multi-monster acceptance.

## 1. Scope and status

This document defines the first Knight combat vertical slice for the Paragon Terra player presentation.

- Design owner: S1 client presentation plus the external IOCP server
- Character presentation: Paragon Terra
- Status: A-D and Q local GAS/input presentation plus combat-action relay implemented, built, and verified in two-client PIE; authoritative damage and Q displacement remain pending
- Authority rule: the external IOCP server owns action acceptance, hits, damage, buffs, death, and final combat state
- Client rule: Enhanced Input and GAS coordinate input, prediction/presentation, animation, cues, and UI; they do not become a second authority

## 2. Verified animation baseline

All animations below use `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra_Skeleton`.

| Role | Source asset | Length | Verified implementation note |
|---|---|---:|---|
| Combo 1 | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_A_slow` | 1.267 s | Existing `_Montage` uses the `UpperBody` slot |
| Combo 2 | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_B_slow` | 1.333 s | Existing `_Montage` uses the `UpperBody` slot |
| Combo 3 | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_C_slow` | 1.133 s | Existing `_Montage` uses the `UpperBody` slot |
| Combo 4 | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_D_slow` | 1.333 s | Existing `_Montage` uses the `UpperBody` slot |
| Guard enter | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/RMB_intro` | 0.833 s | Non-additive base sequence |
| Guard attack | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/RMB_melee` | 1.033 s | Non-additive base sequence |
| Guard exit | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/RMB_out_MSA` | 0.667 s | Only the mesh-space additive exit asset was found |
| Q charge attack | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Q_swing_B` | 3.100 s | Root Motion is disabled; translation must be driven separately |
| E team buff | `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/E_ability` | 2.100 s | Root Motion is disabled |

`ABP_S1Terra` has a verified `UpperBody` slot and a layered blend. The Terra source pack must not be modified directly. S1-specific montages, sections, notifies, and derived poses belong under an S1-owned content folder.

## 3. Input contract

| Input | Trigger | Routed action |
|---|---|---|
| Left mouse | `Started` | Normal combo attack, or guard attack while the guard state is active |
| Q | `Started` | Forward charge attack |

The active first-pass inputs are `Input.Attack.Primary` and `Input.Skill.Q`. The input layer only expresses intent. It must not decide damage, targets, or death.

E is explicitly disabled. Guard input and abilities are deferred until Q movement/collision/timing is fully designed, then guard will be redesigned against the resulting action and cancellation model.

## 4. Facing and movement rules

- The first normal attack snaps once to the camera's horizontal yaw at attack commitment. Pressing LMB during an active swing must not rotate the character; camera movement alone must not steer that swing.
- While a normal attack montage is active, movement-driven facing and remote movement-yaw interpolation are locked.
- A buffered combo input does not rotate immediately. At the current attack's branch boundary, sample the latest camera yaw, rotate once, and then start the next A/B/C/D step.
- Include the facing actually used by each combo step in its combat-action request so remote presentation performs the same boundary turn.
- Q retains its existing behavior: snapshot camera yaw and rotate at Q commitment. Q displacement remains server work.
- Normal W movement can continue to use camera-relative movement and orient the character through the existing movement path.
- Basic attacks initially allow reduced locomotion through the existing `UpperBody` layered slot. The initial movement multiplier is a tunable presentation value, not a server damage rule.
- Airborne basic attack, guard, Q, and E activation are blocked for the first vertical slice.

## 5. Four-hit normal combo

### State flow

`Ready -> A -> B -> C -> D -> A`

Each attack has `Windup`, `DamageWindow`, `ComboWindow`, and `Recovery` phases. A left-click received during `ComboWindow` sets exactly one buffered-next-attack flag. It does not restart the current montage and repeated clicks do not queue multiple future attacks.

When the current attack reaches its branch notify:

- buffered and server-permitted: advance to the next step;
- not buffered: finish recovery and reset to A;
- D buffered: start a new cycle at A;
- interrupted, stunned, or dead: clear the buffer and reset to A.

### Montage authoring

Create one S1-owned montage with sections `Attack_A`, `Attack_B`, `Attack_C`, and `Attack_D`, or four S1-owned montage duplicates if section authoring proves less stable. The preferred final form is one montage because it gives deterministic section transitions and one notify schema.

Required S1 notifies:

- `ANS_S1DamageWindow`: opens and closes the presentation trace/debug window; the server remains authoritative for the real hit.
- `ANS_S1ComboWindow`: accepts one buffered left-click.
- `AN_S1ComboBranch`: consumes the buffer and advances or exits.
- optional `AN_S1Footstep` and Gameplay Cue notifies for presentation only.

The source clips' lengths are verified, but contact frames have not been visually approved. Initial authoring may place the damage window around 25-55% and the combo-input window around 45-80% of each clip, then the user performs one montage visual pass to approve exact frames. These percentages are tuning seeds, not final hit timing.

### Network behavior

- The client may predict the montage start for responsiveness.
- Every accepted attack cycle uses a monotonically increasing client action sequence.
- The current relay validates the authenticated room actor, monotonic sequence, action type, and combo-step range. Cooldown, lockout, combo timing, and authoritative action-state validation remain server work.
- Remote clients start the matching Terra montage from the relayed server event. Authoritative start-time correction/seek is not yet implemented.
- Damage is applied only from the server result. A local weapon overlap never directly changes HP.

## 6. Shield guard and guard attack (deferred)

### Guard flow

`RMB Started -> GuardIntro -> GuardHold -> RMB Completed -> GuardOut -> Ready`

While `GuardHold` is active, left-click routes to `GuardAttack` and plays `RMB_melee`. After the attack:

- RMB still held: return to `GuardHold`;
- RMB released during the attack: queue `GuardOut` and play it after the committed attack finishes.

Death always overrides guard. Stun or guard break may override it when those server rules are added.

### Required derived asset

No dedicated `RMB_hold` animation was found, and only the additive `RMB_out_MSA` exists for guard exit. The implementation therefore needs an S1-owned guard animation layer and a derived hold pose made from the visually approved final stable frame of `RMB_intro`. The additive exit must be tested against that base pose. Do not wire `RMB_out_MSA` as an ordinary non-additive sequence without an additive-base validation pass.

### Server guard rule

The server owns `Guarding`, block eligibility, the incoming-hit angle, damage reduction, stamina/guard-break rules, and the final result. The client mirrors guard state and animation. A recommended first-pass rule is a frontal cone using the defender's authoritative facing; its angle and resource cost remain design-tuning values.

## 7. Q: forward charge attack

`Q_swing_B` supplies the presentation but has no enabled Root Motion. The charge must use server-authoritative displacement rather than extracting motion from the animation.

Recommended flow:

1. Client snapshots camera yaw and sends the Q action request.
2. Server validates the action and defines charge start, direction, distance, duration, collision policy, and one-hit-per-target set.
3. Client predicts only presentation movement within a bounded tolerance.
4. Server broadcasts the action start and resolved hits; clients reconcile position and play `Q_swing_B`.

Q should use a full-body combat slot or a dedicated full-body state, not the normal upper-body locomotion overlay. Exact distance, stop-on-hit behavior, steering allowance, and collision response require combat tuning before final implementation.

## 8. E: self and ally buff (disabled)

`E_ability` remains registered as an available source animation but is not mapped to input, granted as an ability, or included in the active Knight implementation scope.

## 9. GAS responsibilities

Recommended abilities:

| Ability | Responsibility |
|---|---|
| `GA_KnightPrimaryCombo` | Combo index, one-input buffer, montage sections, action request, and reconciliation |
| `GA_KnightChargeAttack` | Q request, bounded movement prediction, full-body presentation, and reconciliation |

The first implementation activates the two native abilities directly from their Input Actions. Gameplay tags will be introduced with the server action/state bridge, when they can represent authoritative state instead of duplicating local-only state. Planned tags include `State.Attacking`, `State.Skill`, `State.Stunned`, and `State.Dead`. Guard tags remain deferred. Hit reaction and death are activated from server events, not Input Actions.

## 10. External-server action contract

Implemented presentation messages:

- `C_COMBAT_ACTION` (`1016`): `action_type`, `combo_step`, monotonic `sequence`, and `facing_yaw`.
- `S_COMBAT_ACTION` (`1017`): server-derived `object_id` plus the accepted action type, combo step, sequence, and facing.
- Supported action types: `COMBAT_ACTION_KNIGHT_PRIMARY` and `COMBAT_ACTION_KNIGHT_Q`.
- The server broadcasts the accepted presentation event to other room members and excludes the sender because the sender already predicts locally.
- Two-client PIE verification on 2026-09-16 confirmed ordered A-D and Q remote playback with matching action type, combo step, sequence, and facing. No rejected combat event or combat-related Blueprint/network failure was logged.

Required authoritative extensions:

- Client request additions when simulation is implemented: optional target identifier and client input timestamp.
- Server start additions: authoritative start time and final validated facing/action state.
- Server result: hit targets, damage/block outcome, resulting HP/state, and relevant buff identifiers.
- Server state events: hit reaction, death, respawn, and later guard/buff events when those features are re-enabled.

The server derives the actor from the authenticated session and must not trust a client-supplied actor identifier. Combo step and timing are server-validated even when the client predicts the next section.

## 11. Cancel and priority baseline

Active first-pass priority: `Death > Stun > Q committed action > PrimaryAttack > Locomotion`.

- Death cancels every ability and clears all input buffers.
- A normal hit reaction may interrupt basic attacks unless a future ability grants super armor.
- Q cannot start while attacking, stunned, dead, or airborne in the first pass.
- Guard cancellation rules will be redesigned after Q is finalized. E is disabled.

## 12. Dodge status

No suitable Terra dodge, roll, evade, or dash animation was found. Dodge remains `Blocked` for content, and no unrelated animation should be substituted silently. A dedicated in-place dodge animation retargeted to `Terra_Skeleton` is preferred because the external server owns movement and invulnerability timing.

## 13. Implementation order and validation

1. Add GAS modules and the S1 input-tag bridge without changing server authority.
2. Create S1-owned montage/notify assets for A-D and implement the local combo state.
3. Combat action request/start relay and A-D/Q two-client PIE synchronization are complete. Result/damage messages remain pending.
4. Complete server-driven Q displacement and collision, then redesign guard against the finalized Q action/cancellation rules.
5. Add hit reaction, death, and later dodge after approved assets exist. E remains disabled.

Acceptance for the active scope requires both PIE clients to agree on combo step, facing, Q movement, hit result, HP, interruption, and cleanup. Logs must contain no Blueprint Runtime Error, Ensure, Network Failure, or missing-asset/class error for the flow.

## 14. Decisions still required at the tuning gate

These do not block scaffolding or the basic A-D implementation, but they are required before combat is considered final:

- exact damage/combo notify frames after visual montage review;
- Q distance, duration, steering, stop-on-hit, and multi-target policy;
- guard rules after the Q action model is finalized;
- approved dodge animation;
- portfolio usage-rights note for Paragon Terra.

## 15. PC action-control implementation status (2026-09-16)

Current Q override (user-approved 2026-10-01): Q is an in-place two-hit skill, **not a charge/displacement skill**. Movement remains allowed; only jumping is blocked during Q. Both hits use basic-attack radius 230cm from the server player's current position. Hit0 at 370ms is 360 degrees with coefficient1.0; Hit1 at1100ms is forward180 degrees with coefficient1.5, using the starting yaw. Cooldown is explicitly deferred. The contract-v2 server/Unreal implementation and its verification limits are recorded in `TerraQTwoHitImplementation.md`. Earlier charge/displacement and six-second proposal descriptions do not override this decision. AbilityId11002 remains GuardAura; existing internal Charge names are preserved for compatibility.

The native `S1MyPlayer` camera now starts at 450 cm, 80-degree FOV, and a pivot about 130 cm above the capsule base. Mouse wheel zoom is clamped to 250-700 cm; yaw/pitch sensitivity, pitch inversion, pitch limits, zoom step, camera lag/return speed, and camera height are Blueprint-editable. Spring-arm collision is enabled and rotation lag remains disabled. The camera never auto-rotates to the target.

Terra remains a **four-hit** A-D combo. One click while a step is active buffers at most one follow-up. A buffered click before the 80% branch starts the next step at that branch; a click after the branch but before the step ends starts it at the end. This avoids silently losing repeated clicks near the old narrow boundary. Holding LMB still does not auto-repeat. Each step has a movement speed multiplier and keeps facing locked until the next step; the next step reads the latest camera yaw at its start. Action timers, buffered input, movement speed, and facing lock are cleared on normal completion or explicit teardown. An external hit/death interruption hook is still pending. These are presentation rules, not server hit authorization. A fresh single-client PIE test on 2026-09-16 logged `1→2→3→4→1`, including 14 buffered inputs and three fourth-step starts, with no combat-related runtime error. This revision's two-client remote playback is not yet reverified.

Native local targeting is opt-in through an actor tag `S1.TargetableEnemy`. A 10 Hz soft scan uses camera-centre dot, distance, line of sight, and a retention bonus; Tab cycles visible candidates and Esc clears a lock. A locked target survives temporary occlusion for a tunable grace period, but drops on deletion, `S1.Dead`/`S1.Friendly`, or distance expiry. `bDrawTargetDebug` provides a temporary sphere indicator. No current S1 monster is tagged or supplies health/faction/attackability data, so target selection is compiled but not gameplay-validated, does not affect the camera or damage, and is not presented as a complete enemy system.

Still required before a full combat vertical slice: server-authoritative action accept/result, target ID and health/death protocol, approved enemy actor/AnimBP, weapon attachment/trace sockets or an approved server melee volume, hit VFX/SFX/reactions, Q displacement/collision, and approved dodge/strong-attack/cleave/guard animations. RMB, Shift, numeric skills, Alt UI cursor mode, target name/HP UI, hit shake, and strafe animation are not wired. Do not infer immunity frames or damage from client animation events. Editor/PIE checks remain pending because Unreal MCP tools were unavailable in this session.
