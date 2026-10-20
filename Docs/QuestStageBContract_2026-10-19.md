# B단계 구현 계약 검토안 — 2026-10-19

**현재 상태: 2026-10-19 사용자 구현 승인 후 적용 중.** 아래 본문은 승인 당시의
계약/제안 기록이며 '아직 미구현'이라는 과거형 표시는 현재 진행 상황이 아니다.
실제 구현·빌드·격리 DB 테스트·미검증 사항은
`QuestStageBImplementation_2026-10-19.md`에서 별도로 관리한다.
승인만으로 PIE/패키지 검증 완료 또는 운영 DB 적용을 의미하지 않는다.

## 1. 범위 및 사용자 확정 규칙

- NPC 실제 수락 → Dusk3마리 → 목표달성까지. 캐릭터당 같은 퀘스트1회.
- 수락 후 서버 확정 피해(amount>0)를 한 번이라도 준 캐릭터 전원이 기여자.
  다른 캐릭터가 마무리해도 몬스터 사망 시 각 기여자에게+1.
- 기본공격/Q 모두 기존 최종 피해 경로를 사용. 헛스윙/거부/벽 차단은 제외.
- 같은 몬스터 생명에서 다회·Q 다단 피해는 한 번만 집계. 재생성은 새 생명.
- 복귀 중·부분 회복 후 재교전: 기록 유지. 스폰 복귀 후 HP 완전 회복: 기록 초기화.
- 사망 순간 같은 Room에 접속 중인 기여자만 인정. 몬스터 드랍의 기존 소유권은
  변경하지 않는다. 인벤토리/보상 트랜잭션은 이번 범위가 아니다.
- 보상은 표시 데이터만 유지. 포션3개 지급은 C단계 승인·구현 전까지 하지 않는다.

명시적 경계 제안(아래까지 포함해 구현 승인 요청):
- character_id 기준으로 기여자를 유지한다. 같은 캐릭터가 재접속해 사망 전에 같은
  방으로 돌아오면 인정. 사망 시 오프라인/다른 방이면 제외; 사망 후 복귀는 소급 없음.
- 사망한 플레이어라도 접속·방 조건을 만족하면 인정(추가 생존 조건을 넣지 않음).
- 사망 시 별도 거리 제한은 추가하지 않음. 퀘스트 활성 상태에서 실제 타격한 기록이
  필요하므로 수락 전 때린 뒤 수락만 하는 경우는 제외; 수락 후 다시 맞혀야 함.
- 수락은 DB commit 후 Room에서 승인 상태를 반영한 시점부터 유효. 승인 대기 중
  처치는 소급하지 않음. 클라이언트는 그전에 성공 대사를 표시하지 않음.

## 2. 실제 조사 근거

서버 경로는 `E:/Git Fork/GameServer/GameServer/` 기준.
- GameServer/Room.cpp::ApplyMonsterDamage: 기본/Q 공통; amount>0일 때 HP 확정,
  nextHp0일 때 DEAD 전환 후 QueueDeathDrop. 여기 두 지점에 기여 등록/처치 확정을 연결.
- Room.cpp::UpdateMonster: homeRecoveryStarted/InitialHp로4초 회복. 기존 HP 변경 후
  maxHp 도달 지점에 clear를 추가한다. RETURN 진입 자체에는 clear하지 않는다.
- SpawnDuskMonster: generation 증가; spawnId UUID는 LootService Ready와 무관하게 생성.
  반면 deathId는 loot 생성 조건 안에 있으므로 퀘스트 집계를 QueueDeathDrop 성공에
  종속시키거나 기존 deathId 존재를 전제로 하지 않는다.
- Room.h::MonsterRuntime에는 현재 기여자 목록이 없다. Player의 CharacterInfo에
  영속 character_id, session/player/room 일치 검사 경로가 이미 있다.
- AsyncLootDB는 bounded FIFO worker이며 DBConnectionPool::Pop은 부족 시 nullptr.
  같은 작업 실행 패턴을 QuestService에서 재사용하되 Loot/Inventory 책임은 늘리지 않는다.
- 현재 Protocol.proto C_ENTER_GAME tags1..8, S_ENTER_GAME1..10 사용.
  양쪽 handler에서 마지막 패킷1040. 퀘스트 패킷은 아직 없음.
- 현재 S1QUEST1은 정의만 로드하고 NPC 월드 위치는 없음. 수락 거리/가림을 검증하려면
  신뢰된 서버 NPC 배치 데이터가 필요하다. 클라이언트가 요청으로 보낸 좌표는 사용 불가.
