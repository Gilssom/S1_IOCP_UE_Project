# 회복 포션 드랍/획득 — 2026-10-01

최신 범위 안내 (2026-10-06): 아래 초기 기록의 포션 사용/HP+50 제외는 당시 획득 작업 범위다. 후속 사용자 승인으로 [PotionUseImplementation.md](PotionUseImplementation.md)의 서버 소비·회복/UI/FX를 구현하고 빌드·자동화·패키징을 검증했다. 이후 사용자가 이번 수정본의 실제1-/2-client PIE에서 회복량·수량 감소·양쪽 FX 각1회·최대HP 무소비·재접속 수량 유지·FX 높이 확인을 완료했다. 기존 G획득 인수를 재사용한 것이 아니라 별도의 최신 사용 기능 인수다. 최신 높이 config 재패키징·장애 주입·전체 회귀의 미검증 항목은 후속 사용 문서에서 별도 추적한다.

## 상태와 범위

공유 DB·프로토콜 최소안 **진행 승인 전달받음**. 현재는 실제 소스/읽기 전용 MCP/DB 메타데이터 조사 및 독립 UI·표현/DB 기반 구현 단계다. 드랍→G획득→DB확정→재접속 전체 기능이 완료됐다는 뜻이 아니다. 포션 사용/소비/HP+50/사운드는 제외(HP+50은 다음 단계 초안).

확정: 마지막 유효 타격의 캐릭터 소유, G로 한 드랍 획득, 두 화면에 드랍/효과 표시, 소유자에게만 `G 회복 포션 x1 획득` 안내. Dusk 시체와 드랍 수명 분리. BP_Bag는 참고만 사용하고 장비/통화/사용 버튼은 노출하지 않는다.

승인된 초기 테스트 설정(콘텐츠 최종 정책 아님): 매 사망 포션1개/100%,30초 만료,XY 중심거리150cm, 임시 ItemDefinitionId20001, 스택99/20슬롯, 서버 바닥Z=-0.5의 평지 기준. 높이차는 플레이어 캡슐 중심과 드랍 바닥을 바로 비교하지 않고 발 위치 기준100cm 이내. I로 인벤토리 열기/닫기 예정, 기존 G/I 바인딩 없음; UI 입력 중 공격/카메라/G 억제 예정. 기존 Tab/Esc/Q/Space와 충돌시키지 않는다. 아래 제안 중 승인 범위에 포함된 항목도 실제 런타임 구현/검증 상태와 구분한다.

## 확인된 현행 근거

- Unreal은 Git없음. Server Git에 이전 소스/proto/생성물/IDE/빌드 변경 다수 존재; 보존한다.
- 서버 `GameServer/CharacterDB.h`와 ClientPacketHandler의 C_ENTER_GAME은 DB 캐릭터를 인증 AccountId로 확인. ObjectUtils::CreatePlayer가 CharacterInfo를 복사하므로 `objectInfo.character_info.character_id`로 영구 귀속을 유도할 수 있다. ObjectId는 새 접속마다 생성되는 런타임 식별자다.
- Room::ResolvePlayerHit/ResolveKnightQHit의 유효 피해로 nextHp=0일 때 DEAD 설정. 공통 죽음 확정 지점을 두 경로에서 호출해 사망 UUID/몬스터 출현 UUID마다 한 드랍만 생성하도록 한다. MonsterDefinitionId1 유지.
- 실제 MySQL GameDB의 테이블은 account/characterdata/item 모두 InnoDB. 읽기 전용 SHOW CREATE TABLE 확인. CharacterData.character_id는 BIGINT UNSIGNED PK. Item은 item_id BIGINT UNSIGNED PK AUTO_INCREMENT,character_id BIGINT UNSIGNED FK, item_template_id INT UNSIGNED, item_count INT UNSIGNED default1,slot_index INT UNSIGNED. UNIQUE(character_id,slot_index),FK characterdata ON DELETE CASCADE. 조사 시 Item행0. 기존 Item 재사용; 별도 인벤토리 수량 테이블로 중복하지 않는다.
- 저장된 `codex_md_edit/db/schema.sql`은 현행 DB 전체가 아닌 Account 초안이며 GameDB.xml은 과거 Gold 예제다. 이를 현행 MySQL 스키마로 간주하지 않는다.
- 기존 패킷 handler IDs1000..1022, loot/inventory 패킷 없음. combat contract2 유지.
- 기존 DBConnectionPool/DBBind는 ODBC 동기 API. ClientPacketHandler는 직접 DB호출. 전용 비동기 DB 계층은 조사 범위에서 발견되지 않았다. JobQueue::DoAsync는 첫 Push 스레드에서 즉시 Execute할 수 있어 DB를 거기에 옮기는 것만으로 IOCP 비차단이 되지 않는다. 기존 연결 풀·DBBind 재사용 + 작은 전용 DB worker queue 필요. 기존 인증 흐름 전체 리팩터링은 하지 않는다.

## 공유 DB 변경안 (제안, 미적용)

기존 Item/Account/CharacterData 데이터·컬럼을 변경하지 않는다. MySQL8 InnoDB의 추가 테이블만 사용한다. ID UUID는 서버 생성16바이트(BINARY16), wire는 정확히16bytes, characterID는uint64이다. UTC는DATETIME(6)/UTC_TIMESTAMP(6). 스키마 버전1.

| 새 테이블 | 주요 필드·제약 |
|---|---|
| LootDrop | drop_id PK UUID,death_id UNIQUE UUID,spawn_id UUID,owner_character_id BIGINT UNSIGNED,room_id/server_id/server_boot_id,monster_definition_id,item_definition_id,count,x/y/z,created/expires_utc,status AVAILABLE/CLAIMED/EXPIRED,definition_version/drop_table_version; 수량양수 CHECK,expiry index. 역사 보존을 위해 소유자 삭제 CASCADE 금지. |
| ItemPickupRequest | PK(character_id,request_id UUID),drop_id,payload_fingerprint,result/reason,committed_count,item_instance_id,inventory_version,created_utc. 같은RequestId 다른DropId는 REQUEST_CONFLICT, 기존 결과 유지. 성공·정상 거부 내구성 저장; DB장애는 확정 거부로 영구 저장하지 않는다. |
| ItemGrantLedger | grant_id BIGINT UNSIGNED AUTO_INCREMENT PK,drop_id UNIQUE,character_id,request_id,death_id/spawn_id,item_definition_id,count,before_count/after_count,inventory_version,committed_utc; 요청FK(복합키),지급 수량양수 CHECK. 스택 합쳐져도 각 사망 출처 보존. |
| InventoryState | character_id PK/FK CharacterData,version BIGINT UNSIGNED; Item 갱신과 함께 증가. 현 단계 이 지급 경로만 version 변경; 다른 미래 writer도 같은 계약 필요. |
| CombatItemOutbox | event_id UUID PK,event_type,operation_id,occurred_utc,character_id,room/server,death_id/spawn_id/drop_id,itemID/count,result/reason,version/전후수량,definition/drop_version,payload JSON,delivered_utc,attempts; EventId 기준 중복 제거. |

아이템 정의/드랍표는 서버 전용 소규모 버전된 C++ 데이터(포션1종)로 시작; ItemDefinition DB 범용 프레임워크는 추가하지 않는다. 사용 효과는 현재 None/미구현이며 HP+50 자동 적용 금지.

### 트랜잭션/경쟁

1. 서버 Room은 인증된 Player/Character/Room 세대와 현재 생존·거리·차폐·만료를 검증하고 같은 드랍 claim을 처리중으로 예약한다. DB에 생성 확정되기 전에는 드랍을 방송하지 않는다. DB가 느리거나 실패해도 살아있는 다른 엔티티는 막지 않는다.
2. 드랍 생성 트랜잭션에 LootDrop(UNIQUE death_id) + MonsterDied/DropCreated outbox를 함께 저장. 원격에게 생성은 커밋 후. 실패시 Room의 동일death_id 작업으로 제한 재시도; 새DropId나 새사망을 만들지 않는다.
3. 획득 DB worker: START TRANSACTION → 동일RequestId 결과 확인/등록 → LootDrop SELECT FOR UPDATE → CHAR/InventoryState행 lock으로 캐릭터 갱신 직렬화 → owner/status/UTC만료 재확인 → Item 같은 template의 여유 스택 혹은 가장 낮은 빈 슬롯 선택 → Item 추가/증가, InventoryState version+1, LootDrop 조건부 AVAILABLE→CLAIMED(영향행1), UNIQUE DropId 원장, 요청성공 결과, ItemPickupCommitted outbox → COMMIT. 모든 변화는 하나의 트랜잭션. 조회후 더하기만 하는 구조 아님.
4. 같은 RequestId 재시도는 기존결과(이전 version 포함)만 반환. 다른RequestId로 같은DropId 지급은 row lock/조건부 갱신/UNIQUE 원장이 차단. 클라이언트는 과거version snapshot으로 최신 수량을 되돌리지 않는다.
5. 정상 거부는 ItemPickupRequest + ItemPickupRejected outbox를 커밋, Item/원장 변화 없음. malformed/unauthenticated는 서버 진단만 하며 허위 characterID를 원장에 넣지 않는다. 요청량 제한 필요.
6. 만료 worker도 같은 LootDrop행 lock/조건부 AVAILABLE→EXPIRED + DropExpired outbox 트랜잭션. 획득이 만료UTC 전에 claim검증을 통과해 lock을 보유하면 그 작업이 승리; 늦은 최초 claim은 만료. 처리중 TTL 이후 미확정 아이템을 확정 지급으로 표시하지 않는다. COMMIT 결과 불명확하면 같은RequestId를 새 연결로 조회하여 복구, 재가산하지 않는다.
7. DB완료는 Room::DoAsync로 게임 결과 반영(이때는blocking DB없음). 캡처한 weak session/Player/Room generation/characterID가 일치할 때만 응답; commit은 접속종료와 무관하게 유지. 성공 드랍 제거는 모든 살아있는 Room 클라이언트에 커밋 결과로만 broadcast. 오래된 세션은 새 플레이어/UI를 변경하지 않는다.
8. 응답 유실은 명시적 같은RequestId 재확인/재시도로 복구. 타임아웃=미확정 표시; 자동 새RequestId/무조건재전송 금지. 성공도 inventorysnapshot/version으로 재확인 가능.

