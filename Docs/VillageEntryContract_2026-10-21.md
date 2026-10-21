# VillageMap 초기 로그인 진입 — 승인 계약 및 구현 기록

상태: 사용자가 최소 지역 계약을 승인했고 코드·생성 패킷·저장맵 배치 데이터 적용 및
오프라인 검사를 완료했다. 새 버전의 실제1-/2인 PIE는 사용자 확인 완료이며 패키지 검증은 진행 중이다.
DB 스키마/신규 패킷ID/포털은 추가하지 않았다. 아래 조사·계획은 역사적 설명이며
현재 실행 증거와 한계는 문서 끝의 구현 기록을 우선한다.

## 범위

- 최초 로그인·캐릭터 선택 및 재접속은 VillageMap으로 진입한다.
- Lobby → 기존 LoadingMap → VillageMap 흐름, 기존 인증/입장 권한 검사를 유지한다.
- 한 서버 안에 Combat(MainMap=1)과 Village(VillageMap=2) Room을 구분한다.
- Village는 Dusk 생성/타격/죽음/재생성 루프를 시작하지 않는다.
  기존 전투Room의 수치/패킷/몬스터/정적Nav 데이터는 유지한다.
- 포털 왕복, 플레이 중 지역 변경, 전투→마을 사망 이동, 마지막 지역 DB 저장은 제외.
  후속 전환에는 별도 승인/로딩완료·지역세대 계약이 필요하다.
- DB 스키마 변경 없음. DB에 이미 있는 room_id는 실제 Room 값으로 전달한다.
- 마을에서 기본공격/Q 표현을 막는 새 콘텐츠 규칙은 임의 추가하지 않는다.
  몬스터 없는 공간과 PvP가 없는 기존 서버 규칙을 유지한다.

## 확인된 문제

현재 client LevelFlow의 MainLevelName은 MainMap 고정이다. server는 GRoom 단일 입장이며
S_ENTER_GAME에 목적지 지역이 없다. client SpawnPlayer는 PlayerStart가 아니라 서버
PosInfo로 로컬 pawn을 재배치한다. client 맵 이름만 변경하면 spawn/monster/NPC 불일치가 생긴다.
일부 퇴장 handler도 실제 player->room 대신 GRoom을 호출하므로 다중Room에 맞춰야 한다.
서버 QuestNpc parser/UE Export는 room1/MainMap만 허용하고, Loot/포션/필드드랍에는
room1 기본값과 FlatCombatGroundZ 상수가 있다. 그대로 두면 마을 기록/표현이 틀어진다.

## 최소 패킷 계약 (사용자 승인 후 구현)

Canonical: E:/Git Fork/GameServer/GameServer/Common/Protobuf/bin/Protocol.proto.

| 메시지 | 새 필드 | 번호 | 의미 |
|---|---|---|---|
| C_ENTER_GAME | uint32 region_contract_version | 11 | client가 initial-region 계약1 지원을 알림 |
| S_ENTER_GAME | uint32 region_id | 14 | 서버가 결정한 목적지.1=Main combat,2=Village |
| S_ENTER_GAME | uint32 region_contract_version | 15 | server가 계약1을 확인 |

클라이언트는 목적지 경로를 요청하거나 서버 좌표를 정하지 않는다.
서버가 정한 지역ID를 client whitelist(/Game/Maps/MainMap,/Game/Maps/VillageMap)로 매핑한다.
새 패킷 종류/ID는 없으며 기존 필드 번호·combat/quest/loot 계약 의미는 바꾸지 않는다.
`.pb.*`는 canonical proto에서 양쪽을 재생성하고 직접 편집하지 않는다.

### 호환성

- 새server/새client: version1 확인 후 서버 선택 지역으로 진입.
- 새server/구client(version0): Player 생성/Room 등록 전에 S_ENTER_GAME.success=false로 거부.
- 새client/구server(version0): 응답 수락/인벤토리 초기화/스폰 큐/레벨 전환 전에
  비호환으로 처리하고 접속을 정리한다. MainMap이나 Village로 임의 fallback하지 않는다.
- unknown ID/version 및 malformed success 응답도 거부한다.
- 이 승인은 초기 접속에만 해당한다. region_id만으로 실행 중 포털 이동까지 지원한다고 하지 않는다.

## 입장 흐름

