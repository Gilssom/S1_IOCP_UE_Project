# NPC 대화형 퀘스트 구현 전 조사 — 2026-10-16

후속 상태: 사용자 승인으로 A단계 구현 및 BP/WBP/MainMap 선택 저장까지 진행.
최신 구현·검증·미검증은 NpcDialogueStageAImplementation.md를 따른다.
아래는 구현 전 제안이며 B/C 계약·퀘스트 수치를 승인한 기록이 아니다.

상태: 조사·제안만. 코드/프로토콜/DB/Blueprint/맵 변경 및 빌드/PIE 없음.
이번에 변경하는 것은 본 문서와 CurrentAssetRegistry.md뿐이다.
기획 채팅 전달 내용을 새 서버 계약 승인으로 간주하지 않는다.

## 사용자 방향과 미확정 제안

확정 방향: NPC 대화로 수락/거부, 페이드로 가리는 투샷 카메라 전환/원복,
수락 시 제목·스토리·조건·보상 상세 표시, 필요한 리소스 안내.
제안: 우측 상세, 하단 대화, 수락/거부 후 종료, NPC 복귀 보상,
작은 추적 UI 및 상세 다시 열기. 위치/폰트/문구는 Designer 편집 가능하게 한다.
NPC 외형, 목표3마리/포션3개, 마지막 타격자, 캐릭터당1회는 미확정.
최근 폰트 추천은 실제 선택·임포트가 아니다.

## 현재 상태와 실측

- S1 main d80737f: 포커스/Dusk cook 수정 커밋·푸시 완료.
  기존 PortfolioNextStepPlanning_2026-10-15.md는 미추적 기획 문서로 보존.
- 서버/에디터 실행 관측. 초기 MCP 8000 거부 후 사용자 StartServer 실행,
  새 세션 연결 성공, IsPIERunning=false. MCP는 직렬 읽기 전용으로 조사.
- AS1MyPlayer::PickupPotion은 G를 Inventory::RequestPickup에 직접 연결한다.
  Inventory::SelectDrop은 거리150cm/수직차100cm/소유권/가림/화면상태 필터로
  후보를 선택하고 Tick에서 힌트를 갱신한다. NPC에 별도 G 바인딩하면 중복 위험.
- MyPlayer::CanUseGameplayInput은 커서/일시정지로 이동·시야·공격·점프 등을
  제한한다. I 토글은 별도 경로다. QuickSlot은 별도 CanUseQuickSlot 및
  Slate viewport focus 검사를 사용한다. 커서만 켜는 것으로 전 경로가 막힌다고 보지 않는다.
- 카메라는 MyPlayer CameraBoom+FollowCamera; 절대 회전 arm, 회전 lag 비활성.
  새 로컬 CameraActor view target로 분리해 기존 콤보 카메라 수정을 보존한다.
- Room::ApplyMonsterDamage에서 DEAD 전환 후 QueueDeathDrop을 호출한다.
  spawnId는 매 생명 생성, runtime.generation도 증가. deathId는 드랍 경로가
  Ready일 때 생성하므로 퀘스트 이벤트를 드랍 생성 성공에 종속시키면 안 된다.
- LootService/AsyncLootDB는 bounded FIFO 작업과 Room::DoAsync 복귀 패턴.
  LootDB 인벤토리 변경은 CharacterData→InventoryState/Item 잠금 구조를 사용한다.
  기존 Pickup은 드랍 대상 지급이지 임의 보상 함수가 아니다. 퀘스트 보상을
  가짜 드랍/획득으로 우회하지 않고 같은 트랜잭션 규칙의 전용 함수가 필요하다.
- 조사한 양쪽 패킷 표는1000~1040까지. 현 퀘스트 패킷·NPC 클래스·퀘스트
  서브시스템은 발견하지 못했다. 신규 번호는 제안 단계에서만 예약 검토한다.

## 에셋 후보 (MCP 실조회)

| 후보 | 메시 | Idle | Skeleton/상태 |
|---|---|---|---|
| Manny, 임시 추천 | /Game/Characters/Mannequins/Meshes/SKM_Manny | /Game/Characters/Mannequins/Animations/Manny/MM_Idle | 양쪽 /Game/Characters/Mannequins/Meshes/SK_Mannequin, dirty=false |
| Terra, 기존 외형 재사용안 | /Game/ParagonTerra/Characters/Heroes/Terra/Meshes/Terra | /Game/ParagonTerra/Characters/Heroes/Terra/Animations/Idle | 양쪽 Terra/Meshes/Terra_Skeleton, dirty=false |