### 복구·이벤트

같은 서버 프로세스/방 재접속: AVAILABLE 드랍을 캐릭터와 무관하게 양쪽 snapshot에 포함, 소유자만 안내. 인벤토리는 DB에서 새로조회. 서버 재시작: 이전 server_boot_id의 미획득 드랍을 EXPIRED로 조건부 변경하고 outbox 기록(포션 지급 없음); 확정 Item/원장/요청은 유지. 부팅 복구 완료 전 loot기능 off. DB pending/commit불명확은 row상태/기존RequestId로 정리.

필수 이벤트 MonsterDied,DropCreated,DropExpired,ItemPickupCommitted,ItemPickupRejected. EventId·RequestId/OperationId·UTC·CharacterId·Room/Server·Spawn/Death/DropId·ItemId/수량·result/reason·InventoryVersion/전후수량·정의/드랍표버전 연결. DB outbox가 사실의 원본, worker가 구조화 JSONL sink로 발행후delivered표시; 재시작 중복 발행 가능하므로 소비자는 EventId로 제거. 콘솔로그는 원장 대체가 아니다. 별도AIAgent/Kafka/제재/획득횟수 임계값 없음. 비밀번호/토큰 기록 금지.

### 마이그레이션/롤백

승인 후 명시 SQL 파일 작성, SHOW CREATE/권한/기존Item 데이터 재확인, DB백업 또는 복구가능 checkpoint 후 별도 테스트 DB 먼저 검증. 실제 스키마 자동 DDL/DBSynchronizer 변경 없음. 신규FK 타입은 실제 BIGINT UNSIGNED와 일치. 적용은 새5개 테이블 CREATE + migration version기록(실제 환경 기록 방식 확인).

롤백은 loot 기능/계약 광고 중지 및 구바이너리 복귀; 새 테이블/Item 지급·원장 데이터는 보존. DROP/TRUNCATE/Item 감산 자동 롤백 금지. 수동 데이터 삭제는 별도 승인. 계약 미지원 클라이언트에는 신규패킷 전송 금지.

## 패킷 변경안 (제안, 미생성)

아래 계약의 proto/생성물/typed dispatch는 2026-10-01 단계적 구현 기록에서 적용됐다. 서버 Room/DB/G/I/월드 소비자는 아직 미연결이며 버전1 지원 광고는 활성화하지 않았다. 계약 정의와 플레이 가능 구현을 구분한다.

canonical `E:/Git Fork/GameServer/GameServer/Common/Protobuf/bin/{Protocol,Struct,Enum}.proto`에 append, 기존 메시지순서/ID1000..1022 유지. Server/DummyClient/S1 outputs는protoc+기존 escape보정/handler생성 방식 점검 후 동시 갱신. 생성파일 직접편집 금지.

S_ENTER_GAME field4 `uint32 loot_contract_version=1` 광고(기존combat field3 유지). C_ENTER_GAME field2 `uint32 loot_contract_version=1` 요청. 서로1일 때만 신규 loot메시지/기능 허용. 기존클라이언트 version0은전투만, 새클라이언트 oldserver0은G/I지급기능숨김. 양쪽업데이트/서버재시작 필요.

| ID | 메시지·필드(필드번호) |
|---|---|
|1023| S_LOOT_SNAPSHOT: repeated LootDropInfo drops(1),uint64 snapshot_sequence(2),uint32 contract_version(3). 방 입장 전체목록 교체; 초기ready전 LevelFlow buffer. |
|1024| S_LOOT_CREATED: LootDropInfo drop(1),uint64 event_sequence(2). |
|1025| S_LOOT_REMOVED: bytes drop_id(1),enum reason PICKED_UP/EXPIRED(2),uint64 event_sequence(3). |
|1026| C_ITEM_PICKUP: bytes drop_id(1),bytes request_id(2),uint32 contract_version(3). 캐릭터/수량/위치 보내지 않음. |
|1027| S_ITEM_PICKUP_RESULT: bytes drop_id(1),request_id(2),enum decision COMMITTED/REJECTED/UNCONFIRMED(3),reason(4),uint64 inventory_version(5),uint64 item_instance_id(6),uint32 resulting_count(7). |
|1028| C_INVENTORY_QUERY: bytes request_id(1),uint32 contract_version(2). |
|1029| S_INVENTORY_SNAPSHOT: bytes request_id(1),uint64 character_id(2),uint64 inventory_version(3),repeated InventoryEntry entries(4),bool success(5),reason(6). |

LootDropInfo: bytes drop_id(1),uint64 owner_character_id(2),uint32 item_definition_id(3),uint32 count(4),float x/y/z(5/6/7),uint64 expires_utc_ms(8),uint32 definition_version(9). InventoryEntry: uint64 item_instance_id(1),uint32 item_definition_id(2),uint32 count(3),uint32 slot_index(4). ID20001과 패킷1023..1029는 승인된 구현 예정 계약이며 아직 현행 바이너리에 구현되어 있지 않다.

거부 사유: INVALID_REQUEST,REQUEST_CONFLICT,NOT_FOUND,WRONG_ROOM,NOT_OWNER,DEAD,OUT_OF_RANGE,OCCLUDED,EXPIRED,INVENTORY_FULL,STACK_LIMIT,ALREADY_CLAIMED,UNSUPPORTED_CONTRACT. DB_UNAVAILABLE는 미확정/재시도 가능 상태이며 성공이나 영구 거부로 단정하지 않는다.

## 차폐/지형 제안

현 MainMap Nav path거리만으로 모든 벽 차폐를 보장하지 않는다. 승인 후 기존 S1NavigationTools의 read-only export에 MainMap 정적벽 collision OBB 목록을 별도 데이터로 추가하고 서버가 XY획득선분 vs벽범위를 검사하도록 하는 최소안. 경사/계단/동적장애물/Nav재작성 제외. 실제테스트벽 Actor/충돌/배치 확인 후 정확한export 대상과 파일을 보고한다. 단순 Nav path≤150을 완전한 LOS라 보고하지 않는다. XY거리/발높이/직선차폐 모두서버검증; 로컬안내는힌트일뿐권한아님.

## 조사된 에셋

- potion_large02: StaticMesh, 슬롯1 potions_mat, 로컬bounds min(-12.584,-12.633,0),max(14.584,12.633,42.573)cm. dirtyfalse; referencer Loot_Set_Layout. 시험scale0.4(높이17.03cm),pivot바닥Z0이므로wrapper바닥offset0 시작; 실제모양/바닥은미검증.
- Rair_Potion: Texture2D,sRGBtrue,TC_Default,maxTextureSize0. 최초 조사 때 dirtytrue/로컬uasset없음. 진행 승인에 포함된 해당 패키지 첫 저장을 수행했다. 저장직전 class/속성/dirty 재확인; 초기 referencers 오류는 미저장 registry 상태였다. save_assets에 정확히 이 패키지만 전달, 반환true; 저장후 dirtyfalse, referencers=[], 디스크21746bytes, 최초 저장본 SHA-256 체크포인트 일치. 이미지 픽셀/실제 UI 표시는 미검증.
- NE_drop_effects01: 이름NE지만NiagaraSystem. 7활성Sprite emitter(CPU5/GPU2),사용자변수없음. materials M_beam/M_cell/M_down/M_glow. dirtyfalse,referencerDrapEffet/Maps/main. 모듈loop/lifetime실값·shader·실제표현은미검증. wrapper컴포넌트소유로획득/만료/EndPlay에 Deactivate+Destroy; 원본수정/무조건loop추정금지.
- BP_Bag: UserWidget parent WidgetBlueprintGeneratedClass BP_Bag_C,UMG텍스처 다수,dirtyfalse,참고맵OverviewMap. 타입 API는자동생성CDO로반환해ParentClass프로퍼티 조회실패; AssetRegistry tags로UMG parent확인. 디자인참고만사용.
- 네에셋소유/출처·영상공개권한미확인: Candidate로기록,공개전확인. 새구매/UE5_RPG이관없음.