1. Account가 기존 character ownership/auth 검사를 거쳐 C_ENTER_GAME(version1) 전송.
2. 서버는 버전 검사 → DB 캐릭터 로드 → trusted village spawn/capsule 중심 좌표 적용
   → Village Room 등록. DB의 옛 전투좌표를 새 마을좌표로 재사용하지 않는다.
3. S_ENTER_GAME(version1,region2,serverPos)와 Village 내 다른 플레이어 snapshot 전달.
4. client GameInstance/Account가 동일한 계약 검증 후 LevelFlow에 pending 지역 등록.
   기존 LoadingMap을 열고, 완료 후 whitelist의 VillageMap으로 진입한다.
5. 기존 S1MainGameMode의 완료 흐름으로 pending player/loot를 flush한다.
   Main phase 명칭은 현재 gameplay-ready 단계 의미로 보존하여 무관한 UI refactor를 피한다.
6. 연결 종료/재접속 때 region pending과 이전 world registry/예측 상태를 정리한다.

입장 중에는 village 몬스터 피해가 없고, 로컬 pawn은 준비 전 입력을 보내지 않도록
기존 readiness 경로를 유지한다. 초기 입장 snapshot 보류는 기존 기능을 재사용한다.
미완성 상태를 감추는 클라이언트-only 순간이동/몬스터 숨김은 하지 않는다.

## 서버 데이터 / NPC / 아이템

- 별도 trusted village 배치 데이터에서 PlayerStart 위치·Yaw와 capsule 중심 기준을 검증.
  최신 저장 VillageMap을 읽어 추출하며, 이전 조사 좌표를 무검증으로 고정하지 않는다.
- NPC 기존 Export를 VillageMap/room2까지 whitelist 확장. NPCID/퀘스트ID/보상은 유지.
- Quest accept/turnin에 NPC room/map과 실제 플레이어Room 일치 검사를 추가한다.
- Village 장애물 Export/로드는 별도 VillageMap.lootobb로 분리한다.
  MainMap 장애물을 마을 시야 검증에 사용하거나 장애물 검사 자체를 생략하지 않는다.
- 인벤토리/포션/퀘스트/필드드랍의 기존 기능을 보존하고 room_id를 실제 Room으로 전달한다.
  마을 필드드랍 높이는 고정 전투평지Z가 아니라 서버가 보유한 플레이어 capsule-foot 기준으로
  처리할 필요가 있다. 이것은 지형 Export/몬스터 경사 대응을 구현했다는 뜻이 아니다.
- 기존 MainMap Nav/spawn/lootobb는 덮어쓰지 않는다. Village Nav 진단 파일도 서버Combat에 배포하지 않는다.
- 초기 마을 입장만 구현하므로 기존 전투지역은 보존하지만 일반 로그인에서 접근할 포털은 없다.
  전투 회귀는 기존 오프라인 테스트로 유지하고 실제 왕복 플레이는 후속 과제다.

## 변경 예정 파일 (실재 경로 확인)

Client root E:/Git Fork/S1_UE5.8:
- Source/S1/Account/S1AccountSubsystem.cpp: 입장 지원버전/응답 검증.
- Source/S1/S1GameInstance.cpp: 계약검증 후 session/spawn 초기화 순서.
- Source/S1/Level/S1LevelFlowSubsystem.h/.cpp: pending region과 whitelist 목적지.
- Source/S1/Quest/S1QuestNpc.cpp: saved VillageMap NPC Export.
- Config/DefaultGame.ini: VillageMap MapsToCook 추가, 기존3맵 유지.
- Plugins/S1NavigationTools/Source/S1NavigationTools/Private/S1ExportNavigationCommandlet.cpp:
  두 map의 장애물 Export whitelist/header 분리.
- 신규 Source/S1/Level/S1RegionContract.h 및 계약 자동검사 파일.
- 필요시 saved VillageMap의 spawn marker 비활성(본 맵만 선택 저장; dirty/체크포인트 선행).
- Network/Protocol.pb.h/.cc 등 재생성 산출물, Docs/CurrentAssetRegistry 및 구현 기록.

