# MainMap server navigation implementation

Date: 2026-09-22

## Scope and authority

This is the approved first obstacle-aware Dusk slice. The external IOCP GameServer remains authoritative for target selection, movement, attack eligibility, hit validation, HP, death and respawn. Unreal exports navigation data and presents server state; no Unreal AIController, Blackboard, Behavior Tree, RPC or Replication authority was added.

The implementation is deliberately MainMap-only and flat-XY. Authoritative terrain height, slopes, patrol, generic multi-map navigation, dynamic obstacle updates and crowd avoidance remain deferred.

## Export pipeline

- Editor-only plugin: `Plugins/S1NavigationTools`
- Commandlet: `S1ExportNavigation`
- Source world: `/Game/Maps/MainMap`
- Format: `S1NAV 2`
- Output checked into the server input location: `GameServer/GameServer/Data/MainMap.nav`
- Exported values: Recast agent settings, polygon centers, directed adjacency and left/right portal endpoints
- Current export: 74 nodes, 250 directed edges, agent radius 35 cm, agent height 144 cm, maximum slope 44 degrees

The commandlet only loads the saved map and writes text data. It does not save or modify MainMap. Re-export is required whenever saved MainMap navigation geometry changes.

## Server behavior

- `NavigationGraph` validates and loads the data, selects start/end polygons, computes an A* corridor and applies funnel/string-pulling reduction.
- Dusk CHASE and RETURN use the resulting waypoints at the existing 220 cm/s speed.
- Target paths are refreshed every 350 ms, immediately when exhausted, or after the destination moves at least 75 cm.
- Attack start keeps the existing 170 cm threshold but now uses navigable path distance.
- Dusk-to-player and player-to-Dusk contact validation keep their existing direct distance, height and facing checks and additionally require a valid navigation path within 190 cm and 230 cm respectively.
- Missing, invalid or incompatible navigation data fails closed by disabling Dusk movement and attacks. It never falls back to the former wall-crossing straight movement.
- Server Z remains the approved flat-ground capsule center `61.5 cm`. Portal Z is exported for a future terrain phase but is not currently applied.

## Verification evidence

- `S1Editor Win64 Development`: succeeded.
- `S1ExportNavigation`: succeeded with 0 errors; 74 nodes and 250 edges. The existing stale `Orion_Proto_Retarget` Rig warning and MCP EULA warning remained.
- `GameServer Debug x64` and `GameServer Release x64`: succeeded with project-reference rebuilding disabled after the Debug library side effect described below was recovered.
- Release server startup: process remained running and logged successful account selection.
- Navigation smoke tests:
  - Clear path `(0,600)->(0,0)`: direct 600 cm, path 600 cm.
  - Wall-corner path `(0,600)->(800,0)`: direct 1000 cm, path 1030.26 cm, corner near `(570,323)`.
  - Opposite wall sides `(300,0)->(700,0)`: direct 400 cm, path 837.455 cm around the south side.

On 2026-09-23, the user reported completing the two-client MainMap PIE checks for (1) Dusk pursuing around the test wall, (2) no attack or damage through the wall, (3) returning around the wall to its spawn after losing pursuit, and (4) two-player HP, death, respawn and reconnect regression. No failure was reported. This is user-reported acceptance evidence; this implementation turn did not capture PIE logs or independently replay the test.

## Build blocker outside this scope

A full Debug solution rebuild reaches the pre-existing malformed dispatch call in `ServerCore/IocpCore.cpp` and fails. An earlier parallel solution build also exposed the existing generated-file prebuild race. Neither unrelated source issue was left modified in this slice. The failed rebuild deleted the tracked Debug ServerCore library before compiling; it was regenerated with the intended dispatch line temporarily restored, and the source was then returned to its prior state. Final affected GameServer Debug and Release targets compiled and linked successfully with project-reference rebuilding disabled.

## Stationary CHASE presentation correction — 2026-09-28

At the 170 cm chase stop boundary, a sub-0.01 cm movement budget could prevent actual motion while `UpdateMonster` still advertised CHASE, making Dusk jog in place. `MoveMonsterAlongPath` now returns whether XY position actually changed, and the caller broadcasts IDLE when no movement occurred. No navigation data, path algorithm, attack range, hit rule, protocol or client asset changed. Both Release and Debug GameServer targets compiled and linked. After the user stopped the old Debug process, the new Debug binary started and accepted a local TCP connection on port 7777; authenticated PIE and visual verification remain pending.

## CHASE route reversal investigation and correction — 2026-09-28

The user reported Dusk briefly moving toward an old-looking point before resuming pursuit while the player retreated, both near the test wall and on open ground. A standalone probe using the actual `MainMap.nav` and `NavigationGraph.cpp` reproduced two route-selection defects:

- In open ground, `(0,600)->(0,200)` took an 800 cm detour via `(0,0)` instead of a direct 400 cm path. Selecting the nearest polygon center can choose a neighboring polygon even when the point lies inside another polygon. `FindNearestNode` now prefers a polygon with a containing center-to-portal triangle and retains nearest-center fallback for points not covered by the exported portal triangles.
- Near the wall, `(700,350)->(0,0)` used the north-side route of 809.65 cm, but moving the goal only 50 cm to `(0,-50)` changed the center-cost A* corridor to a 1339.13 cm south-side route. `FindPath` now tests one-edge alternatives to the first A* corridor and selects the shortest resulting funnel route. The same sample is now 840.78 cm via the original north-side first waypoint `(494,342)`. The next goal `(0,-100)` is 874.43 cm with that same first waypoint. The wall-grid sample at X=-100,-10,0,10,100 and Y=100,50,0,-50,-100 showed the north-side first waypoint throughout.

The existing smoke routes remain 600 cm for `(0,600)->(0,0)`, 1030.26 cm for `(0,600)->(800,0)`, and 837.455 cm for `(300,0)->(700,0)`. GameServer Release and Debug x64 built successfully. The user confirmed the editor and server were closed, but the previous Debug server remained as a background process on port 7777; it was stopped by exact PID after verification. The newly built Debug server started as PID 28968 and accepted a local TCP connection on port 7777. Account selection succeeded. Navigation is loaded on first Dusk startup after a client enters MainMap, so startup alone did not verify the live navigation load. Two-client moving-target PIE, pursuit/return, wall-contact combat, and live gameplay logs remain unverified for this correction. No nav export, Unreal asset, protocol, speed, repath cadence, combat threshold, or terrain-height rule changed.

After testing the new server, the user reported that the pursuit motion was "much better." This is user-observed improvement of the reported reversal symptom, not yet confirmation of the complete two-client combat/return/reconnect regression checklist or a reviewed PIE log.

## User acceptance update — 2026-09-29

The user subsequently reported completing the attack-hit validation, return to spawn, and two-client PIE regression checks for this navigation correction. No new issue was reported. This supersedes the earlier "PIE pending" status for those user-tested behaviors. The report is user-provided acceptance evidence; this turn did not independently replay PIE or review the corresponding gameplay logs. The open-ground and wall-side moving-target pursuit improvement remains the user's visual observation, supported separately by the deterministic navigation probe above.
