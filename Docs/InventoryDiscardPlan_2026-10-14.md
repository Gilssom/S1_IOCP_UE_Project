# Inventory permanent discard — proposal, 2026-10-14

Status: user approved this contract with '진행'; source implementation in progress.
Protocol and native code changes started. No schema migration or Blueprint edit
performed yet. Actual evidence is tracked in InventoryDiscardImplementation.md.

## Observed baseline
- Unreal MCP connected, PIE=false. GameServer PID40464 / Editor PID26720 observed.
- Read-only ODBC: GameDB MySQL8.0.45,9tables, incoming foreign keys to Item=0.
  No passwords/account rows emitted, no production data mutations.
- Grid parent S1InventoryWidget,25widgets; split parent S1InventorySplitWidget,
  13widgets; quick-slots parent S1QuickSlotWidget,30widgets. All3packages clean.
  These counts supersede earlier records for this read only; user design edits
  are preserved, not reverted to old checkpoint files.
- Current inventory20usable/80visible, potion20001/99cap.
- Protocol1035/1036 action1..3 only, negotiated stack contract1. Do not silently
  introduce discard action4 under v1: old code rejects it and result validators
  assume a destination with positive count.
- Current DB uses character-row lock, inventory version CAS, use-reservation
  protection and asynchronous worker. Merge already removes exhausted Item row.
- Source-only Git: prior uncommitted stack work exists; server prior staging
  preserved. No commit/push requested for this step.

## Authorized feature intent and proposed UX
User requested permanent deletion now, future floor-drop expansion excluded.
Inventory-origin potion dragged outside actual inventory panel (not merely
outside a cell) opens discard modal. Drops inside panel gaps, locked cells,
self cells, other UI, quick-slots, drag cancel/Esc/teardown must not delete or
open destructive confirmation via generic DragCancelled.
- Count1: final confirmation immediately.
- Count>=2: integer quantity1..current count, recommended default1; selection
  advances to separate final confirmation with item name and quantity.
- Final text: 정말 포션 N개를 버리시겠습니까? 영구 삭제되며 복구할 수 없습니다.
  Yes/No buttons (예/아니오). No/Esc/I/backdrop cancellation sends no packet.
  Default focus is No; quantity confirmation must not also trigger final Yes.
- Revalidate item/slot/version/count/window/session before each stage and submit.
- While modal/pending, block competing stock actions; do not optimistically
  remove items. UI images and existing split bindings/design remain unchanged.
- Retain definition-based quick-slot registration. Show zero/unavailable if the
  last potion was deleted, instead of silently clearing user's assignment.

## Proposed packet contract — NOT implemented
Append new messages at canonical Protocol.proto tail; generated IDs expected
1037 C_INVENTORY_DISCARD /1038 S_INVENTORY_DISCARD_RESULT, verify generation.
No reordering or renumbering existing messages.
C fields: request_id bytes1(nonzero16), source_item_id uint64/2,
source_slot uint32/3(0..19), amount uint32/4(1..current count),
expected_inventory_version uint64/5, contract_version uint32/6(1).
S fields: request_id bytes1, authenticated character_id uint64/2,
source_item_id uint64/3, source_slot uint32/4, requested_amount uint32/5,
decision uint32/6(1committed/2rejected/3unconfirmed),
reason uint32/7(0none/1invalid/2stale/3sourceMismatch/4amount/5busy/
6DBUnavailable/7unsupported),
inventory_version uint64/8, remaining_count uint32/9(0fullydeleted).
Negotiate inventory_discard_contract_version1 independently:
C_ENTER_GAME append field7; S_ENTER_GAME append field9.
Legacy missing0 disables discard only; existing move/stack/pickup/use unchanged.
Client requests through InventorySubsystem -> Network; server authenticated
Room -> LootService -> DB worker, result -> typed Network -> InventorySubsystem.
Never use client-provided character identity.

## Proposed DB contract — no schema migration
In one transaction, lock CharacterData row, check version, ownership/current
source slot, potion definition, positive valid quantity and pending use/refund.
Partial discard UPDATE count; full discard DELETE exact owned Item row.
Check affected row count1. InventoryState version increments once, atomically.
Retain shared room pending/rate-limit guards for stock operations.
Rollback leaves count/version unchanged.
Same-version duplicate/concurrent operations cannot apply twice. This is CAS
protection, NOT a durable exactly-once replay journal or same-ID payload history.
Commit loss/timeouts: unknown, fresh snapshot only, no automatic resend.
Late result cannot clear newer session/request. Already-submitted DB operation
may commit after disconnect; reconnect reads actual persisted state.
No HP/heal/world-loot changes; no deletion of pickup/use history.

## Exact expected files
Server root E:/Git Fork/GameServer/GameServer:
Common/Protobuf/bin/Protocol.proto; GameServer/Player.h;
GameServer/Room.h/.cpp; GameServer/LootDB.h/.cpp;
GameServer/LootService.h/.cpp; GameServer/ClientPacketHandler.cpp;
DummyClient/ServerPacketHandler.cpp; generated outputs from existing generator.
New Tests/InventoryDiscardDBTests.cpp.
Client root E:/Git Fork/S1_UE5.8/Source/S1:
UI/S1InventoryWidget.h/.cpp (outside-panel drop receiver);
UI/S1InventoryDragOperation.h and UI/S1InventorySlotWidget.cpp as needed;
Loot/S1InventorySubsystem.h/.cpp; Network/S1NetworkSubsystem.h/.cpp;
Account/S1AccountSubsystem.cpp; S1GameInstance.cpp; ClientPacketHandler.cpp;
generated ClientPacketHandler.h and Network/Protocol.pb.h/.cc;
new UI/S1InventoryDiscardWidget.h/.cpp;
new Loot/S1InventoryDiscardTests.cpp.
Assets proposed: new /Game/S1/UI/WBP_S1InventoryDiscardDialog;
minimal drop-receiver wiring in WBP_S1InventoryGrid only if required after
native implementation inspection. Do not edit split/slot/quick-slot designs.
Docs: this plan + implementation record + CurrentAssetRegistry.
Before binary changes, capture current user-edited assets with verified hashes;
report dirty packages and save only intended ones.

## Verification plan, not yet run
Isolated S1LootTest_20261001 DB fixtures only: partial/full/single quantity,
0/negative-equivalent/overflow/excess, foreign source/slot/stale version,
pending use/refund, duplicate and concurrent CAS, mid-operation rollback,
separate-connection persistence, unrelated rows unchanged.
Client automation: panel-gap/quick-slot/cancel never discard; two-stage safety,
integer validation; stale modal and old response/session cleanup;
nonoptimistic state, unknown snapshot recovery, no repeat submission.
ServerDebug + S1 + S1Editor builds, affected WBP compile, cook inclusion for
new dialog (hard native reference after asset creation), package smoke.
User PIE: count1 / count>=2 partial+all, No/Esc/I cancellation, reconnect,
two account isolation and potion/quick-slot/move/swap/merge/split regression.
Do not claim unexecuted PIE/package/fault tests passed.

## Approval gate
Approve the above dedicated1037/1038 contract, enter-game capability fields,
no schema migration, permanent deletion and two-stage confirmation.
Quantity default1 is recommended to reduce accidental mass deletion.
After approval implement; restart server/editor when applying native/protocol
build, request user closure rather than forcibly terminating processes.