- LootObstacles::Occluded는 저장된 정적 OBB의 직선 가림 검사다. NavigationGraph의
  경로 거리와 다르다. 기존 MainMap.lootobb를 NPC 시야 검사에도 재사용 가능하다.
- MCP: PIE=false, 현재 editor level LobbyMap. 메모리에 로드된 저장 MainMap NPC 조회:
  NpcId1, (-350,250,-0.5), interactionRadius250cm, Q/Dialogue DT 연결.
  Q1은 아직 disabled, 목표3/보상20001×3. 관련 DT2/BP/WBP dirty=false.
  이번 조사에서 레벨 이동/저장/서버 재시작은 하지 않았다.

## 3. 상태·기여·이벤트 수명

진행 상태: NotAccepted0(행 없음), Active1, ReadyToTurnIn2. 이번에 Claim/Rewarded를
지원하지 않는다. PK(character,quest)로 버전 변경 후에도 재수락·진행 초기화 금지.

MonsterRuntime에 `(character_id, quest_id, accepted_instance_id)` 집합을 둔다.
accept UUID는 새 영속 수락1회마다 서버가 생성; 계정ID/ObjectId가 아닌 캐릭터 구분.
캐릭터의 현재 quest cache가 Active이며 목표 몬스터가 일치할 때만 amount>0 후 등록.
사망 처리 전에 마지막 타격자도 등록 → DEAD 전환에서 해당 집합을 스냅샷으로 복사.
현재 세션·Room 접속을 검사하고 유효 기여자별로 `(accepted_instance_id,spawnId)`
credit를 생성한다. 전체 회복/재생성 때 집합을 비우고 과거 임시 캐시를 재사용하지 않는다.
spawnId UUID 생성 실패 시 그 생명의 quest credit를 중단/오류 기록; 다른 생명 키를
조작해 합치지 않는다. 드랍 service 준비 여부와 독립이다.

사망 시 확정된 credit는 **그때의 접속 여부**를 기준으로 한 불변 입력이다. 이후 DB
처리 중 플레이어가 끊겨도 commit 자체는 허용; 응답 송신만 현재 세션 확인 후 실행.
재접속 조회로 결과를 복구한다. DB callback에서 사망 시의 자격을 다시 판단하지 않는다.

## 4. 패킷 변경 제안 (append-only, 아직 미구현)

기존 메시지에 추가할 필드:
```proto
// C_ENTER_GAME 기존1..8 유지
uint32 quest_contract_version = 9;
// S_ENTER_GAME 기존1..10 유지
uint32 quest_contract_version = 11;
bytes quest_session_id = 12; // 서버 생성16바이트 UUID, 입장/캐릭터 변경마다 새 값
```
v1은 양쪽 코드·정의v2·NPC/장애물·DB readiness가 준비됐을 때만 협상한다.
구버전/불일치/미준비는0. quest_session_id는 인증수단이 아니라 과거 응답 차단용이다.
구버전은 append-only 필드를 무시하고 기존 전투/아이템을 유지한다.

새 메시지는 정본 Protocol.proto **맨 끝에 이 순서로** 추가하고 생성기 실제 번호를 확인:

| 제안 ID | 메시지 | 필드(tag) |
|---|---|---|
|1041|C_QUEST_QUERY|request_id bytes(1), quest_id uint32(2), contract_version uint32(3)|
|1042|S_QUEST_STATE|request_id bytes(1), character_id uint64(2), contract_version uint32(3), quest_session_id bytes(4), query_status enum(5), QuestStateInfo state(6)|
|1043|C_QUEST_ACCEPT|request_id bytes(1), quest_id uint32(2), definition_version uint32(3), npc_id uint32(4), contract_version uint32(5)|
|1044|S_QUEST_ACCEPT_RESULT|request_id bytes(1), character_id uint64(2), contract_version uint32(3), quest_session_id bytes(4), decision enum(5), reason enum(6), QuestStateInfo state(7)|

Struct.proto 신규 QuestStateInfo:
quest_id(1), status(2), definition_version(3), accepted_instance_id bytes(4),
progress uint32(5), required uint32(6), revision uint64(7), monster_definition_id(8),
title string(9), description string(10), accept_npc_id(11), return_npc_id(12).
단일 quest snapshot만 응답하여 기존 uint16 packet-size 한도 내로 제한한다.
reward 지급 정보는 이번 결과로 보내지 않는다(아이템 지급으로 오인 방지).

