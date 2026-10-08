# 회복 포션 사용 — 조사 및 승인 검토안 (2026-10-02)

## 상태

2026-10-07 후속: 승인된 1–4 클라이언트 퀵슬롯이 기존 RequestUseItem/서버 소비·회복 경로를 재사용하도록 구현됐다. 아래 퀵슬롯 제외 문구는 이전 단계의 범위다. 최신 구현/빌드·자동 검사 및 실제 PIE 미검증 구분은 QuickSlotsImplementation.md를 따른다. 서버/DB/패킷 계약 변경 없음.

2026-10-06 사용자가 구체 DB/패킷 변경 및 손실 방지 복구 정책 구현을 승인했다. 현재 **운영 migration·Room/패킷/UI/회복 FX 연결 구현과 빌드·자동화·패키징 검증 완료, 최신 실제 1-/2-client PIE에서 회복량·수량 감소·양쪽 FX 1회·최대HP 무소비·재접속 수량 유지·FX 높이를 사용자 확인 완료**다. 아래 10월2일 계약과 중간 단계 기록은 역사적 계획/진행 상태이며 말미의 최신 사용자 인수 기록이 우선한다. 현재 높이 설정 재패키징과 장애 상황 검증은 별도로 남아 있다.

확정 기능: I에서 포션 스택 선택→사용; 포션 1개 소비, 실제 회복량 min(50, 최대HP−현재HP). 최대HP/사망/0개는 소비·FX 없이 거부. 추가 버프, 사용 사운드, 모션/이동 잠금, 포션 고유 쿨다운, 퀵슬롯 UI/키/저장은 제외한다. 미래 퀵슬롯은 동일 RequestUseItem API를 호출할 수 있게만 한다.

## 확인한 현재 구현

- 서버 `Player.h/.cpp`의 currentHp/maxHp 초기값은 100/100이다. `ObjectUtils::CreatePlayer`는 새 ObjectId와 Player를 생성한다. `CharacterDB::GetCharacter`는 신원·레벨·직업·위치를 읽지만 HP를 읽지 않는다. `Room::ResolveMonsterHit`는 런타임 HP를 변경하고, `UpdatePlayerRespawns`는 5초 후 최대HP로 복귀한다. **HP는 현재 이 경로에서 DB 영속화되지 않으며 재접속은 새 100HP 플레이어다.** 이 단계에서 HP 전체 영속화를 추가하지 않는다.
- DB worker는 `LootService`→`AsyncLootDB` bounded FIFO128/전용 스레드다. IOCP/Room에서 동기 DB를 실행하지 않는다. 기존 Pickup은 CharacterData FOR UPDATE→요청 조회→LootDrop→InventoryState→Item 순서로 락을 잡는다. 소비도 CharacterData 락부터 시작해야 한다.
- `ItemGrantLedger`는 양수 지급 및 after=before+count 제약이 있다. 소비를 음수 지급으로 저장하거나 기존 ItemPickupRequest에 소비 결과를 섞지 않는다.
- 클라이언트 `FS1InventoryState`는 현재 합산 포션 수량만 보관한다. 스택 선택을 위해 검증된 InventoryEntry 목록도 보관해야 한다. 동일 버전 충돌 검사도 합계뿐 아니라 전체 항목으로 확장한다.
- `US1InventoryWidget`는 아이콘/합계/상태/닫기만 있는 수동 뷰다. 새 스택 선택/사용 intent를 InventorySubsystem에 전달하고, 뷰는 직접 수량·HP·패킷을 변경하지 않는다.
- `ApplyPlayerStatus`는 HP뿐 아니라 위치와 MovementMode도 변경한다. 회복에 이 경로를 그대로 재사용하지 않는다. `ApplyDamageResult`의 순서 필터와 PlayerStatus가 현재 공통 HP 순서 커서를 공유하지 않으므로 피해/상태/회복에 공통 캐릭터별 RoomEventSequence 필터를 추가할 필요가 있다. 생존 상태 회복 패킷이 오래된 사망을 되돌리지 않게 한다.
- Server Git은 기존 소스/생성물/빌드/IDE 변경이 많다. 이번 조사에서 보존했고 global safe.directory 설정은 바꾸지 않았다. S1은 Git이 없다.

## 승인된 복구 정책

DB 소비와 런타임 HP는 단일 원자적 자원이 아니다. 이를 원자적 exactly-once 회복으로 주장하지 않는다. 작은 보상 상태 머신을 제안한다.

