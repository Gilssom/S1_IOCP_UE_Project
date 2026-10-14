# Dusk return re-engagement and home recovery — 2026-10-08

## Latest user acceptance — 2026-10-08

User confirmed current-revision 1-/2-client PIE re-engagement and recovery
synchronization complete. Record this as user manual acceptance, not independently
replayed or log-audited by Codex. The pending wording below is the earlier handoff
record. Unspecified wall/drop/potion/death/reconnect regression and packaged-client
acceptance are not promoted to passed. No code/asset change or commit/push in this
acceptance update.

## User-approved behavior

During RETURN, a player entering the existing detection range re-engages Dusk
without resetting HP. With no target, home arrival starts linear recovery of the
missing HP over4seconds, not an immediate maximum-HP overwrite. Re-engagement
during recovery stops it at the last server-confirmed HP. Recovery starts at home,
not while travelling. Repeated exits begin a new4second missing-HP recovery after
the next home arrival. Normal new-life respawn remains fullHP.

Existing acquisition800cm, loss1000cm and home leash1200cm retained. Player must
remain alive/inside that home leash; this is not unlimited pursuit. Existing Nav
path/wall/melee validation, target selection/tie-break, attack interval and reserved
hit ownership remain. RETURN remains non-damageable until the server tick detects
and transitions into combat; old RETURN immunity test remains. No forced attack
through walls or transferred old hit reservations.

## Cause / implementation

Old Room::UpdateMonster RETURN returned before AcquireMonsterTarget; arrival
set current_hp=maxHP. New RETURN scans first, clears stale path/attack ownership,
transitions back into existing chase/attack resolution without touching HP. If no
candidate, it retains home path. Arrival switches to IDLE with recovery epoch and
startingHP. Target-free home IDLE computes floor(missingHP*elapsedMs/4000), capped
at4seconds, and publishes changed HP via existing S_MONSTER_UPDATE only. Acquisition
is checked before recovery every tick; a valid target clears recovery. Death early
return prevents healing corpses; SpawnDuskMonster resets recovery per new life.
Recovery uses monotonic elapsed time, independent of capped movement dt. Room
serialized jobs remain the sole authority. No timers survive actor life for healing.

## Files / boundaries

Server root E:\Git Fork\GameServer; modified GameServer/GameServer/Room.h/.cpp,
DuskDefinition.h (HomeRecoveryDurationMs=4000), GameServer/Tests/RoomMultiMonsterTests.cpp.
No DB/protobuf/generated protocol/Unreal code/BP/map changes in THIS follow-up.
Existing client S_MONSTER_UPDATE applies authoritative HP automatically. The prior
HP WBP task remains uncommitted separately; no auto commit/push in either repository.
Existing GameServer staged/unstaged user changes preserved.

Checkpoint C:\Users\user\Documents\Unreal IOCP Server\checkpoints\DuskRecovery_20261008:
original4files copied/hash matched, Debug/Release build logs, test builder and log,
startup stdout/stderr. Test builder writes here, not previous MultiDusk checkpoint.

## Executed verification / limits

GameServerDebugBuild.log and GameServerReleaseBuild.log full compile/link exit0.
RoomTestsBuild.log exit0; existing offline-link /EDITANDCONTINUE OPT:ICF warning.
RoomTests.log: real Room multi-monster43checks/failures0. Uses actual saved MainMap
Nav and disconnected in-memory sessions, no DB/IOCP/client worker loop. Covers
three independent monsters, A-D/Q multi-hit/duplicates/lifetimes, A-to-B retarget,
home-leash exclusion, return re-engage preserving wounded60HP, home60HP with no
reset,2sec130HP,4sec200HP, a new80HP recovery110HP at1sec, interruption preserving
110HP, no further recovery during attack, other monster unaffected, death/respawn.
Offline 'Death loot unavailable; boot recovery not ready' is expected because
the fixture does not attach a DB loot service, not live-server validation.

User closed Debug GameServer/debugger. New Debug server launched PID15616 using
Binary/Debug working directory and existing environment credential (not printed).
Authenticated1-/2-client PIE of this revision remains pending. Test return with
same player and another player, noHPjump on reengage, no-heal during combat,
home4sec recovery, recovery interruption and repeat-cycle, wall return/attack,
HP synchronization, death/respawn/drop/potion/reconnect regression. Do not treat
earlier user combat/HP-bar acceptance as this server-policy revision acceptance.
The3sec noncombat HP-bar hide policy remains: recovery may finish while hidden.
