# Terra Q 서버 2타 구현 — 2026-10-01

2026-10-06 다중 Dusk 후속: 사용자가 기본 공격/Q 모두 범위 내 전체 유효 적 타격을 승인했다. 기존 Q timing/범위/방향/피해/입력 제한은 유지하고, 단일 Dusk 세대 검사 대신 승인 시 각 ObjectId의 생명 목록을 캡처해 각 타별 독립 판정한다. 구현/최신 검증은 MultiDuskSpawnImplementation.md를 따른다. 아래 과거1-/2-client 인수를 현재 다중 개체 검증으로 재사용하지 않는다.

## 승인된 동작과 변경 범위

사용자 승인: 기본 공격과 같은 **수평 중심거리 230cm**, 첫 타360°/둘째 타 전방180°, 쿨다운 연기. Q 중 **이동 허용·점프만 차단**. HitIndex 및 Q 승인/거부 계약2 확장 승인. 높이 허용은 기존 기본 공격과 같은 중심 Z차160cm를 유지한다.

- 요청 즉시 기존 Q 몽타주/사용자 Trail을 로컬 예측 재생한다. 회전은 시작 yaw 유지, 카메라 회전·평소 이동 속도는 유지. 이동 가능한 동작이므로 각 타의 중심은 **각 판정 시점의 서버 플레이어 위치**이며 시작 위치에 고정하지 않는다. 돌진/루트모션은 추가하지 않는다.
- 서버 승인 후370ms HitIndex0(360°), 1100ms HitIndex1(전방180°,±90° 경계 포함). 피해 원천은 기존 서버 기본 공격 고정값25에 맞춘 서버 전용 정의. ×1.0=25, ×1.5=38(양수 half-up 정수 반올림), 실제 남은 HP로 clamp한다. 장비/DB 공격력 시스템을 만들었다는 뜻이 아니다.
- Q는3.10초 동안 다른 Q/기본 공격과 상호배제한다. 이것은 동작 중 제한이지 쿨다운이 아니다. 정상 종료 후 재사용 가능. Q 뒤 기본 공격은 A부터 시작하도록 서버 콤보 단계 리셋.
- 동일 요청은 기존32건 캐시 응답만 재전송하며 중계/타격을 반복하지 않는다. 동일 ActionId의 각 Hit는 mask로 한 번 처리, 헛스윙도 처리 완료된다. 타별 대상 위치/생존을 독립 검사한다. 현재 Dusk한 개체만 처리한다.
- 시전자 사망·퇴장·세션/Room 불일치·다른 ActionId·행동 만료 시 예약을 무효화한다. Dusk 재생성 세대도 검사해 이전 예약이 새 스폰에 적용되지 않도록 한다.
- Q 수락/거부를 요청자에게 전달하고 수락만 상대에게 중계한다. 수락은 명중이 아니다. 로컬 수락은 모션/Trail을 다시 시작하지 않는다. 거부가 현재 Q sequence와 일치할 때만0.10초 BlendOut/타이머/상태 정리. 오래된 거부는 새로운 Q/A–D를 끊지 않는다.2초 지연은 결과 미확인 로그이며 자동 재전송/거부 판정하지 않는다.
- 점프 입력과 `CanJumpInternal_Implementation`에서 Q중 점프 금지. Q시작 시 남은 Jump hold를 해제. 서버도 Q중 Jump 이동 상태와 airborne Q 요청을 거부한다. 이것은 일반적인 클라이언트 제공 이동 좌표의 완전한 물리/치팅 검증이 아니다.
- 피해 중복 키는 Source+ActionId+Target+HitIndex(최근256건)이며 대상별 EventSeq를 wrap-safe 검사한다. 첫 타 중복으로 HP가 되돌아가지 않고 둘째 정상 타는 통과한다. 연결/월드 정리 시 캐시를 비운다. 피격 FX는 명시 ActionType2 매핑으로 기존 승인된 `P_Terra_Primary_Impact`를 재사용하고 서버 양수 피해에만 표시한다.

## 패킷 / 호환성

Canonical `Common/Protobuf/bin/Protocol.proto`: `S_DAMAGE_RESULT.uint32 hit_index=8` 추가. 기존 단타는0, Q는0/1. 새 패킷 ID나 cooldown 필드는 추가하지 않았다. `S_COMBAT_ACTION_RESULT`와 C/S_COMBAT_ACTION 기존 필드 재사용.

