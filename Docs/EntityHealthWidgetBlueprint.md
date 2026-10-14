# Entity overhead HP Widget Blueprint — 2026-10-08

Server-policy follow-up: DuskReturnRecoveryImplementation.md describes RETURN
re-acquisition/current-HP preservation and4sec home recovery via existing updates.
No new client asset/code changes for that follow-up. The3sec bar hide remains;
healing after home arrival may be invisible until next combat. Its PIE acceptance
is separate from this widget/visibility revision.

2026-10-08 user confirmed the follow-up server revision's 1-/2-client PIE
re-engagement and recovery synchronization. This does not independently accept
all HP-bar visibility/timing or packaged scenarios; see DuskReturnRecoveryImplementation.md.

## Scope and authority

User requested player AND monster overhead HP bars, superseding the earlier
planning suggestion of fixed local HUD/preserved remote text. One shared passive
WBP serves local player, remote player and each Dusk independently. No targeting,
new fixed HUD, RPC/replication, server, protocol, DB or input changes.
WorldEntity already applies spawn/damage/heal/state snapshots. Those existing
ApplyServerHealth and Dusk RefreshLabel paths now update one cached widget model.
UI never calculates damage or queries the server. Snapshot-before-widget creation
is retained by the component and replayed on InitWidget.

## Editing

/Game/S1/UI/WBP_S1EntityHealth derives from US1EntityHealthWidget. Designer owns
layout/fonts/fill colors. Preserve names/types: HealthBar(ProgressBar),
HealthText(TextBlock), NameText(TextBlock). Tree has 6 widgets/3 native bindings;
initial size180x56, bar height12, centered name/HP. Design preview75/100 is not game
state. Runtime only updates percent/text; Max0 is safe and over-max visual clamp
does not change server values.

BP_MyPlayer/BP_Player/BP_S1DuskMinion inherit EntityHealth (US1EntityHealthComponent).
Relative Z185(player)/150(Dusk) is capsule-center-relative, not feet-relative;
adjust location and DrawSize in BP component for presentation only. If enlarging
WBP root, also adjust component DrawSize to avoid clipping. WidgetClass is a
hard reflected class reference after the cook correction, applied at BeginPlay. Do not change collision
or Dusk scale/capsule to tune the bar. bHideOnDeath true for Dusk, false for players
(0/max DEAD visible). Unknown max hides bar. Destroying entity destroys component;
no persistent viewport/global actor bindings to survive reconnect.

Screen-space bar does not rotate camera or actor. No collision, keyboard focus or
mouse hit-testing. It stays constant screen size; occlusion by walls and distance
culling are NOT implemented/verified in this minimal stage. Do not infer an LOS
indicator from its visibility. Existing text still updates for diagnostics/tests
but is hidden in BeginPlay unless bShowHealthDebugText is enabled in class defaults.
Enabling that debug option intentionally shows both. Shared WBP means editing style
affects all these entities; separate player/monster skins are not implemented.

## Files / checkpoints

Modified Source/S1/Game/S1Player.h/.cpp, S1DuskMinion.h/.cpp and
Source/S1/World/S1WorldEntitySubsystem.cpp (existing local/target ID presentation only). New
Source/S1/UI/S1EntityHealthWidget.h/.cpp, S1EntityHealthComponent.h/.cpp,
S1EntityHealthWidgetTests.cpp. New WBP_S1EntityHealth.uasset only asset authored/saved.
No maps, vendor assets, existing BP or inventory/quick-slot WBP saved by Codex.
Source-only Git does not track WBP; no commit/push performed for this task.