1. Room에서 인증 세션/CharacterId/ObjectId/생명 세대, 생존·HP 부족, 계약을 검증하고 캐릭터당 사용 1개를 pending으로 둔다. 카메라·이동·공격·피격은 막지 않는다.
2. DB 트랜잭션에서 같은 RequestId 결과를 먼저 확인한다. 처음 요청이면 선택 스택 소유·종류·수량을 확인하고 1개 감산, 사용 기록, InventoryVersion+1, ItemUseConsumed outbox를 함께 커밋한다. 이때 상태는 CONSUMED_PENDING_EFFECT이며 **회복 완료가 아니다**.
3. Room 콜백에서 동일 ObjectId/세션/생명 세대를 재검증한다. 살아있고 아직 HP 부족이면 **콜백 당시 최신 HP**에 부족분 최대50을 가산한다. DB 제출 당시 HP를 덮어쓰지 않는다. Room 직렬 실행 구간에서 operation을 적용됨으로 표시하여 같은 콜백/재확인이 HP를 두 번 더하지 않게 한다.
4. 실제 HP 적용 때 새 RoomEventSequence/EventId의 공개 회복 이벤트를 한 번 보낸다. 이후 worker가 APPLIED 상태·실제 HP 전후/회복량·ItemUseApplied outbox를 기록한다. 요청자의 내구성 성공 응답은 APPLIED 기록을 확인한 뒤 보낸다. ACK가 미확정이어도 HP를 다시 적용하지 않는다.
5. **HP 적용 전** 사망/퇴장/세션·생명 세대 변경/최대HP가 되었으면 HP·FX 없이 동일 operation을 REFUNDED로 끝낸다. DB에서 1개 반환+InventoryVersion+1+ItemUseRefunded를 원자적으로 기록한다. 실패/응답 손실은 같은 operation 상태를 조회하고 재가산하지 않는다.
6. **HP 적용 후** 피격·사망·퇴장은 정상 소비를 취소하지 않는다. 원래 객체가 사라져도 DB APPLIED 마무리는 진행하며 새 세션에 옛 HP/FX를 전달하지 않는다.
7. 새 server boot에서는 이전 boot의 CONSUMED_PENDING_EFFECT를 반환하고 APPLIED는 소비 유지한다. 복구 완료 전 사용 지원 광고를 하지 않는다. 기존 재접속 100HP 정책을 유지하고 옛 회복은 재생하지 않는다.

한계/선택: HP 적용 직후 APPLIED DB 기록 전 강제 종료하면 이전 boot pending 소비가 반환될 수 있다. 기존 HP 비영속화·재접속 최대HP 정책에 맞춰 **영구 아이템 손실 방지를 우선**하는 제안이다. 그 짧은 구간의 옛 생명에서 회복이 보였더라도 다음 boot에서 포션이 반환될 수 있으며, 이를 완전한 사용 exactly-once로 주장하지 않는다. 이 정책을 원치 않으면 HP 영속화/보다 강한 효과 원장이 필요하여 별도 범위가 된다.

COMMIT 응답 손실은 같은 요청을 다른 leased 연결에서 읽어 상태를 확인한다. 새 RequestId 자동 재시도는 없다. rollback이 확인된 deadlock(1213)만 같은 payload로 worker 안에서 제한 재시도(권장 최대2회); commit 여부 불명확/연결 손실은 재실행하지 않고 조회한다. DB outage가 길면 unknown/pending으로 유지하며 성공·거부를 꾸며내지 않는다.

### 스택·획득 동시 실행

반환 공간을 보장하기 위해 pending 소비 스택은 Item 행과 슬롯을 유지하고 해당 행을 다른 획득/소비 writer의 수정 대상으로 선택하지 않는다. 마지막1개면 일시 count0 행이 남되 inventory snapshot에서는 제외한다. Pickup은 다른 스택/빈 슬롯을 선택하고 예약 슬롯은 재사용하지 않는다. APPLIED 마무리 때 아직0이면 삭제, REFUNDED면 같은 행에1개 복구한다. terminal 후 빈 슬롯은 기존 선택 방식으로 재사용한다. 임의 21번째 슬롯·99초과 반환·다른 스택 덮어쓰기를 하지 않는다. 다른 스택 획득은 캐릭터 락/버전 직렬화 아래 계속 가능하다.

실제 Item의 count0 허용 제약은 migration 준비 시 최신 SHOW CREATE로 재확인한다. 금지된 경우 기존 테이블 제약을 몰래 완화하지 않고 별도 예약 공간 대안을 제출한다. 사용 트랜잭션 락 순서는 CharacterData→ItemUseOperation→InventoryState→Item; 획득도 첫 CharacterData 락을 유지한다. 부팅 복구 역시 캐릭터부터 같은 순서로 처리한다.

## 최소 DB 변경안 — 제안, 미적용

기존 5개 테이블 및 Item/Account/CharacterData 컬럼을 ALTER하지 않고 신규 **ItemUseOperation 1개**를 추가한다. 요청 결과와 소비/복구 원장을 함께 담으며 지급 원장은 보존한다.

| 필드 | 타입/제약 |
|---|---|
| character_id, request_id | BIGINT UNSIGNED + BINARY(16), 복합 PK; 역사 보존을 위해 CASCADE 없음 |
| operation_id | BINARY(16) NOT NULL UNIQUE, 서버 생성 |
| item_instance_id, item_definition_id | BIGINT UNSIGNED, INT UNSIGNED; exact payload 매칭 |
| state, reason | TINYINT/INT UNSIGNED; REJECTED/CONSUMED_PENDING_EFFECT/APPLIED/REFUNDED, 허용값 CHECK |
| server_id, room_id, server_boot_id, source_object_id, life_generation | 서버 유도 신원/수명, uint/BINARY16 |
| consumed_count, before_count, after_count | uint, consumed는0또는1; 성공 소비 after=before−1 |
| inventory_version, refund_inventory_version | BIGINT UNSIGNED; 반환은 별도 더 큰 버전 |
| hp_before, hp_after, actual_heal | uint; 효과 확정 전0, APPLIED에서 실제 값 기록 |
| effect_event_id, room_event_sequence | BINARY16 + uint; 중복 표현 방지와 순서 |
| created_utc, consumed_utc, finalized_utc | DATETIME(6); 단계 미발생 시 nullable |