S_ENTER_GAME은 계약2 광고. Primary는version1/2 지원, Q서버 전투는version2만 지원. 새 클라이언트는 서버≥2에서 Q입력을 시작한다. 구 Q요청은 서버 피해/중계 없이 거부한다(구 클라이언트는 새 응답 복구를 지원하지 않으므로 로컬 표현만 남을 수 있다). 서버+두 클라이언트를 함께 갱신해 테스트한다.

protoc로 Protocol 출력만 별도 임시 출력 경로에 생성하고 기존 `FixProtobufEscapes.ps1` 처리 후 Server/DummyClient/S1에 배치, 출력 해시 일치를 확인했다. 생성 pb파일은 직접 편집하지 않았다. 패킷 ID/handler 표 변경이 없어 전체 GenPackets/DB생성을 실행하지 않았다. 생성된 handler표에 이미 선언돼 있지만 구현이 누락된 DummyClient 결과 소비 함수는 기존 no-op 소비 방식으로 연결해 링크를 복구했다.

## 파일 / 에셋

서버 루트 `E:\Git Fork\GameServer\GameServer`:

- `GameServer/Room.h/.cpp`, `GameServer/Player.h`.
- 신규 `GameServer/KnightQDefinition.h`, `Tests/KnightQDefinitionTests.cpp`.
- `Common/Protobuf/bin/Protocol.proto`, Server와DummyClient의 `Protocol.pb.h/.cc` 재생성 출력.
- `DummyClient/ServerPacketHandler.cpp` 결과 소비 함수 정합. Enum/Struct/schema DB 변경 없음.

클라이언트 루트 `E:\Git Fork\S1_UE5.8`:

- `Source/S1/S1GameInstance.h`, `Game/S1MyPlayer.h/.cpp`, `Combat/S1KnightCombatComponent.cpp`.
- `World/S1WorldEntitySubsystem.h/.cpp`, `Network/Protocol.pb.h/.cc` 재생성 출력.
- `Combat/S1CombatImpactSettingsTests.cpp`, 신규 `Combat/S1KnightQContractTests.cpp` (모두 Source/S1아래).
- `Config/DefaultGame.ini`: Q명중 효과 ActionType2 매핑만 추가.
- 문서: 이 기록, `TerraQTwoHitImplementationProposal_2026-10-01.md`, `KnightCombatDesign.md`, `CurrentAssetRegistry.md`.

바이너리 에셋 수정/저장 없음. Q몽타주/Player BPs/Niagara/Dusk/MainMap 유지. 읽기 전용 MCP에서 기존 Q UpperBody·3.10초·Rate1/Loop1 재확인. 복구용 Source/Config/Server/Schema 복사본을 `Saved/Checkpoints/QTwoHit_20261001/`에 저장하고 각각 hash일치 확인. S1은 Git없음; Server저장소의 기존 소스/IDE/빌드 변경은 되돌리지 않았다.

## 검증 증거

빌드나 단위 테스트는 새1인/2인 PIE 결과를 대신하지 않는다.

| 실제 실행 | 결과 / 증거 (S1 Saved 기준) |
|---|---|
| GameServer Debug/Release 최종 | 성공. `Logs/QTwoHit_ServerDebug_Final_20261001.log`, `Logs/QTwoHit_ServerRelease_Final_20261001.log` |
| DummyClient Release | 성공. `Logs/QTwoHit_DummyRelease_20261001.log`. 초기 링크에서 기존 미구현 result handler를 확인해 no-op 소비 함수 추가 후 통과. |
| S1 Win64 Development 최종 | 성공. `Logs/QTwoHit_GameBuild_Final_20261001.log` |
| S1Editor Win64 Development 최종 | 성공. `Logs/QTwoHit_EditorBuild_Final_20261001.log` |
| 실제 서버 Q 정의 단위 테스트 | C++ 컴파일/실행 성공.370/1100/3100ms,25/38,반경/높이/±90°/후방/중심/NaN/독립 위치 샘플 경계. `Logs/QTwoHit_NativeTests_20261001.log`, `QTwoHitNativeTests/KnightQDefinitionTests.exe` |
| 최종 Unreal 전투 자동화3개 | 모두Success,0errors. ImpactSettings0warning, KnightChargePresentation 기존GAS GameplayCueNotifyPaths경고1, KnightQApproval0warning. `Automation/QTwoHit_20261001_Final/index.json`, `Logs/QTwoHit_Automation_20261001_Final.log`; process exit0. |
| 새 버전 1인·2인 PIE 사용자 검증 (2026-10-01) | 사용자가 서버·에디터 재실행 후 전방 두 타, 후방 첫 타, 이동·점프 차단·종료 후 복귀 검증 완료를 보고했다. 사용자 수행 결과이며 이번 문서 갱신에서 에이전트가 PIE를 실행하거나 해당 실행 로그를 독립 감사한 것은 아니다. |

