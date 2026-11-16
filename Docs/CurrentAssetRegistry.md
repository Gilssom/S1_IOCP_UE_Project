# S1 Current Asset Registry

2026-11-15 `/Game/RPGEnvironmentVFX/VFX/Niagara/NS_TeleportZone` — Candidate:
user selected it for a portal. MCP identified NiagaraSystem, clean package; direct
material dependencies include MI_Mist and MI_Particles, with one demo-map referencer.
Local SHA256 5E40D18F94398A5D4EF4F3B0F4AC1F9F303A756CB0EF98B48D2F2E830F21FF16.
Source/license for public portfolio use is not yet recorded. `/Game/S1/World/BP_S1Portal`
is a saved Blueprint derived from `/Script/S1.S1Portal`, with this Niagara system in
its inherited PortalEffect component and a Pawn-overlap TriggerVolume. Candidate until
the user checks visual presentation and a real packaged build. This scaffold has no
overlap callback, fade, server request or map travel. No original Niagara asset was edited.

2026-10-21 VillageMap initial-entry integration (user PIE accepted; package pending):
User renamed the saved staging map to `/Game/Maps/VillageMap`; MainMap_Test now remains
a redirector. Approved region contract1 sends initial/reconnected logins via existing
LoadingMap into server VillageRoom2 (no Dusk). Combat MainMap1/vendor map unchanged.
Saved PlayerStart(6840,1880,192),Yaw-180 and NPC1(5830,1760,100) exported/deployed with
separate Village collision data. No map/Blueprint binaries saved in this integration;
VillageMap matches its verified checkpoint. Existing village walking/visual acceptance
does not certify the new server login flow. Editor/Server/Game builds and offline tests
passed. User subsequently accepted the full latest1-/2-player PIE village flow,
NPC/inventory/potion/fielddrop and reconnect; fixed-entry reconnect is intentional.
After Editor closure, new Windows Development Build/Cook/Stage/Package/Archive succeeded
(ExitCode0, cook1597 packages,0 errors/0 warnings); all four maps are in the cooked registry.
Two fresh archived clients reached LobbyMap. Authenticated two-account packaged gameplay
acceptance is still pending; first LiveCoding-blocked attempt is retained separately.
User then completed the package gameplay checklist but reported missing Main HUD.
Actual client logs/registry confirmed MainHUD/QuestTracker absent, with Minimap/Currency/
QuestJournal also not cooked. Explicit native hard references now cover MainHUD/Tracker/
Journal; Blueprint design and all content binaries are unchanged. The HUD-fix package
Build/Cook1605/Stage/Package/Archive passed with0 errors/0 warnings and all five WBP names
in the cooked registry. Automation40 passed (one pre-existing GameplayCue warning).
User accepted the latest two HUD-fix package clients' HUD/quest window/dialogue hide-restore
visual check. Logs confirm both Lobby→Loading→Village entries and no WBP-load/Fatal/Ensure/
BlueprintRuntime/NetworkFailure matches. Client1 had three explained pre-entry invalid-login
rejections, then entered normally; do not report the entire log as Error0.
See VillageEntryContract_2026-10-21.md for files, evidence, limitations and partial backup.

2026-10-21 MainMap_Test editor navigation integrated (Candidate; not server deployed):
enabled navigation, added NavMeshBounds_S1Village over landscape bounds, generated Static
Recast35radius/144height/44slope. First saved-data check found zero polygons; explicit
Build Paths and selective map resave fixed this. Independent commandlet read3297polys/
7406edges, exit0. Diagnostic only in Saved/Diagnostics, not serverData/deployed.
Node heights110..1170 are NOT compatible with existing flatZ-0.5 server contract.
Before/after map checkpoint verified; old MainMap/vendor map unchanged, existing
ABP_S1Terra dirty state preserved/unsaved. No new PIE/chase/package acceptance.
See MainMapTestNavigation_2026-10-21.md for exact settings, evidence and remaining scope.

2026-10-21 MainMap_Test inspection (Candidate, user-authored staging map):
/Game/Maps/MainMap_Test created/saved by user. MCP confirms S1MainGameMode override,
NPC1(ID1/quest DT/nonpreview), spawn marker1(ID1/type1/enabled), PlayerStart1.
Map clean; old MainMap and vendor map hashes unchanged. Navigation disabled/no bounds;
spawn feetZ100 violates current exporter flatZ-0.5 contract, so not server-deploy-ready.
User's latest2 PIE worlds started/stopped, but both logged ABP_S1Terra divide-by-zero;
no server-entry evidence in that interval. No new agent-run PIE or asset changes.
See MainMapVillagePreflight_2026-10-21.md latest inspection; not Active runtime acceptance.

2026-10-21 Elderboom MainMap preflight (Candidate; no replacement):
MCP connected to S1 with L_ElderboomVillage open, PIE=false; both map packages clean
before/after read-only checks. defaultGameMode=None, navigation disabled, no Nav bounds,
no S1 NPC/spawn markers; PlayerStart(6840,1880,192) and nonflat Landscape present.
No LevelScriptActor found; graph inspection is not certified. Direct dependency/referencer
queries completed. Existing MainMap/server flat-ground data cannot be reused unchanged.
25-file hash-verified partial checkpoint retained, no asset saved/map replaced/server data deployed.
See MainMapVillagePreflight_2026-10-21.md for backup location, findings and pending checks.

2026-10-21 Elderboom village environment intake (Candidate; migrated, not S1 runtime verified):
user migrated /Game/ElderboomVillage/Maps/L_ElderboomVillage and dependencies from
E:/UnrealEngine/ElderBoomHollowMassiveMed (EngineAssociation5.8, logs5.8.2).
S1 Content/ElderboomVillage contains410 files/about6.37GiB; source has411 files,
with only Maps/L_ShowCase.umap absent from destination (separate showcase not selected).
Source AssetTools log records successful migration to S1. Packages were rewritten during
migration; file sizes/map SHA differ, so no byte-identical-copy claim is made.
Source .uproject has no enabled explicitly listed plugin (SkeletalMeshModelingTools disabled);
map-level dependency/parent/dirty checks remain pending because MCP8000 refused connection.
User reported source project appears normal in5.8. Source logs show a stale missing
BP_FirstPersonGameMode config reference and Nanite foliage WPO/raytracing warning;
do not migrate source Config or use its GameInstance/GameMode in S1.
S1 Config has no tracked diff; existing MainMap/default maps/server unchanged by this check.
Provenance: Fab ElderBoom Hollow Massive Medieval Village Environment, listing
https://www.fab.com/listings/e12de9d5-be28-40df-a387-42ae6f84e05c (karaman).
License entitlement/portfolio-use terms not yet recorded; confirm before public release.
No UE5_RPG assets copied. Imported assets remain excluded from source-only Git.
Next: open migrated map in S1, inspect dependencies/World Settings/Level Blueprint/dirty state,
and validate visuals/collision before configuring a separate VillageMap or server travel.

2026-10-21 Monster Data Table user acceptance:
user reported the full latest 1-/2-client PIE checklist complete, including multi-Dusk
spawn/combat/navigation/return/reengagement/recovery/death/respawn, potion/quest and
reconnect regression. DT_S1MonsterDefinitions and DT_S1MonsterPresentations are
Active for this user-accepted PIE flow. No new agent-run gameplay/log audit is implied;
latest packaged two-account gameplay remains unverified. See MonsterDataTables_2026-10-20.md.

2026-10-20 Monster Data Table integration (history; PIE acceptance superseded above):
new /Game/S1/Monsters/Data/DT_S1MonsterDefinitions (S1MonsterDefinitionRow) and
DT_S1MonsterPresentations (S1MonsterPresentationRow), Dusk row ID1, authored from
existing server values without balance changes. Presentation class references existing
BP_S1DuskMinion; scale0.7/yaw+90/capsule28x62/flat center61.5 unchanged.
Only two new tables saved after dirty report/checkpoint; existing BP/MainMap clean.
WorldEntity now resolves ID via tables, rejects unknown/missing classes without Dusk
fallback; server loads validated offline export before DB/network and seals it.
No new assets from UE5_RPG/vendor, protocol/DB change, or new AI/loot rules.
See MonsterDataTables_2026-10-20.md for tuning/export/deploy/restart and validation.
Final Game/Editor/Debug server builds,92 parser checks,43 actual Room offline checks,
38 Unreal checks passed. Fixed a discovered native soft-path Cook omission with
explicit hard table CDO roots; final Windows package and IoStore inclusion plus
packaged two-table/Dusk-class load smoke passed. Server/Editor/MCP restarted;
new-revision1-/2-client PIE/economy/quest/reconnect and packaged gameplay remain pending.

2026-10-20 latest Main HUD user acceptance:
user reported all requested checks complete for the current HUD expansion.
WBP_S1MainHUD, WBP_S1Minimap, WBP_S1Currency and the generated
T_S1BasicAttackIcon/T_S1ChainSlashIcon are **Active for user-accepted PIE presentation**.
This supersedes the visual-acceptance-pending status in the historical entries below.
Exact client count was not specified; no agent-run gameplay/log audit or current
packaged acceptance is implied. Minimap remains a configurable compass/map UI
without terrain texture; currency0 is a marked placeholder and cooldown is unimplemented.
Documentation-only acceptance update; see MainHUDExpansion_2026-10-20.md.

2026-10-20 Main HUD expansion (editor-integrated; PIE visual acceptance pending):
generated original S1_BasicAttack_Icon_v1.png / S1_ChainSlash_Icon_v1.png are
Candidate integrated source art in ArtSource/UI/MainHUD. Built-in image_gen provenance
and exact prompts: SkillIcons_README.md. New WBP_S1Minimap (S1MinimapWidget),
WBP_S1Currency (UserWidget), T_S1BasicAttackIcon/T_S1ChainSlashIcon are created,
selectively saved and referenced by WBP_S1MainHUD. All three WBP compile checks
pass with warnings-as-errors. New textures use UI/no-mips settings. Actual entry
level connected; currency0 is explicitly temporary; map terrain texture absent,
so this is a configurable map UI/compass, not a verified terrain minimap.
See MainHUDExpansion_2026-10-20.md for build/automation evidence and pending PIE.
Final S1/S1Editor Development builds and37 automation checks passed (36clean plus
one existing GameplayCue warning); fresh-editor MCP reload/clean confirmed.
This is not1-/2-client PIE or packaged visual acceptance.

2026-10-20 Main HUD refinement (integration, visual acceptance pending):
WBP_S1MainHUD now has aligned60×60 LMB/Q +1–4 square cells, numeric-only
consumable stock, hidden future skill Icon/CooldownShade/CooldownText variables,
and rounded metallic-outline HP rail with diamond endcaps inside the existing
generated backplate. No invented cooldown/MP/EXP and no new art import.
Serial MCP compile/warnings-as-errors passed; only MainHUD saved after checkpoint
and dirty report. Original QuickSlots and MainMap clean. See
MainHUDImplementation_2026-10-20.md for exact tuning names/evidence and pending PIE.

2026-10-19 Quest B core two-client PIE user acceptance:
user confirmed contributor progress increases on both clients, progress retained
for both after reconnect, goal3 achieved display, and movement/attack/camera restore
after dialogue. WBP_S1QuestTracker is now **Active for the user-accepted PIE quest
progress presentation**, not packaged-certified. This record does not certify
decline/rejection/delayed-response exceptions or reward delivery. Documentation-only
acceptance update; no new code/asset/DB changes or agent-run log audit.

