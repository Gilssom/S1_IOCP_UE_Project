# Quest C — 완료 보고·1회 보상 구현 (2026-10-19)

상태: 승인 범위 구현·자동 검사 완료. 사용자가 최신 PIE의 안내한 전체 항목 검증 완료 보고.
최신 패키지 검증은 미실행.
계약: [QuestStageCRewardContract_2026-10-19.md](QuestStageCRewardContract_2026-10-19.md).
B단계의 이전 사용자 검증을 C단계 성공으로 재사용하지 않는다.

## 플레이 동작

- Dusk3 목표 달성 후 지정 NPC에게 G로 대화하고 기존 주 버튼으로 완료 보고.
- 수락 당시 DB에 저장된 포션20001×3을 서버가 캐릭터별 1회 지급.
- 기존 스택을 슬롯 오름차순으로99까지 채우고 빈 슬롯에 나머지를 배치한다. 실제 용량20칸 유지.
- 공간 부족이면 전량 미지급/Ready 유지. 사용 예약 중이면 Busy. 확정 거부 후 다음 클릭은 새 요청.
- 응답 지연은 실패가 아니다. 조회로 확인하며 명시적 재확인은 동일 UUID. 자동 재지급 없음.
- 재접속 후 DB 원장으로 수령 완료/인벤토리 복구. 보상은 클라이언트 수량 덧셈이 아닌 snapshot 재조회.
- G는 기존 Line 진행 전용이며 보고 버튼을 대신 누르지 않는다. 늦은 응답으로 닫힌 대화를 다시 열지 않는다.

## 변경 파일

Unreal 루트 `E:/Git Fork/S1_UE5.8`:

- `Source/S1/Account/S1AccountSubsystem.cpp`: 보상 capability 요청.
- `Source/S1/ClientPacketHandler.cpp`, `Network/S1NetworkSubsystem.h/.cpp`: typed 결과 dispatch.
- `Source/S1/Quest/S1QuestSubsystem.h/.cpp`: 보고 요청·세션·타임아웃·paid 단조성·tracker.
- `Source/S1/Quest/S1DialogueSubsystem.cpp`, `S1QuestNpc.h`: 상태별 시작 노드·명시적 보고·늦은 callback 보호.
- `Source/S1/Loot/S1InventorySubsystem.h/.cpp`: 최소 inventory version 지정 후 재조회 API.
- 신규 `Source/S1/Quest/S1QuestRewardTests.cpp`.
- canonical generator로만 생성한 `Network/{Protocol,Struct,Enum}.pb.*`, `ClientPacketHandler.h`.
- `Docs/CurrentAssetRegistry.md`, C 계약 및 본 구현 기록.

Server 루트 `E:/Git Fork/GameServer`:

- `GameServer/GameServer/QuestRewardDB.h/.cpp` 신규: 분산 스택 계획·원자적 지급·요청/원장 조회.
- 같은 폴더 `QuestRuntime.h`, `QuestDB.h/.cpp`, `QuestService.h/.cpp`, `RoomQuest.cpp`,
  `Room.h/.cpp`, `Player.h`, `ClientPacketHandler.cpp`, `GameServer.vcxproj`.
- `GameServer/Common/Protobuf/bin/{Protocol,Struct,Enum}.proto`와 생성물(서버/Dummy/Unreal).
- `GameServer/DummyClient/ServerPacketHandler.cpp`: 미광고 호환 수신.
- 신규 `GameServer/Tests/QuestRewardDBIntegrationTests.cpp`.
- `codex_md_edit/db/QuestStageCRewardMigration.sql`: 신규3테이블만, 기존 테이블 DDL 변경 없음.
- `GameServer/GameServer/Data/QuestDefinitions.s1quest` 및 Debug runtime Config 배치.

일반 구현 세부 차이: 순수 지급 계획 검사도 DB 통합 검사 실행 파일 안에 포함했다.
별도 `QuestRewardPlanTests.cpp`와 `S1DialogueSubsystem.h` 수정은 필요하지 않았다. 콘텐츠 계약 변경 없음.

## 패킷·권위·DB

- 기존 quest contract1 유지. 별도 quest_reward_contract_version1 협상.
- 1045 C_QUEST_TURN_IN /1046 S_QUEST_TURN_IN_RESULT 추가. 필드/enum 번호는 승인 계약대로 생성.
- CharacterQuest.status1/2 유지. QuestRewardGrant 존재로 rewarded를 합성하며 revision과 같은 JOIN에서 읽는다.
- CharacterData 잠금으로 기존 아이템 변경과 직렬화. Item·InventoryState·quest revision·Request·Grant·GrantItem을 단일 TX commit.
- accepted UUID 유일성 + 요청 UUID 결과 기록으로 같은/새 UUID 중복 지급 차단. Item 삭제 후에도 지급 영수증 보존.
- 사용 예약 state2의 수량0 Item 행은 덮어쓰지 않는다.
- Room은 생존/공격/Q/최근 피격3초/위협/NPC 거리·높이·정적 OBB LOS를 검사한다.
- DB bounded worker와 weak Room/session callback 사용. Unreal이 DB에 직접 접근하지 않는다.
- 구버전 클라이언트는 보상 기능을 광고하지 않는다. C schema 미준비이면 B 기능만 유지.