캐릭터당 pending 1개는 CharacterData 락 아래 DB에서도 확인한다. 같은 RequestId 다른 item payload는 REQUEST_CONFLICT이며 원래 결과를 덮어쓰지 않는다. 원장만으로 원래 지급 DropId를 대신 만들지 않는다. CombatItemOutbox 기존 JSON payload로 operation/item/HP/stage 정보를 확장하고 drop/death/spawn 비해당 ID는 명시적인 zero16로 둔다. 소비 사건과 실제 효과 사건은 구분한다. 공개 로그에 계정 비밀정보를 넣지 않는다.

Migration SQL은 승인 후 별도 테스트 schema/backup checkpoint 및 최신 제약·권한 확인을 거쳐 작성·적용한다. runtime account CREATE 권한을 확대하지 않고 자동 DDL을 실행하지 않는다. 롤백은 사용 계약 광고 중단/바이너리 복귀이며 신규 원장과 기존 지급 기록을 삭제하지 않는다.

## 최소 wire 계약 — 제안, 미생성

기존 ID1000~1029/loot1/combat2를 보존한다. Protocol.proto 끝에 3개 append, Enum.proto에 독립 사용 enum 추가. 양쪽 generated 파일은 protoc/기존 handler 생성 경로로 갱신한다.

| ID | 메시지, 필드번호 |
|---|---|
|1030| C_ITEM_USE: request_id bytes(1), item_instance_id uint64(2), item_definition_id uint32(3), contract_version uint32(4). 수량/HP/소유자 입력 없음. |
|1031| S_ITEM_USE_RESULT (요청자 전용): request_id(1), operation_id(2), character_id(3), item_instance_id(4), item_definition_id(5), decision(6), reason(7), inventory_version(8), resulting_count(9), actual_heal(10). 성공 과거 HP는 UI에 적용하지 않는다. |
|1032| S_PLAYER_HEALED (지원 관전자 포함): object_id(1), effect_event_id bytes(2), actual_heal uint32(3), current_hp(4), max_hp(5), room_event_sequence uint32(6). 인벤토리·RequestId·CharacterId 방송 없음. 위치/이동 상태 변경 없음. |

C_ENTER_GAME 새 field4/S_ENTER_GAME 새 field6 item_use_contract_version=1을 독립 협상한다. old peer0이면 사용 버튼 비활성/새 이벤트 송신 없음; G/I/전투는 유지한다. use1은 loot1 및 사용 DB/worker 준비 완료 조건에서만 지원한다. 서버는 피해 뒤 포션 성공을 같은 Room 스트림 순서로 전달한다.

decision: APPLIED/REJECTED/REFUNDED/UNCONFIRMED. reason: NONE/INVALID_REQUEST/REQUEST_CONFLICT/NOT_OWNER/NOT_FOUND/WRONG_ITEM/NO_COUNT/DEAD/FULL_HP/SESSION_CHANGED/UNSUPPORTED_CONTRACT/DB_UNAVAILABLE. 일시 처리량·pending은 UNCONFIRMED이며 영구 거부로 기록하지 않는다. REFUNDED는 소비 복구 완료이지 회복 성공이 아니다.

UI는 처리중 연타 억제; 5초 뒤 미확정 표시, 수동 재확인은 같은 payload/RequestId(최소1초 간격). 이는 쿨다운 정책이 아니다. UI 닫기만으로 진행 요청을 취소하지 않고 결과는 subsystem이 관리한다. reset은 이전 세션 요청·선택·미확정 상태를 비운다. 실제 HP/FX는 WorldEntity의 신규 공개 이벤트만 적용하고 중복 EventId/과거 sequence/죽음·despawn·세션 reset을 검사한다.

## 회복 Niagara 실제 조사

2026-10-02 실행 중인 Editor PID32236/GameServer PID33888 확인; localhost8000 MCP initialize/list_toolsets/describe_toolset 및 읽기 전용 조사 성공. IsPIERunning=false. 호출은 게임 스레드에 직렬 제출한다.

- exact `/Game/VFX_StatusEffect01/FXS/FXS_healing000.FXS_healing000`, class NiagaraSystem, dirty=false. 현재 referencer는 `/Game/VFX_StatusEffect01/DEMO/VFX_StatusEffect01`뿐이며 S1 runtime 연결 없음.
- 활성 이미터10개 전부 CPU, Sprite/Light renderer, user variables 없음. GetSystemDependencies는 Curve/ColorCurve/VectorCurve/SpriteRendererInfo 인터페이스를 반환했다.
- 이미터 입력 조사에서 Loop Behavior Infinite/Loop Duration1s를 확인했다. literal lifetime1s인 이미터가 있고 동적/unsupported lifetime도 있어 모두1s라고 단정하지 않는다. 원본을 수정하지 않고 재생1.5s 후 Deactivate, 최대4s 후 강제 Destroy 시작값을 제안한다(시각 검증 후 조절).
- 게임 패키지 재귀 dependency25개 조회 완료/조회 실패0/남은 queue0. direct material instance9개, master/function/textures를 포함한다. 이것은 registry 관계 확인이며 전체 shader/cook 성공 증거가 아니다.
- GetSystemCompileState 호출은 timeout으로 결과 미확인. 이전 호출의 summary를 compile 성공으로 취급하지 않는다. CaptureAssetImage는 Niagara asset type을 지원하지 않는다고 반환했다. **움직이는 미리보기/UE5.8 최종 compile/render/cook은 아직 미검증**이며 통합 후 transient component/실제 PIE로 확인한다.
- 예정: S1 설정의 exact soft path + native cook anchor, 발밑(local capsule Z−halfHeight), scale1 시작, 회전0. aura 축/캐릭터 크기/밝기는 실제 미리보기와 사용자 확인 후 선택한다. actor-owned transient component, 중복 이벤트 재활성화 금지, death/despawn/EndPlay/reset 정리. 벤더 Niagara/맵 저장은 하지 않는다. 출처/영상 사용 권한은 공개 전 확인 필요.

