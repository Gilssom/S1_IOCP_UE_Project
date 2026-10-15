# 인증 UI 포커스 오류 수정 — 단계1,2026-10-15 KST

후속 단계 상태: 이후 승인받아 최신 서버 및 Dusk cook 누락 수정까지 포함한
Windows Development 패키지를 생성했고, 사용자 실제 두 계정 통합 검증을
완료했다. 최신 증거와 잔여 제한은 PackagedIntegration_2026-10-15.md 및
AstraCompletionHandoff_2026-10-15.md를 따른다. 아래 단계1 당시의
미실행 기록을 최신 전체 상태로 오인하지 않는다.

최신 상태: 사용자2인 실제 PIE 인수 통과. 로그인 → 캐릭터 선택·생성 화면
전환 → Main 진입 후 이동·공격·카메라·커서 복귀 테스트 성공을 보고받았다.
아래 에이전트 검증 당시의 미검증 목록과 구분하며, 이 사용자 보고로
해당 플레이 항목을 완료 처리한다. 사용자 실행 이후 로그를 새로 조사한
것은 아니므로 새 로그 무오류까지 독립 확인했다고 주장하지 않는다.
실제 회원가입/캐릭터 신규 DB생성 성공, 새 패키지2계정 검증, 장애 주입은
이 보고만으로 완료 처리하지 않는다. 다음 단계는 별도 승인 후 진행.

## 조사 근거와 수정

기존 GameInstance ShowAuthUI/ShowCharacterSelectUI는 FInputModeUIOnly에
UUserWidget::TakeWidget() 루트 SObjectWidget을 전달했다. 두 네이티브
위젯은 focusable 루트를 지정하지 않아 엔진 SetFocusAndLocking의
SupportsKeyboardFocus 검사에서 기존 Non-Focusable 오류가 발생했다.
실제 입력 컨트롤SEditableTextBox는 해당 검사를 지원한다. 경고 억제나
전체 UI 루트 focusable 강제 대신 실제 활성 컨트롤을 지정한다.

변경파일:
- Source/S1/S1AuthWidget.h/.cpp: GetInitialFocusTarget 로그인/회원가입
  아이디 입력란. 패널 버튼 FReply.SetUserFocus로 해당 Slate user 이동.
- Source/S1/S1CharacterSelectWidget.h/.cpp: 목록에서 활성PlayButton,
  없으면OpenCreateButton; 생성 패널CharacterNameInput. 비활성 요청중
  버튼은 대상 제외. 리스트 재생성에도 수명이 안정적인 버튼 사용.
- Source/S1/S1GameInstance.cpp: 기존 UIOnly/게임모드·커서 정책 유지하고
  위젯 getter의 실제 대상만 전달. HideAuthUI/HideCharacterSelectUI의
  GameOnly 복귀는 변경하지 않음.
- Source/S1/UI/S1AuthFocusTests.cpp: 실제 Slate targetfocus지원, 비밀번호
  mask, 로그인/가입·선택/생성3회전환, 요청중차단, hiddenasync guard 검사.

실패 응답·가입/생성 완료시 표시 중인 위젯에만 owningLocalPlayer의
SlateUser 포커스 복구. 숨겨진/제거된 위젯이나 타PIE user에 전역 keyboard
focus를 강제로 주지 않음. 타이머/지연 raw widget 포인터 없음.
기존델리게이트 NativeConstruct/AddUnique·NativeDestruct/Remove 유지.
로그인 UI 권한검사/LoginFromAuthUI/비밀번호/패킷/DB/LevelFlow 변경 없음.
Tab 및 Enter를 가로채는 새 key handler나 auto-submit 추가 없음:
입력란의 기본commit과 버튼의 기본키동작을 유지한다.

## 작업 상태 / 증거

작업 전S1 clean HEAD1d82ea1, MCP정상/PIEfalse/서버36624·에디터29192.
사용자 에디터 종료 후 소스5파일 체크포인트 SourceBefore 보관.
Documents/Unreal IOCP Server/checkpoints/AuthFocus_20261015/S1EditorBuild.log:
S1Editor Win64 Development Succeeded/exit0. Diff check 통과(기존CRLF안내만).
첫 자동검증은 새 테스트가 초기화되지 않은 NewObject 위젯에 TakeWidget을
호출해 UMG에서 충돌했다(exit3). FixtureFailure 로그로 보존했다. 테스트를
transientWorld+CreateWidget 정상초기화 경로로 고치고 재빌드/재실행했다.
최종 UnrealAutomation.log/index.json:27개통과(26success+기존Q경고1),
실패0/미실행0, 새 AuthFocusTargets 성공. 초기충돌을 숨기거나 이전 결과로
대체하지 않았다. S1 game Development build도 Succeeded/exit0.

새 GUI editor21836/MCP 재실행 후 in-process floating2PIE 인증창 관찰:
첫 창에서 클릭아이디 -> Tab3회 -> Enter로 회원가입화면 진입,
실제signup아이디[focused] 확인. 빈계정생성 클릭은 로컬 validation메시지
표시(서버계정생성 요청 없음). 로그인↔가입3회 반복 후 로그인아이디
[focused] 확인. 비밀정보 아닌 FocusProbe 문자열 입력 도구성공,
Tab 후비밀번호[focused] 확인; ShiftTab/CtrlA/Backspace로 임시입력 정리.
Observer해제/StopPIE 완료. 두번째창의 개별키조작/async응답은 미검증.
EditorStartup.log 검토결과 UIOnly/Non-Focusable0,Fatal/Ensure/BlueprintRuntime0.
Native form 생성시뿐 아니라 실제 키/패널이동을 확인한 증거이며,
성공로그인/캐릭터선택/DB생성/Main복귀 확인으로 대신하지 않는다.
새 에디터는 유지, 기존서버36624 유지. 코드5파일+테스트+본문만 변경.

이번 요청은 단계1이다. 단계2 새 배포용패키지·서버재빌드와 단계3패키지
2계정통합은 아직 시행하지 않았다. 서버/DB/설정/Blueprint/Content 변경·
저장·커밋/푸시 없음. 기존아이템조작/신규계정생성 등 DB변경 테스트 없음.
에디터 재실행 후 빈입력 검증과 패널전환은 MCP로 확인할 수 있으나,
실제 인증은 사용자에게 요청하고 비밀번호를 툴/로그/문서에 담지 않는다.

## 사용자 인수 항목

로그인 처음진입 아이디입력,Tab으로비밀번호·버튼 이동, 기존Enter동작,
회원가입↔로그인 반복 및 실패후입력,캐릭터목록↔생성 반복/실패후입력,
실로그인→캐릭터선택→Main의 이동·공격·시야회전·커서복귀.
두PIE 각각의 포커스가 다른창을 빼앗지 않는지와 종료/재시작 확인.
사용자 검증과 자동 Slate 구조검증을 구분하고,실행하지 않은 회원가입
DB생성/성공·packaged2client게임플레이를 통과로 기록하지 않는다.
