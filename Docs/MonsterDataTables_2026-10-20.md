# Monster Data Tables — 1차 Dusk 정의 전환

## Unity 빌드 중복 함수 수정 — 2026-10-21

사용자의 S1Editor 빌드에서 C2995 발생. S1MonsterData.cpp와 S1QuestData.cpp의
anonymous namespace 안에 같은 시그니처의 Find<Row>(UDataTable*,int32)가 있었다.
두 cpp가 Module.S1.cpp에 함께 포함되면 재정의된다. 마을 에셋 이관 때문이 아니다.
이전 Adaptive non-unity 빌드는 작업 파일을 분리하여 이 결함을 노출하지 못했다.
각 내부 helper를 FindMonsterDataRow / FindQuestDataRow로 구분하고 호출부만 변경했다.
공개 API/행 검색 동작/콘텐츠/서버/프로토콜 변경 없음.

수정 후 S1Editor Win64 Development 일반 빌드 성공(39.69초).
이어 -DisableAdaptiveUnity를 이번 빌드 호출에만 적용해 두 cpp가 실제 같은
Module.S1.cpp에 포함되는 것을 확인하고 Unity 컴파일·링크 성공(32.46초, exit0).
영구 Target/빌드 설정은 변경하지 않았다. 첫 샌드박스 실행은 결과 로그 없이 exit1로
종료되어 성공으로 계산하지 않고, 접근 권한이 있는 실행으로 위 두 빌드를 검증했다.
이번 수정의 새 PIE/Blueprint 컴파일/패키징은 미실행이며 이전 인수를 재검증으로 쓰지 않는다.

## 최신 인수 기록 — 2026-10-21

사용자가 직전에 제시한 전체 체크리스트의 1인·2인 PIE 테스트 완료를 보고했다.
다중 Dusk 스폰/크기/방향/바닥/HP, 벽 우회 추적, A–D/Q와 몬스터 타격,
벽 너머 공격 차단, 복귀·HP 유지 재교전·4초 회복/중단, 사망·재생성,
포션 드랍/획득과 퀘스트 기여 처치, 양쪽 표시 및 재접속 회귀를 사용자 확인 완료로 기록한다.
이 기록은 아래의 당시 PIE 미검증 상태를 대체한다. 새 에이전트 플레이나 로그 감사는 아니다.
최신 패키지의 2계정 실제 플레이는 별도 미검증이며, 기존 패키지 시험을 대체하지 않는다.

## 범위와 권위

사용자 승인: 현재 Dusk 수치를 보존하면서 전투/표현 정의를 Data Table로 분리하고,
검증된 Export를 IOCP 서버에서 로딩, 스폰 ID와 연결한다. 새 AI/몬스터 콘텐츠,
경사/계단/동적 장애물, DB 스키마/프로토콜 변경은 없다.

전투 권위는 서버다. Unreal이 전투 DT의 HP/공격력으로 피해를 계산하거나 서버에 보내지 않는다.
서버는 관리자 로컬에 배포된 파일을 시작 시 DB/네트워크보다 먼저 검증하고 한번만 로딩·고정한다.
없거나 잘못되면 종료코드2로 시작을 거부한다. 과거 hardcoded Dusk로 조용히 fallback하지 않는다.
실행 중 reload는 지원하지 않는다. runtime definition pointer 수명은 서버 실행 내내 유지된다.
DefinitionVersion은 메타데이터이며 새 패킷 협상/클라이언트 버전 검증을 구현한 것은 아니다.

## 에디터 작성 위치

`/Game/S1/Monsters/Data/DT_S1MonsterDefinitions`
(FS1MonsterDefinitionRow), `/Game/S1/Monsters/Data/DT_S1MonsterPresentations`
(FS1MonsterPresentationRow). 첫 행 이름은 Dusk, 실제 연결키는 MonsterDefinitionId=1이다.

| 전투 항목 | 이전값 = 이번 DT/Export 값 |
|---|---|
| ID / DefinitionVersion / BehaviorId |1 /1 /1|
| MaxHP / Damage / MoveSpeed |200 /10 /220cm/s|
| CapsuleRadius / CapsuleHalfHeight |28 /62cm|
| AcquireRadius / HomeLeash / LoseRadius |800 /1200 /1000cm|
| AttackStartRadius / HitRadius / HitHeight / HitFacingDot |170 /190 /150cm /0.57|
| AttackIntervalMs / AttackDurationMs / HitDelayMs |2000 /1000 /420|
| DespawnMs / RespawnMs / HomeRecoveryMs |3000 /8000 /4000|

표현 행: ID1, DisplayName Dusk, ActorClass BP_S1DuskMinion, Icon 미지정(향후 UI용 필드).
메시/AnimBP/애니메이션·스케일0.7·Yaw+90°·사망 Fade/블렌드는 기존 BP/네이티브 표현에 유지한다.
클라이언트 capsule/mesh 바닥 offset만 정의행으로 구성, 서버 HP는 snapshot 그대로 사용한다.
스폰 중심 Z = 스폰 발 위치Z(-0.5)+해당 정의 HalfHeight이며 이동/복귀는 그 Z를 보존한다.
따라서 현재61.5cm 불변. 지형 Z 계산을 추가한 것이 아니다.