Enum.proto 신규:
- QuestStatus:0NotAccepted/1Active/2ReadyToTurnIn.
- QuestQueryStatus:0None/1Ready/2Unconfirmed/3Unsupported.
- QuestAcceptDecision:0None/1Committed/2Rejected/3Unconfirmed.
- QuestAcceptReason:0None/1InvalidRequest/2Unsupported/3Disabled/4DefinitionMismatch/
  5UnknownNpc/6TooFar/7Occluded/8StateBlocked/9Busy/10AlreadyAccepted/
  11DbUnavailable/12RequestConflict/13NotReady/14UnknownQuest.

Query 응답 실패는 '미수락' snapshot으로 덮지 않는다. 기존 state를 보존하고 동기화
미확정 표시. 서버 처치 확정 push는 S_QUEST_STATE(request_id empty) 재사용.
같은 캐릭터·세션·퀘스트에서 revision이 낮은 응답은 무시. NotAccepted는 Ready 조회가
성공한 경우에만 revision0으로 표시한다. 입장/캐릭터 변경 시 캐시/요청을 정리한다.

request_id는 기존 규칙과 같은 길이16의 nonzero UUID. 캐릭터/계정/좌표/피해/킬수/
보상량은 요청 권위 입력으로 받지 않는다. 인증된 현재 GameSession::player에서 얻는다.
쿼리·수락 rate limit/요청1개 pending/bounded queue로 스팸을 제한한다.
협상0인 구클라이언트에는 새 패킷을 보내거나 수락을 허용하지 않는다. 서버가 이미
수락된 캐릭터 상태를 정상 로드했다면 처치 기여 판단은 그 권위 상태로 하며 클라이언트가
보내는 capability 값을 킬 인정의 근거로 삼지 않는다.

## 5. 수락·중복·지연·닫기 계약

1. 캐릭터별 초기 DB 조회를 마쳐 quest cache Ready인 경우에만 새 수락/기여를 허용.
   조회 미완료/DB장애는 UI에 동기화 미확정으로 표시하고 성공/진행을 추측하지 않음.
   기존 전투는 유지하지만 이 기간의 처치를 정상 집계했다고 주장하지 않는다.
2. 같은 UUID의 기존 수락 결과를 우선 조회. 같은 payload면 기존 결과 재전송;
   다른 quest/version/NPC면 RequestConflict, 재실행하지 않음.
3. 신규 요청은 현재 세션/room/생존/행동 가능, 정의 enabled/version,
   NPC matching/250cm/발 높이차100cm/직선 정적 OBB 가림을 검증.
   최근 피격3초/자신을 추적·공격하는 몬스터/공격·Q 중 수락 금지는 기존 대화 정책과
   일치하도록 서버 상태로 검사. UI 자체만 통과했다고 서버 승인하지 않는다.
4. DB commit 후 Room 승인 cache를 갱신, requester에게 결과. 이후 피해만 기여 인정.
5. UI는 승인 UUID + 원래 대화 run/Offer/NPC/quest가 여전히 맞을 때만 수락 대사로 이동.
   닫힌 창의 늦은 승인도 캐시는 갱신하지만 UI/카메라를 재생성하지 않는다.
6. 타임아웃3초 시작값: '확인 중' 표시/Query 재확인. 새 UUID로 자동 재수락하지 않음.
   명시적 재확인은 같은 UUID+payload; DB가 결정 못했으면 Unconfirmed를 유지.
7. Active/Ready인 퀘스트에 새로운 UUID 수락은 AlreadyAccepted+현재 state.
   진행을0으로 되돌리지 않는다. 사용자 거절은 로컬 대사 이동만, 서버 기록 불필요.

## 6. NPC Export v2 제안

기존 정의 v1을 수정해 서버 행동에 조용히 사용하지 않는다. S1QUEST2에 NPCS 섹션을
추가해 NpcId/room_id1/MainMap/위치/상호작용반경/수직허용/시야높이100을 Export한다.
MainMap 저장 배치에서 해당 테이블이 참조하는 NPC들을 수집; 중복·누락·다른 테이블
bundle 혼용·잘못된 월드/좌표면 Export 거절. CDO의 원점 좌표를 배포하지 않는다.
현재 NPC 예시는 (-350,250,-0.5), radius250. 요청에서 좌표를 받지 않는다.
서버에 v1 파일만 있으면 정의 미리보기 로드는 가능하더라도 실제 quest v1 협상은0.
MainMap.lootobb 없거나 잘못되면 수락 거부. 이 정적 가림은 동적 장애물/지형 판정
전체 해결책이 아니다. Nav 경로 거리만으로 벽 검사 통과시키지 않는다.

## 7. 신규 DB 테이블3개 — 검토용 DDL

