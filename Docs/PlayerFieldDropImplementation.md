# Player-public field drops — source integration, 2026-10-14

## Current status (supersedes the historical foundation notes below)

Approved: PlayerFieldDropPlan_2026-10-14.md. Player-discarded potion public;
monster loot owner-only. New server/editor binaries are now built and running.
GameDB migration completed by user and3InnoDB tables confirmed read-only.
Current-version rendered1-/2PIE acceptance remains pending.

Latest user acceptance: after UTC fix server restart, user reports the full
requested2PIE public-drop checklist passed: fresh discard appears on both screens,
other player G whole-pickup removes both, unclaimed30s expiry. This is user-reported
visual/interaction evidence, not agent-controlled PIE. Separate pickup prompt WBP
work begins after this accepted gameplay baseline; it does not change DB policy.

Implemented source: additive3tables, character lock/versionCAS, atomic stock
debit/request/drop/outbox, payload-bound replay, independent uncertain-COMMIT
readback. Public pickup locks receiver/drop, accepts entire quantity across
partial stacks/empty slots or rejects unchanged; allocations/grants/request/
version/status commit together. Pending use/refund protection and prior-boot
expiry retained.30s TTL; no refund or persistent field restoration extension.

Canonical Protocol.proto/Struct.proto now generated on server/Dummy/S1:
1039/1040, C/S_ENTER_GAME field8/10 capability1, LootDropInfo.source_kind10.
IDs verified; Protocol.pb.h and Struct.pb.h match server/client hashes. No manual
generated edits. Public snapshots/create/remove filter negotiated clients only.
Room authenticates, uses player XY/current flat floor, enforces rate/pending/
capacity; publishes committed rows. Public pickup requires capability and
existing range/height/static-wall validation. Private branch stays owner-only.
Room read-only recovery bounded35s; late recovery cannot release a newer stock
operation's pending flag. Unknown never retries Create with a new identity.

New client only1039, never permanent-delete fallback; new server old1037
capability0/Unsupported. Existing quantity -> final Yes/No modal reused. Native
text warns anyone can pickup /30s expiry, optional title binding preserves user
art. LootState/WorldEntity/LootActor preserve actual count/source/immutable
metadata, public candidate/hint allows other authenticated players. UI never
parses packets/accessesDB; stock remains snapshot-driven, no optimistic debit.
Grid/HP/Split/Quickslot assets unchanged. No binary save yet for this feature.

Changed server areas: canonical proto; Player.h, Room.h/.cpp, LootService.h/.cpp,
ClientPacketHandler.cpp, PlayerFieldDropDB.h/.cpp, vcxproj/filters;
Tests/PlayerFieldDropDBTests.cpp, DummyClient/ServerPacketHandler.cpp, generated
outputs, codex_md_edit/db/PlayerFieldDropMigration.sql.
Changed client areas: Account/S1AccountSubsystem.cpp, S1GameInstance.cpp,
ClientPacketHandler.cpp/generated outputs, Network/S1NetworkSubsystem.h/.cpp,
Loot/S1InventorySubsystem.h/.cpp, S1LootState.h/.cpp, S1LootActor.h/.cpp,
World/S1WorldEntitySubsystem.cpp, UI/S1InventoryDiscardWidget.h/.cpp and existing
discard/state/actor tests. Discard names retained for compatibility, not old
destruction semantics. No reference-project edits or asset migrations.

Actual evidence at checkpoints/PlayerFieldDrop_20261014:
- User applied isolated S1LootTest_20261001 migration. DBTestApplied.log exit0:
  atomic/replay/conflict/rollback/quantity/source/version/room/capacity,
  multi-stack whole pickup/conservation,2-recipient race, reservation/refund,
  prior-boot expiry. Task fixtures cleaned. Supplied Occluded rejection tested;
  NOT actual runtime wall/PIE validation.
- GeneratePackets.log exit0, expected1039/1040 and matching output hashes.
- GameServerDebugClCompile.log exit0: source compilation, NOT final exe link.
- S1Build.log Development game target Succeeded/exit0: NOT Editor/PIE.
- Read-only MCP: PIEfalse, discard parent correct,13widgets/5inherited, clean.
  DialogHelp not present (optional); no inferred asset modification.
