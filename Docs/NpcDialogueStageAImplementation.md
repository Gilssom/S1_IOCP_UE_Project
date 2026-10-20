# NPC 대화 A단계 — 2026-10-16

후속: NPC 접근 [G] 안내를 /Game/S1/UI/WBP_S1NpcInteractPrompt로 분리하고
기존 NPC BP DialoguePrompt.WidgetClass에 연결했다. C++ 기본 native 안내는 fallback,
현재 MainMap BP NPC는 Designer 편집 가능한 WBP를 사용한다.
상세 검증·편집 위치는 NpcInteractPromptWidget.md. 대화창 UI/카메라/입력 로직 변경 없음.

최신 사용자 확인: 반투명 상하 gradient 수정본 화면 검증 완료.
별도 디자인 작업으로 UIResources/DialogueFantasy_v1 PNG5종과 적용 설명을
제작했다. 아직 임포트/위젯 적용하지 않은 Candidate이며 기존 UI 로직은 변경 없음.
기존 아래의 gradient 화면 미검증 기록은 사용자 수락 전 시점의 기록이다.

## 시각 인수 및 반투명 그라데이션 후속

사용자가 NPC 뒤에서 시작한 대화 구도, 상대 클라이언트에서 NPC 회전 없음,
종료 후 다른 플레이어 표시 복구를 모두 확인했다. 해당 세 항목은 사용자 인수 완료.
다른 A단계 중단/장애 경로 전체를 자동으로 통과 처리한 것은 아니다.

사용자 추가 요청: 상하 검정이 너무 잘려 보이므로 gradient 또는 alpha0.5.
엔진 ACameraActor constructor에서 CameraComponent->bConstrainAspectRatio=true를
확인했고 WBP에는 검은 상하 패널이 없었다. 고정 화면비 레터박스를 끄고,
US1DialogueWidget native Slate wrapper가 기존 Designer tree 뒤에 soft gradient를 그린다.
상/하 각각 화면 높이8%, 가장자리 검정alpha0.5→안쪽alpha0, 가운데84%는 영향 없음.
UI패널/버튼은 위에 그려져 읽기나 클릭에 영향이 없다. 새 텍스처/머티리얼 구매 불필요.
WBP_S1Dialogue Class Defaults > Dialogue > Edge Shade:
EdgeShadeOpacity(.5,0이면비활성), EdgeShadeHeightFraction(.08,범위0~.25) 조절.
대화 camera viewport/FOV/안전 fallback·로컬 NPC방향·다른플레이어mask는 유지한다.
실제 화면비 고정 해제로 최종 프레이밍은 새 버전 사용자 시각 재확인 필요.

수정은 UI/S1DialogueWidget.h/.cpp, Quest/S1DialogueSubsystem.h/.cpp,
Quest/S1DialogueTests.cpp와 문서. BP/WBP/맵 저장/서버·DB 변경 없음.
변경 전 PIE=false, MainMap/NPC BP/대화WBP dirty=false, source checkpoint
NpcDialogueGradient_20261016/SourceBefore 보존. 최종 Editor/game Development
빌드 exit0, 자동28개 성공(27success/기존GameplayCue 경고1/실패0/미실행0).
실제 WBP를 CreateWidget/TakeWidget해 wrapper 생성·기존6개 컨트롤 보존·
alpha.5 기본값과 camera aspect constraint 해제를 검사했다. Slate 엔진의
gradient stop 방향 규칙을 소스로 확인해 수평 stop(top-to-bottom blend)을 사용한다.
증거 checkpoint S1EditorBuild.log/S1Build.log/UnrealAutomation.log 및
Saved/Automation/NpcDialogueGradient_20261016/index.json. 새 그라데이션의
실제1/2인 화면·최종 패키지는 미검증이며 기존 시각 인수와 구분한다.
새 에디터/MCP 재연결 후 실제 WBP CDO의 opacity0.5/height0.08 재조회,
MainMap/WBP/NPC BP dirty=false 확인. 저장 없이 로그인용 floating PIE 시작.

## 후속 승인: 로컬 NPC 정면/플레이어측 카메라/타 플레이어 제외

사용자 승인으로 대화할 때만 NPC Visual 메시가 플레이어를 향하도록 변경.
Npc Actor root 위치/회전, 플레이어 위치/회전/서버 패킷은 변경하지 않는다.
각 독립 클라이언트/PIE World에만 적용하므로 다른 클라이언트 NPC는 돌지 않는다.
같은 World의 split-screen 다중 로컬 플레이어를 지원한다는 뜻은 아니다.
메시의 기존 relative rotation을 저장해 정상 종료/abort에서 멱등 복구한다.

