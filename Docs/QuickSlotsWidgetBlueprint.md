# 퀵슬롯 Widget Blueprint 디자인 분리 — 2026-10-07

2026-10-07 inventory follow-up: user's designer changes replaced the initial QuickSlotBar layout. Required12 bindings remained valid. The older template-specific anchor assertion was replaced with designer-transform preservation in S1QuickSlotWidgetTests.cpp; full20 native tests then passed. No quick-slot binary or runtime presenter change was made. The older note about updating a fixed anchor expectation below is superseded by this designer-neutral test.

## 최신 사용자 인수 — 2026-10-07

사용자가 위젯 전환 수정본에 대해 “테스트 완료 다음 구현 제안”을 보고했다. 직전 요청의 퀵슬롯 Widget Blueprint 전환 사용자 테스트 완료로 기록한다. 아래 시각/PIE 인수 대기 기록보다 이 확인이 우선한다. 다만 클라이언트 수·등록/해제·양쪽FX·재접속·패키지 플레이 등 개별 결과는 이번 보고에 열거되지 않았으므로 모든 세부 항목을 추가 통과로 확대하지 않는다. 이번 기록 작업에서 새 빌드/PIE/실시간 로그 감사를 실행하지 않았다.

후속 추천은 기존 인벤토리의 Widget Blueprint 디자인 분리다. 새 인벤토리 기능이나 서버/DB/proto 변경을 섞지 않고 기존 스택 선택·사용·퀵슬롯 등록/해제·닫기 intent와 서버확정 표시를 유지한다. 현재는 제안이며 후속 구현을 시작하지 않았다.

## 범위 / 상태

사용자 승인: 퀵슬롯 Widget Blueprint부터 먼저 전환. 인벤토리 디자인 전환, 새 기능, 서버/DB/proto, 입력·소비·저장 규칙은 제외한다.

새 애셋 `/Game/S1/UI/WBP_S1QuickSlots`, 부모 `US1QuickSlotWidget`. 기존 passive C++ presenter에 디자이너 WidgetTree 표시를 연결하고, InventorySubsystem이 이 클래스를 생성하도록 했다. C++은 서버확정 모델의 아이콘·합산수량·상태·등록없음/비활성 표시만 갱신한다. 인벤토리는 기존 native Slate 상태다. 모델은 Widget Blueprint로 이전하지 않는다.

## 사용자가 편집할 위치

Content Browser → S1/UI → WBP_S1QuickSlots → Designer.

| 대상 | 편집 가능한 항목 |
|---|---|
| QuickSlotBar의 Canvas Slot | 하단 중앙 anchor/alignment, 위치, 자동 크기 |
| Slot1Size~Slot4Size | 슬롯 폭·높이 |
| Slot1Border~Slot4Border | 배경·색상·내부 패딩 |
| HorizontalBox / VerticalBox Slot | 슬롯 간격, 아이콘·텍스트 정렬/간격 |
| Key1~Key4 / Count1~Count4 / State1~State4 | 폰트·크기·색상·그림자·줄바꿈 |
| Icon1~Icon4 | 이미지 크기·tint·렌더링 스타일 |
| Class Defaults → Quick Slots / Designer | DesignerPreviewCount, 디자이너 샘플수량(기본99) |

필수 데이터 이름/타입: **Icon1~Icon4: Image, Count1~Count4: TextBlock, State1~State4: TextBlock**. BindWidget 계약이므로 이름과 타입을 유지한다. 삭제·이름변경은 컴파일 실패로 검출된다. 계층/위치/크기/장식은 변경 가능하다. Key1~Key4는 기본 키 번호 표시이며 실제 입력 바인딩은 변경하지 않는다. 수량/상태 Text를 정적으로 편집해도 플레이 시 서버 모델 값으로 갱신된다.

템플릿은 RootCanvas → QuickSlotBar → 각 SlotSize → Border → Contents → Key/Icon/Count/State. 하단 중앙 anchor(0.5,1), alignment(0.5,1), Y=-24, autoSize; 각100폭/간격3/패딩8/아이콘40. 기존 최소 디자인을 유지한 출발점이다. 최종 미감/가독성은 사용자 확인이 필요하다.

## 동작과 책임 경계

디자인 미리보기에서만 포션4칸/x99/사용가능 샘플을 표시한다. 실제 실행은 기존 Definitions/Quantity/Status를 표시하며 디자이너 샘플은 inventory/HP/저장 파일에 쓰지 않는다. SetBrushFromTexture(matchSize=false), Text만 갱신하고 폰트/색상/배경/레이아웃은 덮어쓰지 않는다. 비활성 아이콘은 render opacity0.4, 미등록 아이콘은 Hidden으로 공간을 유지한다.

기존 owning local HUD 하나, HitTestInvisible/비포커스, 외부 IOCP 권위, RequestUseItem/공통 pending·unknown 게이트, PIE 포커스 수정, 0개등록유지·재획득·캐릭터별로컬저장을 유지한다. WBP EventGraph에 패킷/HP/수량 변경이나 소비 로직을 추가하지 않는다. 정확한 native CDO class reference로 cook 대상을 확보하고 폴더 전체 AlwaysCook 설정은 추가하지 않는다. 애셋 누락 시 기존 native Slate fallback은 남아 있으나 정상 경로는 WBP다.