2026-10-19 Quest B implementation/deployment history (acceptance superseded above):
new /Game/S1/UI/WBP_S1QuestTracker (parent S1QuestTrackerWidget, CanvasRoot/QuestText
native binding, right-top320x160, MaruBuri Regular18 gold, non-interactive) is
integration-pending visual acceptance, not yet gameplay-certified Active. Existing
DT2 now enabled quest1/Dusk3 and real acceptance dialogue; NPC BP preview off.
Four packages selectively saved after checkpoint/dirty report, compile clean;
MainMap and WBP_S1Dialogue not saved. v2 trusted placement export validated/deployed.
Isolated DB62 + runtime14 + definition29 passed; server/editor/game builds passed;
Unreal32 passed (31clean+1existing GameplayCue warning). GameDB3 new tables verified,
server readiness1. No reward grant or new external art. See
QuestStageBImplementation_2026-10-19.md for evidence and pending PIE/package gates.
This supersedes the approval-pending statement in the historical paragraph below.

2026-10-19 latest user acceptance: G-next revision confirmed by user; unspecified
client count, not packaged acceptance. B-stage content direction changed from
last-hit to server-confirmed damage contributors. QuestStageBContract_2026-10-19.md
is a source/MCP-backed proposal pending protocol/DB approval, not implemented.
This update changes docs only; DT sample remains disabled and no assets saved.

2026-10-19 dialogue authoring stage1 user acceptance: user reported ordinary-next,
third-node offer, both preview/decline branches and close/Esc control/camera restore
tested without issues. Do not infer unspecified client count or packaged coverage.
Follow-up G-next input implemented; native-only change, no WBP/DT saves.
Line nodes bind a separate next delegate; preview G consumes fresh keydown only,
ignores hold repeats and never triggers Offer/End acceptance. Existing next button
label now includes [G]. Editor/game builds passed;31 automated tests passed
(30clean +1existing GameplayCue warning), including direct preview-key dispatch,
repeat suppression, acceptance isolation, Esc and cleanup. Live MCP/WBP compile
clean. Actual G routing/focus/hold in authenticated PIE pending.
Prior acceptance is not acceptance of the new G input revision. See
QuestDialogueAuthoringStage1.md for exact scope and subsequent evidence.

2026-10-19 Quest/dialogue authoring stage1 integration:
/Game/S1/Quest/Data/DT_S1QuestDefinitions (S1QuestDefinitionRow),
/Game/S1/Quest/Data/DT_S1DialogueNodes (S1DialogueNodeRow). One disabled sample
quest and five nodes: Line1→Line2→Offer3→preview accept4/decline5. Existing NPC BP
hard-references tables, StartDialogueNodeId1, PreviewQuestBranches=true explicitly
test-only. No actual quest acceptance/progress/reward/DB/packet activated. Existing
dialogue WBP/style retained; only DT2+NPC BP saved after dirty report/checkpoint.
Editor/game/Debug server builds, affected BP compile and Unreal30-test run passed
(one existing GameplayCue warning). Actual editor Export and server22 parser cases
passed. Runtime UI/packaged visual acceptance pending. Reference chain and sample
use verified, not a production quest feature. See QuestDialogueAuthoringStage1.md.

2026-10-16 MaruBuri multi-weight Font fix supersedes earlier diagnosis-only state:
/Game/S1/UI/Fonts/MaruBuri_Font (Font) now has ExtraLight/Light/Regular/SemiBold/Bold
entries linked to matching project FontFaces; Default aliases Light to preserve
styling. Four WBP fontObject references replaced (12TextBlocks): Dialogue,
NpcInteractPrompt, NpcNameplate, InventoryDiscardDialog. Original engine assets
untouched. Actual style-to-face links verified in Font editor via serial MCP;
asset dependency graph includes five project faces and four WBP referencers.
Four BP warnings-as-errors compiles and 29 automated tests passed (one pre-existing
Q GameplayCue warning). Before/after binary checkpoint MaruBuriFont_20261016;
only ten intended packages saved after dirty reports. Integration verified;
multi-weight rendered and packaged visual acceptance pending. No C++/DB/server
changes. See MaruBuriFontIntegration.md for provenance/evidence and editing steps.

2026-10-16 NPC name distance follow-up supersedes initial always-visible policy:
ShowDistance1500cm/HideDistance1800cm, previous-state hysteresis, per-client local
pawn 3D root distance checked by NPC actor every0.1s; no pawn hides/reset. Native
settings exposed on BP NpcNameplate component. UI styling and binary assets remain
unchanged; no engine/font/server/DB edits. New 29-test run (28clean +1pre-existing
GameplayCue warning,0failures) includes enter/exit/reapproach/no-viewer assertions.
See NpcNameplateWidget.md for separate current build/runtime evidence.

MaruBuri read-only diagnosis: current NameText references EngineFonts/
MARUBURI-LIGHT_Font (Font, style Default), dependent only on Light FontFace.
Regular/Bold/SemiBold/ExtraLight exist as FontFace, not styles linked into this Font.
All five OTF/TTF weights exist in user Downloads/maruburi.zip. Current Light is
user-applied; other faces are unconnected candidates. No fonts changed/imported.
Recommend project-owned composite Font with all styles, rather than engine edits.

2026-10-16 NPC overhead name integration (manual visual acceptance pending):
/Game/S1/UI/WBP_S1NpcNameplate parent /Script/S1.S1NpcNameplateWidget,
generated WBP_S1NpcNameplate_C, SizeBox300x36 → Canvas → bound NameText.
BP_S1QuestNpc.NpcNameplate hard-refers to the new WBP; component caches existing
DisplayName and supplies it on initialization/recreation. Screen/height250cm/
nonfocusable/HitTestInvisible; always-on passive name, no distance/occlusion policy.
Separate G prompt and dialogue Designer remain unchanged. Only name WBP and NPC BP
selectively saved after dirty report/checkpoint; MainMap not saved. S1Editor/S1
Development builds, 29 automation tests (one pre-existing Q warning, zero failures),
affected BP warnings-as-errors compile, MainMap reload and short Simulate PIE
runtime component connection passed. Actual name text/update/recreation tested
against authored WBP in automation. Status integration verified; visual acceptance
and packaged runtime pending. Before/after checkpoint NpcNameplate_20261016;
see NpcNameplateWidget.md. No server/DB/protocol/new external art changes.

2026-10-16 NPC interaction prompt WBP wired (visual acceptance pending):
/Game/S1/UI/WBP_S1NpcInteractPrompt parent /Script/UMG.UserWidget,6widgets,
SizeBox200x48 + Canvas for free positioning, KeyText/InteractionText static labels.
BP_S1QuestNpc inherited DialoguePrompt.WidgetClass now hard-refers to this WBP.
Height210/Screen/WindowFocusablefalse unchanged. Only new WBP+existing NPC BP saved,
both compile warnings-as-errors passed; MainMap/dialogue WBP not saved and clean.
Map reload and short Simulate PIE confirm runtime widget class. Native prompt
remains fallback only for native NPC instances. No C++/server/DB/input changes.
Dependencies BP->newWBP verified; actual G approach/rendered acceptance and package
pending. Before/after checkpoint NpcInteractPrompt_20261016; see NpcInteractPromptWidget.md.

2026-10-16 DialogueFantasy_v1 Candidate (external, not imported/applied):
UIResources/DialogueFantasy_v1 contains DialoguePanel_v1(2171x724),
NpcNameplate_v1(2172x724), QuestDetailPanel_v1(1086x1448),
DialogueButton_Accept_v1/Decline_v1(each2172x724), all PNGs corner alpha0,
center253 sampled; original pixels preserved. Built-in imagegen skill used
InventoryFantasy_v1 panel/button as style references: midnight navy/silver/gold.
Five separate text-free resources; live text/binding names retained in application
instructions. README/Prompts record final prompts, native WBP mapping and unverified
9-slice starting values. No font/art import, WBP/Content save or new UI play test.
User separately confirmed gradient dialogue screen visual acceptance before this
art task; this does not accept the unapplied texture designs in actual gameplay.

2026-10-16 user visual acceptance: dialogue from behind NPC frames its front,
NPC does not rotate on other client's screen, and remote players restore on exit.
Follow-up approved presentation change: remove native constrained-aspect opaque
letterbox, replace with native Slate gradients behind WBP_S1Dialogue tree.
Default edge alpha0.5→0 inward, top/bottom8% height, editable WBP Class Defaults
Dialogue/Edge Shade. No texture/material/asset save; new gradient visual acceptance
pending. See NpcDialogueStageAImplementation.md for separate evidence boundaries.

2026-10-16 NPC dialogue presentation follow-up: approved local-only NPC mesh
facing toward interacting player, player-side shoulder camera, and remote-player/
attached-actor camera mask with separate screen-space HP suppression. Original
NPC mesh rotation and only this dialogue's render masks are restored on exit/
abort. No NPC/player root transform, server protocol/DB, BP/WBP/map save changed.
Editor build and28automation passed (dialogue test0warnings; existingQwarning1).
Current two-client visual acceptance/package still pending; see
NpcDialogueStageAImplementation.md. Model/Idle BP replacement contract unchanged.

2026-10-16 NPC dialogue Stage A implementation (play acceptance pending):
/Game/S1/Quest/BP_S1QuestNpc parent S1QuestNpc, placed once in MainMap at
(-350,250,-0.5), native swappable SKM_Manny/MM_Idle defaults (same skeleton).
/Game/S1/UI/WBP_S1Dialogue parent S1DialogueWidget,16widgets/6bindings, lower
dialogue and right detail. Accept disabled: no quest/server/DB integration yet.
Both BP compile warnings-as-errors passed; only these2 assets+MainMap saved,
dirty=false; existing player/Dusk BPs not saved. Map reload preserved actor and
WBP reference. MainMap->NPC BP->WBP cooker reachability inspected, actual cook
and real dialogue play acceptance pending. Editor/game builds and28automation
tests passed (one existing GameplayCue warning). Source-only Git excludes these
assets; ContentBefore/After checkpoint NpcDialogueA_20261016 retained.
No vendor edit or new imported font/art. See NpcDialogueStageAImplementation.md.

2026-10-16 NPC dialogue candidate investigation (not implemented/not Active):
/Game/Characters/Mannequins/Meshes/SKM_Manny and
/Game/Characters/Mannequins/Animations/Manny/MM_Idle share SK_Mannequin;
/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra and Animations/Idle
share Terra_Skeleton. All four assets loaded via serial read-only MCP, dirty=false.
Manny materials MI_Manny_01/02 and dependency/referencer lists inspected.
Manny is proposed as temporary NPC; Terra already serves player presentation.
No NPC chosen/placed, vendor edit/migration/save or font import performed.
Runtime NPC pose/floor/framing/cook not verified. Existing Rair_Potion icon may
be reused. See NpcQuestImplementationPlan_2026-10-16.md for approval gates.

2026-10-15 packaged Dusk discrepancy: live MCP clean BP_S1DuskMinion currently
has MeshFacingYawOffset=-90,visualscale0.7; prior+90 documentation is historical,
not current saved default. Packaged clients missed BP and used native+90 fallback,
causing180degree visual reversal. WorldEntity hard CDO/cook reference fix built;
new Windows Development package smoke loads the BP CDO and -90 yaw successfully.
No Blueprint/mesh/yaw value changed. User confirmed corrected packaged two-client
Dusk movement and attack facing on 2026-10-15. Broader combat/item/reconnect
integration acceptance remains separate.

2026-10-14 keycap art Candidate (external, not imported/not Active):
UIResources/KeycapFantasy_v1/Keycap_Blank_v1.png,1254×1254 RGBA,corner alpha0/
center253 verified. Built-in image_gen original blank square navy/silver/gold
keycap using own InventoryFantasy_v1/Button_Frame_v1.png as style reference.
No baked text: separately overlay existing KeyText. README records full prompt,
UI texture/import settings,44-48px starting size and optional9slice suggestion.
No WBP/Content import/runtime configuration changed; small-size/DPI/PIE fit
not verified. Suitable candidate for new WBP_S1LootPickupPrompt KeyBadge art.