- GameDBBeforeFieldDrop/GameDB.sql208465bytes,9table definitions, completion
  marker, no routing. SHA25673F0FD79DA2965509BC4CBFBCABA724F7DDB44208070AFB9B48F4E91E2FB5544.
  Dump succeeded, old helper manifest executable path failed; manifests repaired
  via apply_patch. Table/data backup only, no routines/grants/PITR/restore test.

Remaining: close editor/server/debugger -> final Debug/S1Editor build and current
automation -> affected WBP compile/dirty inspection/selective save only if needed
-> admin GameDB SELECT DATABASE() + approved migration3tables -> read-only schema
verification -> new binaries restart/startup -> package and user1-/2PIE.
No automatic production DDL; no passwords in tools/reports.

PIE checklist: A outside-drop quantity -> No/Esc/I preserves stock -> Yes
debits once, both screens show exact-count public drop. B G whole-picks -> both
remove -> reconnect retains stock. Self pickup, multi-stack allocation, full
capacity rejection unchanged, simultaneous G once,30s expiry, distance/wall
rejection. Monster non-owner still denied. Use/quickslot/move/swap/merge/split
and combat/HP/death/reconnect regressions. Prior boot must not grant old drops.

Initial pending list above is superseded by the final-build evidence below.
Still unverified: rendered UI/text/floor, old binary compatibility, actual network
race/failure injection and current1-/2PIE. Prior permanent-discard PIE is historical only.
No commit/push/Astra message performed.

## Final build / deployment evidence (same revision)

- User closed editor and approved remaining oldserver PID13812 termination;
  exact Debug exe path validated, only that process stopped. No unsaved editor kill.
- GameServerDebugBuild.log exit0 final Debug executable link.
- S1EditorBuild.log Succeeded/exit0 final DLL link; prior S1 game build exit0.
- UnrealAutomation.log/index.json:24success +1successWithWarnings,0failed,
  0notRun (25total). Existing GameplayCueNotifyPaths warning belongs to Q test;
  field-drop discard/state/actor tests passed. No GPU/net gameplay claim.
- User applied3additive tables to GameDB; read-only information_schema confirms
  PlayerFieldDrop/Request/Grant all InnoDB. Existing9tables not altered.
- New GameServer PID15076,port7777. ServerStartup.log field Supported[1],
  loot Ready[1],potion Supported[1]. Editor PID18968 launched with new DLL.
- Package.log Windows Development BUILD SUCCESSFUL/AutomationTool ExitCode0,
  explicit LobbyMap/LoadingMap/MainMap cook. Archived exe smoke exit0 confirms
  loaded WBP_S1InventoryDiscardDialog_C and WBP_S1InventoryGrid_C.
  Package smoke has the known Auth InputModeUIOnly Non-Focusable error1;
  not introduced/fixed by this feature, not a clean-error gameplay claim.
- Live editor first MCP check refused: launch omitted MCP auto-start flag.
  User ran ModelContextProtocol.StartServer8000; subsequent MCP calls succeeded.
  PIE=false; WBP_S1InventoryDiscardDialog parentS1InventoryDiscardWidget,
  14widgets/7inherited. Affected Blueprint compile warnings_as_errors=true
  completed without tool error (void return); editor log records compile.
  Dirty=false before/after. No broad save or binary modification performed.

Remaining acceptance: new1-/2PIE checklist,
actual packaged gameplay, old-binary compatibility and failure injection.

## Historical foundation stage (before isolated migration completion)

### PIE-discovered field visibility bug / server UTC fix

User reported no field actor after discard. Latest server log has committed
field requests (decision1/reason0), client receives successful result but no
public spawn; monster-owned loot still spawns. Read-only GameDB confirms
session.time_zone=SYSTEM; UNIX_TIMESTAMP(expires_utc) yielded epoch ~32400000ms
behind direct UTC DATETIME conversion. Room suppresses already-expired metadata.
Root cause: public ReadDrop reinterpreted UTC DATETIME in Korean session time.

