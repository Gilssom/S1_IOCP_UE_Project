# Quest C — 완료 보고·보상 지급 계약 검토안 (2026-10-19)

상태: 사용자 승인 후 C단계 코드·패킷·DDL과 NPC 대화 연결 구현. 최신 결과는
[QuestStageCRewardImplementation_2026-10-19.md](QuestStageCRewardImplementation_2026-10-19.md)를 따른다.
아래 본문은 승인 당시 계약이며 ‘제안/예정’ 표기는 당시 시점을 나타낸다.
B단계의 이전 검증과 C단계의 최신 검증을 구분한다. C단계는 이후 사용자가 안내된 PIE 전체 항목 확인 완료를 보고했으며, 패키지·강제 장애 주입 PIE는 별도 미검증이다.

## 1. 플레이 동작 / 추천 범위

1. Dusk3마리 목표를 달성한 캐릭터가 지정 NPC에게 다시 G로 대화한다.
2. 기존 대화창에서 보상 내용을 확인하고 `완료 보고·보상 받기`를 누른다.
3. 서버가 유효성을 확인하고 **수락 당시 저장된 회복 포션20001×3**을 지급한다.
4. DB commit 후에만 완료 대사·보상 완료 표시·인벤토리 수량을 갱신한다.
5. 캐릭터당 해당 퀘스트 보상1회. 이미 완료한 B단계 캐릭터도 재처치/재수락 없이 보고 가능.

인벤토리 공간이 부족하면 일부 지급 없이 전체 취소하고 ReadyToTurnIn을 유지한다.
공간을 확보한 후 사용자가 다시 누르면 새로운 요청으로 시도한다. 자동 필드 드랍 없음.
보상 후 재수락 불가라는 기존1회성 규칙 유지. 자동 지급, 경험치/골드, 새 아이템,
반복 퀘스트, 선택 보상,80칸 전체 해금, 새로운 UI 아트는 이번 범위가 아니다.

## 2. 실제 조사 / 유지할 구조

조사한 서버 루트: `E:/Git Fork/GameServer/GameServer/GameServer`.
- QuestRuntime.h::State는 rewardItem/rewardCount/accepted UUID/returnNpc/revision을 보유.
- QuestDB.cpp는 수락 당시 보상 snapshot을 CharacterQuest에 이미 저장한다.
  최신 DT의 보상량으로 기존 캐릭터의 약속된 보상을 덮어쓰지 않는다.
- 현재 DB CharacterQuest.status CHECK는1Active/2Ready만 허용한다.
  무작정3Rewarded를 UPDATE하면 제약 위반이다.
- LootDefinition.h: 실제 슬롯20개, 포션 MaxStack99.80칸 UI와 실제 수용량을 혼동하지 않는다.
- LootDB.cpp/PotionUseDB.cpp/PlayerFieldDropDB.cpp는 CharacterData 행 잠금으로
  캐릭터별 인벤토리 변경을 직렬화한다. 포션 사용 state2 예약이 있으면 보상은 Busy.
- ItemGrantLedger는 drop_id와 ItemPickupRequest에 결합되어 있다. 보상용 가짜 drop/death
  UUID를 만들어 이 원장에 우회 기록하지 않는다.
- S1InventoryState::RequireVersion은 Max를 사용한다. 보상 결과는 인벤토리 최소 revision을
  요구하고 기존 snapshot을 재조회한다. 클라이언트에서 포션 수량+3을 직접 계산하지 않는다.
- 현재 정본 Protocol.proto 마지막1044. 신규2패킷은 뒤에 추가하고 양쪽을 생성한다.
- 기존 NPC 대화/QuestSubsystem/QuestTracker를 확장한다. 별도 퀘스트 권위 시스템 없음.

읽기 전용 확인: MCP 정상, PIE=false, LobbyMap. Dialogue/Tracker/Quest DT clean.
DB 연결 정상; 저장된 보상은20001×3이며 Active/Ready 행 존재. 아래 신규3테이블은 없음.
계정 비밀정보·개별 캐릭터 식별자는 이 조사 기록에 포함하지 않는다.