2026-10-14 pickup prompt Active (user-reported play acceptance):
/Game/S1/UI/WBP_S1LootPickupPrompt, parentS1LootPickupWidget,8widgets/3bindings,
new native UMG design, no imported art. MCP warnings-as-errors compile passed;
only this new asset saved and checkpoint hash verified. Existing UI/maps/Terra
were clean and not saved. Screen WidgetComponent replaces old TextRender hint;
private/public candidate/pending/server authority policies unchanged. Designer
owns fonts/colors/layout; Potion Presentation Pickup UI exposes height/screen
offset. Final Editor/game builds,26automatic tests and Windows package passed;
archived exe loads new generated class (known Auth focusError1 separately).
Fresh GUI MCP readback same parent/8widgets/3bindings/clean; asset editor opened.
User subsequently reported this UI testing complete; agent-controlled PIE and
packaged2client gameplay not claimed. See LootPickupUIImplementation.md.

2026-10-14 player-public field-drop source integration (pending new runtime
validation): reuses WBP_S1InventoryDiscardDialog and existing potion mesh/Niagara
without new art. Native confirmation warns anyone can pickup /30s expiry;
quantity is passed to world presentation. Monster drops remain owner-only.
MCP readback: parentS1InventoryDiscardWidget,13widgets/5inherited, dirtyfalse.
No asset saved in this feature yet. Game/Editor/server final builds and25automatic
tests passed (known Q warning1); GameDB3table migration applied, server supported1.
Windows package/archived class-load smoke passed (known Auth focusError1).
Live MCP reconnected; affected BP warnings-as-errors compile passed, clean
before/after (14widgets/7inherited in latest readback). No save performed.
Rendered1-/2PIE remains pending.
Old permanent-discard acceptance below is historical,
not new field-drop validation. See PlayerFieldDropImplementation.md.

2026-10-14 overhead name/HP art Candidate (external, not imported/not Active):
UIResources/OverheadFantasy_v1 Nameplate_v1.png / HP_Track_v1.png / HP_Fill_v1.png,
all2172x724, built-in image_gen original navy/silver/gold style referencing own
inventory button. Corner alpha0/center253 verified; large transparent vertical
padding remains (crop-only image_gen edit failed to remove it; original selected).
README records manual alpha-preserving crop need, binding-safe placement/tint
guide, prompts and limitations. Read-only MCP WBP_S1EntityHealth parent correct,
8widgets/dirtyfalse; NameText/HealthBar/HealthText intact. No binary/code/server/
visibility policy changes, Content import or rendered acceptance claimed.

2026-10-14 permanent discard: /Game/S1/UI/WBP_S1InventoryDiscardDialog is created
and native-wired, Active after user-reported requested PIE acceptance. Parent
/Script/S1.S1InventoryDiscardWidget,14widgets/5bindings, warnings-as-errors compile
passed and only this asset saved clean. Native UMG/no new imported art.
Existing grid/split/quickslot user designs remain SHA-identical to this task's
checkpoint. Latest Editor build and25automatic tests passed (discard warnings0/
errors0, existing Qwarning1). Windows package passed; actual cook registry and
archived exe class-load smoke confirm inclusion (existing Auth focusError1).
Server final Debug link passed and new server/editor launched. User confirmed
the requested1-/2-client PIE checklist: outside-drop partial/full deletion,
single-item confirmation, No/Esc/I cancellation, reconnect persistence and
quickslot/move/swap/merge/split regression. This is user-reported, not inferred
from startup; packaged gameplay and injected-failure checks remain unverified.
See InventoryDiscardImplementation.md for evidence and remaining gates.

2026-10-14 split UI: /Game/S1/UI/WBP_S1InventorySplitDialog (parentS1InventorySplitWidget)
new runtime-wired editable quantity modal,14widgets/4bindings. Initial soft native
class reference omitted it from cooked registry despite packaging exit0. Replaced
with hard native class reference after asset creation; game/editor builds and final
24automatic tests/package passed. Actual cook registry and archived exe generated-class
load confirm inclusion; headless smoke has existing Auth focusError1. Compile/warnings-as-errors
and selective save clean; actual saved-class
automation passed. Existing grid/slot/quick-slot binaries SHA unchanged. Supports
approved Shift split / Ctrl swap / normalmerge via IOCP;20slots/99cap unchanged.
User confirmed requested PIE split/quantity/cancel, swap, merge, reconnect persistence
and existing quick-slot/potion use on2026-10-14. Client count unspecified; packaged
gameplay and injected-failure acceptance remain pending. InventoryStackOperationsImplementation.md
records code,checkpoint,provenance(nativeUMG/no new imported art) and evidence.

2026-10-12 external inventory-title art candidate (not imported / not Active):
InventoryTitle_KO_v1.png2132x738 in UIResources/InventoryFantasy_v1, original built-in
image_gen Korean lettering '인벤토리' with silver/gold ornament, outer alpha0 verified.
Title_README.md stores prompt and placement notes. Text-bearing image must retain
aspect ratio, not Box9-slice. No WBP/code edits or runtime acceptance claimed;
small-size legibility and alpha-edge color fringe require visual review.

2026-10-12 external button art candidate (not imported / not Active): original
Button_Frame_v1.png2172x724 saved in UIResources/InventoryFantasy_v1; generated and
reframed with built-in image_gen. Transparent corners verified. Blank navy/silver
frame designed for Box9-slice; Button_README.md records prompts and initial
ImageSize/Margins/state-tint settings. No imported texture, WBP/code change or
size-by-size PIE compatibility validation claimed; extreme-small sizes need tuning.

2026-10-12 external UI art candidate (not imported / not Active): generated original
InventoryPanel_v1.png and InventorySlot_Default_v1.png, stored in workspace
UIResources/InventoryFantasy_v1 with prompts/UMG placement notes in README.md.
Midnight navy/silver/cyan fantasy styling, transparent outer pixels verified.
Generated by built-in image_gen, no extracted Aion2 assets. Inspected grid420width
and cell42square; candidate references existing InventoryGrid/InventorySlot only.
No Unreal texture paths exist yet; no WBP binary/code changed or runtime validation
claimed. Final import/style acceptance and downsizing/input regression are pending.

2026-10-12 inventory-slot movement follow-up: existing WBP_S1InventorySlot gains
native empty-cell drop routing (no asset design/binary modification). Server-backed
slots1-20 only; swap/discard/real80capacity deferred. Client builds,23automatic tests,
isolated DB CAS/persistence/use-reservation checks and Windows packaging passed.
New server Debug build/link succeeded after debugging shutdown; new server/editor
started and MCP/DB readiness confirmed. User reported all requested PIE checks
complete: empty-slot move, reopen/reconnect position/count persistence,
locked/occupied rejection and existing quick-slot/potion controls. Client count
unspecified; packaged play and fault-injection acceptance not inferred.
See InventorySlotMoveImplementation.md. Latest package includes the prior quick-slot
RootCanvas correction; its packaged mouse interaction has not been manually tested.

2026-10-12 quick-slot follow-up: WBP_S1QuickSlots RootCanvas visibility corrected
from HitTestInvisible to SelfHitTestInvisible after user PIE right-click failure.
Design geometry unchanged; backed up, Blueprint compile and selected save/readback
passed. User confirmed corrected PIE right-click unassignment and drag registration
on 2026-10-12. Updated package validation remains pending.

## Inventory grid UI stage — 2026-10-12

New WBP_S1InventoryGrid (S1InventoryWidget) and WBP_S1InventorySlot
(S1InventorySlotWidget) are Active runtime wiring; user confirmed quick-slot drag
registration and right-click unassignment. Remaining UI checks not inferred.
80visible cells, but real server capacity20 unchanged and future cells
locked. Main exact native class ref and cell CDO ref verified, compile/selective
save clean, actual grid/drag automatic test passed. WBP_S1QuickSlots original design
preserved; native drag registration/instant right-clear added. Old WBP_S1Inventory
and WBP_S1InventoryStackRow are retained legacy/test assets, no longer active window;
historical Active descriptions below refer to earlier revision. No imported packs,
new icon or license change. No slot move/discard/server/DB/proto in this stage;
permanent discard is deferred user-selected policy. See InventoryGridAndDrag.md.

Latest grid stage game/editor builds,22automatic tests and Windows packaging pass;
cooked registry+actual packaged generated-class smoke confirm new grid/slot present.
Known Auth focus error remains. Corrected PIE drag/right-click user acceptance passed;
no latest packaged2-account gameplay claimed.

## Entity overhead HP widget — 2026-10-08

`/Game/S1/UI/WBP_S1EntityHealth` (WidgetBlueprint / S1EntityHealthWidget): Active
runtime wiring for both player classes and individual Dusk EntityHealth components;
user visual acceptance pending. New6-widget/3-bindings design, exact reflected soft
class references read back from real BP CDOs, compile/selective-save clean, real
WBP automatic test passed. Only this new asset saved; vendor/maps/existing BP and
inventory/quick-slot assets untouched by Codex. Source-only Git excludes binary;
initial saved copy/hash in EntityHealth_20261008 checkpoint. No new pack/license
needed. Shared style, screen-space no wall occlusion/distance culling; see
EntityHealthWidgetBlueprint.md for editing, build/21-test evidence and pending PIE.

Initial BuildCookRun omitted HP WBP; the subsequent hard-class-reference fix was
rebuilt/recooked successfully, latest cook registry includes HP+3maps and actual
archived client loaded its generated class1object/exit0. Existing Auth focus error
remains. Follow-up21automatic tests pass. User's initial bar-visible report applies
to the old always-visible version; new observer-local combat3s Dusk/remote-player
hiding revision awaits user PIE acceptance. No commit/push for this task.

## Inventory designer assets — 2026-10-07

Final validation update: after editor shutdown, designer-neutral quick-slot test adjustment was built and the full20-test suite passed (existing Q GameplayCue warning1). Latest game/editor build, Windows package and packaged inventory/row class-load smoke passed. User quick-slot binary unchanged. Final editor restarted with MCP autostart disabled; a new live dirty readback was unavailable until the user starts the server. Actual inventory1-/2-client visual/use acceptance remains pending.

| Asset | Class / parent | Role / state | Evidence |
|---|---|---|---|
| `/Game/S1/UI/WBP_S1Inventory` | WidgetBlueprint / S1InventoryWidget | Active runtime wiring; inventory window, user visual/PIE acceptance pending | Native subsystem exact class ref,37 widgets/16 bindings, compile/readback, actual intent/model test, cook/package/generated-class load |
| `/Game/S1/UI/WBP_S1InventoryStackRow` | WidgetBlueprint / S1InventoryStackWidget | Active runtime wiring; selectable stack row | Main StackEntryClass ref,4 widgets/2 bindings, native click/selection test and packaged class load |

Project-created layouts reusing the existing potion icon/engine font. Only these two new binaries were authored/saved. Existing user-edited quick-slot layout was preserved. Its older test's fixed QuickSlotBar/anchor assumption failed and was adjusted in test source only; full rerun was pending editor shutdown when this record was written. See InventoryWidgetBlueprint.md for stage-by-stage validation and editable names/types. No server/DB/proto/map/config/vendor migration changes.

## Quick-slot designer asset — 2026-10-07

Latest acceptance: the user reported testing complete for the Widget Blueprint conversion. Record user acceptance of this revision; unspecified per-client/reconnect/packaged scenarios are not independently promoted to passed. See QuickSlotsWidgetBlueprint.md for the acceptance scope; earlier pending-acceptance entries below are historical.

| Asset | Class / parent | Role / state | Evidence |
|---|---|---|---|
| `/Game/S1/UI/WBP_S1QuickSlots` | WidgetBlueprint / `/Script/S1.S1QuickSlotWidget` | Active runtime wiring; owning-player passive HUD layout. Visual/PIE acceptance pending. | Exact InventorySubsystem native CDO class ref, 30-widget designer tree/12 bindings, compile/parent readback, actual UMG instance data/style test, Windows cook/package and packaged generated-class load. |