## 변경 예정 파일 (클라이언트 root S1_UE5.8,서버root GameServer/GameServer)

독립 준비: `Source/S1/Loot/S1PotionPresentationSettings.h/.cpp`, `Source/S1/UI/S1InventoryWidget.h/.cpp`, 이문서/CurrentAssetRegistry. 기존gameplay연결·proto·DB변경없음.

승인 후 서버: GameServer/Room.h/.cpp,Player.h,ClientPacketHandler.cpp,GameServer.cpp,GameServer.vcxproj; 신규 LootDefinition.h,LootService.h/.cpp,LootDB.h,AsyncLootDB.h/.cpp; Tests/Loot*; codex_md_edit/db/PotionLootMigration.sql/검증SQL; canonicalproto및양쪽handler/generatedoutputs. ClientPacketHandler.h/ServerPacketHandler.h는기존generator출력방식확인후생성.

승인 후 클라이언트: Network/S1NetworkSubsystem.h/.cpp,ClientPacketHandler.cpp,World/S1WorldEntitySubsystem.h/.cpp,Game/S1MyPlayer.h/.cpp,S1GameInstance.h/.cpp,Level/S1LevelFlowSubsystem.h/.cpp(실제 Source/S1/Level 위치 확인),신규 Loot/S1LootActor.h/.cpp,Loot/S1InventorySubsystem.h/.cpp,위UI/설정,Config/DefaultGame.ini(softasset cook포함만),S1.Build.cs(Niagara dependency),자동테스트. 이 목록의 기존 gameplay/network 파일은 현재 미변경.

## 인수/남은 확인

승인 후 단위/DB테스트: 같은사망중복,RequestId재시도/충돌,다른RequestId동시claim,Item/원장/outbox원자성,rollback실패·commit응답유실,만료경쟁,캐릭터귀속,DB완료시세션변경,재시작복구. 격리된테스트DB/정리범위먼저보고; 실제계정비밀로그금지.

1인·2인 PIE 인수 예정: 실제처치/G/소유안내/DB확정/바닥제거·인벤토리수량·재접속,양쪽표현일치,연타·비소유자·벽/높이/거리,기존전투/페이드/Nav회귀. 서버DB원장에서 DropId→DeathId→CharacterId 일치 및중복성공0 확인. 실행한 기반 빌드와 아직 실행하지 않은 DB/플레이 검증은 아래 Foundation implementation 기록으로 구분한다.

## Foundation implementation and evidence — 2026-10-01

Implemented standalone foundation, **not a playable completed potion feature**:

- Server new `GameServer/LootDefinition.h`: test policy constants, exact16-byte nonzero ID parsing, finite XY/foot-height range guard. ID20001 is not yet a live item-definition registration; no existing Item rows were found at the initial read-only inspection.
- `GameServer/AsyncLootDB.h`: bounded FIFO dedicated worker, never inline execution, accepted-job draining, rejection under queue pressure, required exception completion callback, optional worker initialization hook. Runtime integration must pass `ThreadManager::InitTLS` and marshal callbacks into Room. Not yet attached to the server process.
- `GameServer/LootDB.h/.cpp`: existing DBConnection/DBBind reuse; drop creation + two outbox events; CharacterData lock serializes pickup requests; exact request/drop correspondence; drop row lock, conditional claim, Item increment/slot allocation, version, result, unique grant ledger and outbox in one transaction. Rejections do not increment Item. Failed/uncertain commit returns UNCONFIRMED. No blind regrant. ReadInventory checks row count to reject a partial fetch. Expire locks the same drop and records one expiration event. First foundation pass was compilation only; the subsequent isolated DB checks below have now executed. UUID creation, boot recovery, outbox publishing, Room/session validation and response reconciliation are still integration work. Expire now returns explicit Expired/NoChange/Unconfirmed; a successful no-op is not a removal signal.
- Client new `Source/S1/Loot/S1PotionPresentationSettings.h/.cpp`: exact user asset paths, visual-only scale0.4/floor offset0/effect scale1; no server ownership/damage rules.
- `Source/S1/UI/S1InventoryWidget.h/.cpp`: passive icon/name/count/status/close-only Slate view. No socket/protocol/DB operations. Not yet shown or input-bound in gameplay.
- `Source/S1/Loot/S1PotionPresentationTests.cpp`: settings automation added; NOT executed yet.

Executed:

1. Seven existing source/document checkpoints copied and SHA-256 verified under `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PotionLoot_20261001` before edits. Existing dirty server Git changes preserved.
2. S1 Win64 Development gather build succeeded after correcting UE5.8 AllowedClasses metadata to full script paths. New settings/UI/test sources compiled and S1.exe linked. Evidence `PotionFoundation_S1Build.log` in that checkpoint directory. **S1Editor build and settings automation not yet run**; running editor still has the previous native module.
3. `Tests/LootFoundationTests.cpp` built and executed successfully: ID length/zero rejection, finite range boundaries, worker thread distinction, capacity rejection, drain, stopped submission, exception completion/continuation. `LootFoundationTests.exe` retained in checkpoint directory.
4. LootDB.cpp compiled standalone to LootDB.obj successfully against existing server headers. Existing Core/protobuf header warnings remain; this is NOT a complete GameServer target build or DB validation.
5. `Tests/LootDBIntegrationTests.cpp` compiled and linked to `LootDBIntegrationTests.exe`. Test schema is hard-coded `S1LootTest_20261001`, never accepts live GameDB override. Runtime tests authored for creation retry, repeated request, conflicting request, owner/room/death/occlusion rejection, concurrent claim, expiration, injected ledger constraint rollback, and fresh ODBC reconnect snapshot. **Executable not run yet; no claims of these checks passing.** It retains identifiable isolated fixtures rather than deleting data. Process restart, commit response loss and network reconnect remain separate tests.
6. Consistent logical backup via `codex_md_edit/db/BackupPotionBaseline.ps1`: mysqldump single-transaction/skip-lock-tables, three baseline table CREATE declarations checked, SHA-256 obtained; no password/account rows printed. Backup `GameDB_before_loot.sql` in checkpoint directory. Restore rehearsal is NOT performed yet. Keep this credential-sensitive backup local; do not attach/publish it.
7. Saved only `/Game/_Resources/Rair_Potion`, with exact path/dirty/class/props checks and first-save hash checkpoint. Pre-save scope: Rair_Potion=true, LobbyMap=true; MainMap/LoadingMap/potion mesh/dropNiagara/BP_Bag=false. LobbyMap left untouched. This is a scoped inventory, not an exhaustive editor-global dirty scan.

### Current external blockers / safe next step

Read-only SHOW GRANTS confirmed game_server@127.0.0.1 has SELECT/INSERT/UPDATE/DELETE/EXECUTE on GameDB but no CREATE/ALTER or test database privileges. Do not elevate this runtime account or request the administrator password in chat.

Administrator in MySQL Workbench:

1. Open `E:/Git Fork/GameServer/codex_md_edit/db/PotionLootTestSetup.sql` and execute using the administrator connection. It creates isolated schema and empty baseline CharacterData/Item copies; only DML rights for runtime test account.
2. Select **S1LootTest_20261001** as default schema, open `PotionLootMigration.sql` in the same directory, execute its five additive CREATE statements. Do NOT execute the first trial against GameDB.
3. Notify Codex: verify schema and ID collisions read-only, run the real C++ integration tests; perform isolated backup restore rehearsal before any live migration. Then apply live additive migration using administrator connection, preserving existing data. No DROP/TRUNCATE/reverse Item adjustment.

Editor was subsequently closed and the S1Editor build completed as recorded below. GameServer can stay running until runtime integration/build application. No live schema, canonical proto, generated files, existing gameplay code, original vendor asset, map or server binary changed in this foundation step. G/pickup/world drop/DB persistence/remote UI are **not yet playable**. Niagara render, authenticated1-/2-client PIE and packaging all pending.

## Isolated DB verification follow-up — 2026-10-01

User reported finishing administrator setup after the initial CREATE-denied error. Read-only verification established all seven isolated tables as InnoDB and DML grant on `s1loottest_20261001` for the runtime account. No global CREATE/ALTER privilege was added; GameDB rights unchanged. Tests explicitly connect only to this schema and verify DATABASE() before writing fixtures.

Executed real C++/ODBC integration test before and after narrow hardening. Final executable exit0 and all assertions passed:

