# 플레이어 공용 필드 드랍 구현 계약 — 2026-10-14

상태: 사용자 '승인'으로 구체적인 신규 패킷·DB 스키마까지 승인됨.
승인된 방향을 다시 묻지 않음: 플레이어가 버린 아이템만 공용,
몬스터 드랍의 기존 소유권은 유지. 이번 조사에서는 코드·패킷·DB·BP를
수정하지 않은 조사 당시 기준이며, 이후 DB 기반 소스/SQL 구현을 시작했다.
현재 실행 중인 버전은 아직 영구 삭제 구현이다. 단계별 실제 진행은
PlayerFieldDropImplementation.md 참조.

## 확인한 현행과 경계
- 외부 IOCP가 차감/드랍/획득/만료를 결정; Unreal은 입력·표현만 담당.
- MCP 정상, PIE=false; Grid/DiscardDialog 올바른 부모, 모두 clean.
- GameDB8.0.45,9tables; LootDrop.death_id NOT NULL/UNIQUE.
- LootDB::Create는 몬스터 death/spawn ID와 소유자, 포션1개를 전제.
- LootDB::Pickup/Room/클라이언트 힌트 모두 소유자 전용 검사.
- FS1LootState는 count1만 허용; WorldEntity는 액터에 count1 하드코딩.
- 기존 승인 버리기1037/1038은 영구 삭제 계약이다. 필드 버리기로 의미를
  바꾸거나 사망 이벤트로 플레이어 드랍을 위장하지 않는다.
- 기존 몬스터 LootDrop/ItemGrantLedger와 해당 소유권/회귀 증거를 보존.

## 1차 동작 (별도 콘텐츠 질문 없이 이 범위로 구현)
포션20001/20사용 슬롯/99스택만. 기존 밖 드롭·수량·Yes/No UI와
포션 외형·Niagara·G 획득 재사용. 선택 수량1..스택수량은 필드 드랍
1개로 묶는다. 같은 방의 인증된 플레이어 누구나 전량 획득 가능.
부분 획득 없음; 기존 스택의 여유와 빈 슬롯을 합산해 전량 수용할 수
없으면 차감/드랍 상태 변경 없이 거부. 몬스터 드랍은1개/소유자 전용.
필드 좌표는 서버에 기록된 플레이어X/Y와 현재 MainMap 평지 높이 기준.
임의 좌표를 클라이언트에서 받지 않음; 경사/계단/높이 Export는 제외.
기존150cm 획득 거리/벽 가림 검사를 유지. 생성 후30초 만료,
서버 재시작 시 이전 boot의 미획득 드랍은 만료(반환/영구 보존 없음).
현재 몬스터 드랍 복구 정책과 일관되며 새로운 AWS/거래/우편은 제외.

## 신규 패킷 제안 (아직 구현 아님)
기존 ID/의미 변경 없이 canonical Protocol.proto 뒤에 추가:
- 1039 C_PLAYER_FIELD_DROP:
  request_id bytes1(16nonzero),source_item_id uint64/2,source_slot uint32/3,
  amount uint32/4,expected_inventory_version uint64/5,contract_version uint32/6(1).
  client owner/좌표 필드 없음.
- 1040 S_PLAYER_FIELD_DROP_RESULT:
  request_id bytes1,authenticated character_id uint64/2,source_item_id uint64/3,
  source_slot uint32/4,requested_amount uint32/5,decision uint32/6,
  reason uint32/7,inventory_version uint64/8,remaining_count uint32/9,
  drop_id bytes10(성공 또는 성공 재조회 시).
  decision1committed/2rejected/3unconfirmed.
  reason0none/1invalid/2stale/3source/4amount/5busy/6DB/7unsupported/
  8room-or-position/9request-payload-conflict/10capacity-or-rate-limit.
- C_ENTER_GAME 새 field8 / S_ENTER_GAME 새 field10:
  player_field_drop_contract_version1.
- Struct.proto LootDropInfo 새 field10:
  source_kind0legacy-monster-owned /1monster-owned /2player-public.
  owner_character_id는 기존0불허 클라이언트/액터와의 호환을 위해
  private는소유자/public은버린캐릭터의 출처 ID 유지. 공용 여부는 명시적인
  source_kind만으로 판단하며 출처 ID를 획득 권한으로 쓰지 않음.
- 기존 S_LOOT_CREATED/REMOVED/SNAPSHOT + C_ITEM_PICKUP/RESULT 재사용.
  새 기능을 협상한 클라이언트에만 public 드랍을 보내며 레거시 snapshot에
  섞어 전체 Decode를 실패시키지 않는다. 획득도 capability 검사.
  old monster0도 여전히 count1/소유자 전용; unknown kind는 거부.
- 새 클라이언트 UI는 신규 계약에만 제출하고 미지원이면 버리기 비활성.
  1037영구삭제로 fallback 없음. 새 서버는 영구삭제 capability0 광고 및
  구1037 요청 Unsupported 거부: 이전 클라이언트도 실수로 삭제하지 않음.
- 생성 ID는 실제 생성기 결과를 확인; 양쪽 pb/handler 모두 재생성,
  생성 파일 직접 편집 금지.

## DB 추가안: 새 테이블3개, 기존 테이블 ALTER 없음
1. PlayerFieldDropRequest
   PK(character_id,request_id), 입력 source item/slot/amount/expected version,
   decision/reason, 결과 inventory_version/remaining_count/drop_id, created_utc.
   같은 요청의 결과를 영속 기록. 동일 ID 다른 payload는 충돌 거부.