Project-created layout; no vendor content copied/modified. Existing `/Game/_Resources/Rair_Potion` icon and engine font reused. Widget geometry/background/font/spacing are designer-editable; Icon1–4/Count1–4/State1–4 names/types are the native data-binding contract. Only this new binary was created/saved. Inventory remains native Slate. Source changes, selective-save/checkpoint evidence, test boundaries and user editing instructions are in QuickSlotsWidgetBlueprint.md. Earlier quick-slot native-only/no-new-binary notes describe the previous revision.

2026-10-07 usage update: native quick-slot HUD reuses the existing S1PotionPresentationSettings.Icon and its established native cook reference. No imported/new/binary content asset, Blueprint, input mapping asset or map was modified/saved. Quick-slot implementation/build evidence and outstanding visual acceptance are tracked in QuickSlotsImplementation.md; previous potion acceptance does not imply quick-slot acceptance.

## Purpose

This file is the canonical inventory of content assets used by the active S1 gameplay and portfolio flows. It prevents candidate/reference content from being mistaken for production content and provides a stable handoff between manual content work and Codex implementation.

Update this document whenever an asset is added, replaced, wired into gameplay, retired, or found to have a material dependency problem.

## Status definitions

| Status | Meaning |
|---|---|
| `Active` | Referenced by the current project and verified for its stated runtime role |
| `Candidate` | Present in S1 but not yet approved or wired into the runtime flow |
| `Planned` | Required by an approved feature but not yet added |
| `Blocked` | Present or planned, but cannot be used until a dependency, compatibility, license, or design issue is resolved |
| `Deprecated` | Must no longer be referenced; retained here only as history |

## Intake information

When the user adds an asset, capture as much of the following as is known:

- Display name and exact `/Game/...` content path
- Source pack/project and license or usage-rights note
- Intended role: class, monster, weapon, animation, VFX, SFX, icon, UI, environment, or data
- Asset class, skeleton, parent class, and important dependencies
- Whether retargeting, sockets, collision, materials, Niagara bindings, or Blueprint wiring are still needed
- Runtime referencers, dirty state, verification performed, and verification date

An asset must remain `Candidate` until its real use is verified. Filename similarity alone is not sufficient evidence.

## Active maps and flow assets

| Asset path | Type | Runtime role | Status | Verified evidence |
|---|---|---|---|---|
| `/Game/Maps/LobbyMap` | World | Editor/game startup, authentication and character selection | `Active` | Required project map and level-flow baseline |
| `/Game/Maps/LoadingMap` | World | Loading hold between Lobby and Main | `Active` | Required project map and level-flow baseline |
| `/Game/Maps/MainMap` | World | Multiplayer gameplay world | `Active` | Required project map and main-level baseline |
| `/Game/Blueprints/BP_GameInstance` | Blueprint | Configures the S1 game instance and remote-player class | `Active` | References `BP_Player` through the current account/world flow |
| `/Game/Blueprints/BP_GameMode` | Blueprint | Current gameplay mode asset | `Active` | References `BP_MyPlayer` |
| `/Game/Blueprints/BP_S1GameModeBase` | Blueprint | S1 game mode configuration | `Active` | References `BP_MyPlayer` |

## Active player character assets

| Asset path | Type | Runtime role | Status | Important dependencies / notes |
|---|---|---|---|---|
| `/Game/Blueprints/BP_MyPlayer` | Data-only Character Blueprint | MainMap local/default pawn; initial Warrior presentation | `Active` | Native parent `S1MyPlayer`; Terra mesh, `ABP_S1Terra`, default input assets |
| `/Game/Blueprints/BP_Player` | Data-only Character Blueprint | Other-player spawn class; initial Warrior presentation | `Active` | Native parent `S1Player`; Terra mesh and `ABP_S1Terra` |
| `/Game/Blueprints/Characters/ABP_S1Terra` | Animation Blueprint | S1-safe Terra locomotion for local and remote players | `Active` | Targets `Terra_Skeleton`; retains generic movement/air/aim logic and excludes sample `TerraPlayerCharacter` attack/delegate bindings |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra` | Skeletal Mesh | Default player and initial Warrior character presentation | `Active` | User-added ParagonTerra pack; uses `Terra_Skeleton`, `Terra_Extents`, Terra materials, and clothing runtime; player components temporarily force LOD0 because lower-LOD weapon skinning deforms in UE 5.8; 153 bones; portfolio usage-rights note still to be recorded |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra_Skeleton` | Skeleton | Default Warrior animation skeleton | `Active` | Target skeleton of `ABP_S1Terra` and bundled Terra animations |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra_Extents` | Physics Asset | Terra character physics bodies | `Active` | Bundled physics asset assigned by the Terra mesh; no component override is active |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Idle` | Animation Sequence | Warrior idle locomotion | `Active` | Dependency of `ABP_S1Terra` |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Jog_Fwd_Start` | Animation Sequence | Warrior movement start | `Active` | Dependency of `ABP_S1Terra` |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Jog_Fwd_Stop` | Animation Sequence | Warrior movement stop | `Active` | Dependency of `ABP_S1Terra` |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Jump_Start` | Animation Sequence | Warrior jump start | `Active` | Dependency of `ABP_S1Terra` |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Jump_Apex` | Animation Sequence | Warrior jump apex | `Active` | Dependency of `ABP_S1Terra` |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Jump_PreLand` | Animation Sequence | Warrior pre-land | `Active` | Dependency of `ABP_S1Terra` |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Jump_Recovery_Additive` | Animation Sequence | Warrior landing recovery | `Active` | Dependency of `ABP_S1Terra` |

## Retained template and weapon candidates

| Asset path | Type | Intended role | Status | Important dependencies / notes |
|---|---|---|---|---|
| `/Game/Characters/Mannequins/Meshes/SKM_Manny` | Skeletal Mesh | Fallback/template character | `Candidate` | No longer referenced by `BP_MyPlayer` or `BP_Player` |
| `/Game/Characters/Mannequins/Animations/ABP_Manny` | Animation Blueprint | Fallback/template locomotion | `Candidate` | No longer the default player AnimClass |
| `/Game/ARPG_Halberd/Demo/Weapon/Mesh/Halberd_weapon` | Skeletal Mesh | Initial Warrior weapon candidate | `Candidate` | User-added ARPG Halberd pack; separate 3-bone `Halberd_weapon_Skeleton`; no sockets; not attached yet because Terra has no conventional hand-weapon socket; portfolio usage-rights note still to be recorded |
| `/Game/ARPG_Halberd/Demo/Weapon/Mesh/Halberd_weapon_Skeleton` | Skeleton | Halberd weapon rig | `Candidate` | Weapon-only skeleton; not animation-compatible with `Terra_Skeleton` |
| `/Game/ARPG_Halberd/Demo/Weapon/Mesh/Halberd_weapon_PhysicsAsset` | Physics Asset | Halberd presentation physics | `Candidate` | Dependency of `Halberd_weapon` |
| `/Game/ARPG_Halberd/Demo/Weapon/Materials/Halberd` | Material | Halberd surface material | `Candidate` | Dependency of `Halberd_weapon` |

## Active input assets

| Asset path | Type | Runtime role | Status |
|---|---|---|---|
| `/Game/Blueprints/Input/IMC_Default` | Input Mapping Context | Current local-player input mapping | `Active` |
| `/Game/Blueprints/Input/Actions/IA_Move` | Input Action | Player movement | `Active` |
| `/Game/Blueprints/Input/Actions/IA_Look` | Input Action | Camera look | `Active` |
| `/Game/Blueprints/Input/Actions/IA_Jump` | Input Action | Character jump | `Active` |

## Knight input candidates

| Asset path | Type | Intended role | Status | Verification |
|---|---|---|---|---|
| `/Game/Blueprints/Input/Actions/IA_Attack` | Input Action | LMB Knight primary combo intent | `Active` | Mapped to `LeftMouseButton`; local activation and remote A-D presentation verified in two-client PIE on 2026-09-16 |
| `/Game/Blueprints/Input/Actions/IA_KnightQ` | Input Action | Knight Q charge intent | `Active` | Mapped to `Q`; local activation and remote Q presentation verified in two-client PIE on 2026-09-16; displacement is still pending |

## Planned character and combat assets

These entries describe required roles, not approved source assets.

| Required asset role | Initial target | Status | Acceptance requirement |
|---|---|---|---|
| Player class presentation data | `PlayerClass -> Mesh / AnimClass / Weapon / UI portrait` | `Planned` | Local and remote players resolve the same class presentation from stable data |
| Ability presentation data | `AbilityId -> Montage / Cue / Sound / icon / socket` | `Planned` | Uses stable server IDs; contains no authoritative damage values |
| Knight weapon | `/Game/ARPG_Halberd/Demo/Weapon/Mesh/Halberd_weapon` | `Candidate` | Add/approve a Terra hand socket and visually validate attachment alignment; collision remains presentation-only |
| Knight combat animation set | Terra A-D combo and Q charge active scope; guard deferred, E disabled; hit, death, dodge still missing | `Active` | Local GAS activation and IOCP-relayed A-D/Q multiplayer playback verified; authoritative hit/damage, Q displacement, S1 notifies, and final visual timing remain |
| Archer weapon and projectile presentation | Bow, arrow, projectile visuals | `Planned` | Server projectile remains authoritative; client asset is visual representation |
| Archer animation set | Aim, basic shot, hit, death, dodge | `Planned` | Supports aim direction and remote playback |
| Mage cast presentation | Cast montages and hand/staff VFX | `Planned` | Niagara/Cue timing maps to server action events |
| Monster base presentation | Mesh, skeleton, AnimBP, hit/death/attack | `Planned` | Supports server FSM state and target/action events |
| Combat UI presentation | HP, party/ally state, cooldown, target feedback | `Planned` | Mirrors server state and survives two-client PIE validation |
| Combat audio | Attack, hit, death, skill cues | `Planned` | Source/license recorded and cue ownership defined |

## Reference-only content

Update 2026-09-17: the user has added `/Game/ParagonMinions` directly to S1. The inspected subset below is now S1 inventory, independently of the UE5_RPG reference copy. This does not authorize importing anything else from UE5_RPG.

## Paragon Minions intake — 2026-09-17

### S1 Dusk presentation asset — 2026-09-21

| Exact asset path | Class | Role / evidence | Status |
|---|---|---|---|
| `/Game/S1/Monsters/BP_S1DuskMinion` | Blueprint, parent `/Script/S1.S1DuskMinion` | Runtime Dusk presentation class selected by `US1WorldEntitySubsystem`; current presentation is scale `0.7`, yaw `+90°`, capsule `28×62`, mesh Z `-62`, attack-transition grace `0.05 s`. Collision size is fixed by the client/server Dusk definition, while presentation scale/yaw remain artistic tuning. Compiled with warnings-as-errors and transiently grounded on MainMap; fresh authenticated two-client visual retest pending. | Active |
| `/Game/S1/Monsters/ABP_S1Dusk` | AnimBlueprint, parent `/Script/S1.S1DuskAnimInstance` | Dusk-only blended Idle/Jog/Attack_A/Death_A presentation. Targets `Minion_Lane_Core_Skeleton`; native proxy evaluates the four clips while the AnimBlueprint supplies the runtime class. Linked to the minion mesh; user reported visual naturalness check complete on 2026-09-28, while full 1-/2-client regression remains pending. | Active |

Implementation follow-up: the Dusk mesh and idle/jog/attack/death clips are runtime references of `AS1DuskMinion` and its S1-owned Blueprint child. Windows Development cook/package startup passed on the earlier revision, and the user completed the two-client combat/HP/death-respawn/reconnect checks before the 2026-09-28 blend change. `HitReact_Front` remains unwired because its Local Space additive pose is unsafe as a standalone sequence. On 2026-09-28, direct SingleNode playback was replaced by `ABP_S1Dusk` and a native four-clip blend proxy. S1Editor and S1 game targets built, both affected Blueprints compiled, and fresh-load asset inspection found the AnimBlueprint link and zero dirty packages. Authenticated 1-/2-client visual testing and a new cook remain pending. See [DuskVerticalSliceImplementation.md](DuskVerticalSliceImplementation.md).