CameraOffset은 이제 NPC 고정 Yaw 대신 NPC→플레이어 수평 축 기준이다.
|X|는 플레이어측 거리, Y는 어깨측 치우침, Z는 높이. 반대 어깨/중앙 후보를
검사하되 NPC 뒤로 넘어가는 역방향 후보는 제거. 플레이어 실제 회전은 유지하므로
플레이어가 NPC 반대 방향을 보고 서 있을 때도 강제로 돌리지 않는다.

다른 AS1Player와 재귀 attached actors를 OwnerPC.HiddenActors로 가린다.
진입 후 생긴 플레이어/장착 액터도 갱신, 숨긴 대상은 카메라 충돌/시야 검사에서 제외.
screen-space HP는 renderer mask만으로 숨겨지지 않아 EntityHealthComponent에
별도 dialogue suppression을 추가. 서버 HP는 계속 갱신하되 화면만 억제하며
종료 때 최신 정상 표시 정책으로 복귀한다. 원래 숨겨진 플레이어는 숨김 유지.
WorldEntity registry/despawn이나 실제 actor hidden/collision을 변경하지 않는다.

변경: Quest/S1DialogueSubsystem.h/.cpp, Quest/S1DialogueTests.cpp,
UI/S1EntityHealthComponent.h/.cpp 및 본 문서/CurrentAssetRegistry.md.
BP/WBP/맵 저장 없음. 변경 전 MCP PIE=false 및 MainMap/NPC BP/대화WBP dirty=false.
SourceBefore checkpoint NpcDialogueFacing_20261016 보관.
회귀 항목 추가: 네 방향 접근의 플레이어측 camera, NPC메시만 회전·원복,
플레이어 transform 불변, PC별 render mask, 기존 hidden 보존, 늦은 attachment,
HP 갱신 중 억제 및 복귀. 최종 빌드/자동·시각 검증 결과는 아래 후속 기록으로 구분.

후속 검증: S1Editor Win64 Development exit0, 자동28개 통과(성공27/기존
GameplayCue 경고 포함 성공1/실패0/미실행0), DialogueStageA 자체는 경고0/오류0.
증거 checkpoint NpcDialogueFacing_20261016/S1EditorBuild.log, UnrealAutomation.log,
Saved/Automation/NpcDialogueFacing_20261016/index.json. Game Development 빌드도
exit0(S1Build.log). 에디터 재실행 후 새로운 2인 실제화면 검증은 아직 미실행이다.

사용자 승인: 임시 Manny, 하단 대화/우측 상세, 전투 중 진입 제한.
서버 수락/보상/DB/패킷/퀘스트 수치는 범위 밖. 기본 전투·아이템 인수 재요청 없음.
모델 교체 시 BP의 NpcMesh/IdleAnimation 및 MeshOffset/MeshYaw/MeshScale 조절;
새 Idle의 Skeleton이 메시와 같아야 한다. 다른 Skeleton 애니메이션은 자동 retarget하지 않는다.

## 구현

- Quest/S1QuestNpc.h/.cpp: 고정 로컬 NPC, 교체 가능한 메시/Idle과 대화/카메라
  기본값. 기본 Manny+MM_Idle은 hard native 참조. 이동AI/서버 entity 아님.
  collision/nav 영향은 없음. 최종 외형/물리 장애물이 필요하면 별도 설계.
- Quest/S1DialogueSubsystem.h/.cpp: Game/PIE World별 tickable coordinator.
  계획의 Interaction 별도 클래스 대신 G 선택과 modal 수명을 한 곳에 두었다.
  후보 거리 비교 및20cm 유지 편향; 최근접 유효 대상 하나만 표시·G 처리.
  이미 진행 중인 loot 요청은 원래 UUID 재확인 경로 우선, NPC로 전환하지 않는다.
- NPC 반경250/이탈350cm, 수직100cm/시야 필터. 전투 동작/공중/인벤토리/커서
  조작/추적·공격중인 Dusk/피격후3초는 진입 거절. 모두 클라이언트 연출 제한이며
  서버 행동 승인이나 무적·전투 정지를 의미하지 않는다.
- FadeToDialogue→RevealDialogue→Active→FadeToGame→RevealGame. 시작값0.2초.
  NPC 카메라 offset과 양 인물 발 위치 midpoint+height, FOV/종횡비로 거리 확보.
  네 방향 후보의 Camera sphere sweep/overlap 및 양 인물 visibility 검사.
  실패하면 기존 게임 카메라를 유지하는 안전 fallback. teleport/강제 캐릭터 회전 없음.
- 기존 카메라 arm/FOV 설정은 수정하지 않고 임시 CameraActor view target만 사용.
  저장 view target이 소멸했으면 현재 pawn으로 복구. 다른 시스템이 view target을
  바꿨으면 그 target/회전을 덮어쓰지 않고 대화를 종료한다.
