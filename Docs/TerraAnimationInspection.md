# Terra animation MCP inspection — 2026-09-17

## Scope and evidence

35 AnimSequence assets loaded successfully via the live Unreal MCP. All use `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra_Skeleton.Terra_Skeleton`, have Root Motion disabled, rateScale 1, bForceRootLock false and default bLoop false. Same skeleton establishes skeleton compatibility only, not validated S1 AnimGraph integration. PIE was not running. Each inspected animation reported dirty=false. No content asset was saved.

The live Slate window identifies S1; the local current S1 log records `E:\Git Fork\S1_UE5.8\S1.uproject` and the UE_5.8 build path. These are corroborating observations, not an MCP project-path API response.

## Inspected properties

Base paths below are relative to `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/`. Additive type comes from the actual property, not the filename. Full dependency/referencer values were queried through AssetTools. Empty registry referencers do not exclude native string-based loads.

| Asset | Seconds | Additive | Base sequence | Registry referencers |
|---|---:|---|---|---|
| HitReact_Front | 1.167 | AAT_LocalSpaceBase | Idle.Idle | None returned |
| HitReact_Back | 1.167 | AAT_LocalSpaceBase | Idle.Idle | None returned |
| HitReact_Left | 1.167 | AAT_LocalSpaceBase | Idle.Idle | None returned |
| HitReact_Right | 1.167 | AAT_LocalSpaceBase | Idle.Idle | None returned |
| Death | 1.200 | AAT_None | None | None returned |
| Stun_start | 1.667 | AAT_None | None | None returned |
| Stun_Idle | 12.300 | AAT_None | None | None returned |
| Knockback_front | 2.033 | AAT_None | None | None returned |
| Knockback_back | 2.033 | AAT_None | None | None returned |
| RMB_Shield_pose | 0.067 | AAT_None | None | None returned |
| RMB_Shield_pose_MSA | 0.067 | AAT_RotationOffsetMeshSpace | Idle.Idle | None returned |
| RMB_Shield_Jog_pose_MSA | 0.067 | AAT_RotationOffsetMeshSpace | Jog_Fwd.Jog_Fwd | None returned |
| RMB_HitReact | 1.500 | AAT_None | RMB_HitReact.RMB_HitReact | None returned |
| RMB_HitReact_MSA | 1.067 | AAT_RotationOffsetMeshSpace | RMB_HitReact_MSA.RMB_HitReact_MSA | None returned |
| RMB_melee_recovery | 0.467 | AAT_None | None | None returned |
| RMB_melee_recovery_MSA | 0.467 | AAT_LocalSpaceBase | Idle.Idle | None returned |
| RMB_end | 1.167 | AAT_None | Idle.Idle | None returned |
| RMB_out_MSA | 0.667 | AAT_RotationOffsetMeshSpace | Idle.Idle | None returned |
| RMB_intro | 0.833 | AAT_None | None | None returned |
| RMB_melee | 1.033 | AAT_None | None | None returned |
| Primary_Melee_A_slow_Recovery | 1.733 | AAT_None | None | None returned |
| Primary_Melee_B_slow_Recovery | 1.600 | AAT_None | None | None returned |
| Primary_Melee_C_slow_Recovery | 1.800 | AAT_None | None | None returned |
| Primary_Melee_D_slow_Recovery | 1.467 | AAT_None | None | None returned |
| Jog_Left_Combat | 0.833 | AAT_None | None | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/BlendSpaces/Jog_Combat1D |
| Jog_Right_Combat | 0.833 | AAT_None | None | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/BlendSpaces/Jog_Combat1D |
| Jog_Fwd_L_Combat | 0.833 | AAT_None | None | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/BlendSpaces/Jog_Combat1D |
| Jog_Fwd_R_Combat | 0.833 | AAT_None | None | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/BlendSpaces/Jog_Combat1D |
| Jog_Bwd_L_Combat | 0.833 | AAT_None | None | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/BlendSpaces/Jog_Combat1D |
| Jog_Bwd_R_Combat | 0.833 | AAT_None | None | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/BlendSpaces/Jog_Combat1D |
| Primary_Melee_Air | 1.167 | AAT_None | None | None returned |
| R_Intro | 1.767 | AAT_None | None | None returned |
| R_ending | 1.433 | AAT_None | None | None returned |
| Cast | 1.300 | AAT_None | None | None returned |
| Q_swing_B | 3.100 | AAT_None | None | None returned |

## Findings and limitations

- Four HitReact directions are Local Space additive, relative to Idle frame 0. They require an appropriate additive path; do not treat them as ordinary full-body sequences.
- RMB_Shield_pose is a non-additive 0.067-second pose sequence. A guard-hold source exists; authoring a new hold pose is not yet justified.
- RMB_Shield_pose_MSA is Mesh Space additive relative to Idle; RMB_Shield_Jog_pose_MSA uses Jog_Fwd. RMB_melee_recovery_MSA is Local Space additive despite its suffix.
- RMB_HitReact_MSA refers to itself at frame 0 as its additive base. Non-MSA RMB_HitReact also carries a self reference but is AAT_None. Neither observation alone proves corruption; evaluate the additive variant before choosing it.
- RMB_end is a non-additive 1.167-second exit candidate. Visual transition suitability remains pending.
- Death and Stun_start/Stun_Idle exist as non-additive sequences. Stun_Idle length does not define server stun duration; runtime looping must be selected deliberately.
- All six inspected combat jogs have the source Jog_Combat1D as a registry referencer. This does not prove that S1 uses that blendspace at runtime.
- RMB_Shield_pose was opened and visually sampled in Persona: shield held in front. Preview displayed LOD 1 with severe weapon-region deformation, resembling the documented lower-LOD issue. LOD0 visual verification remains pending; this is not a clean visual acceptance.
- No complete frame-by-frame motion review, additive composition, transition/slot validation in ABP_S1Terra, Blueprint compilation, or PIE gameplay validation was performed. Candidate status remains unchanged.
- No new gameplay, asset purchase, migration, or content integration is approved by this report.