Selected first enemy: Dusk lane melee minion. The wired runtime subset is `Active`; unwired reactions and alternatives remain `Candidate`. Exact attack contact timing is still provisional, and the user-added pack's acquisition source and portfolio usage rights remain pending confirmation.

| Exact asset path | Class | Role / evidence | Status |
|---|---|---|---|
| `/Game/ParagonMinions/Characters/Minions/Dusk_Minions/Meshes/Minion_Lane_Melee_Dusk` | SkeletalMesh | Runtime mesh of `BP_S1DuskMinion`; two-client combat/death/respawn presentation user-verified before the 2026-09-21 axis-transition adjustment | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Core_Skeleton` | Skeleton dependency | Runtime skeleton of the Dusk mesh and active sequences; no Terra retarget required | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Melee_Physics` | PhysicsAsset dependency | Assigned runtime physics asset; authoritative combat collision remains the server geometric check | Active |
| `/Game/ParagonMinions/Characters/Minions/Dusk_Minions/Materials/Melee/MI_Minion_Dusk_Melee` | Material dependency | Assigned runtime Dusk material slot; cooked/package startup and user PIE presentation verified | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/NonCombat_Idle` | AnimSequence | 10.167 s, non-additive, Root Motion off; runtime idle presentation | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Combat_JogFwd` | AnimSequence | 1.800 s, non-additive, Root Motion off; runtime chase/return presentation | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Attack_A` | AnimSequence | 1.000 s, non-additive, Root Motion off; runtime attack presentation, provisional server contact remains 420 ms | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Death_A` | AnimSequence | 2.133 s, non-additive, Root Motion off; runtime death presentation | Active |
| `/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/HitReact_Front` | AnimSequence | 0.667 s, Local Space additive, own frame 15 as reference pose; composition requires validation | Candidate |

MCP inspection: mesh registry referencer is `/Game/ParagonMinions/Characters/Maps/Minions`, not an S1 gameplay map. Idle and jog are referenced by the source `IdleToRun_A_Combat` blendspace; inspected attack/death/hit returned no registry referencers. No AnimBlueprint was returned by an AnimBlueprint-class search under `/Game/ParagonMinions`. Native/string references have not been exhaustively audited.

Dependency traversal from selected mesh and five clips visited 75 unique non-script paths and exhausted its queue. The only missing path was `/Game/Characters/Minions/LaneMinions/Meshes/Melee/Minion_Lane_Melee_Dawn`, matching the five clips' stale PreviewSkeletalMesh tag. All existing visited assets reported dirty=false. Do not create assets at this obsolete path to hide the issue. Diagnose and repair the relevant S1-owned presentation copies if needed, then verify cook; do not bulk-resave the imported pack. The mesh's shadow physics dependency is `/Game/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Melee_ShadowCyl`. No content package was changed or saved during intake.

`E:\Git Fork\UE5_RPG\UE5_RPG` is a read-only reference project. Its assets are not part of S1 until explicitly approved and migrated.

High-value reference candidates currently identified:

- `ParagonSunWukong`: melee character, animation, and VFX reference
- `ParagonAurora`: mage character and spell VFX reference
- `ParagonMinions`: monster presentation reference
- `_Input`, `DT_AttackData`, `DT_SkillData`, GameplayCue and GameplayEffect assets: schema and presentation-flow reference

Do not copy UE5_RPG Blueprint assets directly when they depend on UE5_RPG native parents. Rebuild the narrow S1 equivalent against the S1 architecture and external-server authority contract.

## Knight combat animation candidates

The gameplay roles below are user-approved, but the assets remain `Candidate` until S1-owned montages/notifies and multiplayer presentation are wired and verified. Design contract: `docs/KnightCombatDesign.md`.

| Asset path | Type | Intended role | Status | Important dependencies / notes |
|---|---|---|---|---|
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_A_slow_Montage` | Animation Montage | Knight combo step 1 | `Active` | `UpperBody` slot; user reports timed Niagara Trail authored and visible for local and remote players in 1-/2-client PIE (2026-09-30) |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_B_slow_Montage` | Animation Montage | Knight combo step 2 | `Active` | `UpperBody` slot; user reports timed Niagara Trail authored and visible for local and remote players in 1-/2-client PIE (2026-09-30) |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_C_slow_Montage` | Animation Montage | Knight combo step 3 | `Active` | `UpperBody` slot; user reports timed Niagara Trail authored and visible for local and remote players in 1-/2-client PIE (2026-09-30) |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_D_slow_Montage` | Animation Montage | Knight combo step 4 | `Active` | `UpperBody` slot; user reports timed Niagara Trail authored and visible for local and remote players in 1-/2-client PIE (2026-09-30) |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/RMB_intro` | Animation Sequence | Future shield guard redesign | `Candidate` | Guard is deferred until Q design is complete; no runtime wiring now |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/RMB_melee` | Animation Sequence | Future guard attack redesign | `Candidate` | Guard is deferred until Q design is complete; no runtime wiring now |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/RMB_out_MSA` | Additive Animation Sequence | Future shield guard redesign | `Candidate` | Guard deferred; additive-base validation will be revisited during redesign |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Q_swing_B` | Animation Sequence | Q forward charge attack | `Active` | Local and remote presentation verified; 3.100 s and Root Motion disabled, so external server-driven displacement remains pending |
| `/Game/Blueprints/Characters/AM_Terra_Q_AttackMontage` | Animation Montage | User-authored Q presentation and Trail | `Active` | Live MCP confirmed Terra skeleton, `Q_swing_B` (3.100 s), `UpperBody`, auto blend out and `NS_Trail_14`. Both player Blueprints reference it. Game/editor builds and Q automation passed. User confirmed current-revision Q motion/Trail, movement/basic-attack recovery and remote presentation in MainMap 1-/2-client PIE on 2026-09-30. Packaged runtime and live death/disconnect interruption remain unverified; see `TerraQMontageIntegration.md`. Q server movement/collision/damage remain pending. |
| `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/E_ability` | Animation Sequence | Disabled Knight E candidate | `Candidate` | User disabled E; no input, ability grant, or runtime wiring |
| Terra-compatible dodge animation | Animation Sequence / Montage | Knight dodge | `Blocked` | No suitable dodge/roll/evade animation found in the Terra pack |

## Terra combat discovery follow-up (2026-09-17)

Follow-up: live MCP property inspection is now available in [TerraAnimationInspection.md](TerraAnimationInspection.md). 35 sequences loaded, matched Terra_Skeleton, and reported no unsaved changes. The earlier filesystem-only limitation below describes the initial discovery, not the later property audit. Candidate status remains: full visual and runtime integration validation are pending. General HitReact assets are Local Space additive; RMB_melee_recovery_MSA is also Local Space, while shield-pose MSA variants are Mesh Space. A non-additive RMB_Shield_pose and RMB_end were confirmed. Persona guard-pose preview showed lower-LOD weapon deformation; do not mark visual compatibility complete.

Read-only filesystem discovery found the following packages under `/Game/ParagonTerra/Characters/Heroes/Terra/Animations/`. These are `Candidate` entries, not verified runtime mappings. Unreal MCP was not exposed in this planning session; asset class, skeleton, additive settings, root motion, duration, dependencies, referencers, dirty state, and visual suitability remain unverified. Names below are exact package basenames, not inferred gameplay guarantees. No content package was modified or saved.

| Exact package basenames (relative to the folder above) | Proposed inspection purpose | Status |
|---|---|---|
| `HitReact_Front`, `HitReact_Back`, `HitReact_Left`, `HitReact_Right` | Directional player hit reaction | `Candidate` |
| `Death` | Player death presentation | `Candidate` |
| `Stun_start`, `Stun_Idle` | Stun entry and sustained presentation | `Candidate` |
| `Knockback_front`, `Knockback_back` | Server-directed knockback presentation | `Candidate` |
| `RMB_Shield_pose`, `RMB_Shield_pose_MSA`, `RMB_Shield_Jog_pose_MSA` | Existing guard-pose alternatives before authoring a derived hold pose | `Candidate` |
| `RMB_HitReact`, `RMB_HitReact_MSA` | Block feedback | `Candidate` |
| `RMB_melee_recovery`, `RMB_melee_recovery_MSA`, `RMB_end` | Guard attack recovery and possible guard exit | `Candidate` |
| `Primary_Melee_A_slow_Recovery`, `Primary_Melee_B_slow_Recovery`, `Primary_Melee_C_slow_Recovery`, `Primary_Melee_D_slow_Recovery` | Basic attack exit/recovery transitions | `Candidate` |
| `Jog_Left_Combat`, `Jog_Right_Combat`, `Jog_Fwd_L_Combat`, `Jog_Fwd_R_Combat`, `Jog_Bwd_L_Combat`, `Jog_Bwd_R_Combat` | Combat directional locomotion | `Candidate` |
| `Primary_Melee_Air` | Optional future airborne attack; current airborne activation block remains | `Candidate` |
| `R_Intro`, `R_ending`, `Cast` | Future presentation exploration only; no new skill approved | `Candidate` |

Earlier statements that hit/death animations are missing or that a derived guard-hold pose is mandatory must be treated as unresolved integration gaps rather than proof of source-asset absence. Inspect these candidates before requesting new assets or authoring replacement poses. Guard remains deferred; E remains disabled. The discovery does not approve new gameplay, migration, or runtime wiring. Provenance is the existing user-added Terra folder; portfolio usage-rights confirmation remains pending.

## Deprecated assets

| Asset | Status | Rule |
|---|---|---|
| `/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter` | `Deprecated` | Intentionally removed; never recreate or reference |

## Hit-effect candidates — 2026-09-29

Read-only Unreal MCP inspection originally compared three candidates. The user selected `P_Terra_Primary_Impact` for the Terra-to-Dusk hit effect on 2026-09-29. It is wired in source/config, included in the Windows Development cook, and its two-client PIE appearance was accepted by the user; the other two were not selected or modified.

| Exact asset path | Class / status | Inspection and proposed role |
|---|---|---|
| `/Game/ParagonTerra/FX/Particles/Terra/Abilities/Primary/FX/P_Terra_Primary_Impact` | `ParticleSystem` / Active | Configured for server-confirmed Terra primary damage against Dusk. Eight direct dependencies include Terra hit-arc mesh/material, impact warp mesh, flare/wind materials and a vector field. Headless UE load found a 32-package `/Game` dependency closure with no missing assets; automation test loaded the exact configured path; Windows Development cook included it. User confirmed one effect per A-D hit on each of two PIE screens, no effect on misses/wall-blocked attacks, and suitable size, brightness and facing. Emitter loops and packaged MainMap combat remain unverified. |
| `/Game/ParagonMinions/FX/Particles/Minions/Shared/P_Minion_Melee_Impact` | `ParticleSystem` / Candidate | Alternate melee impact. Four direct material dependencies include flare, smoke and direction-tinted sparks. Appearance inferred only from dependencies; not visually verified. |
| `/Game/ParagonMinions/FX/Particles/Minions/Minion_melee/FX/Impacts/P_Minion_Impact_Default` | `ParticleSystem` / Candidate | Lower-priority preview candidate: nine direct dependencies include liquid/droplet materials and meshes plus Engine WorldGridMaterial. That reference alone does not establish a rendering defect. |

All three resolved through live object-property inspection: system warmup `0 s`, system delay `0 s`, `bAutoDeactivate=true`; asset registry referencer lists empty and package dirty flags false when inspected. These properties do not prove finite emitter loops or particle lifetimes. No skeleton/retarget operation is indicated by the direct dependency lists. No assets were edited, saved, or wired to gameplay.