## 변경 목록 / 체크포인트

- Source/S1/UI/S1QuickSlotWidget.h/.cpp: UMG BindWidget12개, design preview, 모델→UMG 갱신, Blueprint tree Rebuild 경로.
- Source/S1/Loot/S1InventorySubsystem.h/.cpp: 정확한 WBP 기본 클래스 reference/생성만 변경.
- 신규 Source/S1/UI/S1QuickSlotWidgetTests.cpp.
- 신규 Content/S1/UI/WBP_S1QuickSlots.uasset만 생성/의도저장.
- 이 문서/QuickSlotsImplementation/CurrentAssetRegistry 갱신.

checkpoints/QuickSlotsUMG_20261007: 기존4소스 복사 SHA 일치. 신규애셋은 기존 파일 부재 확인 후 생성. 최초 template save 후 바이너리 복사 SHA 일치 보존. 기존 MainMap/BP_MyPlayer/IMC_Default dirty=false, 신규위젯만dirty=true 보고 후 선택저장하여dirty=false 확인. 다른애셋/Config/맵/서버/DB/proto/UE5_RPG 수정 없음.

MCP initialize/PIE=false/LobbyMap 확인, UMG CreateWidgetBlueprint/AddWidget, 각 Widget/Slot list_properties→get_properties→set_properties→readback을 직렬로 수행. 최초 저장의 parent=/Script/S1.S1QuickSlotWidget, CompileWidgetBlueprint=true. 새 native 선언 적용 후 재컴파일/재저장 및 아래 검증이 필요하다.

## 검증 기록

게임 Development/S1Editor 전체 UHT·컴파일·링크 exit0(S1Build.log/S1EditorBuild.log). 새 에디터로 재실행 후 CompileWidgetBlueprint=true 및 해당 WBP만 재저장. MCP readback: parent=/Script/S1.S1QuickSlotWidget, root=CanvasPanel, WidgetCount30/InheritedWidgetCount12, 최종 widget/MainMap/BP_MyPlayer/IMC_Default dirty=false.

UnrealAutomation.log + Saved/Automation/QuickSlotsUMG_20261007/index.json: **19 Success/실패0/미실행0**. 신규 WidgetBlueprintView 테스트 경고0·오류0. 기존 Q GameplayCueNotifyPaths fallback 경고1 유지. 자동화 Fatal/Ensure/Blueprint Runtime Error/Network Failure/`: Error:` 검색0. 실제 저장 WBP 생성·bound UMG 값 갱신과 디자인 속성 유지가 통과했다.

Package.log: Windows Development Build/Cook/Stage/Package/Archive BUILD SUCCESSFUL/AutomationTool exit0, 필수3맵. archive Saved/QuickSlotsUMGPackage_20261007/Windows. cook 프로세스에만 MCP18002를 지정해 열린 에디터8000과 분리했으며 프로젝트 config를 바꾸지 않았다.

PackageWidgetSmoke.log: 실제 패키지 LobbyMap 시작, obj list에서 /Game/S1/UI/WBP_S1QuickSlots.WBP_S1QuickSlots_C **1 Object** 로드 확인, Quit/프로세스exit0. 기존 Auth UI의 Non-Focusable InputMode:UIOnly Error1이 있었으며 새 위젯 로드 오류/Fatal/Ensure/Blueprint Runtime Error는 없었다. 인증/전투/HUD 렌더링 성공을 주장하는 검사가 아니다.

최종 asset SHA256 CC52E6A4C3014940920FC42B5FF6E54CE0BE910FE5E512D706C40230C8B555B2. Content Browser /Game/S1/UI에서 WBP_S1QuickSlots를 선택해 사용자 디자인 편집을 안내했다. 현재 버전 실제1인·2인PIE/시각인수는 별도 대기다.

신규 자동 검사 범위: 실제 저장 WBP generated class/부모/Subsystem 기본ref, 실제 CreateWidget/WidgetTree/12개 타입·바인딩, Count/State/미등록/0개·unknown 표시, designer font/imageSize 유지, 하단 anchor. NullRHI 합성 UI 검사는 실제 인증2인·렌더링 미감·키보드사용 인수를 대체하지 않는다.

하단 중앙 anchor 검사는 최초 템플릿의 기본 위치 검사다. 사용자가 의도적으로 기본 anchor/구조를 변경하면 이 템플릿 검사의 기대값도 새 디자인에 맞춰 갱신해야 하며, native presenter가 위치를 강제하는 규칙은 아니다.

사용자 PIE 인수: 하단4칸표시/폰트·크기, 등록/해제/1–4 사용, 처리중/0개/미확정표현, 각자HUD·상대화면FX, I닫기후숫자키/커서UI입력차단/재접속등록유지. UI전환 전 정상사용 인수는 이번 디자인 전환의 인수로 승격하지 않는다. 패키지2계정 플레이도 별도다.
