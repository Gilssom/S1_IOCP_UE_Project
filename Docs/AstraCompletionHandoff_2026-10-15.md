# Astra 완료 인계 — 2026-10-15 KST

## 완료 범위

인증 UI 포커스 수정 → 최신 IOCP Debug 서버 빌드 → 새 Windows Development
패키지 → 실제 두 계정 통합 검증을 진행했다. 사용자 보고와 자동/로그 검증을
구분하며, 기존 PIE 결과를 이번 패키지 검증으로 대체하지 않았다.

- 인증: 실제 활성 입력란/버튼으로 포커스 지정. 숨겨진 위젯의 비동기 응답이
  포커스를 가져가지 않도록 제한. 인증 권한·패킷·DB 흐름 변경 없음.
- 사용자 2인 PIE: 로그인, 캐릭터 선택·생성 화면 전환, Main 이동·공격·카메라·커서 복귀 통과.
- 최초 패키지에서 Dusk가 뒤를 보고 이동하는 결함 발견. 런타임 LoadClass
  문자열만 있어 BP_S1DuskMinion이 cook에서 누락되고 native fallback이 생성됐다.
  실제 저장 BP의 MeshFacingYawOffset=-90, native +90 차이가 원인이다.
  이전 문서의 +90을 현재값으로 간주하지 않는다.
- WorldEntitySubsystem CDO의 하드 클래스 참조로 정확한 BP를 cook에 포함.
  BP·회전값·스케일·맵·서버·프로토콜·DB는 이 수정으로 변경하지 않았다.
- 수정 패키지 사용자 인수: Dusk 이동·공격 방향, A–D/Q/Trail 상대 표시,
  Dusk HP·사망·재생성 양쪽 일치, 벽 우회·복귀·재교전·4초 회복 완료.
- 이어서 사용자 확인: 아이템·인벤토리·퀵슬롯·포션·필드 드랍 및 종료 후
  재접속 유지 테스트도 모두 완료. 상세 범위는 PackagedIntegration_2026-10-15.md.

## 실행 버전과 증거

- IOCP 서버: E:/Git Fork/GameServer/GameServer/Binary/Debug/GameServer.exe.
  MSVC x64 Debug이며 Unreal Development/Shipping 구분과 다르다.
  SHA256: 6DA5AE3486AF83A6648CD92CC2126968CEE3CB2787A0DCE331189539D365F008.
- 최종 패키지: E:/Git Fork/S1_UE5.8/Saved/PackageIntegration_20261015_DuskFix/Windows.
  실제 exe: 위 폴더/S1/Binaries/Win64/S1.exe. 배포에는 폴더 전체가 필요하다.
  SHA256: 2E7425DF7B01834345BA77BA24ED0670E3D25FBE13F60C823E24D573CDC702CF.
- S1Editor Development 빌드 및 game Build/Cook/Stage/Package/Archive exit0.
- 자동 테스트27개 통과: 성공26, 기존 GameplayCueNotifyPaths 경고 포함 성공1,
  실패0. Dusk CDO cook 참조 및 실제 생성 BP 클래스 동일성 회귀 검사 추가.
- 실제 패키지 NullRHI smoke: 필수 UI7종, BP_S1DuskMinion_C와 Yaw -90 로드 확인.
- 양쪽 ClientA/B 검사: Dusk 누락/fallback, Fatal, Ensure, Blueprint Runtime Error,
  Network Failure 0건. ClientB 인증 거절 Result3 두 건 후 정상 Main 진입.
  실행 중 로그 검사이지 모든 세션의 최종 종료 정리 검증은 아니다.
- 증거: C:/Users/user/Documents/Unreal IOCP Server/checkpoints/PackageDuskFix_20261015.
  S1EditorBuild.log, UnrealAutomation.log, Package.log, PackageSmoke.log, ClientA/B.log.
  자동 보고: Saved/Automation/PackageDuskFix_20261015/index.json.
- 테스트 전 GameDB 12테이블 백업: 외부 checkpoint
  PackageIntegration_20261015/GameDBBeforePackagedPlay/GameDB.sql.
  구조/데이터 백업 검증 완료. 복원 리허설·PITR·권한/루틴 등 전체 인스턴스 복구는 미검증.

## 변경 파일 / 저장소

- Source/S1/S1AuthWidget.h/.cpp
- Source/S1/S1CharacterSelectWidget.h/.cpp
- Source/S1/S1GameInstance.cpp
- Source/S1/UI/S1AuthFocusTests.cpp
- Source/S1/World/S1WorldEntitySubsystem.h/.cpp
- Source/S1/Game/S1MultiMonsterPresentationTests.cpp
- Docs/AuthFocusFix_2026-10-15.md
- Docs/PackagedIntegration_2026-10-15.md
- Docs/CurrentAssetRegistry.md 및 본 문서.

이번 수정의 Content 저장/서버 저장소 변경 없음. S1 HEAD는 1d82ea1이며,
위 최신 수정은 인계 작성 시 미커밋 상태다. 커밋·푸시 완료로 간주하지 않는다.
GameServer의 기존 대량 스테이징·중첩 저장소는 건드리지 않았다.

## 잔여 제한 / 다음 기획 건의

- 기존 AnimBP Divide by zero 경고는 최초 패키지에서 관측했고 미해결이다.
  현재 모든 패키지/플레이 구간이 무경고라고 주장하지 않는다.
- 피격효과·관찰자별 HP 표시 정책의 세부 사용자 확인, 최종 세션 종료 로그,
  DB 직접 수량 대조 및 지연·중복·강제종료 장애 주입은 별도 검증으로 남긴다.
- 현재 기본 통합은 사용자 인수 완료. 같은 항목을 사용자에게 반복 요청하지 않는다.
- 다음 콘텐츠 추천은 Dusk 처치 퀘스트1종: 수락 → 서버 처치 집계 → 완료 →
  보상, 재접속 진행도 및 중복 보상 방지. 새 몬스터/직업보다 기존 전투·아이템·DB를
  완결된 포트폴리오 시나리오로 연결하기 좋다. 아직 승인/구현한 기능이 아니다.
- Astra는 콘텐츠 규칙·범위·인수 조건·구현 프롬프트를 제안해 사용자에게 제출할 것.
  프로토콜/DB 변경이 필요하면 구체안을 먼저 제시하고 승인 범위를 확인한다.
  신규 기능 구현·DB 변경·에셋 이관·자동 커밋·푸시를 이번 인계로 시작하지 않는다.