## 에셋·설정

- `/Game/S1/Quest/BP_S1QuestNpc`: parent `/Script/S1.S1QuestNpc`, generated `BP_S1QuestNpc_C`.
  DialogueQuestId1, TurnInDialogueNodeId6, RewardedDialogueNodeId7, InProgressDialogueNodeId8.
- `/Game/S1/Quest/Data/DT_S1DialogueNodes`: 기존1..5 유지, End6보고대기/7수령완료/8진행중 추가.
- 바이너리 수정 전 두 에셋을 checkpoint에 복사·SHA 확인. 두 dirty package를 보고한 후 이 두 개만 저장.
- MainMap placed NPC가 새 기본값1/6/7/8을 상속하는 것을 확인. MainMap/기존 WBP 레이아웃 저장 없음.
- 실제 에디터 ExportQuestData 실행 → S1QUEST2, quest1/dialogue8/NPC1.
  새 서버 실행 파일의 validate 모드 통과 후 Debug Config 배치. 새 export format 없음.
- 배치 SHA256: `0F0D10A5AE40454086C0E145F59436A544F10ACCF08064C3BE1B750491C9B7C1`.

## 실제 검증·증거

증거 폴더:
`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/QuestStageC_20261019`.

- S1Editor Win64 Development 성공 (`S1EditorBuild.log`).
- S1 Win64 Development 성공 (`S1Build.log`), Debug GameServer 성공 (`ServerBuild.log`).
- 격리 DB 통합140 checks 통과 (`DBTests.log`): 정상/98+1+신규2/전체 rollback,
  동일 UUID/payload 충돌/새 UUID 중복/동시 지급, 다른 연결 readback, 재접속,
  사용 예약 Busy·환불 후 재시도, 이동/획득 경쟁, 지급 Item 삭제 후 원장 유지.
- Unreal 자동 검사33개: clean32/warning1/failed0/notRun0.
  `Saved/Automation/QuestStageC_20261019/index.json`, `UnrealAutomation.log`.
  warning1은 기존 Q GameplayCue 경고 포함 통과이며 새 보상 실패가 아니다.
- 순수 서버 정책/parser 회귀14+29 cases 통과(기존 B helper를 현재 소스로 재실행).
- MCP: PIE=false, NPC compile warnings-as-errors 호출 성공, parent/generated class 및 placed 설정 확인.
- 사용자가 격리 DB와 GameDB에 migration 실행 완료 보고. live 준비 상태는 아래 후속 기록을 따른다.
- 새 Debug GameServer PID27988 실행: 127.0.0.1:7777 LISTENING, DB 연결 및
  `Quest schema readiness[1] Reward readiness[1]` 확인(`ServerStartup.log`).
  Loot/포션/필드드랍 readiness1도 확인. 실제 로그인 capability 협상은 PIE 검증 전이다.
- GameDB 적용 전15테이블 dump 백업244914 bytes, 완료 표식·스키마 라우팅·SHA 확인.
  `GameDBBeforeQuestC/GameDB.sql`, SHA256 `199452EC73951CEA4A39FFE674D2D1EE19AB89A136B6F6188271B9B56C9F35A8`.
  계정 데이터 포함: 공개 저장소/메시지에 첨부하지 않는다. 이번 dump의 복원 시험은 미실행.

## 사용자 PIE 인수 완료 — 2026-10-19

사용자가 직전 안내한 전체 항목을 확인 완료했다고 보고했다:
각자 완료 보고 시 본인 포션만3개 증가, 반복 대화·재접속 시 추가 지급 없음,
인벤토리·퀵슬롯 수량 반영, 대화 종료 후 이동·공격·카메라 정상 복귀.
이는 사용자 실플레이 확인이며 자동 검사 결과와 구분한다.

독립 로그 확인: 서버와 에디터 양쪽에 같은 요청의 Decision1/Reason0/Inventory50/Revision5
보상 commit1건이 기록돼 있다. 해당 에디터 로그 검색에서 Error/Fatal/Ensure condition/
Blueprint Runtime Error/Network Failure 일치는 없었다. 정상 GameInstance requested disconnect도 확인했다.
이 캡처만으로 두 계정 각각의 지급·재접속 전체 과정을 독립 입증했다고 주장하지 않는다.
PIE 종료 시 RecastNavMesh를 찾지 못하는 CrowdManager 경고는 별도 잔여 경고다.

## 추가 검증이 필요한 항목

1. 공간 부족 후 공간 확보·재보고, 지연 결과/닫힌 대화에 대한 강제 장애 주입 PIE는 별도 미검증.
2. 최신 패키지 빌드/2계정 통합 검증, DummyClient 타깃 빌드는 미실행.
3. 이번 GameDB 백업의 복원 시험은 미실행. DB 자동 검사는 실제 렌더링 시험의 대체가 아니다.

신규 콘텐츠·보상량·경험치·골드·반복 퀘스트·전투 규칙 변경 없음.
