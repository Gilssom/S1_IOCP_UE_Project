# Main HUD 확장 — 레벨 / 스킬 아이콘 / 미니맵 / 화폐

## 최신 사용자 확인 — 2026-10-20

사용자가 이번 수정본의 전체 화면 확인 완료를 보고했다. 직전 요청한 레벨·공격 아이콘·상단
배치와 대화 중 HUD 숨김 등의 PIE 시각 확인을 사용자 완료로 기록한다.
정확한 PIE 인원수는 명시되지 않았으며, 에이전트가 재실행한 플레이나 로그 감사가 아니다.
아래의 PIE 미검증 표기는 이 사용자 확인 이전 구현 시점 기록이다.
현재 범위의 MainHUD/아이콘/Minimap/Currency 표현은 사용자 확인 완료.
실제 지형 지도 텍스처·화폐 서버 연동·쿨다운은 여전히 미구현이며 패키지 검증은 별도다.
이번 갱신은 문서만 수정하고 코드·Blueprint·맵·서버·DB는 변경하지 않았다.

## 승인 범위와 데이터 구분

사용자 요청: 기존 Aion2 참고 스타일로 레벨, 기본 공격/Q 연속 베기 아이콘,
우측 상단 미니맵 Widget, 좌측 상단 화폐 UI 추가. 화폐는 임시 표현 허용.
서버·DB·프로토콜·게임 규칙 변경 없음. 이전 HUD 배치 및 사용자 편집을 유지한다.

- 레벨: 기존 S_ENTER_GAME.player.character_info.level에서 AccountSubsystem이 캐시.
  reset/입장 실패는0, UI는 `Lv. —`. 디자인 preview1은 runtime 수치가 아니다.
  레벨업 패킷/경험치 시스템은 새로 만들지 않는다. 현재는 입장 snapshot 기준.
- 아이콘: built-in image_gen으로 독창적 halberd 기본 타격/두 곡선 Q 베기2종 제작.
  게임 원본 UI를 복사하지 않는다. 기존 PrimaryIcon/ChargeIcon에 연결 완료.
  입력·쿨다운·공격 단계·적중 정책은 변경하지 않는다.
- 미니맵: 별도 US1MinimapWidget + WBP_S1Minimap. 북쪽+X, 동쪽+Y의 north-up.
  MapTexture(선택), MapWorldOrigin, MapWorldHalfExtent를 Designer에서 설정.
  실제 지도 텍스처가 없으면 `지도 텍스처 미설정`과 중앙 방향 화살표/실제 좌표(m)를 표시한다.
  텍스처를 넣으면 위치를0..1 투영하고 가장자리4% 안쪽 제한. 적/NPC 위치 수집이나
  SceneCapture, RenderTarget, 지형 이미지 생성은 미구현. 동적 지도인 것처럼 보고하지 않는다.
- 화폐: 별도 WBP_S1Currency. G 문양/0/`임시 화폐` 텍스트, Blueprint에서 편집 가능.
  실제 지갑·보상·저장·구매와 연결되지 않는다. 이후 서버 계약을 별도 승인받아 연결한다.
- 미니맵/화폐는 MainHUD의 자식으로 넣어 기존 대화 중 전체 숨김 정책을 함께 따른다.
  HitTestInvisible로 기존 이동/공격/UI 입력을 가로채지 않는다.

## 정확한 변경 대상

- Source/S1/Account/S1AccountSubsystem.h/.cpp: entry 레벨 읽기 cache/getter, reset 정리.
- Source/S1/UI/S1MainHUDWidget.h/.cpp: optional LevelText, Tick 시 cache 읽기.
- Source/S1/UI/S1MinimapWidget.h/.cpp: 수동 설정 지도 presentation/좌표 투영.
- Source/S1/UI/S1MainHUDTests.cpp: 레벨 entry/reset/표시, 지도 좌표 방향/범위 검사,
  실제 HUD의 생성 icon/child binding 검사.
- WBP_S1MainHUD, 신규 WBP_S1Minimap/WBP_S1Currency,
  T_S1BasicAttackIcon/T_S1ChainSlashIcon (모두 /Game/S1/UI 또는 하위 HUD).
- ArtSource/UI/MainHUD/S1_BasicAttack_Icon_v1.png 및 S1_ChainSlash_Icon_v1.png,
  README/CurrentAssetRegistry/본 기록. PNG/Content 기존 source-only Git 제외 유지.

## 사전 조사 / 체크포인트

