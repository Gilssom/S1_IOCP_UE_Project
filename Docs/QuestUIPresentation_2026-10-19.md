# 퀘스트 UI 표시·보상 행·O 목록창 (2026-10-19)

## 승인 요청과 구현 범위

사용자 요청: 대화/퀘스트 창 외의 UI 숨김, 수락 상세 하단 보상 목록,
완료 퀘스트의 화면 추적 제거, O키 진행/완료 목록창.
클라이언트 표현·입력만 변경한다. 서버·DB·프로토콜·보상량·수락/처치/지급 규칙은 변경하지 않는다.
완료는 기존 서버 `rewarded=true`(보상 수령 완료)로 정의한다.
목표3/3만 달성한 Ready는 보고할 때까지 진행 목록과 추적에 유지한다.

현재 서버/클라이언트는 quest1의 단일 권위 snapshot을 제공한다.
O 창도 실제 제공되는 quest1을 분류한다. 다수 퀘스트/일반 이력 서버 API를 구현했다고 간주하지 않는다.

## 구현 동작 / 조작

- 대화 모달 동안 퀵슬롯·화면 퀘스트 추적·NPC 이름·플레이어/몬스터 HP 이름표를 숨긴다.
  기존 NPC/G획득 힌트의 모달 숨김도 유지한다. 대화 창과 내부 퀘스트 상세는 유지한다.
- 각 소유자가 모달 상태를 확인해 표시하므로 async HP/재고 갱신이 숨김을 덮어쓰지 않는다.
  대화 종료 후 거리·전투·사망 등 기존 표시 정책으로 복귀한다. 다른 클라이언트의 UI는 바꾸지 않는다.
- `WBP_S1Dialogue`의 QuestDetailContents 하단 RewardFrame에 보상 아이콘·이름·수량 표시.
  수락 전에는 DT 정의를, 수락 후 보고/진행/완료 정보에는 서버가 보낸 저장 snapshot 보상을 사용한다.
  현재 포션20001의 기존 icon/DisplayName 설정을 재사용한다. 미등록 아이템은 ID로 표시하고 포션 아이콘을 재사용하지 않는다.
- 서버 보상 완료가 확인되면 추적 text를 비워 해당 퀘스트를 숨긴다. DB 기록은 삭제하지 않는다.
- O로 `WBP_S1QuestJournal`을 열고 O/Esc/닫기로 닫는다. 진행·완료 섹션은 별도 ScrollBox.
  새 snapshot 조회를 요청하며 캐시를 표시한다. 재접속은 기존 권위 조회로 복구한다.
- 대화·인벤토리와 O창의 동시 입력 소유를 막는다. 목록은 조회 전용이며 보상/수락 요청을 발생시키지 않는다.

## 변경 파일 / 에셋

Unreal root `E:/Git Fork/S1_UE5.8`:

- `Source/S1/Quest/S1QuestSubsystem.h/.cpp`: 추적 완료 제거, 모달 숨김, journal 수명·입력·조회·분류.
- `Source/S1/Quest/S1DialogueSubsystem.cpp`: 별도 보상 행에 정의/snapshot 값 전달.
- `Source/S1/UI/S1DialogueWidget.h/.cpp`: 선택적 RewardFrame/Icon/Text 바인딩·표현.
- 신규 `Source/S1/UI/S1QuestJournalWidget.h/.cpp`: 목록 text 바인딩과 O/Esc/닫기.
- `Source/S1/Game/S1MyPlayer.h/.cpp`: O 입력 및 인벤토리 동시 소유 방지.
- `Source/S1/Loot/S1InventorySubsystem.cpp`: 대화 중 퀵슬롯 Collapsed.
- `Source/S1/UI/S1NpcNameplateWidget.cpp`: 대화 중 거리 이름표 숨김.
- `Source/S1/UI/S1EntityHealthComponent.h/.cpp`: 대화 상태 변경 감지·숨김/복귀.
- `Source/S1/Quest/S1DialogueTests.cpp`, 신규 `S1QuestJournalTests.cpp`.
- 변경 `/Game/S1/UI/WBP_S1Dialogue`, 신규 `/Game/S1/UI/WBP_S1QuestJournal`.
  기존 QuestTracker WBP·NPC BP·DT·맵·폰트·벤더 에셋은 수정/저장하지 않는다.

