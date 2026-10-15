# Inventory swap / merge / split implementation — 2026-10-14

## Approved scope and actual player behavior

User approved20usable slots/potion99max, Ctrl occupied-cell swap, ordinary
same-definition merge up to99 with remainder retained, Shift empty-cell split
with quantity1..sourceCount-1. Shift takes priority over Ctrl. Split default is
floor-half (minimum1), an ordinary UI starting value, never automatic consumption.
Ctrl into an empty cell retains existing whole-stack move. Shift occupied/self/
locked/outside/cancel cannot mutate. Other-definition swap is supported by server
and coordinator but no new definitions/production assets or generic inventory
rendering added. Existing UI still displays potion20001 only.
Double-click with Shift/Ctrl does not consume; unmodified double-click unchanged.
Quick-slots remain definition-based; all stock changes come from server snapshot.
No discard,world drop,capacity80expansion,quest or combat changes included.

## Packet and DB contract actually implemented

Protocol.proto canonical source adds1035C_INVENTORY_STACK_ACTION and1036result,
enter-game stack support version1 (C newfield6 / S newfield8). Existing1033/1034
empty-slot move remainsv1 unchanged. Generated server/DummyClient/S1 outputs via
GenPackets.bat; no manual generated edits. New support negotiated only if loot
service ready. Legacy peers disable stacks while old movement/loot remain valid.
Request carries16-byte random request,action1swap/2merge/3split,source item/slot,
destination slot+expected targetID(0split),expected version,split amount,contract1.
Response echoes authenticated identity and action/source/slots; decision1commit/
2reject/3unknown,reason0none/1invalid/2stale/3source/4target/5full/6amount/7busy/
8DB/9unsupported/10type/11reservedtemporary;version,new splitID and counts.
source_count describes remaining source ItemId (0if consumed by merge);
destination_count describes actual final destination slot, including swap.

No migration or new tables. Live and isolated schema inspection confirmed no FK
referencing Item, only Item->CharacterData and UNIQUE(character,slot). Source
deletion cannot cascade/remove historical pickup/use/grant records.
CharacterData FOR UPDATE lock order retained; expected version CAS; reject
pending state2consumption/refund reservations and invalid/zero-count source/target.
Swap moves via internal slot20 within ONE transaction, including full20slots;
checks temporary occupancy and never commits/exposes slot20. Merge changes both
counts/deletes only exhausted source. Split decrements source and inserts new
auto-increment ItemId. Version increments once, all quantities conserved.
Early/mid-failure rolls back. Test-only forceRollback seam never comes from packets.
Same-version duplicate/race cannot mutate twice; no durable request-replay journal
or changed-payload request-history guarantees claimed. No automatic resend after
unknown commit/timeout; fresh snapshot reconciles. Other outstanding use/move
locks and room200msrate limit retained. Reply only to same current ObjectId/character;
disconnect may leave a requested committed durable operation, reconnect queries it.

## UI and lifetime

New /Game/S1/UI/WBP_S1InventorySplitDialog parent S1InventorySplitWidget,
14widgets/4nativebindings: AmountInput(SpinBox),ConfirmButton,CancelButton,RangeText.
Editable280wide centered dialog, dim full-screen input-blocking backdrop. Native
Slate fallback retained for unavailable WBP. Initial authoring used a soft class
path to avoid missing FClassFinder's empty-package issue while asset did not exist.
First package succeeded but actual cook registry excluded the new dialog. After
asset creation, replaced with the same hard FClassFinder/TSubclassOf dependency
pattern as grid/quick-slot. Final editor/game build and hard-reference test passed,
actual cook registry includes dialog and archived executable loads its generated
class. Do not mistake packaging success alone for dependency inclusion.
RootFocusable; cancel/Esc/I close without request; Enter on root confirms, but
child buttons retain their own keyboard semantics. Integer quantity checked again
before request. Image designs/native grid/slot bindings unchanged.
Drag captures modifier,source,version and weak owner. Coordinator revalidates
owner/snapshot/IDs/counts/slot/type/amount/capability/busy before submit. While modal
or operation pending, inventory drag,use,pickup,quick-slot registration blocked.
Local assignment clearing does not alter any outstanding stock operation.
Closing inventory/session cancels modal and unbinds callbacks; accepted new
inventory version invalidates an open stale dialog. Old or foreign responses
cannot clear newer requests. Five-second operation timeout -> one read, not resend.
Pre-commit snapshots cannot unlock; matched current snapshot unlocks and updates
positions/counts. Failed query remains gated and I close/reopen retries read only.

## Files and checkpoint