- Exact drop creation retry leaves two initial outbox records; another DropId with the same DeathId creates no row.
- Same pickup RequestId returns the original item/count/version without another grant; another DropId (including nonexistent ID) with that RequestId is REQUEST_CONFLICT. Conflict gets a durable rejection outbox without rewriting the original request result or inventing missing monster/death IDs.
- Another request for claimed drop is rejected, owner/room/dead/occluded checks exercised. The latter two are **supplied validated-reason cases**, NOT proof of live Room LOS/death validation or actual wall geometry.
- Two connections submit different requests for the same drop: exactly one commit and one grant ledger; inventory count2/version2 after two valid drops.
- Future expiry returns NoChange and preserves AVAILABLE. Expired request rejected; expiry changes state once and emits one event; duplicate expiry returns NoChange. Expire now distinguishes Unconfirmed rather than exposing a misleading bool.
- Injected unique-ledger conflict after inventory/claim work returns UNCONFIRMED and rolls back Item, version, claim, request result. Intentional fault fixture is retained only in isolated schema and is NOT a genuine successful source-chain record.
- Fresh ODBC pool restores count2/version2. This is DB connection restoration, **not player/network reconnect or process restart**.
- Existing99-item stacks do not exceed99;19 occupied full slots admit a new1-item stack at slot19.20 full slots reject with InventoryFull and leave drop AVAILABLE.
- Concurrent forced expiration/pickup yields one winner and at most one grant. Forced expiry exercises the row-lock competition, not actual boot-recovery implementation.

Evidence: `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PotionLoot_20261001/LootDBIntegrationTests_final.log`; source `E:/Git Fork/GameServer/GameServer/Tests/LootDBIntegrationTests.cpp`. Tests retain identifiable synthetic `loot_...` characters and fault fixtures; no production GameDB writes/deletes.

S1Editor Win64 Development gather build succeeded; evidence `PotionFoundation_S1EditorBuild.log` in checkpoint directory. NullRHI automation executed and report confirms5Success,0failed (4 succeeded without warning,1 with existing GameplayCueNotifyPaths warning). Paths: `Saved/Automation/PotionFoundation_20261001/index.json`, `Saved/Logs/PotionFoundation_Automation.log`. Tests: ImpactSettings, KnightChargePresentation, KnightQApproval, Loot.PresentationSettings, DuskDeathFade. No logged Error/Fatal/Ensure in this run. SDK/profile baseline notices remain; no rendering/network/PIE assertion is inferred from NullRHI.

Remaining live-deployment gate: backup restoration rehearsal. Runtime account cannot create a restoration schema. Administrator action prepared in `E:/Git Fork/GameServer/codex_md_edit/db/PotionLootRestorePreparation.sql`: create new empty `S1LootRestore_20261001`, SELECT-only grant, then import private `GameDB_before_loot.sql` into that schema. Backup verified to contain0USE statements and3DROP TABLE statements; do not import against GameDB or the populated loot test schema. If restoration schema already exists, stop rather than overwrite. Authentication backup stays private. After import, Codex must compare restored schema/rows with the captured backup before any live additive migration. Shared proto/runtime hookup/outbox exporter/boot recovery/MainMap static-wall pickup checks/G and inventory binding/PIE/packaging remain pending; this follow-up is not feature completion.

## World presentation preparation — 2026-10-01

The user requested continuing. Read-only DB metadata still exposed only the seven-table loot test schema; restoration schema is not accessible/present. No live migration attempted. Editor is closed, GameServer running, MCP8000 probe unavailable. Earlier live MCP asset intake is retained, supplemented with a fresh read-only headless editor process; no unsupported MCP operations claimed.

New files `Source/S1/Loot/S1LootActor.h/.cpp`, `S1LootPresentationTests.cpp`; `S1.Build.cs` adds private Niagara runtime dependency. Build.cs pre-change SHA-256 checkpoint verified. No existing gameplay/network/authority code changed; actor is **not yet spawned by WorldEntity or wired to G**.

- AS1LootActor is nonreplicated, collision/overlap/nav influence disabled. Root location accepts server floor/pivot coordinates, with only configured visual mesh offset/scale; never reuses Dusk capsule-center Z as potion floor.
- Accepts opaque nonzero32hex server DropId, nonzero CharacterId/count and finite position. Per-actor metadata cannot change owner/count/identity. Duplicate snapshot does not reactivate Niagara. Source positions still require actual Room authority/packet validation during integration.
- Owned mesh and Niagara components load the exact selected settings assets; source Niagara is never edited. AutoDestroy=false because actor owns cleanup; activates once. DeactivateImmediate+DestroyComponent on explicit authoritative retirement or EndPlay; repeated cleanup is safe. No local clock-expiry/destruction, overlap-based grant, inventory increment, montage coupling or alternate UE authority.
- Only selected owner's hint is visible; nonowner/localCharacter0/unselected suppress it. Pending displays processing text while retaining the drop; coordinator still needs request/timeout/result state. Hint faces the local camera. Native TextRender's Korean font readability and glyph coverage are unverified in real rendering; if necessary use a widget overlay in the owning UI integration.
- Retired metadata/hint are cleared and late snapshots cannot resurrect that actor. First visual load is currently synchronous soft-path loading; first-drop hitch/async preloading needs runtime review. Imported effect looping/duration remains unchanged and unverified; do not claim it fills the30s lifetime.

Verification performed:

1. S1Editor Development compiled/linked with Niagara, including new actor/test. Initial test failed because fixture used34hex rather than32; validation was preserved and fixture corrected. Final six-test report: **6Success,0failed** (5 without warnings,1 existing GameplayCueNotifyPaths warning). ActorLifecycle checks immutable identity/ownership, collision disabled, server floor position, owner selection/pending retention, idempotent retirement and late-update rejection. Synthetic world does not BeginPlay/load/activate the selected GPU effect, so this is not GPU lifetime or render proof.
2. Evidence `Saved/Automation/LootActor_Final_20261001/index.json`, `Saved/Logs/LootActor_AutomationFinal.log`, checkpoint `LootActor_S1EditorBuildFinal.log`. Initial failing report retained separately as `Saved/Automation/LootActor_20261001`; do not misrepresent it as successful.
3. `InspectLootAssets.py` headless read-only script loaded exact StaticMesh/Texture2D/NiagaraSystem/WidgetBlueprint and recursively checked hard+soft game-package dependencies: mesh5,icon1,FX10,reference UI34 packages, missing=[] for each. Log `Saved/Logs/LootActor_AssetReadOnly.log`, commandlet exit0/0errors, one MCP licensing warning. Process dirty content only existing Terra mesh, NOT saved. Original/vendor/map/icon packages were not saved in this step. NullRHI is not shader/GPU visual proof.
4. Windows Build/Cook/Stage/Package/Archive initiated for Niagara module compatibility; final result is appended after completion. Unwired soft assets are not automatically considered included by a successful package; exact selected loot asset cook coverage is still a separate integration task.

Pending: actual server drop/packet/world registry ownership, nearest-candidate selection/G/I/UI input gating, expired/claimed broadcasts, outbox exporter/boot recovery, wall pickup checks, live schema and rendered1-/2-client PIE/packaged play. No policy change; HP+50 use/consumption excluded. No active gameplay acceptance claimed.

## Restore rehearsal and exact cook coverage — 2026-10-01

Latest status supersedes earlier inaccessible/restoration-pending records; no playable feature completion is implied.

- Administrator CREATE returned1007 because `S1LootRestore_20261001` already existed. Nothing was dropped or recreated. After administrator SELECT-only grant, read-only metadata showed zero tables. User imported the private baseline backup into this empty restoration schema, not GameDB.
- Read-only mysqldump of restoration schema was compared privately with `GameDB_before_loot.sql`: account/characterdata/item CREATE definitions and INSERT statements all exactly match after line-ending normalization. Item has zero INSERT statements in both. No authentication values were printed. Evidence artifact `checkpoints/PotionLoot_20261001/RestoreRehearsal_readonly_v2.sql` is credential-sensitive and must remain private, just like the original backup. An initial inspection launcher failed before comparison; retry completed successfully without DB mutation.
- User completed administrator execution of `PotionLootMigration.sql` targeting GameDB. Read-only inspection verified all eight tables InnoDB; five new tables are empty and match isolated tested definitions after excluding next AUTO_INCREMENT counters. Existing account/characterdata/item CREATE definitions match the original backup after the same counter normalization, including original foreign keys and checks. Existing Item remains empty. No runtime CREATE/ALTER privileges were granted and no automatic live DDL was performed.
- Initial live-versus-test literal schema comparison reported differences: ledger counter absent/live versus populated/test; baseline copies created with LIKE intentionally lack original foreign keys and use an automatically named CharacterData check. Follow-up comparison correctly uses the original backup for baseline tables and ignores only next AUTO_INCREMENT counters. All eight comparisons passed. Isolated DB tests do not cover original Account foreign-key integration; authenticated live runtime tests remain required. No schema was changed to force a comparison match.
- First successful package omitted the three unwired soft-referenced assets. Native settings now retain exact mesh/icon/Niagara default references through reflected hard properties and ConstructorHelpers; no broad folder cook, vendor asset modification or map save. Configurable visual soft paths remain available; replacing defaults later requires a corresponding cook rule/reference review.
- Latest S1Editor build succeeded. Latest automation `Saved/Automation/LootCookAnchor_20261001/index.json`:5 succeeded,1 succeeded with existing GameplayCueNotifyPaths warning,0 failed,0 notRun. All six tests report Success; no Error/Fatal/Ensure found in `Saved/Logs/LootCookAnchor_Automation.log`. NullRHI does not verify rendering.
- Latest Windows Build/Cook/Stage/Package/Archive succeeded, AutomationTool ExitCode0. Evidence `checkpoints/PotionLoot_20261001/LootActor_CookAnchorPackage.log`; archive `Saved/LootActorCookPackage_20261001/Windows`. Cook registry contains exact potion_large02/Rair_Potion/NE_drop_effects01 package names and LobbyMap/LoadingMap/MainMap. This verifies cook inclusion, not actual GPU effect visibility, packaged gameplay or authenticated two-client behavior.
- Canonical proto, live runtime loot integration, G/I input, world registry, outbox delivery, boot recovery, wall pickup checks and rendered1-/2-client acceptance remain pending. Live additive schema is verified, but no loot flow is attached to the running server yet. No original vendor/map binary saved in this follow-up.

