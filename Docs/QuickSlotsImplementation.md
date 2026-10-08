# 1–4 퀵슬롯 구현 — 2026-10-07

후속 인벤토리: WBP_S1Inventory/WBP_S1InventoryStackRow로 창·스택행 표시를 분리하고 기존 사용/등록 intent를 유지했다. InventoryWidgetBlueprint.md에 구현/검증과 현재 대기를 기록했다. 기존 퀵슬롯 애셋은 수정하지 않았다.

후속 UI 작업: 사용자 승인으로 퀵슬롯의 표시를 WBP_S1QuickSlots로 분리하는 작업을 진행했다. 인벤토리는 그대로이며 기존 입력/소비/저장 계약을 유지한다. 위젯 경로·디자인 편집 방법·이번 버전 검증 범위는 QuickSlotsWidgetBlueprint.md를 따른다. 아래 native Slate HUD 설명은 전환 전 구현 기록이다.

## 최신 사용자 인수 — 2026-10-07

포커스 수정본 재실행 후 사용자가 “PIE 사용 검증 테스트 완료 정상 작동 확인”을 보고했다. 직전 문제인 등록 후 숫자키 포션 사용 불가의 수정본 PIE 정상 작동을 사용자 인수 완료로 기록한다. 아래 수정 과정의 사용 인수 대기 문구는 이 확인으로 대체한다.

이번 보고에는 클라이언트 수, 각 NumPad 키, 재접속/앱 재시작 저장, 0개→재획득, UI·사망 차단, 양쪽 FX 및 패키지 2계정 등의 개별 결과가 열거되지 않았다. 해당 항목 전체를 추가 통과로 확대하지 않는다. 이 인수 기록 작업에서 새 빌드·PIE·실시간 로그 감사를 실행하지 않았으며, 근거는 사용자의 현재 수정본 플레이 확인이다.

## 후속 PIE 포커스 수정 — 2026-10-07

사용자 테스트에서 등록은 가능하나 1–4 사용이 불가능했다. 해당 PIE 로그에서 성공 snapshot(version7/count1), 피격 후HP90/100, 사용 요청 로그0을 확인했다. 이전 자동 검사는 액션 injection/저장·상태를 확인했지만 실제 PIE 포커스 게이트를 다루지 않아 이 결함을 놓쳤다.

원인: CanUseQuickSlot의 정확한 Slate 타입 `SViewport` 문자열 비교. UE5.8 별도 PIE 창은 UnrealEd/PlayLevel.cpp의 `SPIEViewport : SViewport`를 사용하므로 정상 포커스를 차단한다. 사용자 승인으로 해당 플레이어 LocalPlayer.ViewportClient.GetGameViewportWidget의 **실제 인스턴스**와 keyboard focus를 비교하도록 수정했다. 다른 PIE 창/텍스트 입력/모달/무포커스 차단을 유지한다. 기존 사망/커서/인벤토리/로딩/연결/공통 pending·unknown 조건은 유지한다.

변경: Loot/S1InventorySubsystem.cpp, 신규 Loot/S1QuickSlotInputGate.h, Loot/S1QuickSlotTests.cpp 및 이 문서. 키 입력 시 Quick-slot input 로그에 슬롯/게이트 통과/인벤토리·조회·pending·포커스 타입을 기록하여 향후 요청 미전송을 구분한다. 텍스트 입력 내용·계정 비밀은 기록하지 않는다. 서버/DB/proto/Config/Blueprint/맵 변경 없음.

checkpoints/QuickSlotFocusFix_20261007에 변경 전2소스 복사·SHA 검증. 수정본 S1/S1Editor 전체 빌드 exit0. 새 OwningViewportFocus 검사는 같은 파생 viewport 허용, 패키지 viewport 허용, 다른PIE/텍스트/모달/무포커스/무viewport 차단을 확인했다. 실제 UnrealEd 내부 SPIEViewport는 private이므로 runtime 모듈에 editor 의존성을 추가하지 않고 같은 파생 타입 조건의 합성 위젯으로 재현했다.