Checkpoints/EntityHealth_20261008 in the separate Unreal IOCP Server workspace:
four original source copies and three existing saved player/Dusk BP copies with
matching hashes (before user's shutdown/save); this does not capture their then
dirty memory-only edits. Initial new HP WBP saved copy/hash verified. User reported
editor shutdown after request to preserve needed dirty BPs. New editor readback
showed all three existing BPs clean and inherited EntityHealth present. No attempt
was made to revert the user's existing BP changes to the earlier checkpoint.

## Verification

Initial live server PID30016 listened7777, editor PID46248 listened8000; MCP
initialize/list_toolsets and PIE=false/LobbyMap succeeded. Component inspection
showed original HealthLabel/StatusLabel, no HP WidgetComponent in character CDOs.
After shutdown new editor PID13884 started with command-line MCP autostart (no
configuration change). Saved HP WBP compile=true/warnings-as-errors compile;
dirty=true reported then only that package saved -> dirty=false. Existing
players/Dusk/Lobby/Main/inventory/quick-slots were clean at that selective-save check.
Actual CDOs for both players/Dusk had exact WBP path, DrawSize180x56, nonfocusable,
Z185/150 and correct death flags.

S1Editor and S1 Development full builds/link succeeded (checkpoint logs).
First added test compilation failed on TObjectPtr argument deduction; changed
to Get(), preserved TestCompileFirstFailure.log and rebuilt successfully.
UnrealAutomation.log and Saved/Automation/EntityHealth_20261008/index.json:
21 Success (20 plain +1 existing Q GameplayCue warning), failed0/notRun0.
S1.UI.EntityHealth errors0/warnings0: real saved WBP bindings, early snapshot,
heal/death/full/zero/clamped fraction, independent views, font preservation,
passive input and monster hide/restore. NullRHI is not rendered/network PIE.
Automation Fatal/Ensure/Blueprint Runtime Error/Network Failure/error scan found
no new matching errors. Package.log: Windows Development Build/Cook/Stage/Package/
Archive BUILD SUCCESSFUL, AutomationTool exit0. However the initial cook registry
did NOT include WBP_S1EntityHealth (three maps were present); packaging success
alone was insufficient. Hard class-reference correction and recook pending.
No packaged client HP play was executed.

User reported HP bars visible, then requested observer-local Dusk combat visibility
and remote-player HP hidden. Follow-up source uses existing target_object_id in
monster update/attack, and validated local damage events (dealing/receiving). Dusk
bar stays during CHASE/ATTACK targeting local player, then hides3s after exit; local
hit without local aggro keeps it3s after last hit. Unrelated clients do not reveal
the bar. Death hides immediately. Remote player bars hidden, local remains visible.
No range/party feature. New spawn snapshots lack target ID, so already-combat Dusk
waits for update/attack before revealing. Existing controller-specific server
authority remains unchanged.

Latest follow-up verified: S1Editor full build/link succeeded; UAT rebuilt S1 game
target and Windows Build/Cook/Stage/Package/Archive BUILD SUCCESSFUL/exit0. Latest
UnrealAutomation.log/report21Success/failed0/notRun0, EntityHealth error0/warning0
including a gate that snapshots cannot override. Existing Q warning retained.
Cook registry now contains HP WBP plus3maps. PackageHealthSmoke.log actual archived
S1.exe loaded HP generated class1object, Quit/exit0; existing Auth InputMode:UIOnly
Non-Focusable error1 remains, not a new HP error. Headless smoke does not test
rendered HP, actual3s delay or2-client observer filtering.
New editor PID31068 with MCP start flag, initialize/PIE=false and BP_MyPlayer CDO
hard HP class reference readback succeeded. Latest visibility-policy revision
requires user PIE acceptance; earlier 'normally visible' report is the previous
always-visible version. No further asset save, server/DB change or commit/push.

Actual user1-/2-client visual/interaction acceptance is pending: attack/heal/fullHP,
death/respawn, distinct Dusk bars, reconnect cleanup, UI/input/camera no interference,
height/size/color/readability. No authenticated PIE or latest packaged2-account
play executed by Codex in this task. Previously accepted combat tests are not
promoted to this version. Existing Auth focus error remains outside this change.