## 변경 예정 파일 — 승인 후

서버 root `E:\Git Fork\GameServer\GameServer`:

- `Common\Protobuf\bin\Protocol.proto`, `Enum.proto`; Server/DummyClient/Unreal 생성 Protocol/Enum 및 handler 파일.
- `GameServer\LootDB.h/.cpp`, `LootService.h/.cpp`, `Player.h`, `Room.h/.cpp`, `ClientPacketHandler.h/.cpp`; 필요 시 초기 준비 경로 `GameServer.cpp`.
- `Tests\PotionUseDBIntegrationTests.cpp`, `Tests\PotionUseRuntimeTests.cpp` 신규.
- `E:\Git Fork\GameServer\codex_md_edit\db\PotionUseMigration.sql` 신규 승인 SQL.

클라이언트 root `E:\Git Fork\S1_UE5.8\Source\S1`:

- `Loot\S1InventoryState.h`, `S1InventorySubsystem.h/.cpp`, `UI\S1InventoryWidget.h/.cpp`.
- `Loot\S1PotionPresentationSettings.h/.cpp`, `World\S1WorldEntitySubsystem.h/.cpp`.
- `Network\S1NetworkSubsystem.h/.cpp`, `ClientPacketHandler.h/.cpp`, `Account\S1AccountSubsystem.cpp`, `S1GameInstance.h/.cpp`.
- 요청/세션 상태와 HP/FX 수명 자동화: 신규 `Loot\S1PotionUseTests.cpp` 및 기존 Inventory tests.

Binary: 신규 에셋 생성/기존 Blueprint·맵·벤더 에셋 수정 예정 없음. 설정/네이티브 표현만 사용한다. 코드 전 verified source checkpoint, 에디터/서버 종료 확인 후 네이티브·패킷 빌드를 적용한다. binary 변경이 필요해지면 checkpoint/dirty/정확한 저장 대상부터 다시 보고한다. UE5_RPG/UE5.5는 읽기 전용·이관 없음.

## 검증 단위

1. 신규 SQL 검토·승인→격리 schema에서 기본 감소/0 삭제·복구 예약/99·20/동시 pickup-use/중복/요청 충돌/롤백·deadlock·commit unknown/boot recovery 테스트.
2. Room 하네스로 DB 대기 중 피격/사망/respawn/퇴장, HP 적용 후 중복·종료 및 outbox 단계 검증. 실제 소켓 장애와 구분.
3. Server Debug/Release, S1Editor/Game 빌드, affected automation. HP sequence/스택 선택/동일 RequestId/늦은 응답/FX 정리 테스트.
4. 실제 1-/2-client PIE: 40HP→90/90→100/최대HP/사망/0개, 마지막1개, 연타, 상대 HP/FX 1회·상대 inventory 불변, 재접속 수량, UI 클릭 전투 혼선, G/기존 전투/페이드 회귀. 실제 visual 사용자 확인 필요.
5. cook reference 변화 때문에 Windows Development BuildCookRun 및 두 패키지 client 확인. 과거 package 성공 재사용 금지.

현재 실행 검증은 **소스·MCP 읽기 전용 조사뿐**이다. 이번 사용 구현 build/DB DDL/자동화/PIE/패키징 성공을 보고하지 않는다.

## 2026-10-06 기반 구현 기록 — 전체 기능 미완료

위 마지막 문장은 10월2일 조사 단계 기록이다. 10월6일 구현 승인 후 다음 범위만 실제 변경·검증했다.

### 변경/책임

- 신규 `E:\Git Fork\GameServer\GameServer\GameServer\PotionUseDB.h/.cpp`: 별도 소비 원장, CharacterData 선행 락, 같은 요청 readback/다른 payload 충돌, 캐릭터당 pending1개, 감산+버전+Consumed outbox의 트랜잭션, APPLIED/REFUNDED finalization 중복 방지, 부팅 시 이전 boot pending 반환. 소비 outbox와 HP 적용 완료 outbox를 분리했다. 잘못된 소유자의 Item은 자기 캐릭터 락 아래 잠그지 않아 교차 소유 요청의 락 순서 역전을 피한다.
- 신규 `PotionUseRuntime.h`: Room이 사용할 순수 operation 정책. 최신 HP로 최대50 계산, 사망·접속·ObjectId·생명 세대 변경 시 반환, 잘못된/미확정/중복 콜백은 HP 적용을 다시 승인하지 않는다. **아직 실제 Room 콜백에 연결하지 않았다.**
- `LootDB.h/.cpp`: 선택적 protectUseReservations 인자(기존 caller 기본false)로 pending 소비 스택 제외; 슬롯 점유는 유지. snapshot은 예약 중0개 행을 숨긴다. 실제 runtime use schema/boot gate 성공 뒤 기존 서비스 호출을 true로 연결할 예정이며 현재 호출 동작은 아직 활성화되지 않았다.
- `GameServer.vcxproj`: 신규 cpp/header만 등록. 프로토콜·생성물·LootService·Room·Unreal UI/에셋은 이 단위에서 변경하지 않았다.
- 신규 `E:\Git Fork\GameServer\codex_md_edit\db\PotionUseMigration.sql`: 승인된 신규 ItemUseOperation1개만 CREATE, 기존 table/데이터 ALTER/DROP 없음. **아직 적용하지 않았다.** zero-count 예약은 10월6일 읽기 전용 SHOW CREATE에서 현재 Item 제약이 허용하는 것을 확인했다.
- 신규 `GameServer\Tests\PotionUseDBIntegrationTests.cpp`, `PotionUseRuntimeTests.cpp`, `BuildPotionUseTests.cmd`: 격리 schema가 소스에 고정된 테스트. fixture는 use_ 이름으로 식별하고 삭제하지 않는다. 새 표가 없으면 fixture 작성 전에 종료한다.