## Loot protocol transport stage — 2026-10-01

This stage applies the approved wire contract, **not the playable drop/pickup flow**. No new DB writes, binary-asset saves, runtime server restart or balance change.

Implemented:

- Canonical `GameServer/Common/Protobuf/bin/{Enum,Struct,Protocol}.proto`: append packet messages1023..1029, LootDropInfo/InventoryEntry, stable removal/decision/reason enums, C_ENTER_GAME loot version field2 and S_ENTER_GAME field4. Existing23 packet IDs1000..1022 checked against pre-change checkpoint, all preserved. No combat field, Q contract or AbilityId changed.
- Existing protoc/GenPackets/FixProtobufEscapes pipeline regenerated server, DummyClient and S1 six Protobuf files and handler headers. All six generated file hashes identical across the three consumers; no generated file manually edited. First launcher hit the existing script execution policy; rerun used the existing batch workflow's child-process ExecutionPolicy Bypass, not a machine-wide policy change.
- S1 `ClientPacketHandler.cpp` routes five new server packets through `US1NetworkSubsystem` typed multicast events. Notifications require game-thread execution; existing PacketSession queue/ticker stays responsible for thread handoff. Network transport does not create world drops, grant inventory, display UI or perform travel.
- Server `ClientPacketHandler.cpp` contains safe **staged unsupported endpoints**, not pickup implementation: entered-player check, explicit version1,16-byte nonzero request/drop ID validation. Explicit pickup returns REJECTED/UNSUPPORTED_CONTRACT; inventory query returns success=false/UNSUPPORTED_CONTRACT. No DB/job scheduling or grants here. Malformed requests return false. Legacy clients are not proactively sent new messages.
- Both runtimes still default loot version to0 in enter-game packets. Do not advertise1 until Room/DB/client consumers and recovery are wired; current G/I behavior unchanged. DummyClient new handlers consume only for link compatibility; no loot request generation.
- `Source/S1/Loot/S1LootProtocolTests.cpp`: packet IDs, legacy version defaults, unchanged combat version, binary UUID preservation including embedded zeroes, framing size/id, truncated protobuf rejection, actual generated-handler → typed-event dispatch for five packets,64-bit inventory version, UNCONFIRMED distinction. These tests do not exercise sockets/partial I/O/live sessions/room validation or world mutation.

Evidence / pending:

- 29 pre-change source/generated files copied and SHA-256 verified under `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PotionLootProtocol_20261001`. All pre-existing nonblank lines in the three canonical proto files also verified preserved in order against these task checkpoints; Git diff alone includes historical changes and is not this task's diff.
- Server/DummyClient Release full builds passed (`ServerReleaseBuildRetry.log`, exit0). Initial restricted-process build failed with MSB6001 duplicate Path/PATH environment entries; same build with normal approved execution environment passed without editing project settings. Running Debug GameServer PID8396 was left untouched and still uses previous binary.
- S1Editor build passed (`S1EditorBuild.log`). Initial automation report `Saved/Automation/LootProtocol_20261001/index.json`:6Success,1Fail. Protocol test initially created a GameInstanceSubsystem with invalid transient-package Outer, causing Ensure. Test corrected to explicit UGameInstance Outer; preserve failed report and do not claim it passed. Latest rerun results appended below.
- S1 Win64 Development game build succeeded (`S1GameBuild.log`). Corrected S1Editor build succeeded (`S1EditorBuildCorrectedTest.log`). An overlapping build-launch attempt was interrupted while waiting for the game build: batch wait loop could not find ping in process PATH. Retry was serialized and added System32 to that process PATH only, without modifying system/project settings.
- Final NullRHI automation `Saved/Automation/LootProtocol_Final_20261001/index.json`: **7Success,0Fail,0notRun** (6 without warnings,1 existing GameplayCueNotifyPaths warning). `Saved/Logs/LootProtocol_AutomationFinal.log` contains no Error/Fatal/Ensure match. Actual generated dispatch tests passed for all five typed loot events, binary IDs, legacy defaults, framing and malformed protobuf. Test fixture ownership correction is verified; original failed evidence remains preserved.
- Full protocol revision packaging, authenticated1-/2-client PIE and socket packet exchange remain pending; earlier package/test successes are not this revision's runtime acceptance. MCP read-only availability check found port8000 unavailable because editor was closed; no live MCP inspection or editor mutation claimed for this step.

Next coherent runtime unit: boot ID/old-boot expiry/outbox delivery + dedicated DB queue lifetime and Room session-safe completion, then drop creation on one authoritative death point. Follow with client snapshot buffering/WorldEntity registry, G selection and inventory coordinator/UI, static-wall pickup validation and integrated multiplayer acceptance. No HP+50/use implementation. Existing approval covers the declared minimal contract; no blanket reapproval needed.

## Server death/drop runtime unit - 2026-10-02 (integration build blocked)

This record supersedes the previous statement that no live-runtime hookup exists in source. It does NOT mean the running server has been upgraded or the potion is playable. User reported closing Debug GameServer/VS/Editor for the build; process inspection confirmed the server stopped, but an UnrealEditor process still appeared afterward and was not forcibly closed.

### Implemented source, not yet authenticated PIE acceptance

- `GameServer/GameServer/{LootService.h,LootService.cpp}`: process-owned bounded dedicated DB queue, random BootId, previous-boot AVAILABLE expiry for this ServerId, same-ID creation/readback after uncertain commit, expiry/readback and durable outbox publishing. Worker callbacks marshal to Room jobs; they do not capture strong sessions or players. Explicit destructor drains the worker before its referenced service fields/pool are destroyed. DB downtime leaves the feature unready rather than inventing a grant. Initialization recovery has no automatic retry yet.
- `LootDB.h/.cpp`: authoritative drop readback includes immutable source/owner IDs and UTC expiry; WCHAR results use the legacy wrapper's actual byte-length binding without changing its general string-binding contract. UTC DATETIME conversion uses TIMESTAMPDIFF, not session-dependent UNIX_TIMESTAMP. Previous-boot recovery runs in batches of64. Outbox JSONL append is flushed before delivered_utc is committed; lost acknowledgement replays the same EventId, so readers MUST deduplicate EventId and skip blank/incomplete lines. JSONL contains no passwords/account secrets. Current sink is `Logs/CombatItemOutbox.jsonl` relative to server working directory. Single publisher per service; competing opens fail safely and retain DB records.
- `Room.h/.cpp`: new SpawnId per Dusk spawn and DeathId/DropId per authoritative death. Both existing lethal primary/Q resolution branches request the same death helper; server DEAD state still guards repeat death. CharacterId comes from the actual killing player's server character info. Death position captures XY and flat foot Z=-0.5cm, not the62cm capsule-center offset. DB completion does not depend on the now-respawned monster or the original session. Owner departure does not turn the reward into a different player's drop.
- Pending creation queue is capped128; queue pressure/unknown DB outcome retries retain the exact source IDs and DB TTL. Retry intervals are1s on submission pressure and2s on unknown completion; there is currently no attempt-count limit. A full queue or unavailable initialization logs degraded loot and skips a new drop, rather than blocking combat or silently granting an item. This degradation/retry policy is an operational limitation requiring review before final acceptance.
- Room registers AVAILABLE only after confirmed DB readback and removes only after confirmed EXPIRED/CLAIMED state. Creation/removal packets and an independent loot sequence exist, but broadcasts require negotiated `Player::lootContractVersion==1`. `Player.h` field remains0, enter-game advertisement remains0. Thus **no current client receives these packets and no potion appears yet**. Entry snapshots, G pickup, inventory query/coordinator/UI and static-wall pickup checks are still unwired. Existing staged unsupported handlers remain unchanged.
- `GameServer.cpp` owns the service and configures Room before starting IOCP. `GameServer.vcxproj` includes its sources/headers. No .proto/schema changes in this unit, no Blueprint/vendor/map asset saves, no combat balance/geometry change.

### Shutdown defects found during real tests

Initial isolated runs reached all DB assertions but hung on process shutdown. Those logs are failures, not successful completed tests. Temporary instrumentation proved the worker joined normally; the hang was in existing shared DB/global cleanup, not outbox replay.

