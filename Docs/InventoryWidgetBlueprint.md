# 인벤토리 Widget Blueprint 디자인 분리 — 2026-10-07

## 최신 사용자 인수 및 MCP 연결 확인

2026-10-07 사용자가 이번 인벤토리의 PIE 화면·조작 검증 완료를 보고했다. 디자인 작업은 추후 진행한다. 이는 사용자 수동 인수이며 Codex가 직접 PIE를 실행하거나 세부 항목·인원수를 독립 검증한 결과는 아니다. 아래 사용자 인수 대기 문구는 이 보고 이전의 기록이다. 실제 packaged 2계정 플레이는 별도 미검증으로 유지한다.

같은 업데이트에서 읽기 전용 MCP initialize/initialized 및 list_toolsets 응답을 확인했다. IsPIERunning=false, get_current_level=/Game/Maps/LobbyMap. 앞의 에디터 재실행 직후 MCP 미연결 기록과 달리 현재 연결은 정상이다. 코드·설정·Blueprint·레벨 변경 또는 저장은 수행하지 않았다.

## 승인 범위와 책임

사용자 승인으로 기존 인벤토리의 표시와 버튼 intent만 UMG로 전환한다. 새 장비/아이템/정렬/drag-and-drop/DB/proto/서버 기능은 제외. 포션 스택 선택·사용·퀵슬롯 등록/해제·닫기와 기존 RequestUseItem의 공통 pending/unknown, 서버확정 수량·HP·FX, 캐릭터별 퀵슬롯 저장을 유지한다. 퀵슬롯 WBP 및 UE5_RPG/원본vendor/map/config는 수정하지 않는다.

창은 `/Game/S1/UI/WBP_S1Inventory`(US1InventoryWidget), 행은 `/Game/S1/UI/WBP_S1InventoryStackRow`(US1InventoryStackWidget). 창의 StackEntryClass는 행 WBP를 참조하며 네이티브 InventorySubsystem의 정확한 class ref가 창을 cook/생성한다. 디자이너의 geometry/font/brush는 코드가 덮어쓰지 않는다. C++은 검증된 스택 목록/합계/선택/상태와 버튼 허용 여부만 전달한다.

## 디자인 편집

Content Browser → S1/UI → 해당 WBP → Designer. 창 InventoryPanel/Background/Contents, StackListSize/ScrollBox, RegistrationRow를 편집해 배치·크기·배경·폰트·간격을 수정한다. 최초 템플릿은 중앙460폭, dark panel, 200높이 스크롤 목록. 행 SelectButton/RowContents/RowIcon/EntryText에서 각 스택 행의 스타일을 조절한다.

창 필수 이름/타입(BindWidget): PotionIcon(Image), QuantityLabel/StatusLabel/SelectionLabel/UseLabel(TextBlock), StackEntryList(ScrollBox), UseButton/CloseButton/Register1–4/Clear1–4(Button). 행 필수: SelectButton(Button), EntryText(TextBlock). 이름/타입을 유지하며 계층과 장식은 변경 가능하다. 창 Class Defaults→Inventory/Designer→StackEntryClass는 WBP_S1InventoryStackRow_C로 유지한다. 숫자키 바인딩을 UI 텍스트 변경으로 바꾸는 기능은 없다.

디자인 미리보기 샘플은 스택2/5개 합계7, 선택1이며 실제 서버/HP/저장에 쓰지 않는다. design-time 버튼 handler는 intent를 보내지 않는다. 실제 게임은 기존 snapshot 모델을 표시한다.

## 조작/상태

I열기 → 스택 행 클릭 → 선택 표시 → 포션 사용 또는 1–4 등록. 해제 버튼은 선택 없이 동작한다. I/Esc/닫기로 종료. PreviewKeyDown에서 I/Esc를 먼저 처리해 자식 focus에도 닫는다. 기존 GameAndUI/cursor 표시, 게임입력·퀵키 차단, 닫기 후 GameOnly 복귀는 유지한다.