Current selected-effect limits: emitter-level parameters/loops, exact visual duration, forced rejection/duplicate/delayed cases and packaged MainMap combat remain unverified. Two-client PIE appearance and Dusk-scale placement were accepted by the user on 2026-09-29. A 2.5-second client cleanup cap is configured regardless of imported emitter behavior. The asset's empty registry referencer list is expected for a config-built soft path and does not mean it is unused. A cook-directory entry explicitly includes its two-package folder; Windows Development BuildCookRun passed and the cooked asset registry contained the exact particle path. Provenance is the installed ParagonTerra content; publication rights were not independently verified. Niagara conversion is not part of this scope. A separate headless editor inspection reported the Terra mesh package as dirty; its cause was not determined, and it was not saved.

## Attack-trail candidates — 2026-09-29

The original Cascade trial remains Candidate after its PIE visibility failure. The currently accepted visible A-D/Q Trail uses user-authored timed Niagara montage effects; legacy Cascade code remains source-wired pending a separately scoped cleanup.

| Exact asset path | Class / status | Evidence and limits |
|---|---|---|
| `/Game/ParagonTerra/FX/Particles/Terra/Abilities/Primary/FX/P_Terra_AnimTrail` | `ParticleSystem` / Candidate (source-wired, PIE visual failure; replacement under consideration) | Direct dependency: `/Game/ParagonTerra/FX/Materials/Heroes/Terra/Abilities/M_Terra_PrimaryAnimTrail`. UE loaded it; package contains `ParticleModuleTypeDataAnimTrail`. `S1KnightCombatComponent` hard-references it in source and drives existing `FX_Trail_R_01/02` sockets for A-D. Earlier registry referencers were empty because source paths do not appear as package referencers. S1/S1Editor builds, rebuilt-editor asset load and fresh Windows Development cook/package passed. User's 2026-09-30 1-/2-client PIE check found no visible trail on either screen despite begin/end logs; a follow-up emitted 5-9 particles with nonzero bounds. Rendered pixels/material remain unverified; do not mark Active. |
| `/Game/ParagonTerra/FX/Particles/Terra/Abilities/DoubleSweep/FX/P_Terra_Q_Sweep` | `ParticleSystem` / Candidate | Eight direct dependencies: sweep arc/wisp/smoke materials and Terra sweep meshes. Preview candidate for Q-style sweep, not selected for basic attacks. Registry referencers empty, dirty false. Actual appearance, lifetime and placement remain unverified. |
| `/Game/SwordTrailVFX/VFX/NS_Trail_01` | `NiagaraSystem` / Candidate (user-added 2026-09-30) | Exact S1 file exists; package records UE 5.0 release provenance and references `/Game/SwordTrailVFX/Materials/M_Trail_Refraction`. That material package references `T_TrailMask_04` and `RT_Draw_N_01`, all present locally. The bundled demo animations `A_NS_01` and `A_Combo_1` reference `NS_Trail_01` through `AnimNotifyState_TimedNiagaraEffect`, so a timed Niagara state is the closest imported usage pattern. This is package-string inspection, not a UE asset-registry dependency check. Live editor log records `System successfully compiled` for the system in UE 5.8, but Terra attachment, visual result, dirty state and packaged runtime use are unverified; no S1 combat reference yet. User confirmed portfolio use is permitted; specific marketplace provenance was not supplied. |
| `/Game/SwordTrailVFX/VFX/NS_Trail_04` | `NiagaraSystem` / Candidate (earlier user trial) | Earlier selected Trail 4; current saved A-D and Q dependencies instead point to `NS_Trail_14`, confirmed through live read-only MCP on 2026-09-30. Do not assign the accepted current revision to this older candidate. Portfolio use permitted by user; packaged runtime unverified. |
| `/Game/SwordTrailVFX/VFX/NS_Trail_14` | `NiagaraSystem` / Active (A-D and Q montage presentation) | Live MCP dependencies confirm all four saved A-D montages and `AM_Terra_Q_AttackMontage` reference this system through NiagaraAnimNotifies. User confirmed A-D Trail and the current Q motion/Trail, recovery and remote presentation in 1-/2-client PIE. Exact final notify/socket/width/lifetime values were not independently audited; packaged runtime remains unverified. SwordTrailVFX portfolio use was confirmed by the user. |

The 2026-09-29 legacy Cascade path uses `BeginTrails`/`EndTrails` and component-owned A-D timers; it is still present but failed visual acceptance. The user subsequently authored Niagara montage effects; see [Trail history](TerraPrimaryTrailImplementation.md) and [Q integration](TerraQMontageIntegration.md). No separate VFX packet or server damage change was added.

## Death dissolve feasibility — 2026-10-01

- Candidate: `/Game/MaterialFunctionCollection/Materials/Materials/Dissolve/MI_Cave_Dissolve` (MaterialInstanceConstant, user-imported; no S1 gameplay integration). Live MCP parent/dependencies resolve to `M_Cave_Dissolve`, Cave floor diffuse/normal and Cave emissive showcase textures, plus Perlin noise. Referencer: `/Game/MaterialFunctionCollection/Maps/L_MaterialFunctionCollection`. Instance dirty=false at inspection.
- Parent `M_Cave_Dissolve`: Surface/Masked, skeletal-mesh usage=false, material attributes=false; depends on `/Game/MaterialFunctionCollection/Materials/Functions/MF_Dissolve`. Exposes `Dissolve` (current 0.7), `Dissolve Edge Width` (0.035), `Dissolve Edge Emissive` (50), Color and texture parameters. Endpoint semantics and animated appearance remain untested. Direct replacement would use cave-example appearance; prefer an S1-owned Dusk-preserving adaptation if selected. Skeletal shader compatibility/compile must be validated; usage=false is not proof of impossibility.
- Existing Dusk melee material already exposes `FadeOut`, `Erode`, `Ash`, `BurnColor`, `FadeMask`. Parent chain: `MI_Minion_Dusk_Melee` -> `M_Minion_Dusk_Armored` (actually MaterialInstanceConstant) -> `M_Minion_Dusk_Parent` (Opaque, skeletal usage=true, material attributes=true). Root dependencies include `MF_DeathFade` and `MF_CharacterEffects`. These are alternative reuse candidates, not proof of working opacity dissolve: graph wiring and visual effect are unverified, especially with Opaque blend mode.
- Current server Room removes dead Dusk after 3 seconds and broadcasts S_DESPAWN; client DespawnEntity destroys its actor. Proposed effect must fit that lifetime or explicitly design an independent cosmetic remnant; do not delay authoritative removal implicitly. Death/HP remain server-owned; per-monster dynamic material state is client presentation and must reset on respawn.
- Read-only inspection only; no material/mesh/BP save or mutation, no runtime/cook/visual test. Full dependency closure and asset publication rights/source are not independently verified. User source/license confirmation is needed before published use of the newly imported collection.

## Dusk existing-material death fade — 2026-10-01

- Added `/Game/S1/Monsters/Materials/MI_S1Dusk_DeathFade` (MaterialInstanceConstant), inherited from original `MI_Minion_Dusk_Melee`. Runtime reference is native AS1DuskMinion DeathFadeMaterial. Status: **Active; user accepted current-version 1-/2-client PIE lighting, body/armour/weapon disappearance and respawn on 2026-10-01**. This was user-performed visual verification, not an independent log audit; fade-time reconnect/DEAD snapshot, early despawn and packaged two-client play remain unverified. No imported cave dissolve material/function was used.
- Graph actually routes `MF_DeathFade.FadeToBlack` through MF_CharacterEffects, not AlphaFade. OpacityMask passes through; a Masked MIC override alone cannot activate erosion. FadeToBlack opacity is `1 - saturate(2*FadeOut)`. S1 MIC overrides blend to Translucent and VertexDisplacePower=0. Material-sphere MCP renders confirmed visible at0 and invisible at0.5/1. Skeletal thumbnail was inconclusive. This is **opacity fade, not masked erosion dissolve**; lighting/shadow changes need user judgment.
- Living Dusk retains original material. Server DEAD plays existing Death_A once; at1.5s an independent MID ramps FadeOut0→0.5 over1.2s, then hides at2.7s. Server3s removal/HP/damage/navigation/protocol/DB unchanged. DEAD initial snapshots hide immediately without inferred death age; respawn is a new original-looking actor. See [DuskDeathFadeImplementation.md](DuskDeathFadeImplementation.md) for lifecycle, tests and remaining checks.
- Saved only new final MIC; removed unsaved S1 trial MIC. No Dusk BP/AnimBP/map/Paragon package saved. Imported `MI_Cave_Dissolve` remains **Candidate, unused**. Checkpoint/hash and scoped dirty checks performed.

## Potion intake — 2026-10-01

Status: **Candidate / foundation source paths only**, not a completed playable inventory/drop flow. No UE5_RPG import; source and public-video usage rights of these four assets are still unconfirmed.

| Exact asset | Read-only intake and current use |
|---|---|
| `/Game/LootSet/Meshes/Potions/potion_large02` | StaticMesh, one potions_mat slot, bounds min(-12.584,-12.633,0)/max(14.584,12.633,42.573)cm; reference Loot_Set_Layout, dirtyfalse. Presentation settings soft path, proposed visual scale0.4. No world actor/runtime/cook verification yet. |
| `/Game/_Resources/Rair_Potion` | Texture2D,sRGBtrue,TC_Default; originally unsaved import with registry dependency/referencer errors. User-authorized exact-package save succeeded, dirtyfalse, referencers empty after save, file21746bytes and initial-save checkpoint hash matched. Referenced by new presentation settings and passive inventory UI loader; actual display/cook pending. LobbyMap was also dirty and was NOT saved. |
| `/Game/DrapEffet/VFX/NE_drop_effects01` | NiagaraSystem despite NE prefix; seven active Sprite emitters(CPU5/GPU2), no user parameters, four game-material dependencies(M_beam/M_cell/M_down/M_glow), reference DrapEffet/Maps/main, dirtyfalse. Exact soft path in presentation settings only. Loop/lifetime, world placement, rendered GPU behavior and cleanup/cook pending. Original system unchanged. |
| `/Game/MedievalClassicUI/Blueprints/UserInterface/BP_Bag` | WidgetBlueprint, parent UserWidget/generated BP_Bag_C confirmed by registry tags, numerous UMG texture dependencies, reference OverviewMap, dirtyfalse. Design reference only, not loaded as gameplay inventory replacement. Passive native inventory UI exposes no equipment/currency/use controls. |

Full feature scope, DB contract, blockers and executed-vs-pending tests: [PotionLootImplementation.md](PotionLootImplementation.md). HP+50 use/consumption remains excluded.

World presentation follow-up: native AS1LootActor now loads the selected mesh/FX paths into owned components and guards owner-only selected hints/pending retention/retirement. It is NOT wired to runtime WorldEntity packets/G, so these assets remain Candidate rather than Active. S1Editor build and six synthetic automation tests passed. Fresh read-only headless loads confirmed exact asset classes; recursive hard+soft game-package dependency closures mesh5/icon1/FX10/BP_Bag34, with no missing dependency. Headless process dirtied only the known Terra mesh; it was not saved. Native TextRender Korean font readability, VFX lifetime/GPU rendering, scale/floor/brightness and network/cook usage are still unverified.

## Healing potion use intake — 2026-10-02

Latest acceptance — 2026-10-06: `/Game/VFX_StatusEffect01/FXS/FXS_healing000.FXS_healing000` is **Active for server-confirmed potion healing**. The user verified the current version in actual one-/two-client PIE: healing amount, quantity decrement, exactly one effect on each screen, full-HP no consumption, reconnect quantity persistence, and effect height. This supersedes the historical Candidate/pending wording below. Torso placement uses DefaultGame.ini HealingFootOffset Z96 (net capsule-local offset0); original Niagara/Blueprint/map assets were not modified or saved. Updated-config repackaging, full packaged gameplay and fault-injection checks remain pending; source/public-video usage rights remain unconfirmed. No fresh build/PIE/log audit was performed in this documentation/handoff turn.