Server root E:/Git Fork/GameServer:
- GameServer/Common/Protobuf/bin/Protocol.proto 및 양쪽 생성물.
- GameServer/GameServer/Room.h/.cpp: immutable region/ID, Village 무몬스터/지역별 데이터.
- GameServer/GameServer/ClientPacketHandler.cpp: 버전검사/초기Village선택/실제Room 퇴장.
- GameServer/GameServer/GameServer.cpp: 두 Room의 기존 service 연결.
- GameServer/GameServer/RoomQuest.cpp 및 QuestDefinitionConfig.h: 지역 일치/whitelist.
- 신규 지역정의·trusted spawn 설정/테스트와 해당 프로젝트 빌드 목록.
- GameServer/GameServer/Data, Binary/Debug/Data/Config의 승인된 Village 배치/장애물/퀘스트 데이터.
- 기존 LootService/DB 함수는 실제 room 인자를 지원하므로 Room 호출부에서 맞춰 전달.
  추가 변경이 필요하면 실제 diff에 명시한다. DB migration은 예정하지 않는다.

## 검증 계획 (미실행)

1. whitelist/unknownregion/version mismatch/상태 초기화 자동검사.
2. Village Room은 몬스터 snapshot0, Combat 기존43검사 회귀, 실제Room 퇴장/격리 검사.
3. NPC 다른Room 거부/동일Room 거리·시야 검사, room별 drop/potion 입력 검증.
4. 서버Debug·S1Editor/S1 Development 빌드, 의도한 BP/맵 선택 검사.
5.1인 실제 로그인→Loading→Village(serverPos), NPC대화/수락/기존완료퀘스트 보상 상태 확인.
6.2인 접속/이동/상대표현/퇴장/재접속, 마을에 Dusk 없음, 인벤토리/포션/드랍 회귀.
7.Village 포함 Windows Development Cook/패키지 및2계정 확인은 별도 실제 실행으로 구분.

현시점 서버·에디터 프로세스는 조회되지 않았고 MCP8000은 연결 거부였다.
빌드 잠금은 현재 없지만 최신 저장맵 좌표/dirty 확인은 에디터 재연결 또는
저장맵 commandlet 조사로 확인해야 한다. 실행하지 않은 검증을 성공으로 보고하지 않는다.

## 2026-10-21 구현 및 최종 검사

### 구현

- Account 요청 version1 → Network 응답 선검증 → GameInstance spawn/session 준비 →
  LevelFlow의 기존 LoadingMap → region whitelist VillageMap. 계정 delegate 호출 순서에
  따라 검증 전에 travel하는 경로를 제거했다. 연결 해제 시 pending 지역/엔티티를 초기화한다.
- 비호환 응답의 dispatch 안에서 Disconnect할 때 PacketSession이 먼저 파괴되지 않도록
  현재 dispatch의 shared ownership을 유지한다.
- 서버는 초기/재접속 캐릭터를 immutable region2 GVillageRoom에 등록한다. DB 저장된
  옛 MainMap 좌표가 아닌 saved PlayerStart `(6840,1880,192), Yaw -180`을 사용한다.
- Village에는 Dusk/Nav/spawn 초기화를 실행하지 않는다. 기존 Combat GRoom과
  MainMap.nav/spawn/lootobb는 보존했다. 퀘스트/아이템 비동기 완료를 위한 Room tick은 유지한다.
- 퇴장/획득/포션/필드 버리기/기여 처치 기록은 실제 소속 Room ID를 사용한다.
  NPC 수락/보상 검사에는 NPC room/map과 실제 Room 일치 조건을 추가했다.
- 저장된 NPC1은 region2, `(5830,1760,100)`, radius250, vertical100, sightHeight100이다.
  commandlet의 미등록 컴포넌트 transform을 갱신하지 않아 최초 추출이 `(0,0,0)`으로
  나오던 문제를 발견·수정했다. 잘못된 파일은 서버에 배포하지 않았다.
- Village 장애물은 별도 `VillageMap.lootobb`4604개다. 단순 충돌을 쓰는 메시에는
  각 simple collision shape의 보수적 bounds를 사용한다. render 전체 bounds가 NPC 주변
  빈 공간까지 막던 첫 추출은 폐기했다. complex/미지원 simple 집합은 전체 bounds로
  보수적으로 유지하고 Landscape는 bounds로 처리한다. 정확한 삼각형 충돌/지형 높이/경사 대응이 아니다.
  Village만8192개 제한, MainMap은4096개 제한을 유지했다.
- PlayerStart/NPC/퀘스트 DT는 저장된 상태만 추출한다. 원본 MainMap/VillageMap/vendor
  맵을 이 단계에서 저장하거나 변경하지 않았다. VillageMap은 checkpoint와 SHA256 동일.
- `.proto` 변경 후 기존 생성기 및 escape 정리 스크립트로 양쪽 `.pb.*`를 생성했다.
  별도 수동 generated 파일 수정/새 패킷ID/DB migration 없음. AbilityId11002 등 콘텐츠 규칙 유지.