정확한 운영 DB 타입/기존 동명 테이블은 아직 실조회하지 않았다. 아래는 추가-only
제안이며 실제 migration은 사전 검증·백업·격리 DB 테스트·승인 후 만든다.
기존 Account/CharacterData/Item/Inventory/Loot 테이블 변경 없음.

```sql
CREATE TABLE CharacterQuest (
 character_id BIGINT UNSIGNED NOT NULL,
 quest_id INT UNSIGNED NOT NULL,
 accepted_instance_id BINARY(16) NOT NULL,
 definition_version INT UNSIGNED NOT NULL,
 status TINYINT UNSIGNED NOT NULL,
 progress INT UNSIGNED NOT NULL DEFAULT 0,
 required_kills INT UNSIGNED NOT NULL,
 monster_definition_id INT UNSIGNED NOT NULL,
 accept_npc_id INT UNSIGNED NOT NULL,
 return_npc_id INT UNSIGNED NOT NULL,
 title VARCHAR(256) NOT NULL,
 description TEXT NOT NULL,
 reward_item_id INT UNSIGNED NOT NULL,
 reward_count INT UNSIGNED NOT NULL,
 revision BIGINT UNSIGNED NOT NULL,
 accepted_utc DATETIME(6) NOT NULL,
 updated_utc DATETIME(6) NOT NULL,
 PRIMARY KEY(character_id,quest_id),
 UNIQUE KEY uq_quest_accept_instance(accepted_instance_id),
 CHECK(status IN(1,2)), CHECK(required_kills>0),
 CHECK(progress<=required_kills), CHECK(revision>0)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE QuestAcceptRequest (
 character_id BIGINT UNSIGNED NOT NULL,
 request_id BINARY(16) NOT NULL,
 quest_id INT UNSIGNED NOT NULL,
 definition_version INT UNSIGNED NOT NULL,
 npc_id INT UNSIGNED NOT NULL,
 decision TINYINT UNSIGNED NOT NULL,
 reason INT UNSIGNED NOT NULL,
 accepted_instance_id BINARY(16) NULL,
 result_revision BIGINT UNSIGNED NOT NULL DEFAULT 0,
 created_utc DATETIME(6) NOT NULL,
 PRIMARY KEY(character_id,request_id),
 CHECK(decision IN(1,2))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE QuestKillCredit (
 accepted_instance_id BINARY(16) NOT NULL,
 monster_life_id BINARY(16) NOT NULL,
 character_id BIGINT UNSIGNED NOT NULL,
 quest_id INT UNSIGNED NOT NULL,
 server_boot_id BINARY(16) NOT NULL,
 room_id INT UNSIGNED NOT NULL,
 monster_definition_id INT UNSIGNED NOT NULL,
 resulting_progress INT UNSIGNED NOT NULL,
 resulting_revision BIGINT UNSIGNED NOT NULL,
 credited_utc DATETIME(6) NOT NULL,
 PRIMARY KEY(accepted_instance_id,monster_life_id),
 INDEX ix_quest_credit_owner(character_id,quest_id),
 FOREIGN KEY(accepted_instance_id)
  REFERENCES CharacterQuest(accepted_instance_id)
) ENGINE=InnoDB;
```

CharacterQuest는 수락 당시 목표/보상/문구 snapshot을 고정한다. DefinitionVersion 변경으로
이미 수락한 캐릭터의 목표/보상을 바꾸지 않는다. reward_*는 미래 C단계 대비 snapshot
일 뿐 이번에 지급하지 않는다. 보상 테이블/인벤토리 write는 없음.
Unconfirmed는 DB의 최종 결정이 아니므로 QuestAcceptRequest에 결정으로 확정 저장하지
않는다. 유효한 인증 캐릭터/UUID의 최종 Committed/Rejected만 보관한다.
수락 행 생성+요청 결과는 한 트랜잭션. 처치 credit INSERT+progress/revision 증가+
목표달성 전환도 한 트랜잭션(SELECT FOR UPDATE). credit 중복은 증가 없이 현재 state.
DB rollback/commit 응답 불명확 때 같은 key를 재조회하여 판단한다.
특히 제목/설명은 따옴표와 한글을 포함할 수 있으므로 ODBC 바인딩된 매개변수로
저장한다. 클라이언트나 파일의 문구를 SQL 문자열로 직접 이어 붙이지 않는다.

## 8. 장애 보장 수준과 제한 (승인 대상)

독립 QuestService bounded worker + DB pool을 사용, 완료는 weak Room→DoAsync.
세션이 사라졌다고 DB 완료를 취소하지 않으며 오래된 세션에는 송신하지 않는다.
처치 사망 snapshot은 메모리에서 제한된 재시도; DB commit 전에는 진행 UI를 올리지 않는다.
DB commit된 수락/진행은 재접속/서버 재시작 후 복구한다.

