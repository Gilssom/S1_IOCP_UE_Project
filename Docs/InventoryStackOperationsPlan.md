# Inventory swap / merge / split — proposal, 2026-10-14

STATUS: user approved rules and additive protocol/no-migration scope on2026-10-14.
Source and split WBP implemented; see InventoryStackOperationsImplementation.md
for actual evidence and pending rendered acceptance. Earlier proposal wording
below is retained as approved design history, not fresh test evidence.
Baseline S1 commit8d6e0cc pushed/remote verified. GameServer untouched by Git task.
MCP connected/PIEfalse and GameDB8.0.45/9tables confirmed read-only today.

## Proposed player rules (user confirmation required)

- Keep20usable slots and potion definition20001/MaxStack99;80visible cells unchanged.
- Ordinary drag onto empty cell: existing move contract unchanged.
- Ordinary drag onto same-definition cell: merge min(sourceCount,99-targetCount),
  leaving excess in original cell. Target99/full rejects without changes.
- Ordinary drag onto different definition: swap. Current UI only displays potion,
  so this does not add new item definitions or general inventory rendering.
- Ctrl+drag onto occupied cell: explicit swap, including two potion stacks.
  This makes same-definition swaps testable without adding new content.
- Shift+drag into empty cell: open quantity dialog, integer1..sourceCount-1;
  confirm once -> split, cancel/close -> no packet. SourceCount1 rejects.
- Shift split priority over Ctrl if both held; occupied destination rejects.
- Whole-stack empty movement and out-of-inventory cancel remain unchanged.
- Quick-slots are definitions, so relocation/new ItemId does not invalidate them.
- Discard, floor drop,80capacity expansion, equip, new items excluded.

## Proposed minimal protocol (not generated/implemented)

Keep1033/1034 v1 empty movement unchanged. Append C_INVENTORY_STACK_ACTION1035 and
S_INVENTORY_STACK_ACTION_RESULT1036, optional negotiated inventory_stack_contract_version1
(C_ENTER_GAME new field6 / S_ENTER_GAME new field8). Legacy0 sends no new packet.
Request: nonzero16-byte request_id, action(1swap/2merge/3split),source_item_id,
source_slot,destination_slot,expected_destination_item_id(0for split),
expected_inventory_version,amount(split only1..sourceCount-1),contract_version1.
Response: echoed request/action/source/destination,authenticated character,
decision(committed/rejected/unconfirmed),reason,inventory_version;
created_item_id for split and resulting source/target counts optional diagnostics.
Snapshot remains authoritative and validates actual rows/quantity/placement.
Reasons: invalid,unsupported,stale,source mismatch,target mismatch,full stack,
invalid amount,busy,use reservation,DB unavailable. Final schema names/fieldnumbers
to be fixed before generation, old IDs/fields preserved. Client/server regenerated.

## DB design — no migration proposed

Actual Item has unsigned slot/count, auto-increment ItemId and
UNIQUE(character_id,slot_index). Same CharacterData FOR UPDATE lock/order as
pickup/use/move; version CAS; one operation per client; existing use/pickup/move
gates. Reject any pending ItemUseOperation state2 and zero-count reserved items.
Validate ownership,slot IDs,definition and counts1..99 under lock; no foreign locks.
Swap: choose server-only reserved slot20 after checking no row occupies it; move
source->temporary20,target->source,source->target within ONE transaction. Never
commit/publish temporary20. Failure rolls back; full20-slot inventory still works.
Temporary slot choice is an implementation detail, not21st usable inventory slot.
Merge: update target count, reduce source; when exhausted delete source Item row.
Do not erase historical pickup/use/grant records. Validate actual FK constraints
on journal/ledger tables before implementing source deletion.
Split: decrement source and INSERT owned new item at empty destination; server
auto-increment ID; total quantity preserved. Increment InventoryState.version
exactly once per committed operation, rollback all rows on any failure.
No new DB operation journal in minimal proposal. Exact payload duplicates cannot
mutate twice due expected-version CAS; requestId correlation is NOT durable replay
or changed-payload conflict history. Unknown commit/timeout -> fresh snapshot only,
never blind resubmit/new splitting. Explicit new operation after reconciliation
uses fresh request/current version. Document this contract rather than exactly-once
durable request-history claims. Add journal only with a separate schema decision.

## Planned files/assets

Server: canonical Protocol.proto,Player.h,ClientPacketHandler.cpp,Room.h/.cpp,
LootDB.h/.cpp,LootService.h/.cpp,generated protocol/handlers,isolatedDB/runtime tests,
DummyClient compatibility handler. Existing server changes preserved/uncommitted.
Unreal: Account/S1AccountSubsystem.cpp,S1GameInstance.cpp,network delegates/handlers,
Loot/S1InventorySubsystem.h/.cpp,UI/S1InventoryDragOperation.h,
S1InventorySlotWidget.h/.cpp,grid and coordinator tests; native split dialog class
and editable /Game/S1/UI/WBP_S1InventorySplitDialog proposed.
Checkpoint before new binary WBP,read-only MCP -> dirty report -> selective-save.
Existing grid/slot/quick-slot art and bindings not broadly redesigned.

## Phased implementation / evidence required

1. Confirm rules and additive protocol/no-migration scope.
2. Server DB transactions and isolated fixtures first. Swap back/full20slots,
   merge partial/full/exhaustion and split limits/newID/total count; replay/stale,
   foreign identity,target changes,pending use,rollback and concurrent operation.
3. Packet/session negotiation,UI drag modifiers and focused split confirmation.
   Capture source/version at drag start; revalidate before confirmation.
4. Client/game/editor/server builds and actual UMG/input regression; new package.
5. User PIE persistence,quick-slot/potion regression and2-account independence.
   Explicit delay/disconnect diagnostics; no inferred packaged2client acceptance.
Nothing above is executed validation of swap/merge/split at this stage.