Narrow source fixes: `ServerCore/DBConnection.cpp` frees STMT, disconnects DBC, then frees DBC; `DBConnectionPool.cpp/.h` clears owned connections before ENV, explicitly invokes DBConnection::Clear (the type has no cleanup destructor), and avoids runtime/TLS lock profiling in the pool destructor after all users have joined. Runtime Clear still locks. `CoreGlobal.cpp` deletes the DB pool before its allocation/lock dependencies. Static teardown occurs after TLS destruction; entering the old debug lock stack there was invalid. No allocator/lock algorithm or DB schema was redesigned. Temporary printf instrumentation was removed. AsyncLootDB.h is byte-identical to its pre-diagnostic checkpoint.

### Executed verification and exact limitation

- Earlier GameServer Debug target-only builds succeeded, including `ServerDebugFinal.log`, but linked the pre-existing ServerCore library; **they do not validate the above Core fixes**. Explicit ServerCore rebuilding failed on pre-existing `ServerCore/IocpCore.cpp:29`: only the opening GetQueuedCompletionStatus line is commented, leaving its continuation as invalid syntax. Git comparison confirms that one-line working-copy edit existed independently of this unit. File was left unchanged and user approval requested to remove that `//`. No final current Core+GameServer build, Release build or server launch has been claimed.
- To continue unaffected tests, separately compiled the three changed Core implementation files and explicitly linked their objects with current LootDB/LootService objects and the baseline remaining Core library. `Tests/LootRuntimeTests.cpp` is hard-coded to isolated `S1LootTest_20261001`; it checks DATABASE() before synthetic writes. Final run `LootRuntimeTestsVerifiedFinal.log` **exit0**: old-boot expiry, UTC invariance under +09/+00 session timezone, dedicated worker callback, same-ID retry/unchanged expiry, forced expiry, sink-open failure retaining pending events, durable append, no append after ack, same-EventId replay after simulated ack loss, zero grant ledger for expired drop, worker drain and process exit. Actual natural30s Room expiry/tick and live session broadcasting remain untested.
- Rebuilt and ran existing `Tests/LootDBIntegrationTests.cpp` with these same current objects. `LootDBIntegrationTestsCurrent.log` **exit0**: creation/idempotency, ownership/room checks, competing claims, expiration, full-stack/slot behavior, injected transactional rollback and fresh DB connection restoration. Synthetic fixtures remain only in the isolated schema; original live Account foreign keys/authenticated runtime are not covered. Supplied occlusion/dead reasons are not proof of actual Room wall validation.
- Evidence/checkpoints: `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PotionLootServer_20261002`. Original Room/GameServer/LootDB/project/Player/Core files and old Core libraries retained. Earlier hang/compile-failure logs preserved, not relabeled as passing. No production GameDB synthetic fixtures or schema edits made by these tests.
- At initial resume, HTTP MCP initialize/tools/list/list_toolsets succeeded on127.0.0.1:8000. Serial read-only calls reported no PIE and `/Game/Maps/LobbyMap`. No editor mutation followed. Availability after the reported shutdown must be rechecked, not inferred from this earlier successful connection.

Next: resolve the one-line existing IOCP build blocker, build actual Core+GameServer Debug/Release, verify genuine authenticated death/30s expiry against live DB, then implement the separate client/pickup/inventory integration unit. Rendered1-/2-client PIE, reconnect/restart recovery over real sockets, live outbox output and packaging are **not verified for this revision**. No final feature acceptance or asset Active promotion is implied.

Final read-only recheck this turn: GameServer absent; UnrealEditor PID45728 remains running. Existing MCP session answered `EditorToolset.EditorAppToolset.IsPIERunning` with false. Thus editor MCP is reachable and PIE stopped, despite the user's reported editor closure. It was left untouched; no additional editor restart is needed for this server-only unit.

### Approved build continuation - 2026-10-02

User explicitly requested continuing the build. SHA-256 verified checkpoint `IocpCore_before_build_unblock.cpp`; removed only the `//` before the existing GetQueuedCompletionStatus opening line. No IOCP dispatch policy change or other edit to that file. This resolves the above build blocker; previous failed builds remain preserved.

- Actual solution targets built serially: ServerCore Debug, GameServer Debug, ServerCore Release, GameServer Release (Win64/x64). All four commands exited0. Logs `CoreDebugFinalApproved.log`, `GameServerDebugFinalApproved.log`, `CoreReleaseFinalApproved.log`, `GameServerReleaseFinalApproved.log` in the same checkpoint directory. No compiler/linker warning/error patterns found in these four logs.
- Recompiled both isolated test executables against the **actual rebuilt Debug ServerCore library**, without the earlier diagnostic replacement objects. `LootRuntimeTestsActualCoreFinal.log` and `LootDBIntegrationTestsActualCoreFinal.log` both exited0 with all assertions reached and normal process shutdown. The standalone test links emit the pre-existing LNK4075 EditAndContinue/OPT:ICF warning; full target builds did not emit it. These are DB/worker tests, not authenticated multiplayer tests.
- Build-time generated six Protobuf files remain hash-identical across Server/DummyClient/S1. Canonical protocol was not changed by this continuation. No client build, Blueprint/asset edit/save or Editor restart was performed.
- Updated executable paths: `E:/Git Fork/GameServer/GameServer/Binary/Debug/GameServer.exe` and corresponding `Binary/Release/GameServer.exe`. No server process was started in this build-only continuation. Real GameDB drop creation, Room natural30s expiry, outbox path in live server working directory, authenticated1-/2-client PIE and pickup/client integration remain pending.

## Async pickup/inventory DB worker unit - 2026-10-02

User requested the next implementation. To keep one verifiable unit, this stage adds worker APIs and reconciliation, **not G/UI/Room activation**. Read-only MCP IsPIERunning again returned false. No editor mutation, binary-asset save, protocol/schema change or new production DB fixture. Four server source checkpoints verified under `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PotionLootPickupWorker_20261002`; existing runtime test source retained before extension.

### Implemented

- `GameServer/GameServer/LootService.h/.cpp`: Pickup(character,request,drop,room,validatedReason,completion) and QueryInventory(character,completion) use the existing bounded FIFO. No caller/IOCP inline DB execution. Input IDs are copied, not referenced; no session/player/UObject captures. Ready/invalid-ID/zero identity/missing callback/queue pressure returns false before submission. Completion runs on the worker; the future Room adapter MUST marshal it and verify original session/player/room identity before responding.
- Pickup calls the existing transaction once. UNCONFIRMED triggers only a read of the original RequestId/DropId, never a second automatic grant attempt. The original connection remains leased while borrowing another connection for reconciliation; no available second connection/read failure/missing result leaves UNCONFIRMED. This does not recreate broken pool connections or claim recovery from an actual transport-level COMMIT acknowledgement loss.
- `LootDB.h/.cpp`: ReadPickupResult is one read-only query constrained by CharacterId+RequestId and matching DropId. It returns only a valid persisted COMMITTED/REJECTED decision; mismatch/missing/failure leaves default UNCONFIRMED. Conflicting payloads still go through Pickup to produce the existing durable REQUEST_CONFLICT rejection.
- Pickup optionally takes expected ServerId/BootId; the service always supplies its actual process identity. Checks occur against the already-locked LootDrop. A new foreign-server request rejects WRONG_ROOM; a new old-boot request rejects EXPIRED. Cached matching prior request results are resolved first, preserving a historical committed result across death/boot changes instead of inventing a new grant or overwriting it. Defaults keep existing isolated DB callers source-compatible; production callers must use the service/strict identity.
- Existing transaction semantics, stack99/slots20, last-hit ownership, TTL30s, item20001 and no potion use remain unchanged. No implementation of actual position/death/LOS validation is claimed by accepting a server validatedReason argument.

### Executed verification

- GameServer Debug and Release full targets succeeded, exit0, against previously rebuilt ServerCore. Evidence `GameServerDebug.log`, `GameServerRelease.log` in this unit checkpoint directory.
- Extended `Tests/LootRuntimeTests.cpp`, rebuilt against actual current Debug libraries. `PickupWorkerTests.log` exit0: all prior boot/UTC/creation/expiry/outbox/drain checks plus async empty/populated inventory, callback thread distinction, invalid request refusal, one committed grant, durable DEAD rejection, same request replay despite supplied DEAD state, mismatched DropId conflict, read-only cached-result/missing/mismatch behavior, exact one grant ledger/commit event, foreign-server/boot fencing and version1/count1 snapshot.
- Recompiled and ran `Tests/LootDBIntegrationTests.cpp` against current objects: `LootDBIntegrationTests.log` exit0, all existing owner/room/concurrent claim/expiry/rollback/capacity/reconnect assertions passed. Both test executables use hard-coded isolated schema, not GameDB. Baseline standalone LNK4075 remains a test-link warning, not a full target failure.
- No actual COMMIT acknowledgement-loss injection, broken ODBC connection repair, queue saturation through these new service APIs, socket authentication, late-session Room completion, static wall geometry, G/I rendering, authenticated1-/2-client PIE or revision packaging executed. Earlier UI/asset/package tests are not acceptance of this stage.

### Next implementation boundary

Room/handler adapters must validate live player/session, throttle/reserve requests, enforce server position/foot height/static-obstacle LOS and marshal async completion without stale-session replies. Snapshot/removal registry, LevelFlow buffering, G selection and inventory coordinator/UI follow. Until these consumers and validation are complete, enter-game loot contract stays0 and staged unsupported handlers remain active. No potion should appear or be granted through current gameplay yet. No new user approval required for this already approved minimal feature; the next stage is implementation, not a new protocol proposal.