2026-10-06 latest placement: user requested torso playback. Added DefaultGame.ini HealingFootOffset Z96 (current capsule half-height96; net local attachment offset0) and set/read back the live native settings CDO via MCP. No original asset save/modification; scale/emit/maxlife unchanged. Torso visual acceptance and repackaging of this config-only adjustment remain pending.

2026-10-06 implementation update: user approved the concrete use contract/recovery policy; DB/worker isolated tests and pure runtime-policy tests passed. This is not a rendered use-effect test. The healing system remains **Candidate, not yet connected to S1 runtime/cook**; no asset binary/Blueprint/map/vendor system was changed or saved. See PotionUseImplementation.md for migration and remaining integration gates.

Latest 2026-10-06 update supersedes the unconnected wording: healing FX is now integrated through native S1PotionPresentationSettings (soft path + exact CDO cook anchor) and server-confirmed WorldEntity event presentation. Status remains **Candidate / integrated, pending actual1-/2-client visual acceptance**. Final synthetic lifecycle/sequence/death cleanup tests passed; Windows Development package succeeded and cook registry contains the exact system. Two packaged NullRHI startup smokes are not FX/PIE acceptance. Native starting settings: foot attachment/scale1/offset0/emit1.5s/maxlife4s, collision/Nav off. Fresh live MCP reports original system dirty=false; no original system/Blueprint/map saved or UE5_RPG asset migrated. Source/public-video usage rights remain pending.

- User-selected `/Game/VFX_StatusEffect01/FXS/FXS_healing000.FXS_healing000`: **Candidate, not runtime-integrated**. Serial read-only live MCP confirmed NiagaraSystem, dirty=false, demo-map referencer, ten enabled CPU emitters using Sprite/Light renderers and no user variables. Emitter loop inputs are Infinite with 1s loop duration; particle lifetimes include both literal1s and dynamic/unsupported values. Original system was not modified or saved.
- Recursive game-package dependency inspection returned25 packages with no query failures; not a shader/cook proof. GetSystemCompileState timed out; compile status is unverified. CaptureAssetImage explicitly does not support this Niagara asset type; animated preview and UE5.8 rendered/cooked verification remain pending.
- Planned use: server-confirmed successful potion healing only, one local/remote cosmetic event, finite S1-owned component lifetime and death/despawn/session cleanup. No click/HP-snapshot-triggered FX. Attachment/scale/duration require visual check; provenance/public-video rights remain unconfirmed. See [PotionUseImplementation.md](PotionUseImplementation.md) for proposed schema/wire and recovery gate. Existing pickup acceptance does not establish item-use completion.

## Player Main HUD integration — 2026-10-20

New `/Game/S1/UI/WBP_S1MainHUD` (parent S1MainHUDWidget, derived from existing S1QuickSlotWidget) and `/Game/S1/UI/HUD/T_S1MainHUDBackplate`: **Candidate / integrated, user1-/2-client appearance/control acceptance pending**. Original generated transparent navy/steel/gold frame uses the user's Aion2 screenshot as composition reference, not copied game art/logo. Built-in imagegen used; final prompt/source inventory in ArtSource/UI/MainHUD/README.md. PNG2172×724 has sampled corner alpha0; imported as UI/EditorIcon/no-mips. PNG excluded from Git under the existing source-only policy but retained locally.

InventorySubsystem creates one extended QuickWidget rather than duplicate potion/HUD panels; serialized soft class is the cook reference, with old WBP_S1QuickSlots retained as fallback. Actual server HP cache, local LMB4-stage/Q presentation, existing potion1..4/count/drag/clear and I/O window APIs are wired. Alt-hold local cursor added with modal ownership protection. Existing dialogue HUD suppression retained. No MP/XP/currency/minimap or fabricated cooldown; class-role label KNIGHT is not a level claim. Current server CharacterInfo.level exists but selected identity HUD cache is not connected in this revision. Existing potion icon/font reused; attack/Q use text cards pending user-selected icon assets.

Verified partial checkpoint, reported only new dirty texture/HUD2 packages then selectively saved; original QuickSlots/Map clean. Game/Editor builds and36 tests passed (35 clean +1 existing Q warning); real PIE/cook/package remain unverified. Details/files/evidence: [MainHUDImplementation_2026-10-20.md](MainHUDImplementation_2026-10-20.md).

## Quest journal tab/detail revision — 2026-10-19

Latest status: **tab/detail integration implemented / user PIE acceptance pending**. Only reported dirty WBP_S1QuestJournal saved after verified checkpoint; Dialogue/Tracker/MainMap remain clean. Editor/Game builds and35 automatic tests passed (34 clean +1 existing Q warning), including tab filtering, selected saved-description/reward bindings, empty/unready state cleanup, selection persistence and O/Esc. Historical in-progress wording below is superseded.

WBP_S1QuestJournal is being revised under user request into left Active/Completed tab buttons and a scrollable title list, plus right selected quest detail (title, saved description, objective, state, reward icon/name/count). Existing O/Esc/Close ownership remains; read-only UI does not accept/claim quests. The current server supplies quest1 only, so this is not a new multi-quest/history protocol. Native view data is supplied by QuestSubsystem; UI does not parse packets. Status: **integration in progress / current tab revision PIE pending**. Exact files and validation: [QuestJournalTabs_2026-10-19.md](QuestJournalTabs_2026-10-19.md).

## Quest tracker visual design — 2026-10-19

Latest acceptance: user reported ‘체크 완료’ after the tracker visual revision. This is user-reported visual acceptance, not a new independently replayed two-client/package test. It supersedes the visual-pending wording in the historical paragraph below.

User-provided Aion2 screenshot used as layout inspiration, not a copied game texture/logo. Existing WBP_S1QuestTracker now has a lightweight translucent TrackerCard, small diamond marker, gold18pt TrackerTitleText, ivory14pt TrackerObjectiveText, blue-grey12pt TrackerStatusText and O hint. Existing MaruBuri_Font Regular reused. Legacy QuestText remains for compatibility, hidden when the new authored texts are available. No invented quest distance/level or server-rule changes. Existing tracker runtime use remains Active; **new visual revision awaits user PIE appearance acceptance**.

Checkpoint+SHA before mutation; only reported dirty tracker package saved. Game/Editor builds and35 automatic tests passed (34 clean +1 known Q warning), including saved-WBP text binding/empty-state/fallback checks. Dialogue/Journal/MainMap clean; no map/font/vendor/server/DB changes. Actual current-revision1/2-client rendering and packaging remain unverified. Edit/validation details: [QuestTrackerDesign_2026-10-19.md](QuestTrackerDesign_2026-10-19.md).

## Quest UI presentation integration — 2026-10-19

User-approved client-only follow-up: existing WBP_S1Dialogue now has RewardFrame/RewardIcon/RewardText beneath QuestDetailContents. New `/Game/S1/UI/WBP_S1QuestJournal` inherits `/Script/S1.S1QuestJournalWidget`, contains editable progress/completed sections and a close button, and is referenced by QuestSubsystem's serialized soft class for O-key use. Existing potion icon `/Game/_Resources/Rair_Potion.Rair_Potion` and MaruBuri_Font Regular reused. No new external art/fonts or UE5_RPG migration. Status: **Candidate / runtime integration implemented, rendered PIE acceptance pending**. Rewarded quests leave the HUD tracker and remain in the journal's completed section; Ready-to-report quests stay active. Current single-quest server contract is unchanged.

Serial MCP read-only tree/parent/dirty inspection preceded mutations. Clean WBP_S1Dialogue was checkpointed with matching SHA. Only dirty WBP_S1Dialogue and new WBP_S1QuestJournal were reported and saved. Tracker WBP, NPC BP and MainMap remained clean; no DT/map/vendor/font save. Native display rules hide quickslots, tracker, NPC names and entity HP/name bars during local dialogue and restore their normal policies after it closes. Validation details and editing paths: [QuestUIPresentation_2026-10-19.md](QuestUIPresentation_2026-10-19.md).

## Quest C reward integration — 2026-10-19

Latest acceptance: user confirmed all requested current-revision PIE checks: each character receives only their own potion3 reward, no repeat grant after repeated dialogue/reconnect, inventory/quickslot count updates, and movement/attack/camera restore after dialogue. C reward NPC/dialogue use is now **Active / user-accepted PIE**. Server and editor logs independently match one committed reward request; they do not independently prove the entire two-account/reconnect sequence. No matching Error/Fatal/Ensure condition/Blueprint Runtime Error/Network Failure was found in the captured editor log. PIE teardown CrowdManager missing-Recast warning remains separate. Package and forced-failure PIE checks remain pending; the historical pending paragraph below is superseded by this acceptance.

Existing BP_S1QuestNpc and DT_S1DialogueNodes now integrate the approved server-authoritative one-time potion reward flow. BP NPC properties select quest1 / Ready End6 / Rewarded End7 / Active End8; existing dialogue1..5 remain intact. MainMap placed NPC inherits these settings without saving the map. Both binary assets were checkpointed with SHA verification before changes; dirty state was reported before saving only these two packages. Native parent/generated class and warnings-as-errors compile call were verified through serial MCP. Existing WBP_S1Dialogue/WBP_S1QuestTracker layouts were not modified or saved.

Status: **C reward integration implemented / fresh rendered PIE acceptance pending**; previous A/B NPC/dialogue acceptance remains valid history, not C validation. Editor/Game/Debug Server builds, isolated DB140 checks, Unreal33 tests (32 clean +1 known Q warning), server pure14+29 checks and actual eight-node Export v2 validation passed. No new assets/vendor edits/UE5_RPG migration. Current-revision reward PIE and package checks remain unverified. See [QuestStageCRewardImplementation_2026-10-19.md](QuestStageCRewardImplementation_2026-10-19.md).

## Change log

Latest multi-spawn update — 2026-10-06: existing `/Game/S1/Editor/BP_S1DuskSpawnPoint` remains **Active / editor authoring only**, now inherits editable MonsterDefinitionId and unique positive SpawnPointId for 1..16 points. Only Dusk1 is registered. MainMap preserves the user's point1 at (904.402783,15.172481,-0.5), adds point2 at (904.402783,215.172481,-0.5) and point3 at (1104.402783,15.172481,-0.5); all enabled/yaw-90/scale1. MCP warnings-as-errors compile call succeeded; existing BP stayed clean. Only dirty MainMap was saved after reporting dirty state; no player/Dusk/vendor asset saved or UE5_RPG content migrated. New saved-map process exported three validated v2 points and deployed them. Server Debug/Release builds, 26 actual Room offline checks, 29 parser checks and 14 Unreal synthetic tests including multi-target presentation passed. Development packaging succeeded; package MainMap smoke queried zero authoring-marker objects and exited0, with expected server-off connection Error1. Authenticated1-/2-client multi-monster combat/drop/return/reconnect and visual overlap checks remain pending. See MultiDuskSpawnImplementation.md; older one-point entries below are history.

2026-10-06 Dusk spawn authoring: `/Game/S1/Editor/BP_S1DuskSpawnPoint` is **Active (editor authoring only)**, parent `/Script/S1.S1MonsterSpawnPoint`, generated class `BP_S1DuskSpawnPoint_C`. Created under user implementation approval, compile call with warnings-as-errors returned successfully; parent/generated class confirmed through MCP. MainMap contains exactly one initial marker at feet (0,600,-0.5), yaw-90, unit scale, ID1/Dusk1/Enabled=true/editor-only=true. Verified checkpoint before binary changes; queried dirty BP/MainMap then saved only these two packages. Existing BP_MyPlayer/BP_S1DuskMinion were clean and not saved. Saved-map spawn commandlet completed exit0/0 errors/1 existing MCP EULA warning; exported XY/ground/yaw preserve the old server baseline. Source/deployed Debug/Release data SHA256 match. Gameplay 1-/2-client acceptance and cook stripping remain pending. No vendor/UE5_RPG assets migrated. See DuskSpawnPointAuthoring.md.