PlayerFieldDropDB.cpp ReadDrop now uses the same TIMESTAMPDIFF epoch calculation
as existing LootDB.cpp. No DB schema/data, client or asset change. Added isolated
test explicitly switching +09:00/+00:00, asserting identical epoch and future
initial <=30s lifetime. DBBuildTimezoneFix.log exit0; DBTestTimezoneFix.log exit0
includes new time-zone regression plus all original public drop transaction
tests; task-owned fixtures cleaned. Old suite lacked the Room UTC lifetime check.

Server source compile and final Debug link passed (exit0); user instructed PID
15076 termination, exact path validated and only that process stopped. New server
PID18680 listens7777; ServerTimezoneFixStartup.log field Supported1,loot Ready1,
potion Supported1,stderr empty. Existing editor MCP responds,PIEfalse. Repeated
visual1-/2PIE remains pending for this fix. Existing invisible rows are not
refunded/recreated/extended; no automatic
stock adjustment. On next boot existing prior-boot unclaimed rows expire per
approved policy. No claim that earlier generic PIE accepted this feature.

User approved PlayerFieldDropPlan_2026-10-14.md new1039/1040/capability/source-kind
fields and3additive tables. Player-discarded potion only public; monster-owned
loot unchanged. No new content rules, world height expansion or migrations from
reference projects. Runtime is STILL the old permanent-delete build; do not test
field drop until native/server integration and restart are complete.

## Current changes (not yet integrated into runtime)
- New codex_md_edit/db/PlayerFieldDropMigration.sql: three additive InnoDB tables,
  request replay/source fingerprint, public drop lifecycle, per-allocation grants.
  No existing table ALTER/delete and no automatic DDL.
- New server PlayerFieldDropDB.h/.cpp: character lock/versionCAS, original stock
  decrement + durable request + public drop insert + outbox in one transaction.
  Exact replay returns previous result, payload conflict rejects without new drop.
  Read-only result retrieval is available for ambiguous commits.
- Pickup locks receiver then drop; sums all compatible-stack and empty-slot
  capacity, grants entire quantity or rejects with no partial changes. Grant
  allocations and ItemPickupRequest share transaction with status0->1 and one
  inventory version bump. No owner restriction for public rows.
- Pending potion consumption/refund protection; no monster death/spawn IDs
  fabricated. Public outbox types/sourceKind2 distinguish provenance; absent
  monster source IDs use zero sentinels, as existing missing-source diagnostics.
- Expiry/previous-boot recovery functions; schema presence/required columns/
  unique/primary keys checked separately, ready gate to be connected next.
- New Tests/PlayerFieldDropDBTests.cpp: isolated-only fixtures, atomic rollback,
  durable replay/conflict, whole pickup across multiple stacks, quantity
  conservation/capacity/wall/room/foreign/version checks,2recipient race,
  reservation/refund, prior-boot expiry. Require new test tables empty before
  boot-recovery test; cleanup only task-owned characters/drop UUIDs.

## Evidence and boundaries
Partial source and current discard UI checkpoint PlayerFieldDrop_20261014
created; UI backup hash matched. Existing staged/uncommitted work preserved.
DBBuild.log compiled standalone DB foundation/test executable successfully
(existing ServerCore LNK4075 warning). Cleanup-bounds refinement final standalone
build DBBuildFinal.log also exit0. DBTest.log exit4: missing isolated migration,
no fixtures created/no transaction tests run. This is NOT GameServer full
integration/build or gameplay test success.
Requested administrator apply migration ONLY to S1LootTest_20261001 first, with
SELECT DATABASE() confirmation. GameDB schema/data not modified by this step.
No packets generated, source_kind applied, live gameplay changed or BP saved yet.

## Next steps / unverified
Wait for isolated migration -> run DB tests/repair evidence -> add protocol and
Room/LootService negotiation/create/pickup/expiry/legacy filtering -> client/UI
switch (never fall back to permanent delete) -> builds/tests -> production
backup and administrator migration gate -> restart -> actual1-/2client PIE.
Full builds, schema applied, DB transaction tests, Unreal automation, packaging,
rendered pickup/reconnect/monster ownership regressions remain unverified for
this feature. No commit/push or Astra messages performed.