### 체크포인트/실행 증거

`C:\Users\user\Documents\Unreal IOCP Server\checkpoints\PotionUse_20261006`에 수정 전 서버/클라이언트/proto/handler 원본144개를 복사하고 SHA-256 일치를 확인했다. 기존 dirty worktree 변경은 보존했다. 이번 MCP IsPIERunning=false를 읽기 전용 확인했다.

- `BuildTestsFinal.log`: 최신 PotionUseDBIntegrationTests/RuntimeTests 독립 MSVC Debug CRT 컴파일·링크 exit0. **전체 GameServer Debug/Release나 S1Editor/Game 빌드 결과가 아니다.** 기존 ServerCore의 LNK4075(EditAndContinue/OPT:ICF) 경고가 남는다.
- `RuntimeTestsFinal.log`: 실행 exit0, 전체50/부족분10/DB 대기 중 최신HP/사망/respawn/재접속/퇴장/중복/다른operation/미확정/sequence0 거부 정책 assertions 통과. 순수 하네스이며 실제 Room HP 변경·DB·소켓·PIE 증거가 아니다.
- 초기 테스트 빌드는 sandbox vcvars의 SDK include/lib 경로 누락과 ODBC import lib 누락으로 실패했다. 테스트 빌드 스크립트에서 설치된 SDK10.0.22621과 odbc32.lib를 명시하여 해결했으며 프로젝트/엔진 설정을 바꾸지 않았다.
- 격리 DB 테스트는 **미실행**. 읽기 전용 metadata 확인에서 `S1LootTest_20261001.ItemUseOperation` 존재 개수0이었다. 관리자에게 해당 테스트 schema에서만 Migration.sql 실행을 요청했다. GameDB에는 적용/테스트 fixture/감산/HP 변경을 하지 않았다.

DB 테스트가 준비한 항목(실행 성공으로 보고하지 않음): 중복/충돌/동시소비, 정상거부 무감산, 마지막1개 예약/반환/삭제, 부족분회복 기록, 다른 스택 획득과 예약 반환, 99스택/20슬롯 반환 공간, 부팅 복구, 감산 후 UNIQUE 오류 rollback. 실제 COMMIT 연결 손실과 DBConnection native deadlock 진단 기반 bounded retry는 아직 검증/추가하지 않았다. 현재 DB 오류는 UNCONFIRMED로 실패-안전 처리하고 blind 재실행하지 않는다.

### 다음 필수 순서/미검증

1. 사용자가 테스트 schema 기본 선택/SELECT DATABASE()를 확인하고 Migration.sql을 실행(관리자 DDL 필요, GameDB 금지).
2. 실제 C++/ODBC 격리 테스트 수행·결과 확인 후 운영 백업/최신 schema 검증 및 승인된 GameDB migration 적용 단계.
3. bounded worker/Room 수명·재시도 연결, 승인된3패킷 생성/버전 협상, native UI 스택 선택/사용·HP 공통 순서·FX 연결.
4. 전체 대상 빌드, 자동화, 실제1-/2-client PIE·사용자 시각 확인, packaging. 네이티브/패킷 적용 전 서버·에디터 종료 요청.

현재 GameServer PID38376/Editor PID28996는 기존 버전 그대로 실행 중이다(기록 시점 확인). binary/map/Blueprint/vendor Niagara 저장·UE5_RPG 수정 없음. CurrentAssetRegistry의 회복 FX는 Candidate 그대로 유지하며 이번 순수 정책 테스트로 Active 승격하지 않는다.

### 같은 작업의 격리 DB/worker 검증 후속

사용자가 관리자 연결에서 S1LootTest_20261001에 신규 테이블 적용 완료를 알렸다. 신규 표가 없다는 위 기록은 적용 전 확인이며 다음 결과가 최신 상태다.

