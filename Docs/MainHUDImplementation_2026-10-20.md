# S1 Player Main HUD — 2026-10-20

## 요청 / 범위

사용자가 제공한 Aion2 Main HUD의 하단 중앙 구도를 참고하여 S1용 이미지·Widget Blueprint·기존 시스템 연결을 구현한다.
원본 게임 UI 텍스처/로고를 복사하지 않는다. MP/경험치/재화/미니맵은 이번에 임의 구현/가짜 표시하지 않는다.
CharacterInfo에 level은 존재하지만 현재 local pawn에 선택 캐릭터 summary를 전달하는 HUD 캐시가 없으므로
이번에는 레벨 숫자를 임의로 추측해 표시하지 않는다. 현재 등록된 전사 역할은 KNIGHT로만 표시한다.
기본 공격4타, Q2타, 아이템/퀘스트 권위와 프로토콜/DB는 변경하지 않는다.

## 실제 연결

- InventorySubsystem이 기존 QuickWidget1개를 생성할 때 MainHUDClass soft class를 우선 사용한다.
  `US1MainHUDWidget : US1QuickSlotWidget`. 기존 단독 WBP_S1QuickSlots는 fallback으로 보존한다.
  Main HUD 생성은 인벤토리 capability의 선행 return보다 앞에 두어 HP/전투 표시는 독립 유지한다.
  아이템 조회·거래 처리는 기존 bEnabled 조건 뒤에 남겨 미지원 요청을 보내지 않는다.
- 1~4 포션: 기존 Icon/Count/State binding, 서버 인벤토리 수량, 드래그 등록, 우클릭 해제 코드를 상속.
  같은 위젯 인스턴스에 HP와 전투/메뉴 UI를 더한다. 두 개의 퀵슬롯을 동시에 만들지 않는다.
- HP: US1EntityHealthComponent가 이미 보관한 서버 snapshot의 읽기 전용 getter.
  초기 미수신은 ‘HP 동기화 대기’이며100/100을 조작하지 않는다. 사망 시 fill0, 비율0..1 제한.
- LMB/Q 영역: KnightCombatComponent의 현재 로컬 모션 상태와 콤보 단계를 표시.
  이것은 서버 승인/적중/피해 확정 표시가 아니다. 기본 공격4타 유지. Q는 현재 연속 베기 표현.
  Q 쿨다운 정책 미설정이므로 타이머/서버 자원 비용을 만들지 않는다.
- 가방[I]/퀘스트[O] 버튼: 기존 Inventory/QuestSubsystem 공개 API로 연결.
  서로 창을 겹쳐 열지 않으며 NPC 대화 모달 차단 유지. Quest 버튼 진입은 Alt 커서 상태에서만
  기존 키 진입의 커서 조건을 우회하는 명시적 로컬 UI 경로이며 생존/Room/서버 보상 판정을 우회하지 않는다.
- Alt 홀드: 기존 Player 입력에 커서 표시를 추가. 누르면 이동 입력을 정리하고 GameAndUI,
  놓으면 다른 모달이 없을 때 GameOnly 복귀. UIOnly의 key-up 소비는 Slate modifier 조회로 정리.
  대화/인벤토리/퀘스트 모달이 입력을 소유하면 해당 모드를 덮어쓰지 않는다.
- 대화 중 HUD 전체 Collapsed/종료 후 기존 표시 복귀: InventorySubsystem의 기존 정책 유지.
  기본 공격·Q·점프·카메라 등 입력 규칙은 기존 CanUseGameplayInput의 커서/모달 차단을 사용한다.

## 이미지 / 디자인

- 생성 방식: built-in image_gen, transparent background. CLI/API 키 사용 안 함.
- 소스: `ArtSource/UI/MainHUD/S1_MainHUD_Backplate_v1.png`,2172×724 RGBA.
  검사 샘플: 바깥 corner alpha0, 내부 crest alpha253. 실제 이미지로 확인했다.
- Runtime texture: `/Game/S1/UI/HUD/T_S1MainHUDBackplate`.
- WBP: `/Game/S1/UI/WBP_S1MainHUD`, native parent `/Script/S1.S1MainHUDWidget`.
- 얇은 은색·금색 테두리와 남색 내부·중앙 방패 장식. 글자/수치/아이콘/슬롯은 이미지에 굽지 않는다.
- 프롬프트 원문: ArtSource/UI/MainHUD/README.md. 스타일 참고 이미지는 edit target이 아니다.
- 레이아웃은 HUDRoot/MainHUDCanvas. HealthBar/HealthText, PrimaryFrame/PrimaryText,
  ChargeFrame/ChargeText, InventoryButton/QuestButton, Icon1..4/Count1..4/State1..4 binding 이름·타입 유지.
  이미지 위치/크기, 폰트/색/간격/셀 크기는 Designer에서 편집 가능.
