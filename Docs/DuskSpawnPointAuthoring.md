# Dusk 스폰 포인트 에디터 배치 — 2026-10-06

최신 후속 범위: 사용자 승인으로 [MultiDuskSpawnImplementation.md](MultiDuskSpawnImplementation.md)의 다중 포인트/개체별 서버 관리로 확장했다. 아래 단일ID1/v1 설명은 최초 구현 이력이다. 현재 메뉴는 Export Monster Spawn Points, 새 Export는 v2/고유 양수ID/1..16포인트이고 등록 종류는 Dusk1뿐이다. Export→별도배포→새서버재시작 순서는 유지한다. 실제 다중 PIE 인수 상태는 후속 문서를 따른다.

## 승인 범위와 상태

사용자가 스폰 포인트 BP 구현을 승인했다. 소스/BP 배치/저장/Export/배포 구현과 빌드 확인 완료, 실제1-/2-client PIE 인수는 대기다. Dusk 1종, MainMap persistent level의 마커 정확히 1개만 지원한다. 다중 몬스터·경사·동적 장애물·DB/패킷 변경·UI 전환은 제외한다.

## 책임과 좌표

AS1MonsterSpawnPoint는 배치/Export용 editor-only 마커다. BeginPlay에서 몬스터를 생성하거나 클라이언트가 서버 생성 요청을 보내지 않는다. 외부 IOCP Room이 생성·추적 제한·복귀·재생성을 계속 결정한다. SpawnPointId=1은 배치 식별자이고 Loot의 매 생명 UUID spawn_id와 별개다.

마커 원점은 발/바닥이다. 현재 평지 계약은 Z=-0.5cm, 스케일1, Pitch/Roll0이며 Yaw만 설정한다. Dusk 캡슐28×62, 스케일0.7, 메시Yaw+90°와 서버 중심Z61.5는 유지한다. 초기 마커 위치는 기존 서버와 동일한 (0,600,-0.5), Yaw=-90°. 높이와 스케일을 임의 조정하는 기능이 아니다.

## 편집 흐름

1. MainMap의 BP_S1DuskSpawnPoint 마커 위치/방향 및 Enabled를 편집한다. 정확히 한 개를 유지한다.
2. 의도한 MainMap만 저장한다. 지형/벽을 바꾼 경우 기존 Nav/OBB 재Export가 별도로 필요하다.
3. Tools → S1 Server Authoring → Export Dusk Spawn Point. 저장하지 않은 dirty MainMap, 중복/누락 마커, 평지 계약 위반, saved Nav 밖 위치는 거부한다. Export는 맵을 자동 저장하지 않는다.
4. 성공 출력은 Saved/ServerData/MainMap.spawn. 서버 종료 후 Scripts/DeployDuskSpawnPoint.ps1을 실행해 canonical/Debug/Release Data 위치에 배포한다. 기존 배포본은 timestamp 백업하고 SHA256 일치를 확인한다.
5. 서버를 재시작한다. 실행 중 hot reload는 없다. 스폰뿐 아니라 추적 home 반경·복귀·재생성도 같은 읽은 좌표를 사용한다.

Commandlet 대안: UnrealEditor-Cmd.exe S1.uproject -run=S1ExportSpawnPoints -Map=/Game/Maps/MainMap -Output=<경로> -unattended -NullRHI. 저장된 MainMap만 읽고 원본 맵/애셋은 저장하지 않는다.

## 파일 계약

```text
S1SPAWN 1 MainMap
POINTS 1
P 1 1 1 0.000000 600.000000 -0.500000 -90.000000
```

P 필드: 배치ID, MonsterDefinitionId, Enabled(0/1), feet X/Y/Z(cm), actor Yaw(deg). 외부 서버 loader는 버전·맵·정확한 개수·ID·finite/bounds·평지 높이·후행 데이터 검증 후 반영한다. missing/invalid export는 Dusk 생성 비활성; 오래된 고정 좌표 fallback 없음. disabled는 정상 설정이며 재생성도 비활성이다. Nav/OBB 버전 연결 검증과 공격 로직 변경은 이번 범위가 아니다. Export의 saved Nav 투영 검사는 서버의 지형 지원 또는 모든 벽 관통 판정 완결을 뜻하지 않는다.

## 변경 대상

