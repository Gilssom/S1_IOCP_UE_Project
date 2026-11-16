# Village ↔ Main portal transfer — review draft

Status: user approved the overall implementation. A server-side definition parser
and its independent tests have been added; this is not yet wired to startup or
gameplay. The two saved map portal IDs were assigned as noted below. No new packets,
region transfer, fade, Blueprint class edits or DB changes have been implemented yet.
The user requested overlap-triggered travel with fade-out, LoadingMap and fade-in.

## Current evidence

- Saved VillageMap has one BP_S1Portal, PortalId=0, root location (4090,2480,260),
  root Pitch=-90 degrees. Saved MainMap has one BP_S1Portal, PortalId=0, root
  location (-340,-1240,180), root Roll=-90 degrees. Both map packages were clean.
  Values are actor root-relative transforms read in the editor, not verified
  ground heights, portal entrance centers, authoritative volumes or safe arrival points.
- TriggerVolume is attached to PortalRoot, local center (0,0,100), extent
  (150,150,100). Consequently actor pitch/roll rotate both its offset and box.
  These placements must be checked in world space; do not silently export the root
  position as an upright portal trigger or assume the root is at ground height.
  The requested Niagara appearance should be preserved when correcting collision.
- Portal IDs were initially zero. After user approval, Village was assigned ID 1,
  Main was assigned ID 2. Both were edited through MCP, dirty state reported before
  individually saving each map. Village ID1 and Main ID2 were re-read after reopening,
  and the saved packages were clean. Before-change map SHA-verified copies exist under
  `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PortalTransfer_20261115`.
  Their trigger transforms/visuals were not changed and no server definition was deployed.
- Read-only editor vertical traces at Village XY (3690,2480), (4490,2480),
  (4090,2880) hit approximately Z=100; (4090,2080) first hit approximately Z=792.7.
  Main XY (-740,-1240), (60,-1240), (-340,-1640), (-340,-840) hit Z=-0.5.
  These first-hit line traces are only candidates; they do not prove clearance for a
  player capsule, reachable navigation, safe view, or server geometry compatibility.
- Current LevelFlow only permits SetInitialRegion and BeginMainLevelTravel in Lobby;
  gameplay-to-gameplay travel needs explicit states and cannot reuse this entry
  path unchanged. Existing loading and pending-spawn mechanisms are reusable pieces.
- Server has separate Village and Combat Room job queues and routes current player
  actions via player->room. Initial login always chooses Village. Existing Room entry
  also sends initial-login responses, so invoking it as-is is not a complete transfer.
- Existing server quest definitions include NPC1 only for Village. Combat region has
  saved nav/loot blockers and three deployed monster points. Map changes, DB state,
  in-flight actions and old async callbacks require explicit lifecycle checks.

## Proposed UX and authoritative data

- Local-player overlap initiates at most one pending request; no mouse/keyboard input.
  The client sends a nonzero portal ID and unique transfer request ID, never a chosen
  map path or destination position. Effect/fade are client presentation only.
- Export saved portal IDs, source region, source world-space query shape and fixed
  destination capsule-center position/yaw to a separate validated trusted server file.
  IDs must be unique; hard limits, region pair and finite coordinates fail closed.
  Arrival positions must be outside the opposite portal trigger plus a safety margin.
  Choose/verify both arrival points in editor and with player capsule/floor collision;
  collision/nav geometry and Z are not yet verified.
- On valid request server checks session, actual source Room, server-held position,
  trigger bounds, alive/state rules, no pending transfer, trusted portal definition,
  source/destination allow-list and protocol capability. Server-held movement is not
  necessarily anti-cheat validated, so this is not a complete anti-cheat solution.
- Proposed default: accepted -> lock input and world actions, fade out, LoadingMap,
  server transition and fresh destination snapshot, then fade in when client world,
  local actor and required initial snapshot are ready. On rejection restore input,
  no forced fade or disconnect. Avoid repeatedly requesting while still overlapping;
  arrival outside the target plus a temporary exit/rearm guard prevents bounce loops.
- Exact fade duration, combat/pending DB work eligibility, readiness timeout/recovery
  and visual placement are tuning/acceptance choices to finalize before implementation.

## Proposed packet roles — implementation not yet completed

Use canonical `GameServer/Common/Protobuf/bin/Protocol.proto`, regenerate both ends;
do not hand-edit generated files. Existing C/S_ENTER_GAME remain initial login only.

1. C_PORTAL_TRANSFER_REQUEST: contract_version, request_id, portal_id.
2. S_PORTAL_TRANSFER_PREPARE: request_id, accepted/reason, transfer_id, source region,
   destination region and a server-issued generation. No client-supplied position.
