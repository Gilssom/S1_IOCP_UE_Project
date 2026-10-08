# 공용 스폰 포인트 / 다중 Dusk 1차 구현 — 2026-10-06

## 승인과 최신 상태

### 후속 A→B 재탐색 수정 — 2026-10-06

최신 사용자 인수: Debug 수정본 적용 후 사용자가 “검증 완료”를 보고했다. 직전 요청의 A 상실→유효B 전환 재검증을 사용자 확인 완료로 기록한다. 이 보고에 세부 다중 드랍·late join·재접속·전체 전투 회귀 목록이 별도 명시되지는 않았으므로 나머지 모든 인수까지 완료로 확대하지 않는다. 이번 인수 기록·기획 전달에서 Codex가 새 PIE/실시간 로그 감사를 실행하지 않았다.

사용자가 A 상실 후 가까운 B를 공격하지 않는 현상을 보고했고, 공격 종료 시 B를 먼저 재탐색하고 후보 없을 때만 복귀하는 수정안을 승인했다. 실제 Room.cpp에서 ATTACK 종료의 target-invalid 분기가 재탐색 없이 RETURN으로 가며, RETURN은 탐색 전에 early return하는 경로를 확인했다. 이번 수정은 Room.h/.cpp에 공통 AcquireMonsterTarget을 추가해 일반 추적 및 공격 종료에서 같은 후보 기준을 사용한다. 유효한 A는 유지, 새 후보는 탐색800cm/스폰home1200cm/생존 조건 및 기존 거리·ObjectId 우선순위 유지. 공격이 끝난 뒤에만 바꾸며 A의 예약 타격을 B에게 넘기지 않는다. 기존 공격간격2초는 유지하므로 B선택 직후 잠깐IDLE은 정상일 수 있다. RETURN 확정 뒤에는 여전히 재탐색하지 않고 home도착 후IDLE에서 다시 탐색한다. B가 몬스터 가까이에 있어도 home1200cm 밖이면 제외한다.

변경은 서버 Room.h/.cpp와 Tests/RoomMultiMonsterTests.cpp 및 문서뿐이다. Blueprint/맵/Nav/DB/패킷/클라이언트는 변경하지 않았다. 소스 checkpoint는 C:\Users\user\Documents\Unreal IOCP Server\checkpoints\DuskRetarget_20261006에 Room.h/.cpp/테스트3개 SHA256 확인 후 보존했다. 읽기 전용 MCP에서 PIE=false 확인. ClCompile exit0 및 실제 Room 오프라인 테스트 **37 checks/0fail/exit0**: 기존26+재탐색11(ATTACK종료A무효→B, cooldown/예약피해미전이, B신규공격/피해, 유효A유지, home밖B제외, RETURN유지, home도착재탐색)을 확인했다. disconnected fake세션/실제Nav 사용이며 실제2인PIE 인수가 아니다. RoomRetargetTests.log 및 GameServerDebugClCompile.log가 증거다. 사용자 디버거 종료 후 실제 GameServer 프로세스 부재를 확인하고 최종 Debug 전체 빌드/링크도 exit0 성공했다(GameServerDebugBuild.log). Release 전체 빌드도 exit0 성공했다(GameServerReleaseBuild.log). Debug 실행 파일에도 수정본이 적용됐으며 사용자 서버 재실행·현재 버전2인PIE 인수는 대기다. 아래 이전 다중 구현 빌드/패키징 성공을 이 후속 서버 수정본의 완료로 대체하지 않는다.

사용자가 1차 구현안을 승인했다. 추가 질문에 **기본 공격과 Q 모두 판정 범위 내 모든 유효 적 타격**을 선택했다. 단일 최근접 대상 제한은 사용하지 않는다. 새 몬스터 콘텐츠, 포인트당 여러 개체, 순찰/군집 회피, 동적 장애물/경사, UI 전환, DB/패킷 변경은 제외한다.

소스 구현·서버 Debug/Release·S1Editor 빌드·오프라인 네이티브/Unreal 테스트·3포인트 저장/Export/배포 및 Windows Development Build/Cook/Stage/Package/Archive까지 성공했다. 실제 인증1-/2-client PIE 및 두 패키지 계정 플레이 인수는 미실행이다.

## 에디터 설정 / 범위