### 실제 변경 파일 (기존 변경들과 구분)

Client: 위 변경 예정 목록 외에 `Source/S1/Quest/S1QuestNpc.h`,
`Source/S1/Network/S1NetworkSubsystem.cpp`, `Source/S1/Level/S1RegionContractTests.cpp`,
`Plugins/S1NavigationTools/Source/S1NavigationTools/S1NavigationTools.Build.cs`를 변경했다.
에디터 plugin의 Landscape/PhysicsCore/S1 의존성은 Editor 모듈에만 추가했다.
Blueprint/맵 바이너리 저장은0개다.

Server: 위 목록 외에 `GameServer/GameServer/RegionDefinition.h`,
`GameServer/GameServer/LootObstacles.h`, `GameServer/GameServer/GameServer.vcxproj`,
`GameServer/Tests/VillageEntryDefinitionTests.cpp`, `QuestDefinitionConfigTests.cpp`,
`RoomMultiMonsterTests.cpp`, `BuildVillageEntryTests.cmd`를 변경/추가했다.
WorldData 배포는 Village 장애물을 별도 복사하며 기존 Config 정의는 빌드로 자동 덮어쓰지 않는다.
새 배치/장애물/퀘스트 파일은 source Data와 Debug 실행 Data/Config에 명시적으로 복사하고
각 SHA256 일치를 확인했다. 기존 MainMap 데이터를 Village Nav로 교체하지 않았다.

### 실제 실행한 검증

- Debug GameServer 최종 빌드 성공. 최종 pass는 이미 생성한 protocol을 유지하여
  PreBuildEventUseInBuild=false로 실행했다. 앞선 정상 full pass에서 생성기도 실행했다.
- S1Editor Win64 Development 최종 빌드 성공(-DisableAdaptiveUnity).
  S1 Win64 Development 게임 타깃도 성공. 게임 타깃은 이후 test-only 및 Editor exporter
  수정 전 실행했으며 runtime 로직 변경은 없었다. full Cook/Stage/Package 성공을 의미하지 않는다.
- Unreal 자동검사39개:38 Success +1 Success with warning, 실패0.
  `Saved/Automation/VillageEntry_Final_20261021/index.json` 및
  `Saved/Logs/VillageEntryAutomation_Final_20261021.log`.
  경고1은 기존 KnightChargePresentation의 GameplayCueNotifyPaths fallback이다.
  최초 테스트의 잘못된 Subsystem Outer로 발생한 Ensure는 fixture를 GameInstance Outer로 수정하고
  최종 재실행에서 제거했다. 최초 실패 로그는 숨기지 않고 별도 보존했다.
- Village 배치·지역별 장애물 검사16개/실패0. 저장 NPC에 인접한150cm 네 방향의
  Z200 직선 시야4/4 clear. 이것은 마을 전체 LOS/보행/사용자 대화 검증이 아니다.
- QuestDefinitionConfig32개/실패0. 실제 Village quest export를 포함한다.
- 실제 Room multi-monster 회귀49개/실패0(기존 전투43+지역 격리6).
  disconnected offline sessions로 실행했으며 DB/E2E/PIE 검사가 아니다.
- commandlet 최종 export exit0, errors0/warnings0:
  `Saved/Logs/VillageEntryExport_Final_20261021.log`.
- 새 GameServer 시작: DB Account SELECT 성공, loot Ready1, field drop Supported1,
  potion Supported1, Quest schema/reward readiness1. TCP7777 접속 확인.
  `GameServer/Binary/Debug/Logs/VillageEntry_20261021.stdout.log`.
- 새 Editor 실행, MCP initialize/list_toolsets 정상, current level LobbyMap, PIE=false 확인.

### 체크포인트 및 미검증

`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/VillageEntry_20261021`:
이전 source/config/protocol/저장 VillageMap의 **부분** checkpoint다. 이전 서버 executable,
전체 Content 및 DB 복원 보장을 뜻하지 않는다. source Quest 및 vcxproj도 추가 보존했다.
Git 기존 staging은 조작하지 않았으며 commit/push도 하지 않았다.