3. C_PORTAL_TRANSFER_READY: transfer_id/generation after LoadingMap is active.
4. S_PORTAL_TRANSFER_COMMIT: transfer_id/generation, server destination position,
   local entity and full destination snapshot or an explicitly delimited snapshot.
   Destination ready/ACK details are unresolved until packet ordering is audited.

The user approved the complete portal implementation, including the necessary
protocol changes. Names and exact field numbers, version negotiation, IDs, reconnect
and replay handling still require implementation and verification. No DB schema change is assumed. Reconnect
continues Village initial spawn unless separately approved. Avoid reusing C_ENTER_GAME
success as if it were a room-transfer acknowledgment.

## Safety / lifecycle decisions that need source-level implementation and tests

- Single serialized ownership transfer across the two Room queues. A player may not
  belong to both Rooms and an accepted request must not create duplicate actors.
  Freeze/invalidate source action reservations and old async completions; preserve or
  explicitly reject in-flight inventory/quest operations rather than losing results.
- Event boundary is not proven yet. Source Room broadcasts and asynchronous callbacks
  can race with transfer responses; TCP ordering alone is insufficient if old sends
  can be queued after the boundary. Audit all room-scoped sender paths and choose a
  proven barrier/quarantine strategy or explicit region-generation tags; inventory and
  session-level events must not be accidentally dropped. Do not mark this solved.
- If disconnect, timeout or failure occurs between removal and insertion, choose and
  test deterministic ownership and cleanup; do not leave a ghost in either Room.
  Duplicate/reordered READY/REQUEST must be idempotent for the current session only.
- Protect user identity and server logs; never put account secrets into a request or log.

## Expected change areas

Client: `Source/S1/Game/S1Portal.cpp`, `Source/S1/Public/Game/S1Portal.h`,
`Source/S1/Level/S1LevelFlowSubsystem.*`, `Source/S1/S1GameInstance.*`,
`Source/S1/Network/S1NetworkSubsystem.*`, `Source/S1/World/S1WorldEntitySubsystem.*`,
loading/fade UI as needed, generated Protocol outputs and scoped tests.
Server: canonical Protocol.proto and generated/dispatch, `ClientPacketHandler.*`,
`Room.*`, `GameServer.cpp`, region/portal definition parser/export and tests.
Selected saved maps and BP may require edited IDs/trigger transform/arrival marker;
checkpoint and report dirty packages before save. Exact list will be finalized after
packet/order audit; DB migrations and new transport layers are outside this draft.

## Partial implementation — offline server definition validation

New `GameServer/GameServer/PortalDefinition.h` defines a bounded parser for a
separate offline `S1PORTAL 1` definition and an oriented-box point test. It rejects
nonfinite/oversized data, nonpositive/duplicate IDs, invalid region pairs,
non-orthonormal axes, invalid extents, broken reciprocal pairs, trailing input and
destinations within a conservative 100cm margin of the reverse portal box. The
clearance check is not a replacement for world collision or ground testing.
It is not loaded by the running GameServer and authorizes no transfer.
`GameServer/Tests/PortalDefinitionTests.cpp` uses synthetic positions only.
`GameServer/Tests/BuildPortalDefinitionTests.cmd` built and ran 11 checks, 0 failures,
including a rotated box and non-finite query point.
At this point no real portal definition had been exported or deployed; see the
subsequent saved-geometry export below.

## Saved portal geometry export — 2026-11-15

`S1ExportPortalDataCommandlet` reads the saved VillageMap and MainMap portal box
component transforms, including the actors' rotations, and exports an offline
candidate without saving either map. It requires explicit arrival coordinates.
The S1Editor Development build succeeded, and the commandlet exported both portals
to the checkpoint file `portal-candidate.s1portal`. The server parser accepted its
structure and reciprocal portal clearance; the synthetic parser and transition
guard tests passed (11 and 15 checks respectively at that revision). These checks do not validate
walkable ground, the player capsule, world obstacles, or actual transfer.

Candidate trigger centers are Village (4190, 2480, 260) and Main
(-340, -1340, 180); they differ from actor origins because their boxes are rotated.
Candidate arrivals are Main (-740, -1240, 95.5) and Village (4490, 2480, 196).
The candidate arrival coordinates remain provisional until capsule clearance and
visual placement are checked in the real levels. The candidate was copied into
the debug server's `Data/Portals.s1portal` and source `Data/Portals.s1portal`;
both copies matched the checkpoint. The current server loads and validates the
definitions at startup; unavailable data disables portal requests, not login.

## Current implementation — PIE verification pending

The client sends a portal request on local-player overlap. The external server
validates the portal and current region/position, rejects incompatible or busy
requests, and uses a generation and request ID to transfer player ownership via
the source and destination Room queues. The client fades out on acceptance, enters
LoadingMap, waits for the server's destination commit, receives fresh destination
entities and loot, and fades in after the local player exists. A world-ready
acknowledgment releases server-side action restrictions. Request replay history is
bounded to the most recent 64 accepted requests per player session. A 30-second
server timeout disconnects an unfinished transfer. Reconnect uses the existing
initial Village entry rather than persisting the last region.