## 수정/적용 순서

1. 두 DT를 에디터에서 편집한다. 같은 ID마다 유일한 전투행/표현행이 필요하다.
2. 의도한 DT만 저장한다. Export는 asset을 대신 저장하지 않고 dirty table은 거부한다.
3. Tools → **Export Monster Definitions**.
   결과: `Saved/ServerData/MonsterDefinitions.s1monster`.
   또는 `UnrealEditor-Cmd S1.uproject -run=S1ExportMonsterDefinitions -Output=<absolute path>`.
4. `Tools/DeployMonsterDefinitions.ps1` 실행. 실제 GameServer.exe의
   `--validate-monster-definitions <file>`가 성공해야 Config에 복사한다.
   기존 Config 파일은 시간표시 `.before_...` 백업, pending copy 해시 검사 후 교체.
5. GameServer 재시작. 기본 로딩 위치는 EXE 옆 `Config/MonsterDefinitions.s1monster`.
   `GAMESERVER_MONSTER_DEFINITIONS_FILE`은 명시적 서버 경로 override이며 클라이언트 입력이 아니다.
6. 모델/표현/capsule 변경은 새 spawn/재접속으로 확인한다. 기존 actor를 실시간 갱신하지 않는다.
   패키지는 새 DT를 다시 Cook해야 한다. 서버와 클라이언트를 같은 authoring revision으로 배포한다.

서버 `GameServer/Data/MonsterDefinitions.s1monster`는 초기 설치 seed다.
MSBuild는 runtime Config가 없을 때만 seed를 복사하여 기존 배포를 덮어쓰지 않는다.
DT 수정 뒤 Build만으로 새 값이 적용되지 않는다. 반드시 Export/Deploy/Restart를 따른다.
Content는 Git 제외이므로 DT 백업도 별도 필요하다. `ArtSource/Data/Monster*Baseline.json`은
초기 Dusk 복원용 JSON이며 런타임의 별도 source of truth가 아니다.

## 검증/제한

- ID/행 중복·누락·비유한 값·범위·hitDelay<duration≤interval·캡슐·행동종류 검증.
  서버 정수는 signed64로 읽어 음수 unsigned 래핑도 거부한다. 텍스트64KiB/최대256행.
- 현재 BehaviorId=1(기존 Dusk melee FSM)만 지원한다. 새 AI는 행 추가만으로 생기지 않는다.
  ActorClass는 현재 AS1DuskMinion 계열만 지원한다. 공통 새 몬스터 기반 클래스 전환은 후속 작업.
- 스폰 Export는 이제 ID==1 검사 대신 검증된 DT의 ID 존재를 확인한다.
  서버도 등록된 정의만 로딩하며 Nav agent보다 큰 capsule은 해당 스폰을 거부한다.
  현재 Nav agent35×높이144, Dusk28×높이124라 기존 동작 유지.
- 미등록 ID/표현 누락은 Dusk fallback하지 않고 클라이언트 spawn 거부.
  WorldEntitySubsystem의 native CDO hard DT reference → DT hard BP row reference가 cook 경로다.
- 새 ID의 드랍 테이블/DB 등록은 이번 범위가 아니다. ID1만 기존 포션 드랍;
  미래 ID를 Dusk1 드랍으로 잘못 기록하지 않도록 드랍은 미등록 시 거부한다.
  새 몬스터용 퀘스트/드랍·AI/애니메이션은 별도 설계/검증이 필요하다.
- 공격/사망 시간 수치를 바꿔도 애니메이션 속도·사망 Fade를 자동 재타이밍하지 않는다.
- 기존 서버 staging/중첩 저장소는 건드리지 않았고 이번 작업에서 commit/push하지 않는다.

## 변경 코드 / 에셋

Client: Public/Game/S1MonsterData.h, Game/S1MonsterData.cpp 및 Tests,
World/S1WorldEntitySubsystem.h/.cpp, Game/S1DuskMinion.h/.cpp,
S1MultiMonsterPresentationTests.cpp, MonsterSpawnPoint 주석, S1DuskDefinition 주석.
Editor plugin: S1ExportMonsterDefinitionsCommandlet.h/.cpp,
S1ExportSpawnPointsCommandlet.cpp, S1NavigationToolsModule.cpp.
Tools/DeployMonsterDefinitions.ps1, 초기 JSON과 본 문서/CurrentAssetRegistry.
신규 binary2개는 위 DT뿐. BP_S1DuskMinion/MainMap/vendor 애셋을 저장하지 않는다.