선택 없는 일반 사용/등록은 비활성. 처리중 사용/등록은 기존 gate대로 비활성. unknown에서는 행선택·등록을 차단하고 원래 요청 재확인 버튼만 유지한다. old instance ID가 사라지면 선택을 비운다. 같은 ID의 수량·선택 변경은 행을 재생성하지 않아 scroll/행 인스턴스를 유지한다. 목록 ID가 바뀌거나 닫히면 행 delegate를 해제해 제거된 행의 intent가 전달되지 않게 한다. 창 닫기에서도 close/use/register/clear delegate를 모두 해제한다. UI가 packet/HP/수량을 직접 변경하지 않는다.

## 변경 파일 / 체크포인트

기존 Source/S1/UI/S1InventoryWidget.h/.cpp, Loot/S1InventorySubsystem.h/.cpp. 신규 UI/S1InventoryStackWidget.h/.cpp, UI/S1InventoryWidgetTests.cpp. 새 바이너리2개 및 이 문서/CurrentAssetRegistry/QuickSlotsImplementation 갱신.

checkpoints/InventoryUMG_20261007: 기존4소스 copy/SHA 일치, 새2애셋 기존파일 부재 확인. main 초기 template copy/SHA 보존. 초기 MCP PIE=false/LobbyMap/quick-slot clean. 모든 Widget/Slot에 list_properties→get_properties→set_properties→readback 직렬 적용. main37widgets/초기compile=true, main dirty=true/기존quick-slot·MainMap·BP_MyPlayer=false 보고 후 main만 선택저장→dirty=false. row는 새 native 클래스 빌드 후 생성/별도 선택저장한다.

## 검증 진행 기록

최종 후속(사용자 에디터 종료 후): S1EditorBuild.log 최종링크 exit0, UnrealAutomation.log + Saved/Automation/InventoryUMG_20261007/index.json **20 Success/실패0/미실행0**. Inventory.WidgetBlueprintIntents와 QuickSlots.WidgetBlueprintView 모두 경고0·오류0. 기존 Q GameplayCueNotifyPaths fallback 경고1 유지. 자동화 Fatal/Ensure/Blueprint Runtime Error/Network Failure/`: Error:` 검색0. 앞의 첫 자동화1fail은 수정/재검증으로 해소됐다. source 변경에 UI/S1QuickSlotWidgetTests.cpp(디자인 고정가정 제거)만 추가되며 사용자 퀵슬롯 바이너리는 변경하지 않았다.

최종 Package.log: 게임 Development 최신빌드·필수3맵 Windows Build/Cook/Stage/Package/Archive BUILD SUCCESSFUL/AutomationTool exit0. Zen 준비 대기는 재시도 후 해소됐다. 같은 archive의 최종 PackageInventorySmoke.log에서 창/행 class 각1개 로드·Quit/exit0 재확인. smoke의 기존 Auth 비포커스 Error는 별도 유지하며 실제 인증/렌더링 인수는 아니다.

에디터 재실행 PID12160 확인. 해당 재실행에서는 MCP 자동시작=false라8000 리스너가 없었으며 initialize 연결거부로 최종 ContentBrowser 선택/dirty 재조회는 수행하지 못했다. 앞의 저장 후 clean 증거를 최신 재조회 성공으로 대체하지 않는다. 프로젝트 MCP/config는 변경하지 않았고 사용자에게 Output Log의 ModelContextProtocol.StartServer 실행을 안내했다. 사용자 수정 quick-slot SHA256 A71477C0337150334636454D230A697044D7D02BC78C52A682DC2D4BEC9D9E2E가 작업 중/최종 동일함을 확인했다.