**MVP에서는 DB 기록 전 서버 프로세스가 종료되면 아직 미확정인 처치가 유실될 수 있다.**
이를 재시작까지 무손실로 보장하려면 durable journal/outbox가 추가로 필요하며 이번
최소 B단계에는 포함하지 않는 것을 추천한다. queue full/장기 DB장애 시 조용히 성공
처리하지 않고 기능 Unconfirmed/로그로 드러낸다. 최소판의 보장 한계를 숨기지 않는다.
서버 재시작으로 몬스터와 미확정 타격 기여는 초기화된다; 죽지 않은 몬스터 기여의
영속 저장은 범위 밖. 이미 commit한 처치 ledger는 유지한다.

## 9. 정확한 변경 예정 경로

Unreal root `E:/Git Fork/S1_UE5.8/`:
- Source/S1/Quest/S1QuestSubsystem.h/.cpp 신규(GameInstance 캐시/요청/세션 수명).
- Source/S1/UI/S1QuestTrackerWidget.h/.cpp,
  Source/S1/Quest/S1QuestContractTests.cpp 신규.
- Source/S1/ClientPacketHandler.cpp, Network/S1NetworkSubsystem.h/.cpp,
  Level/S1LevelFlowSubsystem.cpp, Quest/S1DialogueSubsystem.h/.cpp,
  Quest/S1QuestData.h/.cpp, Quest/S1QuestNpc.cpp 수정.
- 기존 Network/Protocol.pb.*, Enum.pb.*, Struct.pb.*, ClientPacketHandler.h는
  정본 생성기로만 갱신. 수동 편집 없음.
- Tools/DeployQuestDefinitions.ps1 검증·v2 배포 유지.
- 신규 /Game/S1/UI/WBP_S1QuestTracker, 기존 NPC BP preview off,
  DT_S1QuestDefinitions 샘플 활성/문구 조정 및 목표3 유지. 기존 대사5개 연결 유지.
  저장 전 별도 checkpoint/dirty 보고. MainMap 이동/수정·저장은 원칙적으로 불필요.

서버 root `E:/Git Fork/GameServer/`:
- GameServer/Common/Protobuf/bin/{Protocol,Struct,Enum}.proto 정본 수정/양쪽 재생성.
- GameServer/GameServer/{Room.h/.cpp,Player.h,ClientPacketHandler.cpp,
  GameServer.cpp,QuestDefinitionConfig.h,GameServer.vcxproj} 수정.
- 같은 폴더 QuestRuntime.h, QuestService.h/.cpp, QuestDB.h/.cpp 신규.
- GameServer/Tests/QuestRuntimeTests.cpp, QuestDBIntegrationTests.cpp 신규;
  QuestDefinitionConfigTests.cpp 확장. QuestService lifetime은 main에서 worker drain 보장.
- codex_md_edit/db/QuestStageBMigration.sql 신규3테이블안. 이 폴더는 별도 중첩 Git
  저장소일 수 있으므로 배포/커밋 범위를 섞지 않는다. 이번에 커밋/푸시 안 함.

## 10. 구현·검증 순서

1. 이 문서의 패킷/DDL/NPC Export 및 장애 보장 수준 승인.
2. 격리 DB 테이블 생성/테스트(관리자 DDL은 사용자 실행), 정의v2+서버·Unreal 연결.
3. runtime/DB 자동: 수락 중복·payload충돌·트랜잭션 실패·commit응답 유실·revision/
   세션 변경·수락 전/대기 중 킬 제외·A/B기여/무기여C·Q다단·재생성·부분/완전회복.
4. GameDB 사전 schema 확인/백업/격리 검증 후 migration 실행은 별도 사용자 확인.
5.1인/2인 PIE: A한번타격→B마무리→A/B각+1, 재접속 복구, 실패 시 성공 대사 없음,
   창 닫은 뒤 늦은 응답으로 카메라 재개 없음, 미접속/타방 제외, 벽/거리 수락 거절.
6. 변경된 전투/아이템 경로만 회귀. 피해·드랍 소유권·보상 수량 변경 없음 확인.
7. 최신 패키지 cook/2계정 실행은 별도 실행 증거로 남긴다. 미실행을 통과로 보고하지 않음.

승인 질문: 이 append-only4패킷/협상필드, 신규3테이블, NPC정의v2,
그리고 commit 전 프로세스 종료 시 미확정 credit 유실을 허용하는 MVP 보장 수준으로
구현할지 확인한다. 이미 확정한 기여 규칙을 다시 승인받는 질문은 아니다.