## 3. 완료 상태와 호환성 — 기존 테이블 ALTER 없이

추천: **CharacterQuest.status는 기존1/2 유지**, QuestRewardGrant 존재를 영속 완료 근거로 사용.
- Active: status1, grant 없음.
- ReadyToTurnIn: status2, grant 없음.
- Rewarded: status2, grant 존재. UI/서비스가 합성한 완료 상태이며 status3을 저장하지 않는다.
- 지급 commit에서 CharacterQuest.revision도+1하여 오래된 미지급 응답을 차단한다.
- 조회/수락 재응답은 지급 원장을 함께 읽어 rewarded를 채운다. ReadState의 원래1/2 파서 유지.
- revision과 지급 여부는 **하나의 SELECT/JOIN snapshot**에서 읽는다. 별도 조회를 섞어
  새 revision+과거 rewarded=false를 만들지 않는다. 동일 수락 건에서 true인 rewarded는
  같은/낮은 revision 응답으로 false로 되돌리지 않는다. 원장·소유자 불일치는 조회 실패로 처리.

기존 quest_contract_version1을 의미 변경하지 않는다. **별도 reward capability1**을 추가한다.
기존 클라이언트는 새 필드를 무시하고3/3목표달성 상태만 본다. 보상 요청/새 결과를 받지 않는다.
업그레이드 클라이언트도 reward capability0이면 지급 버튼 비활성. 기능 미준비를 미지급 확정으로
오인하지 않는다. C스키마가 없어도 B의 수락·진행 기능은 유지한다.
과거 B서버 바이너리는 보상 원장을 모르므로 완료 표시를 지원하지 않는다. 이를 C호환 서버로
보고하지 않으며, 롤백 시 보상 기능을 끄고 원장을 보존한다. 기존 지급 아이템은 되돌리지 않는다.

## 4. 최소 패킷 제안 — 아직 미생성

추가 필드(현재 사용 중인 마지막 tag를 확인함):
- C_ENTER_GAME: `uint32 quest_reward_contract_version=10`.
- S_ENTER_GAME: `uint32 quest_reward_contract_version=13`.
- QuestStateInfo: `bool rewarded=13`, `uint32 reward_item_id=14`, `uint32 reward_count=15`.
  수량은 수락 snapshot의 표시값이며 클라이언트가 지급 권한으로 사용하지 않는다.
  rewarded는 지급 원장 조회가 가능한 서버에서만 신뢰; capability0일 때 완료 여부 미지원 표시.

| 제안 ID | 메시지 | 필드(tag) |
|---|---|---|
|1045|C_QUEST_TURN_IN|request_id bytes1, quest_id uint32 2, accepted_instance_id bytes3, npc_id uint32 4, contract_version uint32 5|
|1046|S_QUEST_TURN_IN_RESULT|request_id bytes1, character_id uint64 2, quest_session_id bytes3, contract_version uint32 4, decision enum5, reason enum6, QuestStateInfo state7, inventory_version uint64 8|

Decision:0None/1Committed/2Rejected/3Unconfirmed.
Reason:0None/1InvalidRequest/2Unsupported/3NotAccepted/4NotReady/5UnknownNpc/
6TooFar/7Occluded/8StateBlocked/9InventoryFull/10Busy/11AlreadyRewarded/
12RequestConflict/13DbUnavailable/14InvalidRewardDefinition.
고유 prefix를 붙인 Enum.proto 이름을 정의하며 기존 enum 번호는 변경하지 않는다.

request/accepted UUID는 nonzero16bytes. 캐릭터/보상ID/수량/아이템 행/완료 판정은
요청에서 받지 않는다. 인증된 GameSession.player와 DB 행에서 결정한다.

### 동일 요청·재접속·지연