- 첫 실제 테스트에서 WrongItem 요청이 좁은 item-template 재조회 때문에 Unknown으로 돌아오는 assertion 실패가 발생했다. 이미 검증 거부된 요청은 정상 거부 원장을 기록하도록 수정했다. 실패 로그 `DBIntegrationTests.log` 보존. assertion CRT 창으로 대기한 **우리 테스트 프로세스만** 경로/PID 확인 후 종료했으며 GameServer/Editor는 종료하지 않았다. 이후 테스트의 CRT assertion은 stderr로 보내도록 설정했고 assertions 자체는 유지했다.
- `BuildTestsCorrected.log` 및 `DBIntegrationTestsCorrected.log`: 수정 후 컴파일·링크 exit0, 실제 C++/ODBC 테스트 exit0. 중복/충돌/pending 억제, 마지막1개 예약·반환·0행 숨김·삭제, 정상거부 무감산, 다른 스택 획득/반환 동시 계약, 99스택/20슬롯 반환 공간, 부팅 복구/Applied 유지, 두 연결 동시 소비1회, 감산 후 UNIQUE journal fault 롤백 assertions 통과. fixtures는 격리 schema에 보존하며 운영 GameDB에는 넣지 않았다.
- `LootService.h/.cpp`에 ConsumeItem/FinalizeItem worker API와 UseReady를 추가했다. 없는 선택적 표는 기존 loot만 유지하고, 표가 존재하지만 schema/recovery 확인 실패면 grants까지 잠가 예약 슬롯 손상을 방지한다. 성공한 gate에서만 Pickup의 예약 제외를 활성화한다. uncertain 결과는 별도 physical lease로 같은 요청 상태를 읽고 blind 소비 재실행은 하지 않는다. 아직 Room/패킷 caller 없음.
- `BuildWorkerTests.log`, `DBWorkerTests.log`: 신규 worker 연결 포함 독립 테스트 빌드 exit0, 위 DB assertions 및 실제 bounded worker boot gate/ConsumeItem/FinalizeItem/drain 실행 exit0. 기존 LNK4075 경고는 남는다. worker 성공은 실제 IOCP/Room/PIE 검증이 아니다.
- 운영 GameDB의 private table/data dump `GameDB_before_item_use.sql`을 체크포인트 안에 생성했다(mysqldump exit0, 파일 존재/크기/SHA-256 확인). 비밀번호는 private 환경 변수로만 전달하고 복원 SQL/내용을 출력하거나 공유하지 않았다. single-transaction, skip-add-drop-table/no-tablespaces/skip-triggers 설정이며 stored routines/events/triggers 백업·실제 복원 검증까지 완료했다는 뜻은 아니다.
- 운영 신규 CREATE는 여전히 관리자 권한 작업이다. 사용자에게 기본 GameDB/SELECT DATABASE() 확인 후 **Migration.sql만** 실행하도록 요청했다. private backup을 GameDB에 실행하도록 요청하지 않았다. 적용 확인 후 실제 source/packet/UI 연결을 계속한다. 프로토콜/생성물/Unreal 소스·에셋은 아직 이 단위에서 변경되지 않았다.

신규 코드가 전체 게임 타깃으로 빌드·런칭됐다는 보고는 하지 않는다. 실제 COMMIT transport loss, deadlock native-code 재시도, runtime 사망/퇴장/socket/session, 1-/2-client 화면 및 cook/package는 미검증이다.

## 최신 통합 구현 및 검증 — 2026-10-06

최신 위치 튜닝: 사용자 요청으로 회복 FX를 발밑 대신 몸통 중심에 맞췄다. DefaultGame.ini의 S1PotionPresentationSettings에 HealingFootOffset=(X=0,Y=0,Z=96)만 추가했다. 현재 BP 플레이어 capsule half-height96cm이므로 기존 상대좌표 -96+96=0, capsule/root 중심에 재생한다. 원본 config checkpoint/hash 확인, diff 추가2줄, live MCP native CDO set_properties=true 및 readback Z96 확인. PIE=false에서 적용했으며 다음 새 효과부터 반영된다. 서버/피해/소비/수명/원본 Niagara/Blueprint 변경 없음. 코드 빌드·재시작 없이 live 설정으로 적용했고 **이후 사용자가 실제 1-/2-client PIE에서 FX 높이 확인을 완료했다**. 이전 패키지에는 새 config가 포함되지 않으므로 다음 패키징에 반영해야 한다.

### 구현된 경로 / 운영 DB

사용자가 관리자 연결에서 GameDB Migration.sql 실행 완료를 알렸다. 읽기 전용 확인: ItemUseOperation 컬럼25개, 확인 시 use operation0건. 기존 테이블 ALTER/데이터 초기화는 하지 않았다. 실제 테스트 fixtures는 여전히 S1LootTest_20261001에만 만든다.