Server: MonsterDefinition.h(loader/registry), GameServer.cpp(startup/CLI),
Monster.cpp, MonsterSpawnConfig.h(include), Room.cpp(runtime height/recovery/nav/loot guard),
DuskDefinition.h 주석, GameServer.vcxproj(first-install seed), Data/MonsterDefinitions.s1monster.
Tests/MonsterDefinitionConfigTests.cpp 및 MonsterDefinitionFixture.h,
기존 MonsterSpawn/QuestDefinition/Room/Quest DB 테스트의 explicit fixture 초기화.
테스트 fixture만 baseline 문자열을 사용한다. production fallback은 없다.

## 증거 / 진행 상태

체크포인트 `C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MonsterTables_20261020`.
기존 소스/서버EXE/BP/Map을 Before 보존. MCP PIE=false, 기존 BP/Map clean 확인 후 시작.
새 테이블2개 생성·row readback·dirty 보고 후2개만 선택 저장.
MCP DataTable tool의 Class 값은 schema refPath object와 달리 table JSON string 경로가 필요해
최초 None readback을 발견하고 올바른 BP class string으로 수정/재확인한 뒤 저장했다.
엔진/플러그인 코드는 조사만 했고 수정하지 않았다.

S1Editor/S1 Development 및 GameServer Debug 빌드 성공.
실제 DT commandlet Export 및 서버 CLI 검증 성공, Config에 해시 검증/백업 후 배포.
최종 Unreal 자동 검사38개 통과(37clean+기존 GameplayCue 경고1).
의도적 unknown-ID 거부는 AddExpectedMessagePlain으로 검증하여 예상 경고로 처리한다.
최종 정의 파서33건, 스폰29건, 퀘스트 정의30건, 합계92건 실패0(실제 DT Export 포함).
실제 Room 오프라인43건 실패0: 다중 타격/생명주기/재생성/재타겟/복귀4초 회복 등.
이 테스트에는 DB/실제 접속자가 없으므로 `Death loot unavailable` 로그가 예상대로 발생한다.
WinSDK 자동 선택 실패로 단독 테스트 첫 빌드는 crtdbg.h를 찾지 못했다. 설치된22621 SDK를
명시하여 해결. 테스트 소스 헤더의 기존 코드페이지 경고/링커 EditAndContinue 경고는 별도다.

첫 패키징은 BUILD SUCCESSFUL이었지만 컨테이너 목록 검사에서 두 DT/BP 누락 발견.
네이티브 CDO의 soft reference는 이번 Cook에서 수집되지 않았다. 이를 FObjectFinder /
UPROPERTY TObjectPtr hard reference로 교체하여 해결했다. FirstCookMissing에 첫 목록/로그 보존.
중간 UAT1회는 Editor 빌드와 UBT mutex가 겹쳐 exit10(ConflictingInstance),
SDK 미설치가 아니다. PackageMutexFailure.log 보존하고 빌드 완료 후 직렬 재실행.
최종 Windows Development Build/Cook/Stage/Package/Archive: BUILD SUCCESSFUL / ExitCode0.
ContainerEntries.csv에서 두 DT/BP_S1DuskMinion/필수 맵3개 모두 포함 확인.
패키지 NullRHI 스모크 exit0: obj list가 두 DataTable과 BP_S1DuskMinion_C 실제 로드를
출력했다(PackageSmoke.log). 이것은 렌더링/로그인/전투 플레이 검증이 아니다.
출력 `Saved/MonsterTablesPackage_20261020/Windows`.

서버 시작 guard2건: 파일 없음/잘못된 헤더는 exit2, DB/network 전에 거부됨.
배포 Export SHA2565EFD96C920020BE47C72FDB6285694C00129CDA8C805C695E0B07D09F2F19466.
새 Debug GameServer PID33352,7777 listen; 로그에 definitions loaded and sealed1,
Quest1/Dialogue8, DB 계정조회/loot/potion/quest readiness 정상 확인.
기존 로컬 환경 비밀번호를 프로세스 환경에만 전달, 출력/파일 저장하지 않았고
bootstrap-admin 환경은 제거했다. DB migration/스키마 변경은 수행하지 않았다.
새 Editor PID35004, MCP 재연결, PIE=false, 두 테이블 재로드/Dusk행/clean 확인.
전투 DT 에디터를 열어뒀다. 기존 BP/Map clean 및 이전 SHA 불변 확인.
After에 DT2개/Export 백업. 정의 DT SHA472191DC55E20CA718B9B958B9D4F5E43113CE25557471A3C4660454CE302202,
표현 DT SHA4EA99F9AFF608B59AF0406F39A892EDAFCDC46C7EBDC09930E127FD846CC63BB.

구현 종료 당시에는 실제 최신1/2인 PIE/드랍·퀘스트·재접속 및 패키지2계정 플레이가 미검증이었다.
2026-10-21 사용자 인수로 PIE 체크리스트는 완료(상단 참조). 최신 패키지2계정 플레이는 미검증이다.