2. PlayerFieldDrop
   drop_id PK, UNIQUE(source_character_id,source_request_id), 원본 item ID와
   definition/count, room/server/boot, posXYZ,created/expires UTC,status0/1/2.
   몬스터 death/spawn 컬럼 없음. 원본 차감과 같은 트랜잭션에서 INSERT.
3. PlayerFieldDropGrant
   PK(drop_id,item_instance_id), 획득 character/request/version,
   각 할당의 item_definition_id,amount,before_count,after_count,committed_utc.
   전량을 여러 기존 스택과 새 슬롯에 나누어 넣었을 때 각 증가분 기록.
   기존 ItemPickupRequest와 정합적인 요청 관계를 유지하며 과거 기록 삭제
   또는 Item 행 삭제의 cascade로 획득 증거를 없애지 않음.

필드 DROP:
캐릭터 행 잠금 -> 동일 요청 결과 조회/입력 비교 -> versionCAS/소유자/
수량/사용환불예약 검사 -> Item 차감 또는 행 삭제 -> PlayerFieldDrop 생성
+ PlayerFieldDropRequest 결과 + InventoryState version++ 한 트랜잭션.
COMMIT 뒤에만 Room 등록/양쪽 표시. 불명확한 COMMIT은 같은 요청의
영속 결과를 읽어 복구; 인벤토리 재차감 또는 다른 drop UUID 재생성 금지.
클라이언트 timeout은 결과미확정; 자동 새 요청/임의 재전송 없음.

필드 PICKUP:
획득 캐릭터 행 잠금 -> cached request -> 공용 drop행 FOR UPDATE ->
상태/boot/room/거리/벽/전량 용량 검사 -> 모든 스택/슬롯 할당 + Grant
+ PickupRequest + inventory version++ + drop status1 한 트랜잭션.
다른 플레이어가 동시 접근해도 status0->1 확정은 한 명만.
실패/용량 부족은 드랍을 남긴다. 사용환불 예약 슬롯은 건드리지 않음.
몬스터 private pickup 코드는 기존 경로 유지. 사망/획득/생성/만료
이벤트는 source_kind를 구분; 플레이어 버리기에 MonsterDied 이벤트 금지.

새 테이블 부재/권한 실패: 신규 계약0만 광고; 기존 전투·몬스터드랍·
포션·인벤토리 작동 유지. 자동 DDL 없음. 백업+격리 DB migration/tests 후
사용자가 관리자 연결로 운영 적용. 비밀번호를 요청하거나 기록하지 않음.

## 실제 확인한 변경 예정 경로
서버 E:/Git Fork/GameServer/GameServer/
Common/Protobuf/bin/Protocol.proto,Struct.proto; GameServer/Player.h,
Room.h/.cpp,LootService.h/.cpp,ClientPacketHandler.cpp,LootDB.h/.cpp
(기존 private pickup와 공용 요청 분기/재조회 연결만);
새 GameServer/PlayerFieldDropDB.h/.cpp, Tests/PlayerFieldDropDBTests.cpp;
GameServer/GameServer.vcxproj 및 GameServer/GameServer.vcxproj.filters 신규 cpp
등록, DummyClient/ServerPacketHandler.cpp; 생성 pb/handler.
SQL 추가 예정 절대 경로:
E:/Git Fork/GameServer/codex_md_edit/db/PlayerFieldDropMigration.sql.
클라이언트 E:/Git Fork/S1_UE5.8/Source/S1/
Loot/S1InventorySubsystem.h/.cpp,S1LootState.h/.cpp,S1LootActor.h/.cpp,
World/S1WorldEntitySubsystem.cpp,Network/S1NetworkSubsystem.h/.cpp,
Account/S1AccountSubsystem.cpp,S1GameInstance.cpp,ClientPacketHandler.cpp;
UI/S1InventoryDiscardWidget.h/.cpp(필드 버리기 문구/모드);
Tests 역할인 Loot 및 UI 기존/신규 자동 테스트, 생성 pb/handler.
의도된 BP 변경은 WBP_S1InventoryDiscardDialog의 기본 안내 문구가
필요한 경우만. 이름/바인딩/스타일 보존; Grid/HP/Split/Quickslot 불변.
바이너리 전에 현재 dirty 보고·체크포인트·해시 검증, 의도한 파일만 저장.

## 검증 단위
격리 DB: 차감-생성 atomic rollback/COMMIT재조회/중복/변조/소유자/버전/
예약보호, 여러스택전량수용/부족시불변, 서로다른2캐릭터 동시G획득 한명,
버린 A -> B 획득 총수량 보존, 만료/boot 변경/재접속.
자동: source kind/수량 decode, 레거시 공용 필터, owner-private/public 힌트,
영구삭제 fallback 없음, UI 취소/Unknown/세션 교체, 기존 회귀.
빌드: Debug server + S1/S1Editor, 영향 BP compile/selective save,
Windows package 포함/실행. 새 버전1인/2인 PIE는 사용자 시각/조작 확인:
A버림 양쪽보임 -> B전량획득 양쪽제거 -> B재접속수량유지,
몬스터드랍 비소유자G거부, 동시획득/만료/기존인벤토리와전투 정상.
미실행 결과는 미검증. 새 에셋 구매·복사·DB 관리자 권한 변경·Git
commit/push·Astra 메시지 전송은 이 작업에 포함하지 않음.

## 필요한 승인
이미 승인된 공용/소유자 정책은 유지한다. 추가 승인 대상은 위 신규
1039/1040 및 capability/source_kind 필드와 추가 테이블3개뿐이다.
사용자가 위 범위를 승인했다. 동일 승인을 반복 요청하지 않으며,
관리자 SQL 실행/운영 적용/프로세스 재시작은 필요한 시점에 요청한다.