- (character,request UUID)의 원래 결과를 신규 조건 판정보다 먼저 조회한다.
  동일 UUID에서 quest/accepted UUID/NPC가 다르면 RequestConflict, 재실행하지 않는다.
- 동일 UUID의 Committed 재전송은 같은 결과. **새 UUID여도 accepted UUID의 grant 유일 제약으로 재지급 불가**.
- InventoryFull 등의 확정 Rejected UUID는 결과 고정. 사용자의 다음 명시적 시도는 새 UUID.
- Unconfirmed/3초 timeout은 실패로 단정하지 않는다. quest/inventory를 조회하며 명시적 재확인은 같은 UUID.
  자동으로 새 UUID를 만들어 다시 지급하지 않는다.
- 결과는 기존 quest_session_id/character/revision으로 확인. 이전 세션 결과는 폐기한다.
- 정상 commit 후 접속이 끊겨도 DB 결과를 취소하지 않는다. 재접속 조회로 rewarded와 재고를 복구한다.
- inventory_version은 원래 지급 시점의 최소 version. 이후 이동/사용으로 높아진 version을 되돌리지 않고
  기존 InventorySubsystem에 RequireVersion+query를 요청한다.

## 5. DB 제안 — 신규3테이블, 기존 DDL 변경 없음

다음은 검토용이다. 실행용 migration은 승인 후 작성한다.

```sql
CREATE TABLE QuestRewardRequest (
 character_id BIGINT UNSIGNED NOT NULL,
 request_id BINARY(16) NOT NULL,
 quest_id INT UNSIGNED NOT NULL,
 accepted_instance_id BINARY(16) NOT NULL,
 npc_id INT UNSIGNED NOT NULL,
 decision TINYINT UNSIGNED NOT NULL,
 reason INT UNSIGNED NOT NULL,
 inventory_version BIGINT UNSIGNED NOT NULL DEFAULT 0,
 result_revision BIGINT UNSIGNED NOT NULL DEFAULT 0,
 created_utc DATETIME(6) NOT NULL,
 PRIMARY KEY(character_id,request_id), CHECK(decision IN(1,2))
) ENGINE=InnoDB;

CREATE TABLE QuestRewardGrant (
 accepted_instance_id BINARY(16) NOT NULL PRIMARY KEY,
 character_id BIGINT UNSIGNED NOT NULL,
 quest_id INT UNSIGNED NOT NULL,
 request_id BINARY(16) NOT NULL,
 item_definition_id INT UNSIGNED NOT NULL,
 item_count INT UNSIGNED NOT NULL,
 inventory_version BIGINT UNSIGNED NOT NULL,
 quest_revision BIGINT UNSIGNED NOT NULL,
 server_boot_id BINARY(16) NOT NULL,
 committed_utc DATETIME(6) NOT NULL,
 UNIQUE KEY uq_quest_reward_request(character_id,request_id),
 FOREIGN KEY(accepted_instance_id) REFERENCES CharacterQuest(accepted_instance_id),
 FOREIGN KEY(character_id,request_id) REFERENCES QuestRewardRequest(character_id,request_id),
 CHECK(item_count>0), CHECK(quest_revision>0)
) ENGINE=InnoDB;

CREATE TABLE QuestRewardGrantItem (
 accepted_instance_id BINARY(16) NOT NULL,
 item_instance_id BIGINT UNSIGNED NOT NULL,
 slot_index INT UNSIGNED NOT NULL,
 granted_count INT UNSIGNED NOT NULL,
 before_count INT UNSIGNED NOT NULL,
 after_count INT UNSIGNED NOT NULL,
 PRIMARY KEY(accepted_instance_id,item_instance_id),
 FOREIGN KEY(accepted_instance_id) REFERENCES QuestRewardGrant(accepted_instance_id),
 CHECK(slot_index<20), CHECK(granted_count>0),
 CHECK(after_count=before_count+granted_count), CHECK(after_count<=99)
) ENGINE=InnoDB;
```

