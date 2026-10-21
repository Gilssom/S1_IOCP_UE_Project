# Elderboom → MainMap 교체 전 조사 / 부분 체크포인트

## 작업용 맵 현재 상태 점검 — 2026-10-21 11:26 KST

사용자가 /Game/Maps/MainMap_Test 복제본을 생성·저장한 뒤 점검 요청.
MCP에서 해당 맵이 열려 있고 PIE=false, MainMap_Test/MainMap/원본 마을 모두 clean 확인.
기존 MainMap 및 원본 마을 맵 SHA는 위 체크포인트와 동일, Config tracked diff 없음.
defaultGameMode=/Script/S1.S1MainGameMode 적용. NPC1개(ID1, DT 연결, preview=false),
Dusk 스폰1개(SpawnPointId1/DefinitionId1/enabled=true), PlayerStart1개.
NPC 좌표(5830,1760,100), 스폰 좌표(4810,2440,100).
Navigation=false, NavMeshBoundsVolume0. 현재 spawn exporter는 저장된 MainMap 경로만 허용하고
feetZ=-0.5/저장 Recast Nav를 요구하므로, MainMap_Test 및 현재 스폰Z100은 배포 준비 완료가 아니다.
바닥을 확인하지 않고 스폰만 -0.5로 내리면 안 된다.

최근 사용자 PIE 로그11:23:05~11:23:29에서 두 월드 시작/종료와 S1MainGameMode 확인.
Fatal/Ensure 조건 실패는 이 구간0. 다만 각 PIE 시작에 ABP_S1Terra의
`Divide by zero: Divide_DoubleDouble` 경고가 각각1회(총2회) 발생. 원인 노드는 미조사/미수정.
기존 GameplayCueNotifyPaths 경고도 별도 존재. 이후 MCP property schema의 delegate 경고는
조사 도구의 직렬화 한계이며 이 PIE 구간 경고와 구분한다.
이 구간의 S_ENTER_GAME/S_SPAWN 수신 등 서버 입장 증거는 발견0이므로
두 로컬 PIE 월드 실행을 인증된2계정 서버 통합 성공으로 보고하지 않는다.
에이전트는 새 PIE/빌드/저장/서버 데이터 배포를 실행하지 않았다.
다음 우선순위: AnimBP0나눗셈 원인 조사, 전투 평지 높이/위치 확정,
Nav구성 및 실제 MainMap 적용 후 데이터 Export/배포/로그인2인 검증.

## 승인 범위와 실제 수행

사용자 승인: MCP 연결 확인, 백업 및 조사. 맵 교체·설정 변경·서버 배포는 아직 하지 않았다.
MCP8000 연결 성공. 실행 프로세스 명령행으로 S1.uproject/UE5.8 에디터를 확인했다.
열린 맵은 /Game/ElderboomVillage/Maps/L_ElderboomVillage, PIE=false.
기존 MainMap과 마을 맵 모두 조사 전후 is_dirty=false. 어느 에셋도 저장하지 않았다.
기존 Git 변경(에셋 인수 기록 및 Unity helper 수정)을 그대로 보존했다.

## 백업

`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MainMapVillage_Preflight_20261021_104856`

25개 파일, 약21.25MiB. 각 복사에서 source→copy SHA256 일치 및 복사 중 source 불변 확인.
README.md에 상대 경로·용량·해시 목록을 보존했다.

- S1 MainMap, 이관한 L_ElderboomVillage 맵 패키지.
- S1 .uproject, DefaultEngine.ini/DefaultGame.ini.
- BP_S1DuskSpawnPoint/BP_S1QuestNpc/BP_S1DuskMinion.
- S1MainGameMode.cpp/S1LevelFlowSubsystem.cpp.
- 서버 소스 Data의 MainMap.nav/spawn/lootobb 및 몬스터·퀘스트 정의.
- 실제 EXE 인접 Binary/Debug/Data의 MainMap.nav/spawn/lootobb.
- 실제 EXE 인접 Config의 몬스터·퀘스트 정의.
- Room.cpp/RoomQuest.cpp/DuskDefinition.h/MonsterSpawnConfig.h/QuestDefinitionConfig.h.

기존 MainMap SHA256:
681571215312D1A2196A6227E8746F56C4C1642FCA4514C32FC116204CD66CA0.
마을 맵 SHA256:
E8589D19A1AD5962763ED0099BD80BC42726D2F6FFBA381B433DB074BA13C684.