- Protocol/Enum.proto의 use 계약과 enter capability 필드를 추가하고 기존 GenPackets/protoc/escape 정리 경로로 Server/DummyClient/Unreal을 생성했다. ID1000~1029 유지, 새1030~1032. 생성6개 파일 각각 세 위치의 SHA-256 일치. 새 Busy reason12는 transient UNCONFIRMED 상태이며 콘텐츠 쿨다운이 아니다.
- Room에서 인증된 CharacterId/ObjectId/생명 세대·현재HP 검증, 캐릭터당 pending1개/최대128, 범용200ms 요청 억제, worker callback 요청-ID 재검증을 한다. respawn에서 lifeGeneration 증가. 조회/재시도 중 실제 HP를 과거 값으로 덮어쓰지 않는다.
- 소비 확정 후 Room의 최신HP에 부족분 최대50을 **한 번만** 적용하고 공개 S_PLAYER_HEALED를 지원 peer에게 보낸다. APPLIED DB 최종 확인 뒤 요청자에게만 결과를 보낸다. 사망/세션 변경 등 효과 적용 전 무효화는 반환, 효과 적용 후 퇴장은 DB 마무리만 지속한다. private 결과/중복 응답은 HP/FX를 적용하지 않는다.
- 일반 유지 보수는 unknown 소비를 재실행하지 않고 같은 요청을 CharacterData 락 아래 조회한다. 확실히 기록이 없으면 미소비 pending을 해제하고 UNCONFIRMED로 둔다. 사용자가 명시적으로 같은 ID를 재확인하면 idempotent 소비 경로로 들어갈 수 있다. Finalize 재시도는 HP를 다시 적용하지 않는다. 부팅 복구의 희귀 적용-ACK 사이 반환 가능성은 승인된 정책 그대로다.
- InventorySubsystem에 공통 RequestUseItem(instanceId) 추가. 미래 퀵슬롯은 이 API만 재사용하고 지금 키/저장을 구현하지 않는다. Native I 창의 스택 선택/사용 버튼, 처리중 억제, 5초 미확정 및 같은 요청 수동 재확인, 세션 reset을 연결했다. 성공/반환 result는 최소버전과 fresh inventory query만 갱신한다. UI가 수량/HP를 직접 가산·감산하지 않는다. 동일버전 snapshot은 합계뿐 아니라 전체 instance/slot/definition/count 일치를 확인한다.
- WorldEntity에서 damage/status/heal의 공통 per-actor RoomEventSequence와 death 우선을 검사한다. 회복은 위치·movement mode·몽타주를 건드리지 않는다. snapshot은 FX 근거가 아니다. 효과 EventId 중복 제거 및 actor/world reset을 처리한다.
- Native S1PotionPresentationSettings의 exact healing soft path+CDO cook anchor를 추가했다. actor-owned Niagara를 capsule **로컬 unscaled half-height** 기준 발밑에 부착, scale1/offset0, emit1.5s/maxlife4s 시작값. collision/Nav 영향 off, death/despawn/reset/EndPlay 및 최대수명 정리. 원본 Niagara/Blueprint/map binary 저장·수정 없음. 실제 모양/밝기/크기는 사용자 확인 대기다.

호환성: use1이 negotiated된 참여자에게만 새 heal packet을 보낸다. legacy peer0은 사용/새 FX 이벤트 비활성이다. 구 observer의 새 회복 즉시 표시는 제공하지 않으며 이후 기존 damage/status/spawn에서 HP를 받는다. 모든 현재 테스트 클라이언트를 최신 버전으로 사용할 것. Loading 중 actor-ready 이전의 회복 이벤트 처리 및 혼합 구버전 관전은 실제 검증하지 않았다.

### 정확한 변경 파일

서버 `E:\Git Fork\GameServer\GameServer`: `Common\Protobuf\bin\Protocol.proto`, `Enum.proto`, `GameServer\Player.h`, `Room.h/.cpp`, `ClientPacketHandler.cpp`, `LootDB.h/.cpp`, `PotionUseDB.h/.cpp`, `PotionUseRuntime.h`, `LootService.h/.cpp`, `GameServer.vcxproj`; `DummyClient\ServerPacketHandler.cpp`; Server/DummyClient generated handler/6pb files. Tests의 PotionUseDBIntegrationTests.cpp/PotionUseRuntimeTests.cpp/BuildPotionUseTests.cmd 및 승인 SQL. ServerCore/DB 기존 table/HP 전체 영속화는 변경하지 않았다.

Unreal `E:\Git Fork\S1_UE5.8\Source\S1`: `Account\S1AccountSubsystem.cpp`, `S1GameInstance.h/.cpp`, `ClientPacketHandler.cpp` 및 generated handler, `Network\S1NetworkSubsystem.h/.cpp` 및 generated6pb, `Loot\S1InventoryState.h`, `S1InventorySubsystem.h/.cpp`, `S1PotionPresentationSettings.h/.cpp`, 신규 `S1PotionUseTests.cpp`, `UI\S1InventoryWidget.h/.cpp`, `World\S1WorldEntitySubsystem.h/.cpp`. 기존 MyPlayer input binding/전투 montage/무기/AI/Nav/Q/피해규칙은 변경하지 않았다.

### 실제 실행 증거

체크포인트 `C:\Users\user\Documents\Unreal IOCP Server\checkpoints\PotionUse_20261006`:

- 최신 GameServer Debug/Release 빌드 exit0 (`GameServerDebugBuild.log`, `GameServerReleaseBuild.log`); S1Editor/S1 Development 빌드 성공. 최종 후속 소스는 PackageFinal.log의 두 타깃 rebuild에서도 성공했다. 첫 sandbox 빌드는 PATH/Path 중복(MSB6001), Unreal CLR 환경 오류로 실패했으며 환경 정규화/정상 Windows SDK·캐시 실행으로 해결했다. 프로젝트/엔진 설정을 수정하지 않았다. 실패 로그는 보존했다.
- DBReadonlyGuardTests.log exit0: 새 read-only reconciliation의 미기록 요청이 **전혀 소비하지 않음**까지 포함해 기존 DB/worker assertions 재실행 통과. 운영 GameDB 사용 테스트가 아니다. RuntimeTestsFinal.log는 순수 상태 정책 증거다.
- `Saved\Automation\PotionUse_Final_20261006\index.json`: 총13 Success(12 warning-free,1 existing GameplayCueNotifyPaths warning), 실패0/notRun0. UseTransport/UseCoordinator/HealingPresentation + 기존 loot·Q·impact·DuskDeathFade. 최종 죽음 후 damage/회복으로 부활 불가, 새 alive status 이후 회복 허용 assertions도 통과. `Saved\Logs\PotionUse_Final.log`의 Fatal/Ensure/Blueprint Runtime Error/Network Failure/`: Error:` match0. NullRHI 합성 테스트이며 실제 소켓·GPU 시각 증거가 아니다.
- PackageFinal.log: Windows Development Build/Cook/Stage/Pak/IoStore/Package/Archive `BUILD SUCCESSFUL`, AutomationTool exit0. 최종 archive `E:\Git Fork\S1_UE5.8\Saved\PotionUsePackageFinal_20261006\Windows`. Cook registry에서 exact healing system와 Lobby/Loading/Main package 문자열을 확인했다. 실제 rendered asset 포함 검증은 다음 플레이 단계다.
- 최종 packaged executable 두 프로세스 NullRHI 시작/즉시 Quit smoke exit0. `PotionUse_PackageSmoke1/2.log`: Fatal/Ensure/Blueprint Runtime Error/Network Failure0. **기존 로그인 UIOnly/Non-Focusable SObjectWidget Error가 각1건 남음**. 새로운 인벤토리는 focusable GameAndUI이며 별도 기존 Auth UI 오류를 고친 작업이 아니다. 이것은 packaged 2인 로그인/전투/포션 플레이 검증이 아니다.
- 새 Debug GameServer PID36980: Loot Ready1/Use Supported1, startup stderr0. DB 관리자 bootstrap credential이 실행 환경에 없음을 확인했다. 서버 Nav/OBB 배포본은 원본 SHA-256 일치. 새 interactive Editor PID42468: MCP initialize/list_toolsets/IsPIERunning(false) 성공, 회복 원본 dirty=false. 기록 시점의 PID이며 이후 상태는 재확인해야 한다.

### 조작법·튜닝·남은 인수

G로 소유 포션 획득→I→스택 행 선택→사용. HP가50 이상 부족하면50, 그보다 적게 부족하면 부족분만 회복하고1개 소비. 최대HP/사망/0개는 회복·추가소비 없음. I/Esc/닫기로 인벤토리 종료. 사운드/버프/사용 모션/이동 잠금/퀵슬롯/포션 쿨다운 없음.

튜닝: Project Settings→Game→S1 Potion Presentation→Healing (FX path/scale/foot offset/emit/maxlife). 서버 회복50은 클라이언트 옵션이 아니다. 라이선스/공개 영상 사용 권한은 확인 대기.

필수 사용자1-/2-client PIE: 두 화면 HP/FX각1회, own 수량만1 감소/재접속 유지, 최대HP 무소비·FX없음, 부족분/50 회복, 마지막1개 스택 제거, UI 클릭 공격 혼선·닫기 복귀, G/기존 전투/사망페이드 회귀. 실제 delay/COMMIT 응답 손실/사용 중 disconnect·server kill/로딩 중 회복과 packaged 두 계정 플레이는 미검증. deadlock native-code bounded retry는 아직 추가하지 않았으며 DB 오류는 unknown/readback 처리한다. 기획 콘텐츠 규칙 변경 없음; 안전 복구·legacy 표현 제한을 명시했으며 신규 콘텐츠 결정은 요청하지 않는다.

### 최신 사용자 PIE 인수 — 2026-10-06

후속 패키징 상태: 같은 날 MultiDuskSpawnImplementation.md 작업에서 위치 Z96 조정 이후의 현재 클라이언트로 Windows Development 전체 Build/Cook/Stage/Package/Archive가 성공했다. 따라서 아래 이전 기록의 높이 조정 후 재패키징 대기는 해소됐다. 실제 패키지에서 포션 FX를 시각 확인하거나 두 계정 플레이를 인수한 것은 아니다.

사용자가 이번 수정본의 실제 **1인·2인 PIE**에서 회복량, 수량 감소, 양쪽 회복 이펙트 각 1회 표시, 최대HP 무소비, 재접속 후 수량 유지, 이펙트 높이 확인을 완료했다고 보고했다. 이 항목은 위 중간 기록의 사용/시각 인수 대기 상태를 대체한다. 회복 Niagara는 서버 확정 회복용 Active로 기록한다. 몸통 위치는 DefaultGame.ini의 HealingFootOffset Z96이며 원본 Niagara/Blueprint/map은 수정·저장하지 않았다.

이는 사용자 플레이 검증 증거다. 이 인수 기록·기획 전달 작업에서 Codex가 새 빌드, PIE, 실시간 로그 감사를 실행한 것은 아니다. 최대HP에서 FX가 없었는지 별도 명시 확인, 사망/0개 거부, 마지막 스택 제거, UI 입력 혼선, 현재 버전 전체 전투 회귀는 이번 사용자 보고만으로 추가 완료 처리하지 않는다. 실제 COMMIT 응답 손실, 강제 중복/지연, 사용 중 퇴장·프로세스 중단, 로딩/구버전 혼합 및 두 패키지 계정 플레이도 미검증이다. 격리 DB·합성 테스트 통과와 실제 장애 주입은 구분한다.

앞서 성공한 패키징은 Z96 위치 조정 전 결과다. 최신 config 재패키징 및 실제 패키지 플레이는 남아 있다. 기존 로그인 UI 포커스 오류, GameplayCue 경고, 적용 후 durable Applied ACK 이전 crash 시 다음 부팅에 포션이 반환될 수 있는 승인된 복구 정책, FX 출처/공개 영상 권한 확인도 별도 추적한다. 신규 콘텐츠나 정책 변경 없이 기획 담당에게 이 상태와 후속 최소 검증 범위를 전달한다.