## Display-only world integration - 2026-10-02 (visual acceptance pending)

User asked to continue through visible monster drops and explicitly approved independent presentation negotiation. This supersedes the previous statement that no potion can appear: display is now wired; G/I/pickup gameplay remains disabled.

### Implemented contract and source

- Approved additive canonical Protocol.proto: C_ENTER_GAME field3 and S_ENTER_GAME field5 `loot_presentation_version`, default0;1 enables only snapshot/create/remove display. Existing loot_contract_version remains0, combat2/packet IDs1000..1029 unchanged. Snapshot contract_version1 is the existing drop-data format, not permission to grant items. Legacy peers/absent fields leave display off. No DB schema change.
- Existing protoc/GenPackets/FixProtobufEscapes regenerated Server/DummyClient/S1; six generated hashes match. Initial launcher could not resolve cmd.exe in PATH; absolute System32 launcher succeeded. No generated file manually edited.
- Server Player.h/ClientPacketHandler.cpp/Room.h/.cpp capture display request, negotiate1 only when service Ready, advertise on successful entry, send existing room snapshot after entry/spawns in serialized Room order, and broadcast only to display-negotiated current sessions. Boot-unready entry negotiates0 until reconnect; no mid-session silent enablement. Unsupported full-pickup handlers remain unchanged; no gameplay grant.
- Client Account/S1AccountSubsystem.cpp requests display only. S1GameInstance.h/.cpp binds/unbinds existing three typed events, initializes from negotiated response and delegates data to LevelFlow; flushes world presentation only after Main readiness. Disconnect/error clears data/actors. Network still does not spawn actors.
- New Loot/S1LootState.h/.cpp, owned by Level/S1LevelFlowSubsystem.h/.cpp: retains plain drop data during Loading; exact nonzero16-byte IDs, owner, potion20001/count1/definition1, finite floor and expiry validated. Bounded atomic1024 snapshots, uint64 event sequencing, duplicate/stale suppression, immutable metadata and session reset. Initial serialized server snapshot precedes events; pre-snapshot events are rejected. No local-clock expiry/grant.
- World/S1WorldEntitySubsystem.h/.cpp owns transient DropId->AS1LootActor registry. Repeated flushes reuse actors/effects; authoritative removal/world cleanup retires/destroys once. Logs carry DropId/floor, no credentials. No UE replication/overlap grant/fake death/montage drop trigger. Existing native actor/settings/vendor assets unchanged: floorZ=-0.5, mesh0.4/offset0, FX1; imported Niagara duration/loop unchanged. G hint intentionally hidden until real selection/pickup exists.

### Executed evidence

- Checkpoint PotionLootVisual_20261002 under the local checkpoints folder:31 original source/generated files hash verified. No Blueprint/map/vendor binary saved or modified.
- GameServer Debug/Release, S1Editor Development and S1 Development builds exit0. Logs GameServerDebug.log/GameServerRelease.log/S1EditorBuild.log/S1EditorBuildCorrectedTest.log/S1GameBuild.log. DummyClient generated compatibility checked; its target not rebuilt this revision.
- First automation failed with Fatal in NEW TEST fixture: CreateWorld already initialized WorldSettings; calling InitializeNewWorld again duplicated it. Removed duplicate initialization, not runtime error suppression. Failed Saved/Logs/LootVisual_Automation.log preserved.
- Final Saved/Automation/LootVisual_Final_20261002/index.json: **4Success,0Fail,0notRun**, no test warnings; LootVisual_AutomationFinal.log has no Error/Fatal/Ensure. ActorLifecycle, PresentationSettings, ProtocolTransport, VisualState cover separate legacy/display defaults, buffer/data validation, duplicate/stale handling, reset, one world actor on repeated sync and authoritative removal. NullRHI synthetic worlds do not prove real sockets/kill ownership or rendered GPU effect.
- Started new Debug GameServer with stdout/stderr captured in this checkpoint and administrator bootstrap env unset for the child only. Boot recovery Ready[1], startup stderr0bytes. Working directory Binary/Debug, so live JSONL path Binary/Debug/Logs/CombatItemOutbox.jsonl. Only genuine authenticated combat can create production drops; no synthetic GameDB fixture inserted. Started rebuilt Editor after user-confirmed closure.

### Required visual gate / remaining work

Authenticated1-/2-client MainMap PIE: kill Dusk with A-D/Q; confirm one potion/effect at death floor on both screens, correct size/no hover or floor penetration, monster respawn does not remove/duplicate drop, and server removal occurs about30s after DB creation. Imported FX may finish sooner; potion lifetime still follows server. User visual acceptance, real kill/expiry/client/outbox logs, reconnect and current packaging are pending. G/I/full loot contract/LOS pickup/HP+50 use remain outside this display unit. Previous package success is not this revision's gameplay acceptance.

### Visual acceptance and evidence boundaries - 2026-10-02

User confirmed two-client MainMap PIE: Dusk kill produces potion and effect on both screens; floor placement and size are acceptable; removal after approximately 30 seconds was visually checked. This completes the user visual gate for the display-only unit, not pickup/inventory gameplay acceptance.

Independent log evidence: S1.log records the same DropId `2e17a5f7ab91ca976fadd12ca7edb702` spawned once per PIE client at UTC 02:22:07.198/02:22:07.201, floor (-71.266,42.025,-0.500). CombatItemOutbox.jsonl records one MonsterDied, one DropCreated at 02:22:07.187523 and one DropExpired at 02:22:37.273649 for that drop: 30.086126 seconds. Corrected server runtime confirms MainMap navigation loaded (74 nodes/250 edges). No Fatal, Ensure, Blueprint Runtime Error or Network Failure matched the inspected client log.

Evidence limitation: latest captured PIE worlds began teardown at UTC 02:22:10.302/02:22:10.320, approximately three seconds after creation; there is no client presentation-removal log in that captured run. Therefore the visible 30-second removal is recorded as user-reported verification, not independently proven by these client logs. Teardown includes a RecastNavMesh/UCrowdManager warning; this is separate from the corrected external-server navigation deployment failure.

Only this document and CurrentAssetRegistry.md changed for acceptance recording; no code, protocol, Blueprint, map or vendor asset changed or saved. Prior build/automation results remain as recorded above; no new build/PIE was executed by Codex in this acceptance update. Current-revision packaging, first-load hitch, reconnect with active drops and independently captured client expiry remain unverified. G pickup, I inventory activation and potion use remain unfinished; full loot contract stays disabled.

## G pickup / I inventory integration - 2026-10-02 (core two-client flow accepted; see latest evidence below)

User requested continuing after accepting the display-only unit. Implemented the already-approved minimal pickup/inventory contract; no new proto/schema, item-use or HP policy. Read-only serial MCP initialize/list_toolsets/IsPIERunning succeeded before edits; PIE=false. User requested closure; the verified prior Codex server PID27544 was still running and was stopped with the user's closure authorization. Client/server source checkpoints were copied and hash-verified under `checkpoints/PotionLootPickupIntegration_20261002` before edits. Existing unrelated server Git changes were retained; S1 remains without Git.

### Current behavior and ownership

- Account requests existing loot_contract_version1 plus separate presentation1. Room negotiates full1 only when both requested, service Ready and valid saved MainMap obstacle data loaded; otherwise full0. Legacy display-only/full0 and combat2 remain compatible. No generated files were manually edited; normal server build regeneration does not imply a proto change.
- Room authenticates current session/player/object/room/CharacterId, derives accepted server position and foot Z (capsule half-height96cm), checks alive/owner/XY150cm/foot-height100cm and direct static obstacle segment. DB worker still checks persisted request first, owner/status/UTC/boot identity and performs existing atomic Item+version+claim+ledger+outbox transaction. Cached exact-request results may reconcile after death/removal; missing Room drop is never permission to claim an arbitrary DB row.
- Pickup and inventory queries have separate200ms limits; one active query per player, at most128 active pickup requests/reservations and existing bounded worker. Temporary pressure/in-flight duplicates return UNCONFIRMED, not a durable fake refusal. Unconfirmed claim reservation retains the original operation ID until reconciliation or authoritative expiry; no new-ID automatic retry. Completion always reconciles committed world removal even if requester left, but only replies to the original still-current ObjectId/CharacterId/session. No blocking DB in Room/IOCP.
- New client Inventory subsystem coordinates requests and passive existing UI; Network remains transport, LevelFlow retains drop data, WorldEntity retains actor registry. Main readiness triggers a fresh inventory query; I opens/closes the potion icon/name/aggregate confirmed count/status view, Esc or Close also closes. UI focus/cursor suppress new combat/look/G input; opening clears pending movement input. No equipment/currency/use buttons or Blueprint changes.
- G selects one nearest owned visible drop within the approved range; both nonowners and unsupported peers cannot request through normal UI. Local trace is hint filtering, not authority. Server-confirmed removal remains WorldEntity-owned. Response matching uses the exact opaque16-byte request/drop pair. Five-second delay is unknown, not success/refusal: G explicitly retries the SAME request, never automatically adds items. Inventory count changes only on matching validated snapshots; committed result raises a minimum inventory version, rejects stale precommit reads, and requests a fresh snapshot. Disconnect/leave/lobby clears UI, requests and quantity; late replies cannot populate a new session.