Validation recorded on 2026-11-16: the affected server and S1Editor Development
targets built; synthetic definition tests passed 11 checks and transfer state tests
passed 16 checks. An isolated real Room runner passed 64 checks, including a valid
source/destination handoff, duplicate request, busy inventory, stale acknowledgments,
HP/capability preservation and a disconnected-session attach attempt. Unreal's
`S1.Level.PortalProtocol` packet framing and dispatch automation passed. The real
server starts with both portal definitions and database services ready.

On 2026-11-16, the user confirmed single-client Village → Loading → Main and
Main → Loading → Village movement. Logs also showed those transitions and three
MainMap monsters spawning. The user observed that the camera initially moved from
elsewhere toward the player after arrival. A follow-up change resets the camera
boom's lag history when the local pawn receives its server placement, retaining
normal camera lag during movement. S1Editor Development built successfully, and
`S1.Level.PortalCameraPlacement` and `S1.Level.PortalProtocol` automation passed.
The user then confirmed the camera fix and 1/2-client PIE checks, including
Village ↔ Main travel, same-map player appearance and movement, with no issue
reported. The editor log records both local and remote player spawns after travel.

Separate automated capsule-clearance testing, transfer failure injection, detailed
inventory/quest regression during transfer, and packaged behavior remain unverified.

### Loot presentation regression found after portal integration

In the subsequent two-client gameplay check the user reported no visible monster
drop. The server log recorded four committed monster drops; an investigation found
that `BeginPortalLoading` called the session-reset form of `FS1LootState::Reset`,
disabling the negotiated loot-presentation capability for the remainder of the
session. Region transitions now clear the prior region's snapshot, drops, and
sequence while preserving that negotiated capability. Session/logout reset still
disables it. This is a client display fix; it does not modify drop ownership or DB
records. S1Editor Development built successfully and `S1.Loot.VisualState` passed,
including capability preservation, old snapshot cleanup, destination snapshot and
created event handling, and the unnegotiated-session case. Two-client PIE visual
and pickup confirmation followed in the subsequent two-client re-test.

The first two-client re-test displayed monster drops on both clients, but pickup
was rejected as occluded. The destination Room had never loaded its own obstacle
file because portal arrival bypasses `EnterRoom`. Portal arrival now loads the
destination's obstacle data when loot or quest capability was negotiated; missing
data still fails closed. The server Debug build and actual Room runner passed 67
checks, including open-floor visibility and a blocking wall. A separate isolated
test DB and server then passed real-packet login → portal → monster drop → owner
pickup → removal; the server logged a committed pickup. On 2026-11-16 the user
reported the two-client PIE checklist completed; logs show the same drop spawning
and being removed once on each client, and the server confirmed pickup. The wall
case was covered by the Room test; the user's message did not separately confirm
whether the optional wall scenario was exercised manually.

`GameServer/GameServer/PortalTransferState.h` adds an isolated transition guard for
request ID, monotonically increasing generation and ordered LoadingReady →
SourceDetached → DestinationAttached → ClientWorldReady phases. Duplicate/stale or
out-of-order events are rejected. `GameServer/Tests/PortalTransferStateTests.cpp`
initially ran 15 checks, 0 failures. The guard is now connected to Player and the
Room transfer packet handlers; the helper alone still does not implement rollback,
ownership cleanup, thread synchronization, async DB reconciliation or timeouts.

## Queue and existing-operation audit

`Room::EnterRoom` unconditionally sends S_ENTER_GAME and initializes negotiated
quest/item capabilities; `LeaveRoom` sends S_LEAVE_GAME and clears negotiated state.
They cannot be used as-is to transfer without inadvertently ending inventory/quest
sessions. `Session::Send` enqueues under a lock, but this alone does not prove
source-room callbacks cannot enqueue after a transfer boundary. `JobQueue::Push`
can schedule work to another Room while source processing continues; transfer
ownership and disconnect need an explicitly serialized handoff. Pending potion use
and field drops live in Room-owned maps rather than solely on Player, so checking
only Player's pending booleans is insufficient. Gate or reconcile all async work,
and test delayed completions before enabling movement.

## Acceptance outline

User visually checks fade and placement. Test each direction in 1/2 clients, another
player staying in source/destination, monster spawn and combat, quest/inventory/loot,
close portals/reentry, duplicate requests, invalid/far portal and collision bounds,
disconnect at each transfer stage, stale events/async results and reconnect. Verify
client logs and server state for no duplicates, wrong-room actors, phantom damage,
or unexplained Fatal/Ensure/Blueprint/network errors. Package both maps and retest.
