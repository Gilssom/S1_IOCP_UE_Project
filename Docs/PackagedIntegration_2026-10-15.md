# 최신 패키지2계정 통합 검증 — 2026-10-15 KST

## 실행 버전 및 경계

사용자 승인: 포커스수정 포함 최신서버/Windows Development패키지 생성,
실제패키지2계정통합. 신규콘텐츠/키캡적용/프로토콜·DB설계/커밋푸시 제외.
Source HEAD1d82ea1 + 미커밋 AuthFocusFix 수정5파일/테스트가 최초 빌드입력이다.
후속 DuskFix는 WorldEntitySubsystem.h/.cpp 및 S1MultiMonsterPresentationTests.cpp
수정을 추가한다. 두 패키지의 검증 결과를 구분한다.
Content는 저장된 로컬애셋 사용; 필수 UI7종 MCP dirtyfalse 확인. 저장 없음.
실행시각/버전은 과거PIE/기존패키지와 구분한다.

확인시각2026-10-15 14:06:58 KST:
- 서버구성: MSVC x64 Debug (Unreal Development/Shipping과 다른 구분).
  E:/Git Fork/GameServer/GameServer/Binary/Debug/GameServer.exe
  SHA2566DA5AE3486AF83A6648CD92CC2126968CEE3CB2787A0DCE331189539D365F008.
  GameServerDebugBuild.log exit0,기존서버종료후 새서버PID22348 실행.
  ServerStartup.log fieldDropSupported1/lootReady1/potionSupported1.
- 클라이언트: Windows Development, 새 Build/Cook/Stage/Package/Archive.
  E:/Git Fork/S1_UE5.8/Saved/PackageIntegration_20261015/Windows
  실제게임exe: 위폴더/S1/Binaries/Win64/S1.exe
  SHA2562C63D0F39A795154E0BDA325E5C142ACE3DE68C64C8AE0B606D6EC5D324F7495.
  Package.log BUILD SUCCESSFUL,AutomationTool ExitCode0,1m8s.
  필수LobbyMap/LoadingMap/MainMap explicit cook. 새패키지폴더 전체가 필요하며
  exe만 복사해서 실행하는 배포방식이 아니다. 이전패키지폴더 재사용 안함.

## 백업 및 시작 점검

외부 checkpoint PackageIntegration_20261015/GameDBBeforePackagedPlay/GameDB.sql:
226484bytes,12테이블 정의/완료마커/무라우팅 검증,SHA256
24672058A87DBB2601734AF471C079ED86DF9C3172137568528A500A6780306F.
Single-transaction structure/data backup;복원리허설/PITR/계정권한/루틴·
트리거·외부로그 포함한 전체인스턴스백업은 아님. Git외부비공개 보관.
기존9테이블백업을최신으로오인하지 않음. backup중 운영데이터변경 없음.

PackageSmoke.log 새 archived exe exit0:
HP/InventoryGrid/InventorySlot/QuickSlots/LootPickupPrompt/SplitDialog/
DiscardDialog generated class7종 실제로드. 시작로그 기존Auth Non-Focusable
오류없음. 이것은 GPU·상호플레이·Main전환검증과 다르다.
새 패키지2창 실행 후 로그인은 사용자 직접 입력(비밀번호전달/기록 금지).
클라이언트별ClientA.log/ClientB.log와서버로그로 단계별대조한다.

## 검증표 (미실행을 통과로 기록하지 않음)

| 항목 | 상태 | 근거/다음행동 |
|---|---|---|
| 최신Debug서버빌드/시작 | 통과 | buildexit0,서비스ready 로그 |
| 새Windows Development패키지 | 통과 | UATexit0/필수맵cook |
| 필수UI7종패키지로드 | 통과 | 실제 archived exe smoke |
| 포커스시작오류 | smoke범위통과 | 새package시작로그0건;실로그인별도 |
| 2계정로그인/선택/Loading/Main/상호이동 | 통과(사용자+로그) | 사용자완료보고;양쪽Login/Main/자기·상대spawn확인 |
| A–D/Q/Trail 상대 화면 표시 | 통과(사용자) | DuskFix 수정 패키지에서 사용자 전체 테스트 완료 보고; 피격효과 별도 확인 |
| Dusk HP/사망/재생성/추적·귀환 | 통과(사용자) | 수정 패키지 양쪽 일치, 벽 우회·복귀·재교전·4초 회복 확인 보고 |
| 본인HP/상대숨김/Dusk전투3초표시정책 | 미검증 | 관찰자별;회복중숨김과서버증거구분 |
| 몬스터소유권/획득/퀵슬롯·포션 | 통과(사용자) | 수정 패키지 아이템 관련 전체 테스트 완료 보고 |
| 슬롯이동/99합치기·잔량/Ctrl교환/Shift분할 | 통과(사용자) | 수정 패키지 인벤토리 관련 전체 테스트 완료 보고; 이번 단계 DB 직접 대조는 미실행 |
| 플레이어공용버리기/타인G/양쪽제거/30초 | 통과(사용자) | 수정 패키지 필드 드랍 관련 전체 테스트 완료 보고 |
| 종료·재접속/퀵슬롯복구/중복·잔여위젯 | 통과(사용자) | 수정 패키지 종료 후 재접속 유지 테스트 완료 보고 |
| 종료로그Fatal/Ensure/Runtime/Network/잔여세션 | 미검증 | 테스트구간별최종검토 |