## 디자인 편집 위치

- 보상: WBP_S1Dialogue → QuestDetailContents → RewardFrame/RewardContents/RewardItemRow.
  RewardFrame/RewardIcon/RewardText 바인딩 이름·타입 유지. 아이콘 크기는 RewardIconSize.
  기존 대화 제목/본문/배경/버튼 위치는 보존한다.
- O목록: WBP_S1QuestJournal → JournalFrame/JournalContents.
  ActiveQuestText/CompletedQuestText/CloseButton 이름·타입 유지. 색/크기/배치/폰트는 Designer에서 수정 가능.
- 기존 MaruBuri_Font Regular, 어두운 남색 배경과 금색 제목을 재사용한다. 새 이미지/외부 에셋 없음.

## 체크포인트·검증 상태

증거: `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/QuestUI_20261019`.
WBP_S1Dialogue 수정 전 clean 상태를 MCP로 확인, AssetsBefore 복사·원본과 SHA 일치 확인.
기존 미커밋 작업은 보존하며 이번 작업에 커밋/푸시는 포함하지 않는다.

검증 결과:

- S1Editor Win64 Development 최종 빌드 성공 (`S1EditorBuild.log`).
- S1 Win64 Development 최종 빌드 성공 (`S1Build.log`; 마지막 테스트 헤더 반영 포함).
- 최종 Unreal 자동 검사34개: clean33/warning1/failed0/notRun0.
  `Saved/Automation/QuestUI_20261019/index.json`, `UnrealAutomation.log`.
  warning1은 기존 Q GameplayCue 경고다. 프로세스 종료 코드뿐 아니라 JSON 실패 수까지 확인했다.
- 추가 검사: Active/Ready/Rewarded 분류, paid만 추적 제거, 실제 저장 WBP 로드·필수 binding,
  보상 아이콘·수량·일반 대사에서 보상 숨김, O/Esc·키 반복 억제,
  대화 중 로컬 HP/이름·NPC 이름 숨김, HP 갱신으로 노출되지 않음, 새 HP 패킷 없이 복귀.
- 첫 실행의 DialogueStageA 실패3 assertion은 테스트가 월드에 등록되지 않은 별도 대화 객체를
  사용했기 때문이다. 실제 WorldSubsystem을 사용하도록 fixture를 수정하고 전체 재실행 통과.
  초기 결과는 `AutomationInitialFixtureFailure.json`에 보존했다.
- Serial MCP: 두 WBP compile 성공, journal parent `/Script/S1.S1QuestJournalWidget`,
  GeneratedClass `WBP_S1QuestJournal_C`, 필수 세 binding 확인.
- 저장 전 두 dirty package 보고 후 둘만 선택 저장, 이후 두 package/Tracker/NPC/MainMap clean 확인.
  에디터 All Saved도 확인 후 최종 테스트 빌드를 위해 정상 종료했다. SaveAll 미사용.
- `AssetsAfter`에 저장 결과 복사. Dialogue SHA256
  `07A52BAC60E808C0D0527D36D3F9745421477094A21B6C6CFD2F099AECC1388B`, journal
  `D1ED96CE1B68C66A956B412282D0B52E4F986759B43125CCFA1B990EEF538DDC`.
- GameServer 기존 PID27988/7777 실행 유지. 이번 작업에서 서버/DB migration·서버 빌드 미실행.
- 최종 에디터 PID26608 재실행 후 MCP 연결, PIE=false/LobbyMap 확인.
  저장된 journal warnings-as-errors compile 호출 성공, 두 WBP clean.
  EditorFinalStartup.log에서 Error/Fatal/Ensure condition/Blueprint Runtime Error 검색 일치 없음.

실제1/2인 PIE 외관·입력·HUD 복귀와 최신 패키지는 아직 미검증이다.
사용자 확인: NPC 대화 동안 다른 HUD·이름이 사라지는지/종료 후 복귀,
수락 상세 하단 보상 아이콘·수량·레이아웃, 보상 수령 후 추적 제거와 O 완료 목록,
O/Esc/닫기 후 조작 복귀·재접속 완료 목록 유지·다른 클라이언트 UI 독립성.

문서 갱신: 본 기록 및 CurrentAssetRegistry.md. 콘텐츠 계약과 다른 규칙 변경 없음.