MCP 읽기 전용: PIE=false, MainHUD clean, parent S1MainHUDWidget,53개 위젯.
기존 TrackerCard의 anchor right/top, y320을 실제 조회하여 겹침을 피한다.
Tracker/Map/vendor binary는 변경·저장하지 않는다.
체크포인트 `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MainHUDExpand_20261020`.
현재 사용자 편집을 포함한 원본 HUD SHA256
C5327E1AB5C639FF23A9B5286BE2CDDE392A863E575F91E824EDD2D78C8673B2.
이전 단계 HUD SHA로 덮어쓰지 않는다.

## 진행 / 검증 상태

코드와 generated PNG 준비. 최초 S1 빌드에서 Widget::Slot 이름 가림 C4458 발견,
로컬 MarkerSlot으로 수정. 첫 실패 로그 보존. 이후 S1 Win64 Development 게임 빌드
Result Succeeded/exit0 확인. 이어서 아래와 같이 에디터 적용과 검사를 진행했다.
사용자 종료 확인 후 S1Editor Development 빌드 성공(exit0), 에디터 PID31464로 재실행.
ExpandHUD.ps1을 직렬 MCP로 실행하여 텍스처2개/위젯2개 생성 및 MainHUD 연결.
MCP compile/warnings-as-errors: Minimap parent S1MinimapWidget16 widgets,
Currency parent UserWidget6 widgets, MainHUD parent S1MainHUDWidget56 widgets.
신규/변경5개만 dirty임을 보고한 뒤 해당5개 선택 저장, 모두 clean 확인.
QuestTracker/MainMap clean 유지, 기존 사용자 편집 HUD는 덮어쓰지 않았다.

배치: MainHUD.HUDRoot 아래 MinimapWidget(right/top,-20,+12,252×304),
CurrencyWidget(left/top,+20,+12,204×44). QuestTracker 시작Y320은 그대로.
LevelText는 기존 ClassText 위치의 Y+19에 추가. 포션/스킬 슬롯 기존 위치 변경 안 함.
DesignerPreview.png 확인: 상단 두 영역/하단 icon/레벨 배치. 실제 PIE 증거가 아니다.
새 바이트 백업은 체크포인트 After에5개 보존. HUD SHA256
FF9B5C34CB0AFD725FB2E1842FBD6FA95BB5F9E4E8482715C9B2077DA4DFCE1E.

최초 새 자동 검사37개 중1개 실패(35clean+1기존 경고+1fail):
FS1MainHUDTest가 GameInstanceSubsystem인 Account를 transient package 소유로 생성해 Ensure.
게임 런타임 권위/패킷 오류가 아니라 테스트 harness의 Outer 오류이며,
NewObject<UGameInstance>를 Outer로 사용하도록 수정했다. FirstAutomationFailure에 보고서와
로그 보존. 제가 재실행한 에디터는 PIE=false/저장 완료 상태에서 Slate Close로 정상 종료,
수정 테스트 빌드·재검사를 진행했다. 실패를 통과로 기록하지 않는다.

최종 S1Editor/S1 Win64 Development 빌드 모두 Result Succeeded/exit0.
수정 후 실제 저장본을 로드한 자동 검사37개 통과(36clean+1기존 GameplayCue 경고,
failed0/notRun0). S1.UI.MainHUD / S1.UI.MinimapProjection 모두 Success.
레벨 entry/failure/reset/표시, icon brush/child, HP/기존 퀵슬롯, 쿨다운 미설정 유지,
좌표 중앙/+X북/+Y동/범위 제한/0extent 방어를 확인했다.
최종 UnrealAutomation.log의 Fatal error/Ensure condition/Blueprint Runtime Error/
LogBlueprint Error 검색은0건. 이전 실패 로그는 분리 보존.
리포트: Saved/Automation/MainHUDExpand_20261020/index.json.
실제1/2인 PIE 및 최신 Windows 패키지/Cook는 미실행. 이에 대한 성공을 주장하지 않는다.
최종 Editor PID20976 / 기존 GameServer PID14812 확인. MCP 재연결/PIE=false,
저장 MainHUD 재오픈 및 HUD/Minimap/Currency clean 확인. 텍스처2개 UI/EditorIcon/NoMipmaps
설정을 fresh-process readback으로 확인했다. 런타임 서버·DB는 변경하거나 재시작하지 않았다.

최종 수동 확인: 본인 레벨·스킬 아이콘 가독성, 상단 UI/퀘스트와 비중첩,
미니맵 화살표 방향/좌표·지도 미설정 안내, Alt/I/O/퀵슬롯 입력, 대화 중 숨김/종료 복귀.