- UI/S1DialogueWidget.h/.cpp: 대화·상세 텍스트와 버튼 binding, Esc/거부 종료.
  상세는 같은 WBP 안에 구성하므로 Designer에서 두 영역을 함께 조정한다.
  수락은 비활성, '다음 단계' 표기. 수락 성공/가짜 진행/보상 표시 없음.
  passive NPC G prompt는 native US1NpcPromptWidget.
- MyPlayer: G는 coordinator로 한 번만, 진입 시 이동 입력 잔량/점프 정리,
  modal 동안 gameplay/I 차단. Inventory: modal 동안 획득/사용/퀵슬롯/I 차단,
  loot 후보/힌트 API 제공. 기존 수량·소유권·패킷·DB 처리 변경 없음.
- WorldEntity: 검증된 로컬 피해/사망/registry reset에서 즉시 정리.
  MyPlayer EndPlay/World Deinitialize, NPC 소멸/거리 이탈/연결·Main준비 해제도 정리.
  IgnoreMove/Look은 true/false balanced lease; 타 소유자의 제한을 reset하지 않는다.
  연결 해제/맵전환 때 새 인증UI 입력 모드를 GameOnly로 덮어쓰지 않는다.

## 바이너리/체크포인트

대상: /Game/S1/Quest/BP_S1QuestNpc, /Game/S1/UI/WBP_S1Dialogue,
/Game/Maps/MainMap의 NPC1개 배치. 기존 BP_MyPlayer/벤더 원본 수정·저장 없음.
BP의 DialogueWidgetClass가 WBP hard ref, MainMap의 placed BP가 cook root가 된다.
source-only Git이므로 새/변경 애셋은 별도 checkpoint에 보관한다.
외부 checkpoint: C:/Users/user/Documents/Unreal IOCP Server/checkpoints/NpcDialogueA_20261016.
ContentBefore/MainMap.umap SHA256 1EA38B13A638CB4139CBEF7D3CC2C710E2609C1929A658D03B012561B016E66C.
작업 전 MainMap/BP_MyPlayer dirty=false, MCP PIE=false. 저장 전 dirty를 다시 보고하고 선택 저장한다.

## 검증 기록 (구현/사용자 인수 구분)

첫 Editor 빌드에서 신규 GetPickupCandidate의 const Flow 포인터로 비const
GetLootState 호출 오류가 발생. 포인터 한 곳을 수정, 로그 InitialConstFailure로 보존.
재빌드 성공. 자동28개(성공27+기존GameplayCueNotifyPaths 경고1) 통과.
이후 view target 외부 소유권 방어 보완 후 최종 Editor/game Development 빌드 exit0,
자동28개(성공27/기존GameplayCue 경고 포함 성공1/실패0/미실행0) 재실행 통과.
Native 테스트는 거리 hysteresis/발 기준 focus/멱등 abort/중첩 input lease 유지/
Manny Skeleton 일치/비복제/미수락 표기 검증이며 실제 투샷·UI 플레이 대체가 아니다.
에디터 BP/WBP 생성 및 warnings-as-errors 컴파일 통과. WBP16widgets/6bindings,
부모 S1DialogueWidget 및 NPC부모 S1QuestNpc 확인. NPC 위치(-350,250,-0.5),Yaw0,
MeshYaw-90,scale1. 바닥 trace600→-600의 거리600.50006으로 Z=-0.5 확인.
저장 전 의도한 새BP/새WBP/MainMap dirty=true, 기존BP_MyPlayer/BP_Player/Dusk false;
세 패키지만 저장하고 모두 dirty=false. MainMap→Lobby→Main 재로드 후 동일 NPC1개,
Manny/Idle/WBP/카메라 설정 재조회로 보존 확인. ContentAfter에 세 파일 checkpoint.
BP→WBP 및 MainMap→BP asset registry 참조 확인. cook의 실제 성공은 아직 미검증.
로그인용 floating PIE 시작. 실제 대화/UI/1·2인 플레이 인수와 새 패키지는 미완료.
증거: checkpoint S1EditorBuild.log/S1Build.log/UnrealAutomation.log/EditorStartup.log,
Saved/Automation/NpcDialogueA_20261016/index.json. 이번 단계 서버/DB 재빌드·변경 없음.

사용자 확인: 두 인물 구도/벽가림 fallback, 페이드 속도, 상세 패널 가독성,
G 대상 하나만 표시, Esc/거부 후 이동·공격·시야·I/퀵슬롯 복구,
동시 두 클라이언트 대화 독립, 피격·사망·단절 후 검은 화면/잠금 잔류 없음.