Manny 재질은 MI_Manny_01/02, Physics/PostProcess/ControlRig 의존성도 존재.
Terra는 현재 BP_Player/BP_MyPlayer에서 사용 중; NPC로 복제 배치하지 않았다.
Skeleton 일치는 확인했지만 실제 NPC의 자세/발 위치/크기/가림은 미검증.
포션 아이콘은 기존 /Game/_Resources/Rair_Potion 재사용 가능.
NPC 후보는 아직 Active가 아니다. 최종 외형·배치·사용 권리 기록 확인은 승인 시 수행.
구매할 카메라/Sequencer/립싱크/음성/새 PNG는 필수가 아니다.
UE5_RPG는 이번 조사에서 이관·변경하지 않았다.

## A단계 — 로컬 대화/투샷만 (우선 권장)

예정 신규 파일, 모두 E:/Git Fork/S1_UE5.8/ 기준:
- Source/S1/Interaction/S1InteractionSubsystem.h/.cpp
- Source/S1/Quest/S1QuestNpc.h/.cpp
- Source/S1/Quest/S1DialogueSubsystem.h/.cpp
- Source/S1/UI/S1DialogueWidget.h/.cpp
- Source/S1/UI/S1QuestPanelWidget.h/.cpp
- Source/S1/Quest/S1DialogueTests.cpp
예정 기존 수정:
- Source/S1/Game/S1MyPlayer.h/.cpp: G 라우팅, 대화중 입력/이동 잔량 정리.
- Source/S1/Loot/S1InventorySubsystem.h/.cpp: 기존 후보/힌트 API를 coordinator와
  공유하고, 대화 중 I/퀵슬롯/획득/사용 진입을 명시 차단. 기존 DB 책임은 유지.
예정 바이너리:
- /Game/S1/Quest/BP_S1QuestNpc
- /Game/S1/UI/WBP_S1Dialogue, WBP_S1QuestDetail
- /Game/Maps/MainMap: 승인된 고정 NPC1개 배치만.

G coordinator는 loot/NPC 중 한 후보만 선택·표시·실행한다. 임시 정책 추천은
유효 거리/가림 필터 후 가장 가까운 대상, 비슷하면 기존 후보 유지, 동률 stable ID.
loot pending 재확인은 기존 동일 request 경로를 유지하며 NPC로 바꿔 처리하지 않는다.
구체 거리/우선순위는 승인 전 제안값이며 UI와 실제 수신 대상을 일치시킨다.

대화 상태는 Closed→FadingOut→Opening→Active→FadingOut→Restoring→Closed.
로컬 플레이어별 소유, 중복 진입 거절, 세션 generation으로 오래된 타이머 무효화.
입장 전 view target/컨트롤 회전/관련 카메라 상태 저장. 입력 모드 getter가 있다고
가정하지 않고 기존 Inventory/UI 정책과 명시적 modal 소유권으로 복구한다.
검정0.2초→카메라 전환→밝게0.2초는 편집 가능한 시작 제안값.
NPC별 접근 영역/카메라 anchor/양인물 focus offset 노출, 현 플레이어 위치 기준
framing과 camera sweep/visibility 확인. 실패하면 안전한 게임 시점 대화 fallback.
서버 좌표 변경/플레이어 텔레포트/자동 회전/무적/전체 시간 정지 없음.

입장 시 이동 잔량·기존 누름 상태를 정리하고 새 전투/점프/퀵슬롯/획득/I를 차단.
전투 중 진입은 우선 거절하는 최소안. 허용 여부는 콘텐츠 결정 필요.
Esc/피격/사망/거리 이탈/NPC소멸/네트워크 종료/월드 종료에서 같은 멱등 cleanup.
사망/맵전환은 기존 시점 정책 우선, 파괴된 view target로 복원 금지.
페이드 소유권을 해제하고 커서/포커스/입력 잠금 잔류를 검사한다.
A단계 수락은 서버 연결 전 비활성·미구현 표기: 가짜 성공 팝업/진행도를 저장하지 않는다.

## B/C단계 — 서버 계약 초안 (구현 전 별도 승인)

후보 이름/번호: C_QUEST_QUERY1041, S_QUEST_STATE1042,
C_QUEST_ACTION1043, S_QUEST_ACTION_RESULT1044. 현행 구현이 아니며 확정 전 재중복 확인.
LOGIN/ENTER capability 필드는 기존 tag를 재사용하지 않고 새 미사용 tag를 조사해 추가.
버전 불일치는 기능 비활성/unsupported, 구버전에서 성공 연출하지 않는다.

- Query: request UUID/contract version. 신원은 세션에서 얻는다.
- Action: request UUID/quest ID/definition version/NPC ID/Accept 또는 Claim.
  캐릭터ID·좌표·킬수·보상량을 권위 값으로 받지 않는다.
- State: 캐릭터/quest ID/definition version/state/progress/required/revision.
- Result: 요청 UUID/결정(Committed,Rejected,Unconfirmed)/이유/서버 snapshot revision,
  Claim 성공 시 inventory version. 수락과 목표달성과 보상 완료를 분리한다.