- 기존 `/Game/S1/Editor/BP_S1DuskSpawnPoint`를 유지하며 공용 `AS1MonsterSpawnPoint`를 사용한다. 기존 BP를 삭제/이름 변경하지 않았다.
- SpawnPointId는 양수 고유 배치 ID, MonsterDefinitionId는 종류 ID, Enabled는 생성 여부다. 현재 등록된 종류는 Dusk1뿐이며 999 등 미등록 ID는 Export/서버 로드에서 거부한다. ID만 바꾸면 미등록 몬스터의 콘텐츠/AI가 생긴다는 뜻이 아니다.
- 한 포인트당 살아 있는 개체1개, 전체1..16포인트(기술적 상한), 초기 검증용 활성3개. Disabled 포인트도 고유 ID와 유효 배치가 필요하다.
- 바닥Z=-0.5, 스케일1, Pitch/Roll0; Yaw와 XY 편집. 서버는 등록 정의의 half-height62를 더해 중심Z61.5로 생성한다. 기존 Dusk scale0.7/캡슐28×62/메시Yaw+90 및 saved Nav/OBB를 유지한다.
- 사용자 기존 포인트1 (904.402783,15.172481)은 보존했다. 테스트용 포인트2 (904.402783,215.172481), 포인트3 (1104.402783,15.172481)을 추가했다. 셋 모두 Dusk1/Enabled, yaw-90이다. 겹침/군집 회피는 제공하지 않는다.

## 편집 → 적용

1. MainMap에서 마커를 복제하고 각각 **서로 다른 SpawnPointId**를 지정한다. 종류는 현재1만 가능하다.
2. MainMap을 의도적으로 저장한다. Tools → S1 Server Authoring → **Export Monster Spawn Points**.
3. Export는 Saved/ServerData/MainMap.spawn만 갱신한다. 서버 종료 후 `Scripts/DeployDuskSpawnPoint.ps1`을 실행한다. 기존 배포본은 timestamp 백업하고 canonical/Debug/Release SHA256 일치를 확인한다.
4. 이번 버전으로 빌드된 서버를 다시 실행한다. Export Complete는 서버 배포 완료가 아니며, 실행 중 hot reload는 없다. 데이터만 배포해도 구버전 바이너리가 갱신되는 것은 아니다.

Commandlet은 `-run=S1ExportSpawnPoints -Map=/Game/Maps/MainMap -Output=<경로>`다. 동시에 다른 에디터가 있다면 해당 프로세스만 `-ModelContextProtocolPort=18002` 등으로 분리한다. 프로젝트 MCP 설정은 바꾸지 않는다. dirty 맵, 중복/비양수ID, 등록되지 않은 종류, 17개 이상, 평지 규칙 위반, saved Nav 밖을 거부하고 성공 전 출력 파일을 교체하지 않는다.

## 오프라인 데이터 계약

```text
S1SPAWN 2 MainMap
POINTS 3
P 1 1 1 904.402783 15.172481 -0.500000 -90.000000
P 2 1 1 904.402783 215.172481 -0.500000 -90.000000
P 3 1 1 1104.402783 15.172481 -0.500000 -90.000000
```

행 필드는 배치ID/종류ID/Enabled/feetX/feetY/feetZ/actorYaw다. v2는 다중 양수ID를 지원하며 서버는 기존 v1의 단일ID1 파일도 읽는다. v1 서버는 v2를 지원하지 않으므로 새 서버+Export/배포 도구를 함께 사용한다. 이것은 Protobuf 네트워크 계약 변경이 아니다.

server loader는 전체 목록을 임시 검증한 뒤 반영한다. 실패 시 부분 생성/이전 고정 좌표 fallback 없음. duplicate/unknown ID, finite/bounds, 파일 크기16384bytes·개수16, trailing 데이터 등을 검사한다. 구 데이터 실패 시 여러 후보 경로에서 오래된 파일로 몰래 대체하지 않는다.

## 서버 구조 / 기존 규칙 유지