- 포션 icon은 기존 Rair_Potion과 표시 설정 사용. 현재 S1 Terra 폴더에서 UI/skill icon 파일을 찾지 못했다.
  기본 공격/Q는 텍스트 상태 카드로 제공하며, 전용 아이콘은 이후 사용자 선택 에셋으로 교체 가능.
  E/방어/회피 등 미구현 스킬 슬롯을 활성 기능처럼 표시하지 않는다.

## 변경 파일

- 신규 Source/S1/UI/S1MainHUDWidget.h/.cpp 및 S1MainHUDTests.cpp.
- Source/S1/UI/S1EntityHealthComponent.h: 기존 cache getter3개만 추가.
- Source/S1/Loot/S1InventorySubsystem.h/.cpp: serialized MainHUDClass soft path, 생성 시 선택.
- Source/S1/Quest/S1QuestSubsystem.h/.cpp: trusted HUD 버튼 진입 옵션. 키 입력 기본값 유지.
- Source/S1/Game/S1MyPlayer.h/.cpp: Alt 커서 입력·해제·UI 모드 소유 보호.
- 신규 WBP_S1MainHUD, T_S1MainHUDBackplate. 기존 QuickSlots/Inventory/Quest/Dialogue/Map/폰트 원본은 저장하지 않는다.
- ArtSource PNG/README, .gitignore의 정확한 생성 PNG 경로 제외, 본 기록 및 CurrentAssetRegistry.

## 체크포인트 / 검증