Server: Protocol.proto,Player.h,LootDB.h/.cpp,LootService.h/.cpp,Room.h/.cpp,
ClientPacketHandler.cpp,DummyClient/ServerPacketHandler.cpp,generated outputs;
new Tests/InventoryStackDBTests.cpp.
Unreal: InventorySubsystem.h/.cpp,InventoryDragOperation.h,InventorySlotWidget.cpp,
NetworkSubsystem.h/.cpp,ClientPacketHandler.cpp,AccountSubsystem.cpp,S1GameInstance.cpp,
new UI/S1InventorySplitWidget.h/.cpp,new Loot/S1InventoryStackTests.cpp,generated outputs.
Checkpoint InventoryStacks_20261014: server before-files/proto and original grid/
slot/quick-slot binaries backed up and hashes matched. Existing3binary assets
after authoring remain SHA-identical and clean. New split asset compile with
warnings_as_errors=true passed; dirty=true reported, selected-save=true -> dirty=false.
Saved split asset checkpoint SHA256 C5FD32A6E44EFFE13BF4998286D085095D04B90753C4C6C98D353DC7D009FE46.
No save-all or old-WBP redesign. New changes are not committed/pushed; previous
baseline8d6e0cc remains remote. Original GameServer staging preserved.

## Actual evidence / remaining gates

Isolated DBTest.log: partial30+80=>11+99,full10+80=>source removed/90,persistence
through second connection,split10=>6+4/newID,invalid/zero/whole/overflow/single/
occupied bounds,foreign/source/target/type rejection,full20slots different-definition
swap/back,all3mid-operation rollbacks,concurrentCAS,max1commit,pendinguse/refund.
Fixtures cleaned in S1LootTest_20261001 only; production GameDB not mutated by tests.
Server Debug full build/link and S1/S1Editor full build/link exit0. Initial Windows
min macro,Slate header path and fallback layout bracket errors fixed; failure logs
retained. Latest server diagnostic-count patch revalidated via DB/build before launch.
Automation index Saved/Automation/InventoryStacks_20261014/index.json:24success
(23plain +1existing Q warning),failed0,notRun0. New StackCoordinator warnings0/errors0.
Actual saved split WBP tested:binding,half default,exactquantity,fraction reject,
Esc cancellation; coordinator bounds/identity/version/busy,oldresponse/current
snapshot,non-optimistic state,fulltargetswap,unknown and reset. Physical mouse input
path/network server session is NOT proven by delegate/NullRHI tests.
SDK validation subprocess overlapped game UBT mutex; actual automation report
completed and inspected, not inferred from exit0 alone. First packaging BUILD
SUCCESSFUL/exit0 but split dialog was ABSENT in actual DevelopmentAssetRegistry;
all3maps and existing grid/slot/quick-slot present. Therefore first package is not
accepted as a complete deliverable. After user editor shutdown, hard-reference
correction S1/S1Editor builds passed; rerun24tests(23plain/1existing Qwarning),
failed0/notRun0,StackCoordinator errors0/warnings0 including hard-class equality.
Final Package.log BUILD SUCCESSFUL/exit0; actual DevelopmentAssetRegistry has
split/grid/slot/quick-slot +Lobby/Loading/Main. Actual archived S1.exe headless
PackageSplitSmoke.log lists split/grid WidgetBlueprintGeneratedClass objects and
Quit exits0. This is class-load smoke, NOT rendered/authenticated gameplay.
Existing Auth InputMode:UIOnly Non-Focusable Error1 persists in smoke; no new
split-class load error claimed. Archive Saved/InventoryStacksPackage_20261014/Windows
now supersedes incomplete first output; previous failure evidence retained.
New server PID40464 started with LootReady1/UseSupported1; final editor PID26720
relaunched after verification. No unsafe process kill/save-all performed.
User confirmed the four requested PIE checklist items on2026-10-14: Shift empty-cell
split/quantity/cancel, Ctrl occupied-cell swap, normal same-potion merge, and
reconnect slot/count persistence with existing quick-slot/potion use intact.
This is user-reported gameplay acceptance; client count was not specified.
Authenticated packaged2account play and live delay/disconnect/commit-loss tests
remain unverified. No new
MCP helper/runtime editor dependency introduced. Known Auth focus issue unchanged.

## User acceptance after new server/editor start

Collect at least2potions; I->Shift-drag to empty cell->choose quantity/confirm.
Check two different stackIDs,count total; Ctrl-drag stacks swaps positions/counts.
Ordinary drag merges,exhausted source disappears; repeat split/cancel/Esc then
double-click and quick-slot1..4 use/register/rightclear. Reopen/reconnect retains
counts/slots; occupied Shift/self/locked slots rejected. For2clients verify each
account's inventory independence and unchanged combat/loot/heal results. Partial
99limit/full20case already isolated-tested; don't claim user reproduced them until
reported. Final visual style of split dialog is user-owned and still pending.
Hard-reference packaging/automation correction completed; the four requested PIE
checklist items are user-confirmed. Extended checks above are not all implied by
that confirmation; separate1-/2-client coverage and final visual-design approval
were not specified.
Final serial read-only MCP: PIE=false, grid/slot/quick-slot/split packages all
dirty=false, split parent=/Script/S1.S1InventorySplitWidget,14widgets/4bindings.
ObjectTools does not expose the private subsystem class property for readback;
hard-reference equality automation and actual packaged class-load evidence used
instead, without adding a runtime/MCP accessor or claiming unsupported inspection.