사용자가 최신1-/2인 PIE의 로그인→Loading→Village, 서로 표시/이동, NPC 대화,
무몬스터, 퇴장/재접속, inventory/potion/fielddrop를 전체 확인했고 문제없다고 보고했다.
재접속 시 마지막 위치가 아닌 지정 PlayerStart로 입장하는 동작도 현재 의도된 정책이라고 설명했다.
이는 사용자 수동 확인이며 에이전트가 같은 플레이를 독립 실행했다는 뜻이 아니다.
새 패키지 Cook/Stage/Archive와 두 패키지 계정 검증은 아래 진행 기록과 구분한다.
전투지역 진입 포털이 없으므로 일반 로그인에서 Dusk 퀘스트의 새 처치를 진행할 수 없으며,
현재 마을 테스트가 전투 왕복/전체 맵 LOS/지형 권위/모든 퀘스트 회귀의 완료를 의미하지 않는다.
기존 ABP_S1Terra 경고 수정은 여전히 별도 후순위다.

### Windows Development 패키지 진행 기록

사용자 승인 후 BuildCookRun(build/cook/stage/pak/iostore/package/archive)을 실행했다.
기존 패키지는 보존하고 새 Archive는 `Saved/VillagePackage_20261021`,
새 staging은 `Saved/VillageStage_20261021`로 지정했다.
사전 MCP: LobbyMap, PIE=false, VillageMap clean. 서버는 기존 새 버전으로 유지했다.

첫 시도는 UBT가 열린 에디터의 Live Coding을 감지하여 ExitCode6으로 중단했다.
코드 컴파일 오류/쿠킹 실패로 분류하지 않는다. Cook/Stage/Package에는 도달하지 않았다.
로그: `Saved/Logs/VillagePackageBuild_20261021.log`.
Unreal Editor 및 연결된 디버거 저장·종료를 사용자에게 요청했으며, 종료 확인 후 재시도한다.
패키지 성공/새 패키지2계정 검증 완료를 아직 주장하지 않는다.

#### 에디터 종료 후 재시도 결과

사용자 종료 확인 후 실제 프로세스에서도 UnrealEditor 미실행을 확인하고 같은 명령을 재실행했다.
`Saved/Logs/VillagePackageBuild_Retry_20261021.log`:

- S1/S1Editor Development 빌드 성공, UBT ExitCode0.
- Cook1597개/remaining0, commandlet0 errors/0 warnings, ExitCode0.
  새 마을 SM5/SM6 및 품질별 셰이더의 최초 생성으로 Cook 약864초 소요.
- Stage/Pak/IoStore/Package/Archive 완료, `BUILD SUCCESSFUL`, AutomationTool ExitCode0.
  전체 약17분46초. 첫 LiveCoding 실패 기록은 삭제하지 않았다.
- DevelopmentAssetRegistry.bin에서 LobbyMap/LoadingMap/MainMap/VillageMap 네 이름 확인.
  패키지 `S1-Windows.ucas`2034107968 bytes와 utoc/pak/launcher 생성 확인.
- 새 실행 파일: `Saved/VillagePackage_20261021/Windows/S1.exe`.
  기존 패키지 폴더는 삭제/교체하지 않았다. 새 staging은 별도 폴더다.
- 새 패키지 두 창 실행. 각각의
  `Saved/Logs/VillagePackage_Client1_20261021.log`,
  `Saved/Logs/VillagePackage_Client2_20261021.log`에서 LobbyMap 로드/BeginPlay 확인.
  이 초기 확인 구간에 Error/Fatal/Ensure/Blueprint Runtime Error/NetworkFailure 미발견.
  optional aqProf/VTune/PIX 및 기본 SoundConcurrency 미지정 메시지는 별도 baseline이다.
- 서버12284는 계속 유지했다. DB/프로토콜/코드/맵 바이너리 추가 변경 없음.
- 최초 자동 실행은 숨김 옵션으로 창이 표시되지 않아, 이 작업에서 생성한 launcher/game
  PID4개만 경로 확인 후 종료했다(모두 Lobby 단계). 사용자가 직접 조작하는 검증용으로
  실제 archived game exe를 Normal window로 다시2개 실행했다. 최종 검사 로그는
  `Saved/Logs/VillagePackage_VisibleClient1_20261021.log` 및 `VisibleClient2`다.

최신 두 패키지 계정의 로그인→Loading→Village 및 이동/공격/Q/NPC/inventory/potion/
fielddrop/퇴장/재접속은 사용자에게 확인 요청했으며 아직 결과 대기 중이다.
Lobby 시작 성공이나 이전 PIE 인수로 새 패키지 gameplay 인수를 대체하지 않는다.