- Room `_monsters`는 SpawnPointId별 `MonsterRuntime`을 갖는다. 각 runtime에 home/definition/MonsterRef/generation/대상/공격ID·중복판정flag/타이밍/경로cursor/생명UUID·사망UUID가 있다. Nav graph, Room event sequence, ActionId allocator, 플레이어/아이템 시스템은 Room 공통이다.
- 몬스터 종류 정의는 server `MonsterDefinition.h`에 Dusk만 등록했다. HP200, speed220, 탐색800/home1200/상실1000, 공격준비170/접촉190/높이150/dot0.57, 피해10, 간격2000ms/모션1000ms/접촉420ms, 사망제거3000ms/재생성8000ms는 그대로다.
- 각자 추적·귀환 경로를 쓰며 다른 개체의 경로를 지우지 않는다. 죽은 target에 대한 경로만 해당 몬스터들에서 정리한다. 공격 예약은 ObjectId/ActionId/target lifeGeneration/현재 세션·Room을 확인하고 각 attack window를 한 번 처리한다. 죽은 몬스터는 자신의 attackId를 취소한다.
- 초기 및 late-join spawn snapshot은 모든 현재 개체를 담는다. 독립 사망 뒤3초 제거,8초 재생성에 새 ObjectId/생명UUID를 발급한다. 배치ID는 유지된다. 비활성 포인트는 생성/재생성하지 않는다.
- 기본 공격은 기존230cm/높이160/dot0.34/피해25 및 A–D timing/입력/승인 규칙 유지, 유효 대상 전체에 각1회. Q는 기존370/1100ms,230cm/첫360°·둘째전방180°,25/38 유지, 각 타별 유효 대상 전체에 각1회. 대상당 피해가 같고 대상 수로 나누지 않는다.
- 승인 시 존재하는 몬스터 ObjectId들을 캡처하고 접촉 시 현재 위치/상태/범위를 검사한다. 새로운 재생성 개체로 이전 예약이 넘어가지 않는다. primary 예약은 callback에서1회소비, Q는 hit mask로 중복방지한다. 사망/퇴장/재접속 시 캡처와 입력행동을 정리한다.
- 기존 Nav 경로 거리 검사를 차폐 보조로 유지한다. 직접 LOS/벽 충돌 확장을 한 것이 아니므로 모든 벽 관통 피해를 해결했다고 주장하지 않는다.
- 개체별 생명/사망UUID로 기존 LootService를 호출한다. 배치ID가 DB spawn_id/death_id를 대체하지 않는다. DB 스키마/소비/획득 정책을 바꾸지 않았으며 실제 다중 드랍 DB·PIE 인수는 남아 있다.
- Unreal은 이미 ObjectId별 몬스터 registry, pending spawn map, Source/Action/Target/HitIndex 피해키를 사용한다. 기존 Account/Network/LevelFlow/WorldEntity 경계를 유지했고 RPC/Replication/Unreal AI 권위를 추가하지 않았다.

## 변경 파일·에셋

서버 `E:\Git Fork\GameServer\GameServer`:

- GameServer/Room.h/.cpp, Player.h, MonsterSpawnConfig.h, 신규 MonsterDefinition.h
- Tests/MonsterSpawnConfigTests.cpp, 신규 RoomMultiMonsterTests.cpp/BuildMultiMonsterTests.cmd
- GameServer/Data/MainMap.spawn, Binary/Debug/Data/MainMap.spawn, Binary/Release/Data/MainMap.spawn

클라이언트 `E:\Git Fork\S1_UE5.8`:

- Source/S1/Public/Game/S1MonsterSpawnPoint.h (종류ID 편집 허용)
- Plugins/S1NavigationTools/Source/S1NavigationTools/Private/S1ExportSpawnPointsCommandlet.cpp, S1NavigationToolsModule.cpp
- Scripts/DeployDuskSpawnPoint.ps1 (v1/v2 전체 검증, ValidateOnly 지원)
- Source/S1/World/S1WorldEntitySubsystem.h (테스트 friend만 추가), 신규 Source/S1/Game/S1MultiMonsterPresentationTests.cpp
- Content/Maps/MainMap.umap (마커2개 추가만), Saved/ServerData/MainMap.spawn
- 이 문서 및 DuskSpawnPointAuthoring/KnightCombatDesign/TerraQTwoHitImplementation/CurrentAssetRegistry 문서의 최신 안내

기존 스폰 BP는 native 속성을 상속하며 warnings-as-errors compile 호출 성공/dirty=false였다. 바이너리 스폰 BP·플레이어BP·몬스터BP·원본 Niagara·맵 지형/벽·Nav/OBB는 수정/저장하지 않았다. 저장 직전 MainMap dirty=true, 나머지 관련BP clean을 보고하고 MainMap만 save_assets=true. 체크포인트 `C:\Users\user\Documents\Unreal IOCP Server\checkpoints\MultiDusk_20261006`: 변경 전10파일 SHA256 복사 검증, 추가로 기존 parser 테스트 복사본 보존. 기존 서버 Git dirty 변경은 되돌리지 않았다.

## 실행한 검증 / 증거