`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MainHUD_20261020`.
Before에 원본 quickslot WBP 및 겹치는 소스 보존. 기존 미커밋 작업 유지. source-only 정책에 맞춰
ArtSource/UI/MainHUD/*.png 제외 확인(로컬 파일은 보존). Content도 기존 정책으로 Git 제외.
커밋/푸시 미요청. UE5_RPG/원본UE5.5/엔진·서버·DB 수정 없음.

MCP 초기 연결 거절 → 사용자가 StartServer8000 실행 → PIE=false와 기존30-widget quickslot tree 확인.
새 texture import/새 parent 위젯 생성은 실제 발견한 MCP 기능만 사용한다.
실제 적용·검증:

- S1/S1Editor Win64 Development 최종 빌드 성공. S1Editor 최종 링크는 별도 자식 에셋 창만
  닫힌 상태의 DLL 잠금으로1회 실패했다. 실제 PID4544 정상 종료를 확인한 뒤 재링크 성공.
  실패 로그는 EditorBuildLinkLock.log로 보존했으며 성공으로 은폐하지 않는다.
- MCP TextureTools.import_file로 신규 texture 생성. readback:
  TEXTUREGROUP_UI / TC_EditorIcon / TMGS_NoMipmaps. RGBA 원본 보존.
- UMG 도구로 새 HUD 생성·compile 성공, warnings-as-errors 호출 성공.
  parent /Script/S1.S1MainHUDWidget, generated WBP_S1MainHUD_C 확인.
- dirty 신규 texture/HUD2개를 보고 후 이 둘만 선택 저장. 이후 둘/원본QuickSlots/MainMap clean.
  중간 계층 구성의 필수 binding 자동 컴파일 메시지는 완성 후 해소했다.
- 저장본 After 백업. HUD SHA256 `96D70DB48B50F6F168AFE2041915222E8C2DA5BF95968F52BB1324F464B6F435`,
  texture SHA256 `427BE83856922BAF1C3C15DA0237E6B4B148F609506892DE4265087C6F353EEC`.
- MainHUD synthetic 검사 포함36개 자동 검사 통과(clean35+기존 Q GameplayCue warning1, failed0/notRun0).
  HP 미수신/비율/상한/사망·서버 cache getter, 실제 WBP 로드/HP 숫자, 콤보/Q 표시,
  기존 포션 icon/count binding, 메뉴 binding 및 GI 없는 안전 no-op 검사.
  저장된 리포트: Saved/Automation/MainHUD_20261020/index.json, 증거 폴더 UnrealAutomation.log.
  최종 생성 조건 변경 후에도 재실행36개 통과(clean35/warning1/failed0/notRun0).
- DesignerPreview.png를 확인했다. 이는 디자이너 화면이며 실제 PIE나 네트워크 인수 시험이 아니다.
  디자이너 previewHP75/100·포션99는 디자인 전용이며 runtime은 서버/cache 값으로 바뀐다.
- 서버 PID14812/127.0.0.1:7777 LISTENING 확인. 서버·DB·프로토콜은 변경하지 않았다.

실제1/2인 PIE·HUD 외관·Alt 커서/모달 입력 복귀·DPI/해상도·최신 패키지는 아직 미검증.
필수 수동 확인: HP 피격/포션/사망·리스폰, 본인별 수량과 단일 HUD,
포션 드래그·우클릭·1~4 사용, Alt→I/O 버튼과 창 닫기 후 입력 복귀,
대화 중 전체 HUD 숨김/복구 및 원격 클라이언트 UI 독립성.

## 같은 날 후속 수정 — 정사각형 슬롯 / 장식 HP

이 절이 위의 ‘텍스트 상태 카드’와 `x수량` 표현을 대체한다.

- WBP_S1MainHUD의 PrimaryFrame/ChargeFrame 및 SlotCell1..4를 모두60×60,
  MainHUDCanvas 로컬 Y188에 정렬했다. 스킬2칸 뒤 간격을 두고 소비 아이템4칸 배치.
  중앙 crest와 슬롯이 겹치지 않도록 MainHUDCanvas 높이264, 바닥 anchor 유지.
- 소비 아이템은 등록된 슬롯에 숫자만 표시(예7/0), 미등록은 빈 문자열.
  키1~4는 별도 상단 라벨이며 icon과 실제 서버 수량 경로는 유지한다.
  공통 QuickSlotWidget과 native fallback도 같은 숫자 형식이다. 인벤토리 본문 수량은 변경 안 함.
- Primary/Charge 각각 `Content` Canvas 아래 `Icon` Image, `CooldownShade` Image,
  `CooldownText` TextBlock을 변수로 준비했다. 예: PrimaryIcon/ChargeCooldownText.
  Icon은 Hidden, 쿨다운2요소는 Collapsed/빈 텍스트. 아직 실제 아이콘/쿨다운 모델 연결은 없다.
  아이콘 연결 시 Placeholder 문구를 숨기고 Icon을 HitTestInvisible로 바꾼다.
  서버 쿨다운 계약 확정 뒤 Shade/숫자를 갱신하며, 현재 타이머를 임의 생성하지 않는다.
- LMB/Q 키와 임시 스킬명은 각 정사각형 내부 별도 라벨.
  기존 PrimaryText/ChargeText binding은 Collapsed로 보존하여 런타임 상태 업데이트를 깨지 않는다.
  PrimaryFrame/ChargeFrame의 기존 공격 중 틴트 동작 유지.
- HP rail은 RoundedBox background/fill과 금속색 outline, 양끝 diamond finial로 변경.
  기존 backplate 안에 배치하고 실제 percent/서버 HP 숫자는 그대로 사용한다.
  네이티브 UMG 도형이므로 새 raster/Material/서버·DB 변경은 없다.
- 크기·좌표·색·HP corner radius/outline은 WBP Designer에서 편집 가능.
  포션 Icon의 부모는 기존60×60 SlotCell이므로 드래그/우클릭 위치 판정 구조 유지.

체크포인트: `checkpoints/MainHUDRefine_20261020` (workspace 증거 폴더).
수정 전 clean HUD 백업 SHA25696D70DB48B50F6F168AFE2041915222E8C2DA5BF95968F52BB1324F464B6F435.
MCP 직렬 조사→변경→compile/warnings-as-errors 성공. HUD만 dirty임을 보고한 뒤
해당1개만 선택 저장. 기존 QuickSlots/MainMap clean. 새 저장본 SHA256
D16D60A60AEBD836546225F8FC9F62D96DD5CD4550CC3BFC8B5A8B9E81F19052.
DesignerPreview.png는 이전 native DLL의 preview여서 x99가 보이며, 실제 새 런타임 검증 증거가 아니다.
새 수량 코드·저장 WBP 배치·HP brush·미연결 쿨다운 상태 자동 검사를 추가했다.
후속 수정 검증: S1Editor/S1 Win64 Development 빌드 모두 exit0/Result Succeeded.
새 저장본으로 S1 자동 검사36개 통과(clean35/기존 Q GameplayCue warning1, failed0/notRun0).
S1.UI.MainHUD와 S1.QuickSlots.WidgetBlueprintView 성공. 실제 저장 WBP를 로드하여
60×60/같은Y, numeric count, RoundedBox brush, 미래 쿨다운 hidden/empty를 확인했다.
리포트 Saved/Automation/MainHUDRefine_20261020/index.json, 증거 폴더의 build/automation 로그.
자동 검사 로그의 Fatal/Ensure condition/Blueprint Runtime Error/LogBlueprint Error 검색은0건.
이는 화면 플레이가 아닌 synthetic 검사다. 현재 수정본 PIE/패키지 시각 검증은 미실행.
새 Editor PID32068 재실행 후 MCP 재연결, PIE=false, 저장 HUD 재오픈/clean 확인.
초기 기동 중1회 연결 거절은 기동 완료 후 새 세션으로 해소했다. DesignerNewBuild.png 보존.