GrantItem을 분리하는 이유: 기존98개 스택에1개, 빈 슬롯에2개로 나눠 지급하는 경우의
각 변경을 기록하기 위해서다. Item 행은 나중에 사용/합치기/버리기로 사라지므로, 기록이
유실되는 Item FK나 ON DELETE CASCADE를 넣지 않는다. 지급 기록은 드랍 원장과 분리한다.
InventoryState.version+1은 지급 전체당1회. 기존 Item/InventoryState의 행 데이터는
갱신하지만 기존 테이블 정의는 변경하지 않는다.

### 트랜잭션과 잠금 순서

1. CharacterData를 FOR UPDATE(기존 아이템 갱신과 같은 직렬화 기준).
2. 요청 원장 조회, CharacterQuest를 FOR UPDATE, grant 존재를 current/locking read로 확인.
3. owner/quest/accepted UUID 일치, Ready, snapshot 보상과 신뢰된 NPC/현재 Room 확인.
   신규 지급은 기존 대화 조건처럼 생존/비공격/비Q/최근 피격3초/위협/거리/높이/정적 OBB LOS 검사.
   현재 정의 변경/비활성화로 기존 snapshot 보상을 임의 변경하지 않는다.
4. ItemUseOperation.state2가 있으면 Busy. InventoryState와 대상 Item 행 잠금.
5. 슬롯 오름차순의 기존 동일 아이템 스택을99까지 채우고, 남은 수량은 빈 슬롯 오름차순으로 신규 행에 배치.
   전량 배치 계획부터 검증한다. 수용 불가이면 Item/quest/grant를 전혀 변경하지 않는다.
6. Item 갱신, 재고 version+1, quest revision+1, Request/Grant/GrantItem을 같은 TX에 기록하고 commit.
   모두 성공했을 때만 Committed.
7. commit 응답 불명확은 별도 연결에서 같은 request/grant를 readback. 재지급이 아니다.

기존 QuestService의 bounded worker/weak Room callback을 확장하며 게임/Room 스레드에서
I/O를 기다리지 않는다. 경제 처리는 DB 단일 TX로 끝난다. Room 반영 전 프로세스가 종료돼도
commit된 재고·지급 기록을 재조회할 수 있다. commit 전이면 rollback되어 재시도 가능하다.
신규 durable outbox/외부 발행 서비스는 이번에 추가하지 않는다(DB 지급 원장 자체는 영속).

## 6. NPC·UI — 기존 WBP/Export v2 활용

- 신규 NPC 모델/Widget Blueprint 불필요. 기존 WBP_S1Dialogue의 주 버튼을 보고에 사용한다.
- DT_S1DialogueNodes에 End 행6보고대기/7보고완료/8진행중을 추가하는 안.
  NPC에 해당 시작 NodeId·대상 QuestId 설정을 추가하고 권위 snapshot으로 시작 노드를 선택한다.
  기존1→2→3의 미수락 흐름과 수락/거절 분기는 유지한다.
- 새 NodeKind나 Export v3 불필요. 기존 End를 유지하고 보고 모드에서만 주 버튼을 활성화한다.
- 보상 내용은 서버 snapshot으로 표시. G는 Line 넘기기만 유지하며 G 연타로 지급 요청하지 않는다.
- 정확한 Committed 요청과 원래 대화 run/NPC가 일치할 때만 성공 분기.
  닫힌 대화/카메라는 늦은 응답으로 재생성하지 않는다. 조회로 이미 지급됨이 확인되면 상태를 갱신한다.
- Tracker에서 목표달성→보고가능→보상수령완료를 구분한다. 보고완료 상태 지속 표시는 최소 구현으로 두고,
  완료 연출/자동 숨김/퀘스트 이력 화면은 후순위다.
- 인벤토리/퀵슬롯 등록은 기존 구현 유지. 수량은 snapshot 갱신, 임의 자동 등록 없음.

## 7. 변경 예정 파일