이것은 전체6.37GiB 환경 콘텐츠·플레이어/UI·DB·서버EXE를 복구하는 완전 백업이 아니다.
제한된 교체 대상의 디스크 체크포인트이며 복원 시험은 미실행이다.
몬스터 정의의 source seed(104bytes)와 실제 배포 Config(171bytes)는 다르며 둘 다 보존했다.
복구 시 seed를 배포 Config에 무조건 덮어쓰면 안 된다.

## MCP 조사 결과

| 항목 | 이관된 마을 맵에서 확인한 값 |
|---|---|
| defaultGameMode | None (S1 프로젝트 전역 Lobby GameMode를 그대로 상속하면 안 됨) |
| bEnableNavigationSystem | false |
| NavMeshBoundsVolume | 검색0 |
| worldPartition / WorldComposition | None / false |
| PlayerStart | 1개, (6840,1880,192), Yaw 약-180° |
| Landscape | 1개, bounds X/Y 약-5040..10178, Z 약69.5..2946.6 |
| 배치된 Pawn / CameraActor | 각각 검색0 |
| S1QuestNpc / S1MonsterSpawnPoint | 각각 검색0 |
| LevelScriptActor | 검색0; Blueprint graph 직접 검사는 아님 |

AssetTools로 두 맵의 직접 dependencies/referencers를 조회했다. 직접 referencers는 두 맵 모두0.
이는 문자열·설정·코드 참조가0이라는 뜻이 아니다. 기존 LevelFlow/MapsToCook는 MainMap을 사용한다.
마을 맵 dependencies는 Engine/NavigationSystem/Foliage/Landscape/Niagara 및
/Game/ElderboomVillage의 환경 콘텐츠이며, 조회 목록에 FirstPerson 게임플레이 BP는 없다.
현재 S1.log에서 조회한 Material/Streaming/Linker Error·Warning 및 Fatal/Ensure 패턴은 발견0.
이는 전체 로드/Blueprint 컴파일/충돌/성능/PIE 성공 판정이 아니다.
수직 trace3건은 첫 차단면까지의 거리만 반환하므로 지형 바닥 높이 검증으로 사용하지 않았다.

## 교체 전에 필요한 최소 작업 / 경계

1. 원본 L_ElderboomVillage 보존, 작업용 맵에 AS1MainGameMode 명시.
   LobbyMap/LoadingMap/BP_GameInstance/기존 입력·전투 계약은 그대로 유지한다.
2. NPC·Dusk·플레이어 위치를 새 공간에 맞춰 구성한다. 기존 MainMap의 배치와
   Nav Bounds를 무조건 원래 좌표로 복사하는 것은 안전하지 않다.
3. 현재 서버 NPC는 (-350,250,-0.5), Dusk 스폰3개는 발Z=-0.5 기준이다.
   마을 Landscape는 평지가 아니고 PlayerStart도 기존 좌표와 다르다.
   기존 서버 데이터로 바로 전투하면 바닥 높이·NPC 거리·길찾기가 불일치한다.
4. 서버 몬스터/드랍은 평지 기준이며 경사 대응을 이번 조사로 구현하지 않았다.
   동일 높이의 작은 전투 영역을 정하거나, 별도로 높이 확장 승인을 받아야 한다.
5. 새 MainMap 저장 후 Nav/spawn/lootobb 및 서버 NPC 정의를 재Export/검증/배포한다.
   현재 서버는 실행 중이지만 조사 중 파일을 교체하거나 재시작하지 않았다.
6. 교체 이후1-/2인 PIE와 패키지 검증을 별도로 해야 한다. 기존 인수는 새 맵 인수가 아니다.

다음 구현의 예상 변경 대상은 작업용 /Game/Maps/MainMap, 해당 맵의 S1 NPC/스폰 배치,
Nav 설정과 Export 산출물/서버 배포 데이터다. 원본 환경/Config는 임의 수정하지 않는다.
명시한 파일·콘텐츠 범위를 확정한 뒤 교체를 진행한다. 이번 단계에서는 모두 미변경이다.

## 미검증

- Level Blueprint 내부 그래프 직접 조사(배치 LevelScriptActor0만 확인).
- 마을 전체의 보행 collision, 플레이어/몹 접지, 성능, 시각 품질, 모든 간접 의존성의 로드.
- 새 맵 기준 서버 전투/아이템/퀘스트/재접속, 패키징 및2계정 플레이.
- Fab 에셋 공개 포트폴리오 사용 권한의 사용자 확인 기록.