Saved/Automation/QuickSlotFocusFix_20261007/index.json 및 UnrealAutomation.log: **18 Success/실패0/미실행0**, 새 포커스 검사 오류0·경고0. 기존 Q GameplayCue 경고1 유지. 자동화 Fatal/Ensure/Blueprint Runtime Error/Network Failure/`: Error:` 검색0. 실제1인·2인 숫자키→사용 요청→서버 회복 인수는 수정본 재실행 후 대기이며 아직 해결 인수를 완료로 보고하지 않는다. 재패키징 결과는 아래 후속 증거를 따른다.

수정본 Package.log: Windows Development Build/Cook/Stage/Package/Archive BUILD SUCCESSFUL/AutomationTool exit0. 필수3맵 유지. archive Saved/QuickSlotFocusFixPackage_20261007/Windows. 수정본 에디터 재실행 완료. 이 패키지의 인증2계정 플레이는 미검증이다. GameServer는 수정하거나 재시작하지 않았다.

## 상태 / 승인 범위

Astra의 사용자 승인 최소안으로 클라이언트 구현을 진행했다. 하단4칸 HUD, 인벤토리 명시 등록/해제, 상단1–4/NumPad1–4 단발 입력, 서버 확정 합산 수량, 캐릭터별 로컬 등록 저장이다. 서버/DB/proto/신규아이템/쿨다운/사용모션/이동잠금/drag-and-drop/UI전면개편은 변경하지 않았다. 기존 다중Dusk와 A–D/Q 기준을 유지한다.

S1Editor 빌드·자동 검사·Windows 패키징 성공. 새 에디터 재실행 및 MCP 재연결 완료. 포커스 수정본 숫자키 포션 사용의 PIE 정상 작동은 사용자 인수 완료다. 명시되지 않은 1인·2인 세부 인수와 패키지 플레이는 별도로 남는다. 과거 포션 인수를 이번 퀵슬롯 세부 검증으로 승격하지 않는다.

## 조작 / 정책

G획득 → I → 포션 스택 행 선택 → `1 등록`~`4 등록` → 인벤토리 닫기 → 상단1–4 또는 NumPad1–4. 번호별 해제 버튼으로 비운다. 홀드 반복 없음. 같은 정의를 여러 칸에 등록 가능하며 공통 수량/처리중을 본다. 0개에도 등록 유지, 재획득 후 새 스택으로 자동 재활성.

HUD: 번호·기존 아이콘·서버확정 수량·처리중/미확정(I)/조회중/보유없음/사용대기. 비포커스/HitTestInvisible으로 클릭을 받지 않는다. owning local controller의 Main준비 이후 하나만 생성, 월드전환/세션 reset에서 제거.

퀵키는 사망(MOVE_None), 로딩, disconnect, pause, cursorUI, 인벤토리, 모달, 게임뷰포트 외 키보드포커스에서 차단한다. 인벤토리 사용 버튼은 기존 경로 유지. 사용 중/unknown에서는 다른 슬롯으로 새 ID 생성·재전송하지 않는다. unknown은 기존 인벤토리의 명시적인 같은 요청 재확인만 허용한다.

## 책임 / 저장

MyPlayer는 임시 Enhanced Input actions/context를 생성하고 Started에 바인딩한다. DefaultMappingContext의 기존 키는 덮어쓰지 않고 제외한다. controller변경/EndPlay에서 context정리. 실제 IMC_Default GetMappings를 읽은 자동 검사에서 8키 충돌 없음. UE5.8의 구 Mappings는 비어 있고 새 DefaultKeyMappings가 실제 목록이다.

InventorySubsystem은 정의ID 등록, 사용 시 최신 검증 inventory의 양수 스택 선택(slot_index→instanceID), 공통 게이트, UI수명/저장을 담당한다. 현재 정의20001만 지원. RequestUseItem→기존 C_ITEM_USE→서버소비/회복→기존 HP/FX표현을 재사용하며 UI는 HP/수량을 수정하지 않는다. 과거 응답은 기존 request/character/item 검사와 session reset으로 걸러진다.

저장: Saved/SaveGames/S1QuickSlots_<hash>.sav. namespace는 실제 endpoint(ip:port)+정상로그인 accountID+CharacterId. 버전/identity hash/4개 정의ID만 저장한다. HP/수량/instanceID/비밀번호/토큰은 저장하지 않는다. 다른 계정/캐릭터/endpoint는 분리, 다른PC동기화 없음. 같은PC 동일계정·캐릭터·endpoint는 같은 선호파일이며 동시 편집 마지막 저장 우선. 잘못된 클래스/버전/identity/길이는 빈등록, 미지원정의는 해당칸 초기화. 저장실패는 세션등록 유지 및 경고, 손상파일 임의삭제 없음.