#### 사용자 패키지 검증에서 발견한 HUD 누락 및 수정

사용자는 요청한 두 계정 항목을 확인했으나 Main HUD가 보이지 않는다고 보고했다.
최초 `VillagePackage_VisibleClient1/2_20261021.log`는 실제 LoadingMap→VillageMap 입장을
보여주며 두 로그 모두 WBP_S1MainHUD 및 WBP_S1QuestTracker 로드 실패가 있다.
Cooked registry를 UI별로 대조하니 MainHUD/Minimap/Currency/QuestTracker/QuestJournal이
미포함이었다. 따라서 최초 패키지는 빌드 성공이지만 HUD 포함 기능 검증 성공본이 아니다.

원인: Inventory/Quest subsystem의 네이티브 기본값에 넣은 TSoftClassPtr 경로만으로는
이 쿠킹 경로에서 필수 UI가 수집되지 않았다. 에디터 LoadClass 검사만으로 검출되지 않은 문제다.
기존 inventory/quick-slot와 같은 FClassFinder + reflected TSubclassOf의 명시적 강한
클래스 참조로 MainHUD/Tracker/Journal을 한정 수정했다. MainHUD의 child widget 의존성을
통해 Minimap/Currency도 포함되도록 하며 전체 UI 디렉터리 강제 cook은 추가하지 않았다.

변경 파일:
- Source/S1/Loot/S1InventorySubsystem.h/.cpp
- Source/S1/Quest/S1QuestSubsystem.h/.cpp
- 신규 Source/S1/UI/S1UICookReferenceTests.cpp

UI 디자인/Blueprint/맵 바이너리/서버/DB/프로토콜은 이 수정으로 변경하지 않는다.
세 reflected hard class property와 native default의 정확한 WBP 경로를 검사하는
S1.UI.RequiredCookReferences를 추가했다. 실제 재실행한 자동검사40개는
39 Success+기존 GameplayCue path 경고1, 실패0이다.
`Saved/Automation/VillageHUDCookFix_20261021/index.json`과
`Saved/Logs/VillageHUDCookFixAutomation_20261021.log`가 증거다.

수정본의 S1/S1Editor 빌드 성공 후 별도 `Saved/VillagePackage_HUDFix_20261021`로
재패키징했다. 이전 실패 증거/패키지는 보존했다.

최종 결과: Cook1605 packages/remaining0,0 errors/0 warnings, Stage/Pak/IoStore/
Package/Archive 성공, `BUILD SUCCESSFUL`/AutomationTool ExitCode0. 전체 약4분59초.
`Saved/Logs/VillagePackageBuild_HUDFix_20261021.log`가 실제 결과다.
최종 DevelopmentAssetRegistry에서 MainHUD/Minimap/Currency/QuestTracker/QuestJournal
다섯 WBP가 모두 포함됐음을 확인했다. 기존 패키지에서 이 다섯 개는 모두 없었다.
unused WBP_S1InventoryStackRow 등 다른 UI를 일괄 cook 대상으로 추가하지 않았다.

새 패키지 실행 파일은 `Saved/VillagePackage_HUDFix_20261021/Windows/S1.exe`다.
이 수정본 실제 game exe로 테스트 창2개를 실행했으며 로그는
`Saved/Logs/VillageHUDPackage_Client1_20261021.log` 및 Client2다.
양쪽 Main HUD/퀘스트 O창/대화 중 숨김·종료 후 복귀(진행 퀘스트가 있을 때 진행도 포함)의
실제 시각 재검증 요청에 사용자가 이 수정본에서 "확인 완료"라고 답변했다.
이는 최신 수정 패키지의 사용자 수동 시각 인수이며 자동 화면 판독 검증은 아니다.
최종 로그 재확인에서도 두 클라이언트 모두 Lobby→Loading→Village 전환이 확인됐으며
HUD/WBP 누락, Fatal, Ensure, Blueprint Runtime Error, NetworkFailure는 발견되지 않았다.
Client1에는 마을 입장 전 Login failed Result3(자격 증명 불일치)3건이 있으며 이후 정상 입장했다.
실제 입력값은 기록/인용하지 않는다. 이 설명된 로그인 거절을 HUD/네트워크 실패로 분류하거나
전체 로그가 Error0이라고 보고하지 않는다. Client2의 해당 오류 검색 결과는0건이다.
최초 패키지의 나머지 기능 검증 보고와 수정본의 HUD 집중 재검증은 별도 증거로 유지한다.