새 Q 자동화는 실제 저장 몽타주·배율, wire HitIndex1, 즉시 예측/이동속도·입력 유지/점프 금지, 반복Q 억제, 강제거부 복구, 늦은거부의 최신Q 보호, 승인 비재시작, 세션예측정리와 실제 WorldEntity 소비 경로의 첫/둘째HP·중복Hit·과거HP방지를 검증했다. 거부 진단2건은 의도적 expected diagnostic이며 테스트0errors다. 서버 예약 전체/인증네트워크나 Niagara픽셀을 실행한 것은 아니다. 최종 자동화 로그에서 Fatal/Ensure/Blueprint Runtime Error 및 예기치 않은 Error를 찾지 못했다. 기존 GameplayCue 경고는 설정 변경 없이 보존했다.

빌드 실행 관련: 첫 서버 직접 프로젝트 호출의 SolutionDir 오류와 제한 환경 PATH중복 문제를 확인하고 솔루션 루트를 명시한 정상 컴파일 환경으로 재실행했다. 공용 파일을 수동 생성·검증한 뒤 MSBuild PreBuildEvent를 이번 호출에서만 끄고 DB생성단계를 건드리지 않았다. 임시 protoc 출력의 escape 보정은 ExecutionPolicy 제한 후 별도 PowerShell 프로세스에서 기존 보정 스크립트를 실행해 해결했다. 단위테스트는 불완전한 기본 SDK 대신 설치된10.0.22621.0 포함/라이브러리 경로를 명시해 실행했다. 실패한 초기 실행을 성공 증거로 사용하지 않는다.

## 제한 / 다음 사용자 검사

- 기존 Nav경로 거리≤230을 차폐 보조로 사용한다. 직접 LOS/정적 wall blocker를 추가하지 않았으므로 **모든 벽 너머 무피해를 보장하지 않는다**. MainMap벽 양쪽 인수는 미검증이며 실패하면 별도 정적 차폐 작업이 필요하다.
- 단일 Dusk이며 다중 몬스터 대상 테스트는 미검증. 새몬스터/DB/넉백/쿨다운/경사/Nav수정/legacy Trail정리는 포함하지 않음. Ability11002 GuardAura 유지.
- 일반 server cancel event는 추가하지 않았다. 기존 사망/퇴장 상태 및 로컬거부로 표현 정리; 유효 타격 예약 수명은 서버에서 검증한다. 원격 화면의 실제 Niagara잔상 종료는 PIE에서 확인 필요.
- 서버 승인 도착 기준 예약과 로컬 즉시 예측의 네트워크 시작시각 차이는 남는다. Notify로 서버 피해를 발생시키거나 클라이언트 타임스탬프로 보상하지 않는다.
- 권장1인: 전방은25+38(HP200→175→137), 뒤쪽은첫25만, 반경 밖/두 타 사이진입이탈, Q중 이동/카메라회전 가능·Space차단, 종료후 점프/기본공격복귀, 연속Q/동작중상호배제.
- 권장2인: 같은 ActionId의Hit0/1·HP·효과가 각각 한 번, 헛스윙에는Trail만, 첫타대상사망/시전자사망 후둘째취소, 벽양쪽/우회복귀 및 접속종료/재접속. log의 `Knight Q hit`, `Combat damage applied`, `Q accepted/rejected`와 화면을 대조한다.
- 새 버전의 1인·2인 PIE에서 전방 두 타·후방 첫 타·이동 허용·점프 차단·종료 후 복귀는 사용자 검증 완료(2026-10-01). 벽 차폐, 두 타 사이 진입/이탈, 사망에 따른 예약 취소, 접속 종료/재접속, 타별 HP/FX/EventId의 독립 로그 감사와 최신 cook/package는 별도 미검증이다. 이번 사용자 보고를 나머지 항목의 완료로 확대하지 않는다.