Current Q gameplay status (2026-10-01): existing Q montage/NS_Trail_14 remain unchanged. Q contract-v2 schedules two server hits (370ms360°/1100ms180°, radius230cm), permits movement and blocks jumping; cooldown deferred. Q explicitly reuses the existing primary impact through ActionType2 settings, not through an implicit primary match. User confirmed the current version in 1-/2-client PIE after server/editor restart: front two hits, rear first hit, movement allowed, jumping blocked, and recovery after Q. Wall/death/reconnect, detailed per-hit HP/FX log audit and packaging remain separate unverified items. See `TerraQTwoHitImplementation.md`.

| Date | Change | Evidence |
|---|---|---|
| 2026-10-01 | Added exact native default cook references for selected potion mesh, icon and drop Niagara; retained Candidate/foundation-only status | Latest S1/S1Editor builds, six automation tests and Windows BuildCookRun passed; cooked registry includes potion_large02, Rair_Potion, NE_drop_effects01 and three required maps. No vendor/map binary saved. Actual server-driven drop, GPU appearance, G/I and multiplayer acceptance pending; see `PotionLootImplementation.md`. |
| 2026-09-30 | Integrated and accepted user-authored Q montage for local/remote presentation | Live MCP/checkpoint, UpperBody correction, remote Blueprint compile and two-package save; Game/Editor builds and Q automation passed. User confirmed current-revision MainMap Q motion/Trail, movement/basic-attack recovery and remote display in 1-/2-client PIE. Packaged runtime/live interruption remain pending; see `TerraQMontageIntegration.md`. |
| 2026-09-30 | Recorded user-completed A-D timed Niagara Trail and Trail 4 runtime use; no binary or combat code edited by Codex in this update | User reports 1-/2-client PIE visibility for local and remote players; saved montage/package strings inspected; exact notify properties and packaged runtime not independently verified |
| 2026-09-30 | Registered user-added `NS_Trail_01` as Candidate; no binary asset or combat code changed | Local package and demo references inspected; timed Niagara usage and material/texture path hints found; live load, dirty state and PIE appearance pending |
| 2026-09-29 | Connected selected Terra AnimTrail to A-D client swing presentation in C++ without saving binary assets | UE read-only inspection confirmed ParticleSystem, AnimTrail type data, existing weapon sockets and montage inventory; S1/S1Editor builds, rebuilt-editor asset/class load, Windows Development BuildCookRun and packaged NullRHI startup passed; PIE visual and packaged MainMap combat pending |
| 2026-09-29 | Promoted Terra primary impact particle to Active after two-client MainMap PIE visual acceptance | User confirmed one effect per A-D hit on both screens, zero on misses/wall-blocked attacks, and suitable size/brightness/facing; client log showed two successful particle loads and server log showed accepted A-D hits; effect count was not independently instrumented |
| 2026-09-29 | Wired selected Terra primary impact particle through server-confirmed Dusk damage presentation settings; no Paragon package edited | Actual package loaded as ParticleSystem, 32-package dependency closure with no missing `/Game` references; S1/S1Editor Win64 Development builds passed after editor/debugger closure; 1 settings automation test passed; Windows Development BuildCookRun passed and cooked registry included the particle; PIE/appearance pending |
| 2026-09-28 | Added S1-owned `ABP_S1Dusk` and native Dusk four-clip crossfade; switched `BP_S1DuskMinion` from SingleNode to Animation Blueprint mode | S1Editor and S1 Win64 Development builds; affected Blueprint compile up-to-date; fresh editor-process reload confirmed class link and no dirty packages; user reported visual naturalness check complete; full 1-/2-client network regression and fresh cook pending |
| 2026-09-23 | User completed two-client MainMap PIE acceptance of Dusk wall avoidance, blocked wall-through attack/damage, path-based return, and HP/death/respawn/reconnect regression | User-reported test completion with no issue reported; PIE logs were not independently audited in this update |
| 2026-09-22 | Exported MainMap Recast connectivity to server-owned `S1NAV 2` data and changed Dusk CHASE/RETURN plus melee contact validation to server A* and funnel-reduced navigation paths; flat Z remains deferred | S1Editor Development and GameServer Debug/Release builds; commandlet 74-node/250-edge export; standalone clear/wall-corner/opposite-wall path smoke tests; Release server DB/account startup; authenticated PIE pending |
| 2026-09-22 | Unified the flat-ground Dusk server spawn/chase/return Z with the client capsule (`ground -0.5 + half-height 62 = center 61.5`), aligned native/Blueprint presentation defaults, and saved only `BP_S1DuskMinion` mesh Z `-62` | GameServer Debug and S1Editor Development builds; Blueprint warnings-as-errors compile; MCP floor trace and transient `28×62` capsule inspection; MainMap and Blueprint clean after validation; authenticated PIE retest pending |
| 2026-09-21 | User reported two-client pursuit/combat/HP/death-respawn/reconnect checks complete; introduced an S1-owned Dusk Blueprint presentation and removed the server's 100 ms post-attack Chase/Idle state bounce | User manual test report; Blueprint compile and transient component inspection; S1Editor/GameServer builds; fresh visual retest of this revision pending |
| 2026-09-17 | Wired native Dusk presentation and IOCP-server authority vertical slice without saving binary content; kept asset status as Candidate pending authenticated combat PIE and visual review | S1Editor and GameServer project builds, Windows Development cook/stage and packaged startup, MCP transient visual spawn/remove and MainMap floor/obstacle traces; [implementation record](DuskVerticalSliceImplementation.md) |
| 2026-09-16 | Verified the repeat-click combo fix in a fresh local PIE run; remote playback for this revision remains unverified | `S1.log` recorded step 1 nine times, step 2 six times, step 3 four times, step 4 three times, and 14 buffered inputs; no combat-related runtime error |
| 2026-09-16 | Fixed Terra combo repeat-click handling: one input is buffered throughout the current step and late pre-end clicks chain at completion; removed premature montage-poll cancellation | S1Editor Win64 Development build succeeded; fresh PIE click/hold regression test pending |
| 2026-09-16 | Added native third-person camera controls, 0.2-second early input buffer for the Terra four-hit combo, and opt-in enemy-tag soft/Tab targeting scaffold; no content asset was added or saved | S1Editor Win64 Development build succeeded; Unreal MCP and live PIE verification unavailable in this session |
| 2026-09-16 | Changed Knight combo facing so LMB does not turn during the active attack; movement/remote yaw is locked and the latest camera yaw is applied only at the buffered combo boundary | S1 Win64 Development build succeeded; two-client PIE visual verification pending after editor-module rebuild |
| 2026-09-16 | Verified Terra Knight A-D combo and Q presentation between two PIE clients and promoted the active input/animation entries | Client log sequences 1-20 showed matching `Combat action sent` and `Remote combat action played`; A-D steps 1-4 and Q step 0 succeeded with no combat rejection, Blueprint Runtime Error, Ensure, Fatal, or Network Failure |
| 2026-09-16 | Added the authenticated IOCP combat-action relay for Terra A-D and Q presentation (`C_COMBAT_ACTION`/`S_COMBAT_ACTION`) | S1 Win64 Development, GameServer Debug/Release, DummyClient Release, and S1Editor Development builds succeeded |
| 2026-09-15 | Added and saved `IA_Attack`/`IA_KnightQ`, mapped LMB/Q, and implemented the local GAS A-D/Q presentation scaffold | Unreal MCP mapping/dirty/save verification; S1 Win64 Development build succeeded |
| 2026-09-15 | Narrowed active Knight scope to A-D combo and Q; disabled E and deferred guard until Q is finalized | User direction |
| 2026-09-15 | Registered the user-approved Terra Knight A-D combo, shield guard/attack, Q charge, and E buff animation roles; recorded guard-hold, additive-exit, root-motion, and dodge gaps | Unreal MCP asset class, skeleton, montage slot, duration, additive type, and root-motion inspection |
| 2026-09-14 | Corrected Terra weapon deformation workaround: removed the temporary physics override and forced LOD0 on local/remote player meshes | User screenshot showed skinned-vertex stretching rather than loose rigid-body motion; Terra LOD inspection and component verification |
| 2026-09-14 | Set Paragon Terra as the default local/remote player and initial Warrior presentation; registered ARPG Halberd as an unattached weapon candidate | Unreal MCP property, dependency, skeleton/socket, compile, and save verification |
| 2026-09-14 | Created the registry and recorded the verified map, player, Manny locomotion, and input baseline | Source inspection and Unreal MCP asset-registry dependency inspection |

### Potion display integration update - 2026-10-02

2026-10-07 inventory WBP acceptance update: user reported current inventory PIE visual/control validation complete; design styling is deferred. WBP_S1Inventory and WBP_S1InventoryStackRow retain Active runtime status. This is user-reported acceptance, not independently replayed or a packaged two-account test. Read-only MCP initialize/list_toolsets and editor queries succeeded (PIE=false, LobbyMap). No binary, code, configuration or map changes/saves in this update; see InventoryWidgetBlueprint.md.

Latest G/I acceptance supersedes the historical pending/passive-only wording below: user confirmed current two-client owner G pickup, both-screen removal, I quantity increase and same-character reconnect persistence. Rair_Potion is **Active for the native read-only inventory presentation**; potion_large02/NE_drop_effects01 remain Active for world drops. Client/server/outbox audit confirms one captured commit, two client removals and count1/version1 after reconnect. Exact icon appearance was not independently screenshot-audited. This does not enable potion use/HP+50 or establish malicious-request/wall/timeout/package acceptance. Only documentation changed in this acceptance update; no asset binary saved. Provenance/public-video license notes remain pending. See PotionLootImplementation.md for exact evidence and known UIOnly-focus/teardown warnings.

G/I follow-up: selected Rair_Potion icon is now wired into the existing passive native inventory view opened with I; quantity comes only from validated server snapshots. Status: integration implemented, new rendered G/I acceptance pending (not yet Active inventory visual acceptance). Mesh/FX stay Active for previously user-accepted world presentation; their scale/asset binaries remain unchanged. No BP_Bag replacement/migration or vendor asset save. See PotionLootImplementation.md for new request/authority/version tests and manual gate.

Latest acceptance overrides the integration-pending status in the historical paragraph below: **potion_large02 and NE_drop_effects01 are Active for server-driven world-drop presentation**. User confirmed two-client Dusk-kill potion/effect visibility, acceptable floor placement/size and approximately 30-second visible removal. Independent logs confirm the same drop spawned once per client and server expiry after 30.086126 seconds; latest captured PIE ended three seconds after creation, so client-visible removal is user-reported rather than independently logged. Rair_Potion remains a passive UI/cook reference, not active inventory gameplay. G/I and potion use are not enabled. Scale0.4/floor offset0/FX scale1 remain unchanged; no binary assets saved. Public-video source/license confirmation and current-revision packaged play remain pending. See PotionLootImplementation.md for evidence boundaries.

This newer record supersedes foundation-only wiring descriptions above. potion_large02 and NE_drop_effects01 now have a server-driven WorldEntity runtime presentation path, behind separately negotiated loot_presentation_version1. Rair_Potion remains a passive UI/cook reference; G/I is not activated. Native scale0.4/floor offset0/FX scale1 and all three original assets are unchanged; no vendor/map binary saved. Status stays Candidate/integration-pending until genuine1-/2-client rendered acceptance. Game/Editor and Server Debug/Release builds passed; four loot automation tests passed after correcting a double-world-init test fixture. GPU duration/brightness, floor alignment, first-drop loading hitch, actual two-client kill/expiry and current packaging remain unverified. See PotionLootImplementation.md for exact source and evidence.