기존27자동테스트/2PIE사용자인수는이표의packaged플레이결과대체가아니다.
지연·중복·강제종료장애주입은별도심화검증. 실제실행중문제가나오면
원인과좁은수정을보고하고검증버전을새로구분한다.

## 첫 사용자 체크 완료 후 로그 대조

### 이후 발견한 차단 결함: Dusk Blueprint cook 누락

사용자 전투검증 중 뒤를본채추적(180도회전차이) 보고. 양쪽packaged로그
LoadPackage SkipPackage BP_S1DuskMinion 및 native fallback 경고 확인.
WorldEntity는 런타임LoadClass문자열/Transient캐시만 사용해 cook root가없었다.
MCP실제 /Game/S1/Monsters/BP_S1DuskMinion dirtyfalse,MeshFacingYawOffset=-90,
Scale0.7; nativefallback은+90으로180차이. 이전문서+90은현재저장본과다르다.
회전값을기획값으로임의변경하지않고현재사용자Blueprint를보존한다.

수정: WorldEntitySubsystem constructor FClassFinder 및 non-transient UPROPERTY
하드 CDO참조로정확한BP를cook대상에연결. 기존MultiMonster자동테스트에
CDO참조/실제스폰Blueprint클래스동일성 검증추가. BP/메시/맵/서버/DB변경없음.
새패키지는 PackageIntegration_20261015_DuskFix 별도폴더,이전패키지유지.
이전실제로그인/이동통과는이전버전증거로남기며전투이후는차단/미검증이다.
수정본 S1Editor Win64 Development 빌드 exit0. S1 자동테스트27개 통과
(26 success/1 success-with-warning/0 failed): 기존 KnightChargePresentation의
GameplayCueNotifyPaths 미지정 경고1건. MultiMonsterPresentation은 실제 BP
CDO참조와 생성클래스 동일성을 검증했다. 증거 checkpoint
PackageDuskFix_20261015/S1EditorBuild.log, UnrealAutomation.log 및
Saved/Automation/PackageDuskFix_20261015/index.json.
수정본 Windows Development Build/Cook/Stage/Package/Archive 통과:
PackageDuskFix_20261015/Package.log BUILD SUCCESSFUL, ExitCode0, 2m3s.
새 archive PackageIntegration_20261015_DuskFix/Windows/S1/Binaries/Win64/S1.exe
SHA256 2E7425DF7B01834345BA77BA24ED0670E3D25FBE13F60C823E24D573CDC702CF.
PackageSmoke.log exit0, UI7종과 BP_S1DuskMinion_C 실제로드,
CDO MeshFacingYawOffset=-90.000000 출력 확인. 누락/native fallback/Error/Fatal
경고 없음. NullRHI 시작검증이지 시각/실제전투 검증은 아니다.
원본 BP SHA256 73561474AE8DDD4E7609258F6362770103735A0C2CED09C5568D8A49F3075533
수정 전후 동일. 서버/DB/설정/Content 변경 없음. 새 두 게임 창 실행 후
2026-10-15 사용자: 수정 패키지에서 몬스터 이동·공격 방향 정상 테스트 완료.
새 ClientA/B 로그 모두 Loading/Main 진입 확인. Dusk SkipPackage/native fallback,
Fatal/Ensure/Blueprint Runtime Error/Network Failure는 확인 범위에서 0건.
ClientB에는 잘못된 자격정보에 따른 Login failed Result3 두 건이 있었고,
이후 정상 Main 진입했다. 인증 거절을 숨기거나 전체 Error0으로 보고하지 않는다.
후속 사용자 확인: 안내한 세 항목을 모두 테스트 완료했다고 보고했다.
수정 패키지의 A–D/Q/Trail 상대 표시, Dusk HP·사망·재생성 양쪽 일치,
벽 우회·복귀·재교전·4초 회복을 사용자 인수 완료로 기록한다.
이는 새 자동 테스트/서버 로그 분석을 실행했다는 뜻이 아니다.
추가 사용자 확인: 아이템·인벤토리·퀵슬롯·포션·필드 드랍 및 종료 후
재접속 유지도 모두 이미 테스트 완료했다고 보고했다. 해당 기능의 수정
패키지 사용자 인수를 완료로 갱신한다. 이번 기록 갱신에서 DB 직접 대조나
서버 로그 분석을 새로 실행하지는 않았다. 피격효과와 관찰자별 HP 표시
정책의 세부 수락은 별도 명시가 없어 이 보고로 확대하지 않는다.
추가 양쪽 ClientA/B 로그 검사: Fatal/Ensure/Blueprint Runtime Error/
Network Failure/Dusk 누락·native fallback 0건. Error는 위 ClientB 인증
거절2건만 확인했다. 아직 실행 중인 세션의 최종 종료 로그는 별도다.

새package ClientA/B 서로다른계정3/4,캐릭터5/591 로그인성공,
양쪽 Lobby→Loading→Main,local/remote Object1/5 생성확인.
사용자가 상호표시·이동까지완료보고. 이 구간각client FocusErrors0,
Error0,Fatal/Ensure/BlueprintRuntime/NetworkFailure0. 단 기존AnimBP
Divide by zero 경고는 각2건 관측,해결완료나전체무경고로기록하지않음.
위 첫 체크 기록은 이전 패키지 구간이다. 후속 수정 패키지의 전투·아이템·
재접속 사용자 인수 완료는 앞의 DuskFix 후속 기록 및 검증표를 따른다.