- GameServer Debug/Release 최종 exit0: GameServerDebugBuild.log / GameServerReleaseBuild.log. 이번 호출은 PreBuildEventUseInBuild=false로 프로토콜/DB 생성기를 실행하지 않았다.
- 실제 Room.obj와 기존 native objects를 링크한 오프라인 runner: RoomTestsFinal.log, **26 checks/0 failures/exit0**. 3활성+1비활성, 고유ObjectId·생명UUID, 중심Z, snapshot3 및 corpse제거snapshot2, 기본전체명중/중복, Q전후방/중복, 독립경로·return무적·사망·제거·재생성, 진짜 미판정 Q 예약의 새생명제외, 몬스터중복타격/target수명, 퇴장예약정리 확인. disconnected 세션이며 실제 IOCP/DB 테스트가 아니다. no LootService 상태의 예상 'Death loot unavailable' 로그는 이 테스트의 제한이다.
- 오프라인 테스트 첫 링크는 기존 PCH object 누락으로 실패, 추가 후 통과. checks 통과 후 process-global pool teardown에서 멈춘 첫 실행은 중단했고, runner는 std::_Exit로 그 unrelated daemon teardown을 제외한다. 종료 정리/실제 서버 shutdown을 검증한 것이 아니다. 기존 LNK4075 E&C/OPT:ICF 경고가 남는다.
- parser 28case + 실제 deployed list1 = **29/0fail/exit0**, ParserTestsDeployed.log. v1/v2·3points·disabled·ID정렬·중복/unknown/finite/크기/개수/맵/높이/partial반영방지 등.
- S1Editor 최종 빌드 성공. 최초 새 테스트 소스가 UBT makefile cache에 누락되어 -gather로 다시 수집했다. 새 테스트의 TObjectPtr 자동추론 오류를 수정해 실제 컴파일한 최종 성공 로그만 근거로 삼는다.
- UnrealAutomationFinal.log + Saved/Automation/MultiDusk_Final_20261006/index.json: **14 Success/0warnings/0errors/0failed/0notRun**, 새 S1.World.MultiMonsterPresentation 포함. 같은Action의 다른대상HP/중복/HitIndex1, 독립제거/새ObjectId, unknown종류거부, 전체registryreset 및 기존전투/포션/사망fade 합성회귀. NullRHI이며 실제 rendered2-client/네트워크 검증이 아니다. 해당 로그 Fatal/Ensure condition failed/Blueprint Runtime Error/Network Failure/': Error:' match0.
- SpawnExport.log exit0/0errors/기존MCP EULA warning1: 새 프로세스로 저장 MainMap을 로드해3points saved Nav 투영 확인, v2 Export·서버3배포위치 readback/해시 일치. 프로젝트 MCP 설정 변경 없음.
- Windows Development Package.log: BUILD SUCCESSFUL/AutomationTool exit0, 새 게임 타깃 빌드 및 필수3맵 cook 성공. archive는 E:\Git Fork\S1_UE5.8\Saved\MultiDuskPackage_20261006\Windows. 첫 launcher의 Windows 인용 처리 실패는 스크립트만 수정해 재시도했고 실패 기록을 남겼다. 실제 패키지 로그인/전투 및 런타임 마커 제거 확인은 별도 검증 대상이다.
- Export/canonical/Debug/Release 스폰 파일 SHA256: 59F501E3F205D987E0A19CC18313AC60EFDD3B5FA0E5AAFE3E2BADB5E1465DE9. 데이터 배포뿐 아니라 Debug/Release 바이너리도 이번 소스로 빌드했다.
- 패키지 MainMap NullRHI smoke: PackageMainMapSmokeFinal.log에서 MainMap 실제 로드, `obj list class=/Script/S1.S1MonsterSpawnPoint` 조회0 Objects, Quit 정상 종료/프로세스 exit0 확인. 런타임 배치 마커 제외를 확인한 시작 검사이며 몬스터 생성/전투가 아니다. 서버를 종료한 상태여서 예상 `Game server connection failed 127.0.0.1:7777` Error1이 있다; 이를 네트워크 통과로 보고하지 않는다. 첫 smoke는 세미콜론 구분으로 Quit가 실행되지 않아 제가 실행한 정확한 경로의 PID16068만 종료했고, 쉼표 구분으로 재실행했다. 사용자 Editor/서버 프로세스를 종료한 것이 아니다.

## 남은 인수 / 사용자 조작

서버 재시작 후 두 PIE 계정으로 MainMap 입장. 몬스터3마리 생성 → 모아서 기본 공격 시 각25/한번, Q전방25+38/후방첫25 → 한마리처치가 다른 개체를 멈추지 않음 → 각각포션/제거/재생성 → 각자원점우회복귀 → 늦은입장/접속해제/재접속 시누락·중복없음 → 기존G/I/회복포션·HP·사망·Trail 회귀를 확인한다. 화면 느낌·군집겹침과실제DB드랍은 사용자 확인이 필요하다. 신규몬스터사용은 server등록/클라이언트presentation·애셋·크기/Nav 대응을 별도 승인/구현한 뒤 가능하다.
