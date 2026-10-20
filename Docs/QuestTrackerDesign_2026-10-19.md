# 퀘스트 진행도 디자인 — 2026-10-19

사용자가 제공한 아이온2 화면의 정보 계층을 참고한 S1용 UMG 디자인.
원본 이미지/게임 로고를 복사하지 않고 기존 마루부리 폰트와 UMG 패널·Text로 제작한다.
새 raster 이미지나 외부 에셋은 추가하지 않는다.

## 구성 / 편집 위치

`/Game/S1/UI/WBP_S1QuestTracker`:

- CanvasRoot → TrackerCard: 화면 우측 상단,340×142,오른쪽28/상단170,검은 남색28% 배경.
- TrackerHeader: 작은 마름모 느낌 표식, 금색 TrackerTitleText18pt, 작은 [O] 힌트.
- TrackerRule: 얇은 반투명 금색 구분선.
- TrackerObjectiveText: 들여쓴 밝은 회백색14pt 목표/수치.
- TrackerStatusText: 청회색12pt 안내. 안내가 비어 있으면 Collapsed.
- MaruBuri_Font Regular, 검정1px 그림자 사용. Designer에서 각 색/크기/간격/배경/앵커 조절 가능.

현재 실제 데이터가 없는 레벨·거리·여러 퀘스트 카테고리는 임의 표시하지 않는다.
기존 숨김/완료/수령/서버 조회 규칙과 O키 기능은 바꾸지 않는다.
제목/목표/안내는 기존 TrackerText의 첫 줄/둘째 줄/이후 줄에서 분리한다.
`TrackerTitleText`, `TrackerObjectiveText`, `TrackerStatusText` 이름·TextBlock 타입을 유지한다.
기존 QuestText 바인딩도 남겨 이전 단일-text 에셋과 native fallback을 지원한다.

## 변경 범위

- Source/S1/UI/S1QuestTrackerWidget.cpp: optional authored text 분리 표시.
- 신규 Source/S1/Quest/S1QuestTrackerDesignTests.cpp.
- Content/S1/UI/WBP_S1QuestTracker.uasset.
- 본 기록 및 CurrentAssetRegistry.md.

서버·DB·패킷·다른 UI·맵·폰트 파일·UE5_RPG 변경 없음.
기존 미커밋 파일 보존, 커밋/푸시 미요청.

## 체크포인트 / 검증

증거: `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/QuestTrackerDesign_20261019`.
수정 전 MCP PIE=false, tracker clean 및 두-widget 기존 tree 확인.
AssetsBefore/WBP_S1QuestTracker.uasset 복사와 원본 SHA 일치 확인, 기존 cpp도 보존.
S1Editor/S1 Win64 Development 빌드 성공.
Serial MCP로 기존 tracker에만 계층을 추가, compile 성공 후 dirty tracker1개를 보고하고 선택 저장.
저장 후 Tracker/Dialogue/Journal/MainMap clean 확인. SaveAll/다른 에셋 저장 없음.
최종 자동 검사35개: clean34/warning1/failed0/notRun0. warning1은 기존 Q GameplayCue 경고.
실제 저장된 WBP 로드, 제목/목표/상태 분리, 빈 상태 Collapsed, 초기화 후 제목 잔상 없음,
native 단일-text fallback 호환을 검사했다. JSON과 UnrealAutomation.log를 증거 폴더에 연결해 확인했다.
리포트: Saved/Automation/QuestTrackerDesign_20261019/index.json.
DesignerPreview.png는 디자이너 캡처이며 PIE 화면이 아니다. Designer에서는 숨김 위젯 가이드가
함께 표시될 수 있다. 실제 표시에서는 legacy QuestText를 Collapsed로 유지한다.
저장된 결과는 WBP_S1QuestTracker.after.uasset로 별도 보존했다.
이후 사용자가 ‘체크 완료’로 디자인 확인 완료를 보고했다. 사용자 실플레이 확인이며
독립적인1/2인 전체 로그 감사나 최신 패키지 검증으로 간주하지 않는다.
