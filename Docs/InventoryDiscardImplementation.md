# Inventory permanent discard — requested PIE acceptance confirmed, 2026-10-14

## Approval and boundary
User approved InventoryDiscardPlan_2026-10-14.md dedicated1037/1038 contract,
capability1, no schema migration, permanent deletion and quantity default1.
External IOCP/DB owns mutations. UI only collects explicit confirmation.
No world-drop spawn, HP/heal, combat, capacity expansion or new item types.

## Source implemented; not yet live-editor accepted
- New request/result regenerated via existing GenPackets.bat on both sides.
  Actual generatedIDs1037/1038 verified. Enter capability fields7/9.
- Authenticated Room->LootService->DB worker; shared pending/rate-limit guards.
- Character row lock/versionCAS/owned source slot/quantity/potion validation.
  Pending ItemUseOperation state2 protects refundable items. Fail closed if
  reservation schema query fails. Partial update or full row delete, version++
  and affected-row check in one transaction; no schema/data migration.
- Client retains stock until server snapshot; matched request/character/source/
  amount/version/result validation. Old or foreign responses ignored.
  Timeout/unknown -> snapshot only, no resend. Session reset clears UI/pending.
  No durable request journal or exactly-once history guarantee claimed.
- Separate passive two-stage discard widget: count>=2 quantity1..count/default1,
  next then final Yes/No; count1 final directly. Final confirmation submits once.
  Enter blocked to prevent quantity acceptance also deleting. No/Esc/I cancels.
  Backdrop consumes input without submitting. Native fallback included; optional
  UMG bindings AmountInput/ConfirmButton/CancelButton/MessageText/ConfirmText.
- New lowest-z drop receiver exists ONLY while inventory is open, below quick
  slots and other UI. Inventory-origin drag only; actual InventoryPanel geometry
  proves outside drop; missing/unarranged geometry fails closed. No generic
  DragCancelled deletion. Panel gaps/self/locked remain non-destructive.
- Stock actions blocked while discard modal/pending; quick-slot definition
  preferences retained after last item removed. Existing split design preserved.

## Changed files
Server canonical Common/Protobuf/bin/Protocol.proto; GameServer/Player.h,
Room.h/.cpp,LootDB.h/.cpp,LootService.h/.cpp,ClientPacketHandler.cpp;
DummyClient/ServerPacketHandler.cpp and regenerated outputs.
New Tests/InventoryDiscardDBTests.cpp.
S1 Account/S1AccountSubsystem.cpp,S1GameInstance.cpp,ClientPacketHandler.cpp,
Network/S1NetworkSubsystem.h/.cpp,Loot/S1InventorySubsystem.h/.cpp,
UI/S1InventorySlotWidget.cpp and regenerated outputs.
New UI/S1InventoryDiscardWidget.h/.cpp,UI/S1InventoryDropZoneWidget.h/.cpp,
Loot/S1InventoryDiscardTests.cpp.
New /Game/S1/UI/WBP_S1InventoryDiscardDialog created and selectively saved.
Existing grid/split/quickslot binaries unchanged. No imported artwork or old UI
redesign; default editable native UMG layout retained for user design work.

## Actual inspection/checkpoint/evidence
- Serial read-only MCP connected, PIEfalse; grid25/split13/quickslots30widgets,
  all clean. User designer changes differ from earlier counts and preserved.
- Live DB read-only metadata: MySQL8.0.45/GameDB/9tables/no FK referencing Item.
- Partial checkpoint: checkpoints/InventoryDiscard_20261014/AssetsBefore holds
  current user-edited grid/split/quickslots, hashes verified. SourceBefore holds
  InventorySubsystem h/cpp,Room.cpp,LootDB.cpp,Protocol.proto (partial only).
- DBBuild.log/DBBuildRetry.log compile passed. Initial fixture lacked effect ID;
  DBTest.log records assertion. Fixed fixture IDs; DBTestRetry.log exit0:
  partial/full/single, separate connection persistence, invalid/excess/bounds/
  foreign/unsupported, stale/duplicate/concurrentCAS, partial/full rollback,
  pending-use reservation/refund protection. Only isolated S1LootTest_20261001
  fixtures mutated; production GameDB not mutated. Success fixtures cleaned;
 9failed-run di_ fixtures also removed with schema/ownership guard.