### Static collision data / exact source changes

Server: `GameServer/Room.h/.cpp`, `Player.h`, `ClientPacketHandler.cpp`, `GameServer.vcxproj`; new header `LootObstacles.h`, generated read-only world data `Data/MainMap.lootobb`, standalone `Tests/LootObstaclesTests.cpp`. Client: `Source/S1/Account/S1AccountSubsystem.cpp`, `S1GameInstance.cpp`, `Game/S1MyPlayer.h/.cpp`, `World/S1WorldEntitySubsystem.h/.cpp`, `UI/S1InventoryWidget.h/.cpp`; new `Loot/S1InventorySubsystem.h/.cpp`, `S1InventoryState.h`, `S1InventoryStateTests.cpp`. Exporter: existing `Plugins/S1NavigationTools/.../Private/S1ExportNavigationCommandlet.cpp` adds optional Obstacles output without re-exporting Nav or saving maps.

Actual saved native Floor_0/test-wall components have Mobility=Movable but no ticking/Blueprint movement or physics simulation; they are captured as the existing immutable layout. OBBs conservatively union render bounds and simple-collision bounds; concave openings can be overblocked. This does NOT support animated/dynamic obstacles, slopes or stairs. Unsupported active blockers fail export, missing/invalid OBB fails full-contract negotiation. Unregistered commandlet component transforms must be updated before export; the earlier identity-transform output was NOT deployed. Corrected output has2 boxes: floor top approximately-0.5, wall center(500,0,150), extent(20,300,150). Geometry relocation requires explicit fresh export; no runtime stale-map checksum enforcement is implemented. No claim that path distance solves all wall attacks; existing combat Nav remains unchanged.

GameServer project now explicitly copies unchanged MainMap.nav and generated MainMap.lootobb into Debug/Release Data on build; matching source/output hashes are checked. No Blueprint/map/vendor asset saved, no dirty-package save operation, no UE5_RPG changes/imports. Potion mesh0.4/FX1 and Dusk0.7/28x62/+90/61.5 remain unchanged.

### Executed verification / limits

- Actual GameServer Debug/Release and S1Editor Development builds succeeded; S1 Development game build succeeded. Build evidence is in the above checkpoint directory. Initial Windows min/max macro compile failure was fixed with parenthesized standard functions; failed log retained. Export initial static-Mobility-only filter produced zero boxes, then cached transforms produced invalid coordinates; diagnostic outputs retained, neither used for deployment. Export and automation run in normal Windows environment after sandbox Zen-path access failure; project config was not changed.
- `Saved/Automation/LootPickup_20261002/index.json`:6 success,0 warnings,0 failures,0 not-run. Existing four loot tests plus InventoryState and PickupCoordinator test stale/minimum/equal-version rejection, duplicate slots/items,99/20 validation, actual player Blueprint half-height96, wrong/late request ignoring, unconfirmed ID retention, no optimistic count, new-request isolation and session reset. `Saved/Logs/LootPickup_Automation.log` has no Error/Fatal/Ensure match. NullRHI tests do not prove rendered UI focus/font or live authenticated requests.
- Standalone MSVC `LootObstaclesTests.exe` exit0:13 checks including axis-aligned/rotated/grazing/inside/missing/nonfinite cases and actual exported MainMap wall crossing versus clear flat segment. `LootObstaclesTests.log` records assertions. This is server geometry validation, not a real two-client wall pickup test.
- NEW revision authenticated1-/2-client G grants/removal, ownership, UI input, inventory/reconnect, disconnect-during-commit, actual duplicate socket requests, timeout/transport-loss injection, combat regression and packaging remain unverified. Prior isolated DB/worker success remains valid for those unchanged layers, not proof of newly added Room/socket adapters. No new synthetic GameDB fixtures or DB DDL performed.

### User acceptance procedure

Final launch: GameServer PID33888, Editor PID32236; server boot recovery Ready[1], stderr0bytes. Fresh serial MCP initialize/IsPIERunning returned false after editor startup. Debug/Release Nav and OBB copies hash-match source; all six generated files hash-match actual Server/DummyClient/Unreal Network directories. No genuine new pickup or PIE run performed during preparation. Server runs hidden with checkpoint stdout/stderr capture; do not start a second debugger server without stopping this instance first.

In two-client MainMap: kill Dusk, approach with last-hitting owner, press G once; both screens must remove the potion once and only owner's I count must increment by1. Other player must not see a G hint/grant. I/Esc/Close must restore movement/camera; clicks while UI open must not attack. Repeat pickup with rapid G, try beyond150cm/behind wall, and reconnect the same character to verify persisted count/no stale drop. Wait30s on an unclaimed drop to confirm expiry without quantity increase. Report visible results; Codex then audits new server/outbox/client logs. Potion use/HP+50 remains excluded.

### Latest authenticated PIE acceptance - 2026-10-02

User confirmed the current revision in two-client PIE: owner G pickup, removal on both screens, I quantity increase and quantity persistence after reconnect. This supersedes the pending status for those four specific checks only; it is user-performed gameplay/visual verification, supplemented by the log audit below, not a new Codex-driven PIE run.

- Captured S1.log: DropId `20ae74cc33db11f7791b9e72c0c19b81` spawned twice at UTC03:05:31.732/.735 (one per client), one G request at03:05:33.812 with RequestId `B40D14218669A94C8094F3C6E9B96EEA`, removed twice at03:05:33.852/.853. Requester result COMMITTED/NONE/version1 followed by successful inventory snapshot count1/version1 at03:05:33.870. No repeated removal or pickup result for this drop was found in this captured run.
- Server runtime confirms Character6 accepted pickup once. JSONL has exactly one MonsterDied, one DropCreated and one ItemPickupCommitted for this DropId, same DeathId/SpawnId/CharacterId; committed at UTC03:05:33.837108, before_count0/after_count1/inventory_version1 and matching RequestId. This proves one captured outbox commit event, not an independent SQL recount of the grant ledger in this update.
- Earlier live solo segment also captured Character5's drop `1b88c5f7ec4b7ab4b452976570fc16fc`: one commit, count0->1/version1. After both clients disconnected, server authenticated fresh ObjectIds6/7. Client snapshots at UTC03:05:52.690 and03:06:05.284 each show successful count1/version1; same-character persistence matches the user's reconnect check. Times above are UTC; two-client pickup was12:05:33 KST.
- Server loaded unchanged Nav74/250 and pickup obstacle boxes2. No Fatal error, Ensure condition, Blueprint Runtime Error or Network Failure matched the current client log. Existing UIOnly/non-focusable SObjectWidget Error occurs in lobby/auth/character-selection flow (including03:05:41,03:05:51 and03:06:03); inventory uses GameAndUI, and this audit does not fix or claim all UI logs are error-free. Existing RecastNavMesh/UCrowdManager warning appears on MainMap teardown; separate from external-server Nav success.

Only PotionLootImplementation.md and CurrentAssetRegistry.md were edited to record acceptance; no source, protocol, DB, Blueprint, map or vendor asset changes/saves, no additional build or test launch. Core drop->owner pickup->confirmed inventory->reconnect loop is accepted. Live nonowner/malicious request rejection, out-of-range/wall pickup, rapid duplicate socket requests, timeout/COMMIT-loss injection, disconnect during commit, UI combat-input suppression and full combat regression were NOT specifically reported or independently tested here. New revision packaged two-client acceptance remains pending. Potion use/HP+50 remains excluded. Existing asset provenance/public-video license notes remain unresolved; gameplay acceptance does not grant license rights.

### First genuine PIE attempt: deployment correction - 2026-10-02

User reported Dusk inactive. Captured ServerRuntime.log shows two authenticated players entered and combat relayed, but `MainMap navigation unavailable. Dusk movement and attacks are disabled.` No actual monster hit/death/drop evidence; this attempt did NOT pass visual acceptance. Cause: Codex launched with Binary/Debug working directory, which lacked Data/MainMap.nav; the existing secondary relative candidate resolves to the wrong folder from that working directory. Not a client AnimBP or new loot packet issue.

Read-only MCP confirmed PIE stopped before restarting. Copied the existing validated GameServer/Data/MainMap.nav (S1NAV2,74 nodes) into previously absent Binary/Debug/Data/MainMap.nav; SHA-256 source/destination match. No Nav re-export, wall geometry change, original data edit or AI rule change. Stopped only the verified Codex-launched server PID46828 and restarted the same executable; new runtime logs ServerRuntimeNavFixed.log/ServerRuntimeNavFixedError.log. Startup boot Ready[1], stderr0bytes. Actual Nav loaded/monster movement/drop visualization still require the next authenticated MainMap entry. Automatic build/deployment copying is not added in this corrective step; future launch must retain matching Data beside the executable or use the validated source working directory.