- 클라이언트는 결과 미확정 시 재조회; 새 ID로 무조건 재시도 금지.
  같은 UUID 다른 payload 충돌 거절. 중복 Claim은 기존 결과만 반환.
  닫힌 대화의 늦은 결과는 상태만 갱신, 창·카메라는 열지 않는다.
- 서버 고정 NPC 정의는 ID/맵/위치/상호작용반경/정의버전.
  클라이언트 배치와 export 비교/검증하고 장애물 검사도 실시한다.
  동적 NPC 이동/스폰 protocol은 불필요. 단 위치 대응 도구는 별도 구현 필요.

DB 추가 테이블 제안:
1. CharacterQuest: 캐릭터+퀘스트 PK, 상태/진행/정의버전/revision/시각.
2. QuestOperation: 캐릭터+request UUID 고유, action/payload/result/시각.
3. QuestKillCredit: 캐릭터+퀘스트+accept revision+monster life key 고유.
필요한 보상 audit/outbox는 기존 구조 재사용 가능 여부를 구현 직전 결정.
테이블명·컬럼·DDL은 아직 승인/실행 안 됨; DB 현재 실측도 이번 조사에서 하지 않았다.

Kill은 Room의 권위 사망 전환에서 독립 이벤트를 만들고 해당 accept revision에만 적용.
입장/수락 DB대기 중 이벤트 순서와 컷오프를 Room 큐에서 명확히 정한다.
드랍 service 실패와 무관한 (boot,room,spawnPoint,generation) 생명 키 등을 검토하고
DB 중복 방지+진행증가를 한 트랜잭션으로 처리. 재접속 중 비동기 결과의 세션 확인.
DB 장애 때 처치 유실을 감출 수 없으므로 bounded pending/retry 또는 durable outbox
보장 수준을 구현 전 확정한다. 서버 재시작까지 보장하려면 durable 기록이 필요하다.
보상은 기존 CharacterData→InventoryState 순서와 일관된 잠금, 스택99/실제20칸,
진행·operation·수량·인벤토리버전을 한 트랜잭션으로 확정. 가득 차면 전체 미지급.

예정 서버 변경 E:/Git Fork/GameServer/GameServer/ 기준:
- Common/Protobuf/bin/Protocol.proto (정본), 필요한 Enum/Struct.proto만 추가 검토.
- GameServer/Room.h/.cpp, ClientPacketHandler.h/.cpp, Player.h,
  LootService.h/.cpp, GameServer.vcxproj; 신규 QuestDefinition/QuestDB/QuestRuntime,
  Tests/QuestRuntimeTests.cpp 및 QuestDBIntegrationTests.cpp.
- E:/Git Fork/GameServer/codex_md_edit/db/NpcQuestMigration.sql 신규안.
  별도 중첩 Git 저장소이므로 배포/버전 관리 범위를 혼동하지 않는다.
클라이언트: Source/S1/ClientPacketHandler.h/.cpp, Network/S1NetworkSubsystem.h/.cpp,
Level/S1LevelFlowSubsystem.h/.cpp 또는 GameInstance 세션 시작·정리 연결,
신규 Quest/S1QuestSubsystem.h/.cpp 및 UI binding.
Protobuf 출력은 양쪽 재생성만 수행; 생성 pb 파일 직접 수정 금지.

## 검증·승인 게이트

A: 단독 로컬 대화·G 충돌·투샷/벽 fallback·반복 Esc·입력/카메라 원복;
두 PIE 동시 NPC 대화 독립; 피격/사망/파괴/단절/맵전환 cleanup.
B: 수락 전 소급 없음, 수락 중 kill 경계, 두 캐릭터 독립, 중복 생명 event,
진행도 재접속, DB 지연/실패 보장 수준. C: 공간 부족/동시 아이템 조작/중복 Claim/
응답 유실 후 조회/원자 rollback. D: 실제 패키지 cook 참조와 영향 경로만 검증.
기존 전체 전투·아이템 인수를 이유 없이 재요청하지 않는다.
바이너리 변경 전 MainMap/변경 대상 체크포인트, 저장 전 dirty 보고/선택 저장,
CDO 또는 명시적 cook root로 신규 WBP/NPC 의존성을 포함하고 smoke 검증.

사용자 결정:
1. 임시 Manny로 A단계부터 검증할지, 최종 NPC 에셋을 먼저 추가할지.
2. 제안 퀘스트3마리/포션3개/마지막타격/캐릭터1회 채택 여부(B 전).
3. 우측 상세+하단 대화/수락거부 후 종료, 전투중 진입 거절안 채택 여부.
서버/DB 신규 계약은 A단계와 분리해 구체 DDL·필드안 제출 후 승인받는다.