Unreal root `E:/Git Fork/S1_UE5.8`:
- Source/S1/Account/S1AccountSubsystem.cpp(entry의 reward capability 요청).
- Source/S1/ClientPacketHandler.cpp, Network/S1NetworkSubsystem.h/.cpp(typed dispatch).
- Source/S1/Quest/S1QuestSubsystem.h/.cpp, S1DialogueSubsystem.h/.cpp, S1QuestNpc.h.
- Source/S1/Loot/S1InventorySubsystem.h/.cpp(보상 후 최소 version 지정/재조회의 공개 API).
- Source/S1/Quest/S1QuestContractTests.cpp, 신규 S1QuestRewardTests.cpp.
- 생성만 수행: Network/{Protocol,Struct,Enum}.pb.*, ClientPacketHandler.h.
- 예정 asset: BP_S1QuestNpc, DT_S1DialogueNodes. 기존 Dialogue/Tracker WBP는
  레이아웃 변경 없음을 우선하고, 필요한 경우에만 checkpoint/dirty 보고/선택 저장.
- Docs/CurrentAssetRegistry.md, 신규 C 구현·검증 기록.

Server root `E:/Git Fork/GameServer`:
- GameServer/Common/Protobuf/bin/{Protocol,Struct,Enum}.proto 정본과 생성물.
- GameServer/GameServer/{QuestRuntime.h,QuestDB.cpp,QuestService.h/.cpp,RoomQuest.cpp,
  Room.h,Room.cpp,Player.h,ClientPacketHandler.cpp,GameServer.vcxproj}.
- 같은 폴더에 QuestRewardDB.h/.cpp 신규. 기존 QuestDB.cpp에서 지급 기록 조회로 snapshot 합성.
- 신규 GameServer/Tests/QuestRewardDBIntegrationTests.cpp, QuestRewardPlanTests.cpp.
- DummyClient/ServerPacketHandler.cpp는 기능을 광고하지 않는 호환 수신만 추가.
- 승인 후 codex_md_edit/db/QuestStageCRewardMigration.sql 작성.
- DT 변경의 v2 Export와 Data/QuestDefinitions.s1quest 생성·배치. MainMap 배치 변경/저장 없음.

## 8. 검증 / 적용 순서 / 승인 항목

1. 이 범위·프로토콜·신규3테이블에 관한 사용자 승인.
2. checkpoint, 격리 DB migration(관리자 수동), 코드/packet 생성/빌드.
3. 격리 DB: 정상 지급,98+1/신규2 분산, 빈칸 없지만 기존 여유 용량 있음, 전체 용량 부족 rollback,
   동일 UUID 재전송/다른 payload, 새 UUID 중복 지급, 동시 요청, 사용/이동/획득과의 경쟁, 사용 예약 Busy,
   commit 응답 유실, snapshot 고정, 재접속 조회, 재고·quest revision 역행 방지.
4. 새 GameDB 백업 후 격리 검증을 통과한 DDL만 관리자가 운영 DB에 적용.
   B단계 기존 진행 행을 삭제하거나 재수락시키지 않는다.
5. 2인 PIE: 각자 보고로 각자에게만 포션3개 증가, 한쪽 보고로 상대의 재고/완료 상태가 바뀌지 않음,
   보고 연타/재접속 후 재지급 없음, UI/카메라 복귀, 기본 전투/기존 아이템 회귀.
6. 용량 부족→비운 뒤 재보고, 실패/지연 중 성공 표시 없음, 닫힌 뒤 재개 없음 확인.
   최신 패키지 검증은 별도로 미검증 표시.

추천 승인 범위: **지정 NPC에서 수동 보고, 저장된 포션3개를1회 지급, 전량 수용 불가일 때
미지급 유지, 신규2패킷+capability/상태 추가 필드, 신규3테이블, 기존 DDL 변경 없음**.
구현 전 이 구체안의 승인을 확인한다. 신규 콘텐츠/경험치/골드는 포함하지 않는다.