후속: 에디터 종료 후 S1Editor 전체빌드/링크 exit0. 새 에디터에서 row를 US1InventoryStackWidget으로 reparent/compile=true, main StackEntryClass를 WBP_S1InventoryStackRow_C로 설정/readback/compile=true. 새두애셋dirty=true/기존quick-slot·MainMap=false 보고 후 두애셋만 선택저장→clean. warnings-as-errors compile 두개 완료. main37widgets/16 nativebindings, row4widgets/2 nativebindings 확인. 아래 임시부모·연결대기 기록은 진행 과정이며 현재 연결은 완료됐다.

첫 자동화:20개 중19 Success(기존 Q GameplayCue 경고1 포함)/1fail. 신규 S1.Inventory.WidgetBlueprintIntents는 경고0·오류0으로 성공했다. 실패는 기존 QuickSlots.WidgetBlueprintView의 QuickSlotBar/CanvasSlot 고정가정이다. 읽기전용 MCP에서 사용자 퀵슬롯이 WrapBox/Canvas 중심 디자인으로 변경됐고 필수12개 binding은 모두 유지됨을 확인했다. 사용자 quick-slot 파일은 수정하지 않았다. 기존 검사를 컨테이너 이름/중앙앵커가 아닌 모델갱신의 designer transform 보존 검사로 수정하고 이전 소스copy/SHA와 첫 로그/report를 보존했다. 이 변경 적용·전체재검증은 에디터 종료 후 진행하며 실행 전 통과로 기록하지 않는다.

Package.log: Windows Development Build/Cook/Stage/Package/Archive BUILD SUCCESSFUL/AutomationTool exit0, 필수3맵 유지. archive Saved/InventoryUMGPackage_20261007/Windows. PackageInventorySmoke.log 실제 패키지 창/행 generatedclass 각1개 로드·Quit/exit0. 기존 Auth Non-Focusable InputMode:UIOnly Error1은 남으며 인증/HUD 렌더링 또는 실제2계정 사용의 통과를 뜻하지 않는다.

첫 게임 빌드에서 UWidget::Slot과 RegisterSelected/ClearSlot 매개변수 Slot의 C4458 이름충돌이 발생했다. SlotIndex로 수정하고 실패로그를 보존했다. 이후 빌드/바인딩·컴파일/자동화/패키징 결과는 실행 완료 후 갱신한다.

수정 후 S1 게임 Development 전체 빌드/링크 exit0(S1Build.log). 현재 에디터 PID7096이 실행 중이라 새 에디터 DLL 적용은 사용자 종료 대기다. reflected class 추가이므로 Live Coding으로 대체하지 않는다. 에디터 빌드/최신 자동화/패키징/PIE는 미실행이다.

대기 중 row 디자인은 임시 UserWidget 부모로 생성하고 compile=true/dirty=true 보고 후 row만 선택저장→clean, 바이너리 copy/SHA 검증 보존했다. 새 에디터 DLL 적용 후 US1InventoryStackWidget으로 reparent/compile하고 창 StackEntryClass를 연결해야 한다. 현재 두 애셋은 저장된 디자인 프로토타입이며, 최종 런타임 연결 완료로 보고하지 않는다. row를 native 빌드 후 생성한다는 앞 문구는 이 후속 프로토타입 준비 기록으로 보완한다.

새 자동 검사 범위: 실제 창/행 class/부모ref 및16개 창 binding, 실제 row 클릭 delegate/선택/사용/등록·해제intent, duplicate native construction 단일콜백, font/imageSize 유지, 동일ID행 유지/수량갱신, processing/unknown gate, stale row 제거·외부ID 무시, selection 제거, 원래 요청 재확인intent, close/I/Esc. NullRHI 합성 delegate 검사는 물리마우스·인증2인/DB/렌더링 인수를 대체하지 않는다.

사용자 인수 대기: 1인·2인 I열기/스크롤/선택/등록·해제/포션사용·수량·HP/양쪽FX, I/Esc/닫기와 숫자키복귀, UI 클릭 중 공격없음, 각자UI/재접속. 기존 퀵슬롯 전환 인수를 이번 창의 인수로 승격하지 않는다. 실제 packaged2계정 플레이도 별도다.
