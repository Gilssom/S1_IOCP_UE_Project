# 퀘스트 목록 탭·선택 상세 — 2026-10-19

## 사용자 요청 / 동작

- O 목록창 왼쪽 위에 ‘진행 중 / 완료’ 버튼.
- 선택한 탭에 맞는 퀘스트를 왼쪽 ScrollBox 목록의 버튼 행으로 표시.
- 목록의 퀘스트 클릭 시 오른쪽에 제목·설명·목표 진행·상태·보상 아이콘/이름/수량.
- 탭 변경 또는 목록에서 사라짐/동기화 미준비 시 선택 해제와 상세 숨김.
  단순 진행도 snapshot 갱신은 선택을 유지하며 오른쪽 내용을 갱신한다.
- 빈 목록/선택 전 안내 제공, 선택 탭과 선택 행은 색상으로 구분.
- 상세는 기존 수락/거절 창과 같은 남색·금색/마루부리 스타일과 정보 계층을 참고한 조회 전용 영역.
  수락·포기·보상 요청 버튼을 목록창에 새로 추가하지 않는다.
- O/Esc/닫기 및 기존 입력 소유/세션 정리 유지.

현재 서버는 quest1의 단일 snapshot을 제공하므로 실제 목록 행도 그1종만 존재한다.
일반 다중 퀘스트 조회·전체 이력 API가 구현됐다고 간주하지 않는다.
완료 판정은 기존 rewarded=true(수령 완료). Ready는 진행 중 탭에 남긴다.
설명·목표 수치·보상은 서버가 보낸 수락 snapshot을 사용한다. 최신 DT로 덮어쓰지 않는다.

## 변경 범위 / 편집

Unreal root `E:/Git Fork/S1_UE5.8`:

- `Source/S1/UI/S1QuestJournalWidget.h/.cpp`: plain 표시 view, 탭/선택/상세/빈 목록 및 보상 표시.
- `Source/S1/Quest/S1QuestSubsystem.h/.cpp`: 소유 snapshot을 표시 view로 전달. UI는 패킷을 파싱하지 않는다.
- `Source/S1/Quest/S1QuestJournalTests.cpp`: 목록·선택·상세·갱신·동기화·완료 분류 검사.
- `/Game/S1/UI/WBP_S1QuestJournal`: 좌측 탭/목록, 우측 상세/보상, 기존 닫기 유지.
- 본 기록, CurrentAssetRegistry.md 및 앞선 진행도 디자인 사용자 인수 상태 문서.

Widget Designer:
JournalFrame → JournalLayout → JournalBody.
왼쪽 QuestListWidth/QuestListColumn/QuestTabs/QuestListScroll,
오른쪽 QuestDetailArea/DetailFrame/QuestDetailScroll/QuestDetailContents.
ActiveTabButton, CompletedTabButton, QuestListButton/QuestListTitle, EmptyListText,
SelectionHintText, DetailFrame, QuestTitleText/QuestDetailText/QuestStateText,
RewardFrame/RewardIcon/RewardText, CloseButton의 이름과 타입을 유지한다.
색·크기·간격·폰트·배경·목록 폭은 위젯에서 변경 가능.

서버·DB·프로토콜·다른 UI·맵·폰트·외부 에셋 변경 없음.
기존 미커밋 변경 보존, 커밋/푸시 미요청.

## 검증 / 증거

`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/QuestJournalTabs_20261019`.
시작 시 에디터가 종료돼 MCP 연결 불가 확인. 재실행 후 읽기 전용 tree/상태를 확인하고 에셋을 수정한다.
Before에 기존 WBP 복사·SHA 일치 및 변경할5개 소스 보존.
S1Editor/S1 Win64 Development 빌드 성공.
Serial MCP 재실행 확인: PIE=false, 이전 WBP tree/clean 확인 후 수정.
기존 닫기 버튼을 먼저 이동해 보존하고 이전 본문만 재구성했다. 이름·필수 binding 유지.
dirty WBP_S1QuestJournal1개를 보고 후 이 패키지만 선택 저장. Dialogue/Tracker/MainMap clean 확인.
최종 Widget compile와 warnings-as-errors 호출 성공, 저장 결과 SHA256
`8D3CC223CDD7DB4569606B9F1D5B30FD50284BFA4EE7A1847E1DF235027728B0`.
WBP_S1QuestJournal.after.uasset로 결과 보존. SaveAll 미사용.

최종 Unreal 자동 검사35개: clean34/warning1/failed0/notRun0.
리포트 Saved/Automation/QuestJournalTabs_20261019/index.json 및 UnrealAutomation.log.
warning1은 기존 Q GameplayCue 경고. 프로세스 exit와 JSON.failed/notRun을 함께 확인했다.
확장된 JournalPolicy 검사: 진행/완료 목록 필터, 선택 전 상세 숨김, 선택 후 제목/저장 설명/보상,
동일 snapshot에서 선택 유지, 탭 변경 시 해제, 완료 상태 전환, 동기화 미준비 선택 방지,
기존 O/Esc 반복 키 처리와 닫기.
에디터에서 구성 중 새 필수 widget이 아직 없는 중간 단계의 자동 컴파일 binding 오류는
위젯 계층 완성 후 최종 컴파일로 해소했다. 이 중간 로그를 새 runtime 오류로 간주하지 않는다.
GameServer PID27988/7777은 기존 실행 상태 유지했으며 서버 코드/DB는 변경하지 않았다.

실제1/2인 PIE 탭 클릭/상세 외관·입력 복귀와 최신 패키지는 미검증.
