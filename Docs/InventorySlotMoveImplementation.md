# Inventory empty-slot relocation — 2026-10-12

## Approved scope / current status

User approved server/client protocol changes and durable DB slot position updates.
Move an existing stack into an empty slot within indices0..19; no quantity change,
swap, merge, split, discard or80-slot capacity expansion. Existing grid design and
quick-slot definition assignments stay unchanged. Source implementation added;
build and hands-on acceptance status must be read below, not inferred from prior UI.
After user stopped server debugging, verified zero running GameServer processes,
retried Debug full build/link successfully (GameServerDebugBuild.log exit0).
New server PID21236 and editor PID45752 launched. Server DB boot Ready[1], potion
Supported[1], port7777 owned by new server; editor MCP responds, PIE=false.
User subsequently reported all requested PIE acceptance checks complete (2026-10-12).
This is user-reported validation, not agent-executed gameplay.

## Ownership and packet contract

Canonical GameServer/Common/Protobuf/bin/Protocol.proto adds only append-only
C_INVENTORY_MOVE1033 / S_INVENTORY_MOVE_RESULT1034. Existing IDs unchanged.
C/S_ENTER_GAME add inventory_move_contract_version5/7 respectively. Negotiate1
only with a ready loot service; legacy peers default0 and never send new requests.
All Protobuf and packet-handler headers regenerated through GenPackets.bat, not
manually edited. Server, DummyClient and S1 outputs generated together.

Request: nonzero16-byte request_id, item_instance_id, source_slot,
destination_slot, expected_inventory_version, contract_version1.
Response echoes request/item/source/destination and authenticated character_id;
decision1committed/2rejected/3unconfirmed, inventory_version and reason:
0none/1invalid/2stale/3missing-or-foreign-or-source-mismatch/4occupied/
5pending-use-or-busy/6DB-unavailable/7unsupported. No client-supplied owner.
Acceptance is not a quantity or HP event and not broadcast to other players.

## DB / concurrency

No migration/new table. Reuse Item.slot_index and InventoryState.version.
Dedicated AsyncLootDB worker; CharacterData row lock precedes inventory/item work,
matching pickup/use lock order. Validate expected version; reject active state2
ItemUseOperation (pending refund), nonexistent/foreign/wrong-source item and
any destination row, including zero-count refund reservations. Update slot and
increment inventory version atomically, commit before reporting success.
No cross-owner item row locks. Concurrent same-version requests: at most one commit.

No durable movement request journal is added. Exact duplicate payload is rejected
by its old expected version after the first commit and creates no second movement.
Request ID correlates client responses; it does not promise durable response replay
or payload-conflict history across boots. Uncertain COMMIT remains unconfirmed.
No automatic request retry; read current inventory instead. New intentional moves
use new request IDs/current versions. Room rate limit200ms, one pending move/player,
existing pending consumption blocks relocation. Completion replies only to the
same current ObjectId/character session; a disconnected operation may still commit
its requested durable position, and reconnect reads the authoritative snapshot.

## Client UI / recovery

Drag payload captures item ID, source slot, current inventory version and weak
source window. Empty unlocked cell handles NativeOnDrop; coordinator revalidates
active window, item/source, version, empty destination and all common busy gates.
UI never relocates optimistically. Pending move blocks use/pickup/register/new move;
clearing local quick-slot assignment remains independent and cannot cancel a move.
Matched response triggers a fresh inventory query; committed result establishes a
minimum version before UI unlocks. Old/wrong-character/wrong-item response ignored.
Five-second timeout means delay/unknown, not rejection; submit one fresh read,
never resend the movement. Failed read keeps action gates locked; closing/reopening
I retries only the read. Snapshot success clears reconciliation; session reset
clears old requests/capability. GameAndUI and existing combat input isolation kept.

## Changed files

Server: Protocol.proto, generated protocol/handler outputs, Player.h,
LootDB.h/.cpp, LootService.h/.cpp, Room.h/.cpp, ClientPacketHandler.cpp;
new Tests/InventoryMoveDBTests.cpp. No server project-source entry needed.
Unreal: UI/S1InventoryDragOperation.h, S1InventorySlotWidget.h/.cpp,
Loot/S1InventorySubsystem.h/.cpp, new S1InventoryMoveTests.cpp,
Network/S1NetworkSubsystem.h/.cpp, ClientPacketHandler.cpp, Account/S1AccountSubsystem.cpp,
S1GameInstance.cpp and regenerated network protocol/handler outputs.
No Blueprint/map binary changes this stage. Existing user edits preserved.

## Verification / pending

Read-only MCP: editor reachable, PIE=false, saved slot WBP parent=S1InventorySlotWidget,
6widgets/3bindings. Server Debug ClCompile passed (ServerCompile.log); final Debug
link initially failed LNK1168 because original server still running; after user
stopped debugging, retry succeeded. First compile invocation lacked
SolutionDir and failed CorePch.h lookup; corrected invocation succeeded.
Isolated S1LootTest_20261001 DB test passed empty-slot persistence via separate
connection, count invariant, exact replay/version/source/bounds/occupied rejection,
concurrent CAS. Production GameDB not mutated by tests. Additional pending-use
reservation rejection/refund/count restoration test also passed (DBTest.log).
Client game build initially failed a missing response forward declaration; fixed,
S1 game and S1Editor full compile/link exit0. Initial automation22pass/1fail was a
test fixture's missing MoveItem on the second synthetic request, not a runtime
failure; fixed and rerun:23success/0failed/0notRun, MoveCoordinator warnings0/errors0.
Evidence: Saved/Automation/InventoryMove_20261012/index.json and UnrealAutomation.log.
No Error/Fatal/Ensure/Blueprint Runtime Error found in the final automation log.
Windows Build/Cook/Stage/Package/Archive passed (Package.log BUILD SUCCESSFUL,
AutomationTool exit0), archive Saved/InventoryMovePackage_20261012/Windows; all3maps.
Packaging predates only the test-fixture correction (runtime code unchanged).
DummyClient receives the added compatibility no-op result consumer as well.
Checkpoint/scripts/logs: InventoryMove_20261012 under workspace checkpoints.
New client test covers response matching, no optimistic update, pre-commit snapshot,
unlock by committed snapshot, old response vs newer move, unknown and session reset.

Manual acceptance after restart: I drag potion0->19->empty0, close/reopen and reconnect
position/count unchanged; reject occupied/locked/self drop; quick-slot register/clear
and1..4use preserved; no combat while UI; two accounts' inventories independent.
New editor read-only MCP confirmed grid/slot/quick-slot native parents, all3packages
dirty=false, quick-slot RootCanvas still SelfHitTestInvisible. No Error/Fatal/Ensure/
Blueprint Runtime Error found in new editor startup log. ServerStartup.log contains
DB readiness evidence. ServerStartupError.log retained. No gameplay/credentials
entered by agent. User reported the complete requested PIE checklist passed:
drag into empty slots1-20; close/reopen and reconnect preserve location/count;
locked/occupied destinations rejected; existing quick-slot registration,
instant right-click clear and potion use remain normal. Client count was not
specified in this confirmation; do not infer a distinct two-client run.
Self-drop, UI combat isolation and account separation were not separately requested
in the final checklist and are not inferred from this response. Injected delay/
disconnect fault tests and authenticated packaged play remain unverified.
No production DB test writes, no asset modifications/save-all, no Git commit/push.