- GameServerDebugClCompile.log exit0: source compile only, NOT full link.
- S1Build.log latest full game build/link exit0, including final focus refinement
  and new tests source. Missing Network forward declaration fixed on earlier run.
- Final S1EditorBuild.log full build/link exit0, including hard native WBP class
  reference and final geometry/unknown/late-response test additions.
- Actual Saved/Automation/InventoryDiscard_20261014/index.json: succeeded24,
  succeededWithWarnings1,failed0,notRun0 (25total). Discard test Success/errors0/
  warnings0. Tests actual saved WBP5bindings, hard cooker reference, quantity1,
  fractional rejection, stage-next no submission, final quantity text/hidden
  input, single callback; geometry inside/unarranged fail closed; late/unknown
  replies and session reset. Existing Q GameplayCue path fallback warning1.
  Automation exit0 inspected against report, not inferred as rendered gameplay.
- WBP authoring via serial read-only-first MCP; parent S1InventoryDiscardWidget,
  14widgets/5bindings. Compile warnings_as_errors=true passed; dirtytrue reported,
  save only new WBP -> dirtyfalse. Saved checkpoint SHA256
  622118BFA215572C40AF1E247C4D0EB84D74592E917A2136295A12571FB111B7.
  Existing3UI binary hashes unchanged. No broad save-all.
- Temporary hidden authoring editor needed explicit ModelContextProtocol.StartServer
  8000; initial helper had no mutations. Helpers PID3444/4112 were task-owned and
  cleaned up after confirming no modified relevant packages; no user editor or
  GameServer was killed. No project/engine MCP configuration changes.
- Package.log Windows Development Build/Cook/Stage/Package/Archive BUILD SUCCESSFUL,
  AutomationTool ExitCode0 (116.94sec). This run rebuilt the latest S1 game source.
  Actual Cook DevelopmentAssetRegistry includes discard/grid/split/quickslot and
  LobbyMap/LoadingMap/MainMap. Saved/InventoryDiscardPackage_20261014/Windows is
  the new archive; actual archived S1.exe PackageDiscardSmoke.log exit0 lists
  discard/grid generated classes. This is headless class-load smoke, not login/
  rendered play. Existing Auth InputModeUIOnly Non-Focusable Error1 still present;
  no new discard class-load failure observed. No unrelated Auth fix included.

## Remaining gates
Old GameServer PID40464 remained after debugger/console closure, running without
a console window and listening7777. User explicitly approved terminating that
exact process. Verified executable path then stopped only PID40464; final
GameServerDebugBuild.log full build/link exit0. New server PID13812 launched with
existing environment DB credentials (not logged; bootstrap admin env omitted);
user PIE editor PID21836 launched. Windows package validation completed above.
ServerStartup.log confirms Loot boot Ready1 and Potion use Supported1;
ServerStartupError.log length0. Port7777 owned by13812, MCP8000 by21836.
Final serial read-only MCP: PIEfalse, discard parent correct/14widgets/5bindings,
grid/split/quickslot/discard packages all dirtyfalse. New asset loads on reopen.
User replied '테스트 진행 완료' to the four requested1-/2-client PIE checks:
outside inventory drop -> quantity -> final confirmation -> partial/full delete;
single item goes directly to final confirmation; No/Esc/I preserves quantity;
reconnect retains deletion and existing quickslot/move/swap/merge/split works.
Recorded as user-reported acceptance, not agent-executed gameplay or independent
log verification. No screenshots, detailed per-client traces or test logs supplied.
Additional panel-gap/other-UI negatives,2account data-isolation checks beyond the
listed checklist, injected delay/disconnect/commit-loss and authenticated packaged
play are not separately claimed complete. Existing Auth focus issue remains
separate. No commits/pushes or Astra messages in this feature step.
