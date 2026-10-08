# Astra 진행 인계 — 2026-10-07

## 이번 인계의 근거

사용자가 최신 수정본에 대해 “검증 완료, 오늘 지금까지 진행한 사항들 정리해서 기획_Astra에게 전달”을 요청했다. 직전 구현인 Dusk A→B 재탐색 수정의 사용자 인수 완료로 기록한다. 세부 목록이 없는 검증 보고를 다중 드랍·재접속·패키지 2인 테스트 전체의 완료로 확대하지 않는다.

이 문서는 현재까지의 누적 구현과 이번 사용자 확인을 정리한다. 주요 빌드·자동화 증거는 10월 6일 기록이며, 이번 전달 과정에서 새 빌드·PIE·로그 감사를 실행하지 않았다.

## 구현된 기준

- 회복 포션: 드랍 → G 획득 → I 인벤토리에서 스택 선택·사용 → 서버의 1개 소비 및 부족 HP 최대 50 회복 → 양쪽 회복 FX → 재접속 수량 유지. 최신 1인·2인 PIE에서 수량·회복·FX 각 1회·최대 HP 무소비·몸통 높이를 사용자 확인했다. FX 위치 Z96 포함 재패키징도 완료했다.
- 스폰 포인트: 기존 BP_S1DuskSpawnPoint 유지. 고유 SpawnPointId, 등록 종류 MonsterDefinitionId, Enabled를 편집하며 현재 Dusk 종류 ID 1만 지원한다. 포인트당 1개체, 기술적 상한 16, 현재 MainMap 활성 3개체다.
- 적용 절차: 맵 저장 → Export → 서버 종료 후 DeployDuskSpawnPoint.ps1 → 서버 재시작. Export Complete는 서버 배포 완료가 아니다. 이전 위치 미적용 원인은 Export와 서버 Data 배포의 분리였다.
- 다중 개체: 개체별 추적·복귀·공격·HP·사망·재생성·드랍 수명 분리. 재생성에는 새 ObjectId를 사용한다. 기본 공격 A–D와 Q 모두 범위 내 모든 유효 적을 타격하며, 단일 최근접 제한으로 되돌리지 않는다.
- 기존 Dusk 기준: 스케일 0.7, 캡슐 28×62cm, 메시 Yaw +90°, 평지 중심 Z61.5cm. MainMap 정적 Nav 우회 유지. 경사·계단·동적 장애물·군집 회피는 미구현이다.

## 직전 수정과 이번 사용자 인수

문제: A 추적 후 A가 유효 범위를 벗어나면, 가까운 B가 있어도 공격 종료 분기에서 곧바로 RETURN으로 전환했다.

수정: 일반 추적과 공격 종료에 공통 AcquireMonsterTarget을 사용한다. A가 무효해지면 유효 B를 먼저 찾고, 후보가 없을 때만 복귀한다. 유효 A는 더 가까운 B가 있어도 유지한다. B는 탐색 800cm·스폰 기준 1200cm·생존 조건을 만족해야 한다. 기존 공격 간격 2초를 유지하고, A를 향한 예약 피해는 B로 이전하지 않는다. RETURN 확정 후에는 스폰 도착까지 재탐색하지 않아 왕복 흔들림을 방지한다.

변경: 서버 Room.h/.cpp, Tests/RoomMultiMonsterTests.cpp 및 관련 문서. 클라이언트·Blueprint·맵·Nav·DB·패킷은 이 후속 수정에서 변경하지 않았다. 사용자 디버거 종료 후 Debug/Release 전체 빌드 성공, 실제 Room 오프라인 37검사/실패0. 최신 사용자 보고로 해당 수정의 플레이 검증 완료를 반영한다.

## 증거와 제한

- MultiDusk_20261006: 서버 Debug/Release, S1Editor, 게임 Development, Windows Build/Cook/Stage/Package/Archive 성공. Room 26검사, parser 29검사, Unreal 자동화 14개 성공.
- DuskRetarget_20261006: Debug/Release 빌드 exit0, RoomRetargetTests.log 37검사/실패0. 테스트는 disconnected fake 세션과 실제 saved Nav를 사용한 오프라인 검사로, 실 DB·IOCP 또는 서버 종료 안정성을 입증하지 않는다.
- 패키지: Saved/MultiDuskPackage_20261006/Windows. MainMap 로드·authoring marker 0개·정상 종료 smoke 확인. 인증된 패키지 2인 전투 테스트와 구분한다.
- 포션의 DB 소비와 런타임 HP는 완전한 원자적 exactly-once가 아니다. HP 적용 후 durable ACK 전 crash에서 다음 부팅 시 포션 반환이 가능한 승인된 손실 방지 정책을 유지한다.
- Nav 경로 거리 검사는 직접 LOS 검사가 아니므로 모든 벽 관통 공격이 해결됐다고 주장하지 않는다.

## 남은 확인 및 후속 기획

별도 명시적 인수가 필요한 항목: 다중 개체 각각의 드랍·G 획득·DB 유지, late join·퇴장·재접속, 전체 전투/포션/Trail 회귀, 실제 패키지 2인 플레이, 군집 겹침 시각, 장애 주입. 회복 FX의 출처·공개 영상 권한 기록도 남아 있다.

Astra가 전달한 다음 작업은 하단 1~4 퀵슬롯 최소안이다. 기존 RequestUseItem과 서버 소비·회복 경로를 재사용하며, 정의 ID 등록·사용 시 최신 스택 선택·캐릭터별 로컬 등록 저장·공통 pending/unknown 차단을 유지한다. 신규 DB/프로토콜·아이템·쿨다운·UI 전면 개편은 제외한다. 현재 소스 조사 단계이며 퀵슬롯 구현·빌드·PIE 완료로 보고하지 않는다.

우선 건의: 퀵슬롯의 소규모 구현/인수와 다중 개체 회귀 목록을 구분해 진행하고, 군집 회피·경사 대응·새 몬스터는 별도 기획으로 유지한다. 오래된 문서의 단일 Dusk·퀵슬롯 미승인·높이 재패키징 대기 문구는 최신 후속 기록과 구분한다.

관련 기준 문서: AstraPlanningHandoff_2026-10-06.md, MultiDuskSpawnImplementation.md, PotionUseImplementation.md, DuskSpawnPointAuthoring.md, KnightCombatDesign.md, TerraQTwoHitImplementation.md, CurrentAssetRegistry.md.
