# G 획득 안내 Widget Blueprint 분리 — 2026-10-14

최신 인수: 사용자가 새 획득 UI 작업까지 '여기까지 테스트 완료'로 보고.
사용자 플레이 검증 완료로 기록하며 agent-controlled PIE 결과와 구분한다.
키캡 PNG는 별도 디자인 후보로 제공했을 뿐, Unreal import/적용은 확인하지
않았다. 실제 packaged2client gameplay와 강제 장애 주입은 여전히 미검증.

## 범위와 동작

사용자가 UTC 수정 이후 공용 드랍2PIE 표시/타인G획득/양쪽제거/30초소멸
전체 검증 완료를 보고했다. 해당 서버 판정과 정책은 유지하고 UI만 분리한다.
서버/DB/프로토콜 변경 없음. 직접 입력 버튼, overlap 자동획득 없음.

기존 InventorySubsystem의 거리150cm/높이100cm/벽가림/근접 후보1개 선택과
WorldEntity 로컬 표시 경로 유지. 사적 몬스터 드랍은 소유자에게만 안내,
플레이어 공용 드랍은 인증된 누구에게나 안내. UI는 획득 권한을 결정하지
않으며 최종 검증은 외부 서버다. Pending 중 표시를 유지하고 서버 제거
또는 월드 teardown 시 안내도 제거한다. UI는 focus/click을 받지 않는다.

## 수정 파일·새 애셋

- Source/S1/UI/S1LootPickupWidget.h/.cpp: 수동적인 표시 클래스.
- Source/S1/UI/S1LootPickupWidgetTests.cpp: 표시·바인딩·입력 비간섭 검사.
- Source/S1/Loot/S1LootActor.h/.cpp: TextRender를 Screen WidgetComponent로
  교체. 기존 카메라 방향 Tick 불필요해 제거. 각 PIE 월드 로컬 플레이어에
  연결하고 count/pending만 넘긴다. 기존 mesh/Niagara/서버제거 로직 유지.
- Source/S1/Loot/S1PotionPresentationSettings.h: PickupHintHeight(70cm 기본),
  PickupHintScreenOffset(0,0 기본)를 기존 포션 표현 설정에 추가.
- /Game/S1/UI/WBP_S1LootPickupPrompt: 부모S1LootPickupWidget,8widgets,
  KeyText/ItemText/StatusText3bindings. SizeBox260×58, 남색 배경/금색 G.
  새 외부 이미지/에셋 복사 없음. 정확한 native CDO 클래스 참조로 cook 연결.
  최초 미생성 상태에서는 package 존재 확인 후만 로드하며 fresh runtime/
  cooker에서 저장된 class를 참조한다. 별도 folder AlwaysCook 추가 없음.
- Docs/CurrentAssetRegistry.md,PlayerFieldDropImplementation.md와 본 문서.

## 디자인·튜닝 위치

Content Browser /Game/S1/UI/WBP_S1LootPickupPrompt Designer:
PromptRoot 크기, PromptFrame 배경/여백, KeyBadge 배경,
KeyText/ItemText/StatusText 폰트·색·배치. 바인딩 이름과 네이티브 부모 유지.
문자 내용은 runtime에서 갱신하지만 스타일과 geometry를 덮어쓰지 않는다.
높이/화면 오프셋: Project Settings -> S1 Potion Presentation -> Pickup UI.
획득 거리/TTL/소유권은 이 디자인 설정으로 변경되지 않는다.

## 체크포인트·실행 증거

Documents/Unreal IOCP Server/checkpoints/LootPickupUI_20261014:
SourceBefore 원본 보관. 기존 애셋 수정 없음/새 파일 충돌 확인 후 생성.
MCP readonly PIE=false, 기존 UI/맵/자동로드Terra clean 확인.
새 WBP만 dirty=true 보고 후 명시적인1asset save, dirty=false 확인.
저장 파일/체크포인트 SHA256 모두
2D0675E1E5F329EA299D68D88730859FA3BD24809E608B8293303D4EABB545FD.
MCP warnings_as_errors compile 성공, 부모/8widgets/3bindings 확인.
작업용 headless authoring editor24916만 새 asset 저장 후 종료했다.
S1Editor build 성공; 테스트 소스 재수집-gather build 성공.
UnrealAutomation.log/index.json:26개 모두 통과(25success+기존Q경고1),
새 S1.Loot.PickupWidget 바인딩/수량/pending/focus/click 검사 포함. 실패0.
Windows Development Package.log BUILD SUCCESSFUL/ExitCode0, 게임 타깃 빌드와
Lobby/Loading/Main cook 포함. PackagePickupSmoke.log 실행exit0, archived exe에서
WBP_S1LootPickupPrompt_C 로드 확인. 기존 Auth InputModeUIOnly Non-Focusable
오류1은 별도 기록(이번 UI의 오류 없음 주장과 혼동하지 않음).
이후 새 UI 플레이 테스트 완료를 사용자 보고로 확인. Packaged gameplay는 미검증.
GUI 에디터 PID33816 재실행, MCP 정상/PIEfalse; 새 위젯 reload 부모/8widgets/
3bindings 및 clean 재확인. 사용자 편집을 위해 이 asset editor만 열어둠.
GameServer18680은 유지한다. 이번 단계 기획 정책 변경/추가 결정 사항 없음.
UI 변경을 공용 드랍의 이전 사용자 플레이 검증으로 대신하지 않는다.

## 사용자 확인 필요

1-/2PIE에서 근처에 가면 선택된 드랍 하나에 G/이름/수량 표시,
거리 이탈 숨김, 몬스터 비소유자 숨김, 공용 드랍 타인 안내,
G획득 pending/제거/30초제거와 UI 위치·크기·가독성 확인.
I 인벤토리/퀵슬롯/전투 입력을 가리지 않는지 확인.
실제 packaged2client gameplay, DPI/화면가장자리 가독성도 아직 미검증.