AccountSubsystem에 정상 pending로그인 성공 accountID 조회/인증reset·실패 초기화, Network에 기존endpoint 읽기전용 조회만 추가했다. 인증우회나 전송변경 없음.

## 변경 파일 / 애셋

Source/S1의 기존 Loot/S1InventorySubsystem.h/.cpp, UI/S1InventoryWidget.h/.cpp, Game/S1MyPlayer.h/.cpp, Account/S1AccountSubsystem.h/.cpp, Network/S1NetworkSubsystem.h.

신규 Loot/S1QuickSlotState.h, S1QuickSlotSaveGame.h, S1QuickSlotTests.cpp, UI/S1QuickSlotWidget.h/.cpp.

문서: 이 문서/PotionUseImplementation/CurrentAssetRegistry. 새 바이너리·입력애셋·Blueprint·맵·Config 저장 없음. 기존 PotionPresentationSettings.Icon/native cook reference 재사용. 시작 MCP initialize/list_toolsets/IsPIERunning=false/LobbyMap/BP_MyPlayer의 IMC_Default를 확인했다.

## 실제 검사 / 증거

checkpoints/QuickSlots_20261007: 변경 전 기존9파일 복사 SHA256검증. S1EditorBuild.log 전체UHT/컴파일/링크 exit0. 첫 테스트빌드 TestName 멤버충돌은 TestSaveName으로 수정 후 성공, 실패로그도 보존.

UnrealAutomation.log + Saved/Automation/QuickSlots_20261007/index.json: **17 Success/실패0/미실행0**. 신규3개는 경고0/오류0. 기존 KnightChargePresentation의 GameplayCueNotifyPaths fallback 경고1. Fatal/Ensure/Blueprint Runtime Error/Network Failure/`: Error:` 검색0.

신규검사: 등록4칸/해제/외부스택거부/미지원정의, 서버다중스택 합계와 안정선택·소멸/재획득, 실제SaveGame write/load/delete(고유테스트파일만), endpoint/account/character 분리, 손상구조·외부identity초기화, 공통unknownID보존/다른캐릭터응답거부/reset, 실제EnhancedInput action injection20프레임홀드 Started1회·release/repress2회, 실제IMC_Default8키충돌없음.

NullRHI 합성·파일 검사이며 실제키보드·렌더링HUD·인증2계정·DB사용성공을 입증하지 않는다.

Package.log: 게임 Development 빌드/필수 Lobby·Loading·Main map cook/Stage/Package/Archive, BUILD SUCCESSFUL/AutomationTool exit0. archive Saved/QuickSlotsPackage_20261007/Windows. PackageMainMapSmoke.log: 실제 패키지 MainMap 로드·Quit/exit0. 서버가 종료돼 예상 연결실패 Error1이 발생했으며 인증/네트워크 통과로 보고하지 않는다. Fatal/Ensure/Blueprint Runtime Error 없음.

재실행 에디터 MCP에서 IMC_Default의 DefaultKeyMappings와 기존 Rair_Potion 아이콘 경로를 재확인했다. S1QuickSlotSaveGame native CDO는 해석되지만 SaveGame-only 필드는 ObjectTools editable-property 조회로 읽히지 않았다. 해당 필드 검증 근거는 실제 SaveGame roundtrip 자동 검사이며 MCP 조회 성공으로 대체하지 않는다.

## 사용자 인수 / 미검증

1인: 등록/해제4칸·아이콘/총수량, 상단/NumPad, 홀드/빠른교차입력, 소비/회복/FX, 0개등록유지→G재획득, 최대HP무소비, 인벤토리/텍스트/모달/사망/로딩차단, 재접속·앱재시작등록유지.

2인: 계정별HUD/저장분리, 소유자수량만감소·서버HP·양쪽FX1회, 기존A–D/Q/G/I/3Dusk회귀, 퇴장·재접속옛요청없음. 인증패키지2인·실제unknown/지연장애주입 별도. 기획변경 없음. 화면크기·배치는 사용자 시각인수 필요.