- Source/S1/Public/Game/S1MonsterSpawnPoint.h, Source/S1/Game/S1MonsterSpawnPoint.cpp
- Plugins/S1NavigationTools의 S1ExportSpawnPointsCommandlet.h/.cpp, S1NavigationTools.Build.cs, S1NavigationToolsModule.cpp
- GameServer/GameServer/GameServer/MonsterSpawnConfig.h, Room.h/.cpp 및 Tests/MonsterSpawnConfigTests.cpp
- Scripts/DeployDuskSpawnPoint.ps1, 이 문서, CurrentAssetRegistry.md
- 예정 애셋 /Game/S1/Editor/BP_S1DuskSpawnPoint, /Game/Maps/MainMap 및 Export/배포 .spawn 파일

체크포인트: C:\Users\user\Documents\Unreal IOCP Server\checkpoints\DuskSpawnPoint_20261006. 변경 전 Room.h/.cpp, MainMap.umap, plugin Build.cs 복사본 SHA256 일치 확인. 기존 dirty 서버 변경은 보존했다. 바이너리 저장 직전 dirty 여부를 조회·보고하고 해당 BP/맵만 저장해야 한다.

## 검증 기록과 남은 인수

### 후속 위치 배포 — 2026-10-06

사용자가 위치 이동 후 Export Complete와 서버 재시작에도 위치가 변하지 않는다고 보고했다. 읽기 전용 대조에서 에디터 Export는 feet (904.402783,15.172481,-0.5)/yaw-90이고 canonical/Debug/Release Data는 초기 (0,600,-0.5)인 배포 누락을 확인했다. 실행 프로세스는 새 Debug GameServer.exe였다. 사용자 서버 종료 확인 후 DeployDuskSpawnPoint.ps1로 배포했다. 기존 각 배포본은 스크립트의 timestamp 백업으로 보존했다. 네 파일의 SHA256은 E94F2FA0B146E6BD3DDF07E5334C96FC071319C4157EB2611A01EC306C17A247로 일치하며 새 좌표를 readback 확인했다. 서버 실행·새 위치 실제 PIE 검증은 이번 배포 턴에서 수행하지 않았다. 아래 초기 hash/좌표 기록은 초기 구현 검증 이력이고 최신 배포값은 이 문단이 우선한다. 적용 확인에는 새 Debug 서버 재시작이 필요하며 Release 바이너리는 여전히 이번 기능 새 빌드 미검증이다.

서버 Debug, S1Editor 및 S1 Win64 Development 빌드 exit0 성공. 실제 C++ 스폰 parser 테스트16개/실패0 (ParserTests.log): 정상/disabled, ID·definition·enabled·높이·finite·범위·yaw·중복·버전·맵·개수·oversize·missing 거부 및 실제 배포된 Debug Data/MainMap.spawn 로드/초기값 일치 확인. 에디터 첫 빌드는 공개 헤더 경로 오류로 실패했고 공개 헤더 위치를 수정해 최종 빌드는 성공했다. 초기 sandbox 빌드는 진행되지 않아 중단하고 정규 Windows 환경에서 빌드했다. 실패 로그는 보존한다.

BP 생성 및 warnings-as-errors 컴파일 호출 성공, parent/generated class 확인. 배치 instance의 ID1/Dusk1/Enabled/editor-only 및 transform을 MCP readback 확인했다. 저장 전 BP/MainMap dirty=true, 기존 BP_MyPlayer/BP_S1DuskMinion dirty=false 보고 후 BP/MainMap만 save_assets=true. Commandlet은 새 프로세스로 저장본을 다시 로드해 exact 초기 위치를 Export했다. 최초 시도는 열린 에디터 MCP port8000과 충돌해 출력 생성에도 불구하고 exit1이었다; 프로세스 인자 -ModelContextProtocolPort=18001로 재시도한 SpawnExportFinal.log는 exit0, 0 errors/1 기존 MCP EULA warning이다. 프로젝트 MCP 설정은 변경하지 않았다. canonical/Debug/Release Data 배포 SHA256은 모두 AFEB833AD0F7FDE51B6483A15838BA59F05F18485DC5962E198EE44A5EA4C858이다. Nav/OBB는 수정하지 않았다.

미완료: 누락/중복/dirty/Nav 밖 에디터 거부 분기의 실제 실행, 메뉴 클릭 UI 검증, Release 서버 새 빌드, 패키징 editor-only stripping, 실제1-/2-client 변경 위치 생성·우회 추적/복귀·사망 재생성·기존 HP/포션 회귀. Release Data 복사는 Release 바이너리 갱신 증거가 아니다. 이번 턴에서 서버를 시작하거나 실제 PIE 전투를 재생하지 않았다. 이전 버전 테스트를 이번 검증으로 재사용하지 않는다.
