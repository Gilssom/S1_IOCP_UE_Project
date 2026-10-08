# Astra 기획 인계 — Terra Trail / Q 몽타주

작성일: 2026-09-30. 대상 프로젝트: `E:\Git Fork\S1_UE5.8`.

## 완료 범위와 인수 결과

- 기본 공격은 기존 Terra A–D 4타와 최신 입력 버퍼/80% 연결 규칙을 유지한다. 사용자가 A–D Trail의 1인·2인 PIE 로컬/상대 화면 표시를 확인했다.
- 사용자가 제작한 `/Game/Blueprints/Characters/AM_Terra_Q_AttackMontage`를 기존 Q 표현 경로에 연결했다. 사용자가 이번 수정본의 MainMap 1인·2인 PIE에서 Q 모션·Trail, 종료 후 이동·기본 공격 복귀, 상대 화면의 Q 표시를 확인했다.
- Q는 기존 행동 중계 기반 표현이다. 이번 작업은 서버 이동·충돌·피해·쿨다운 승인 구현이 아니다. AbilityId 11002의 GuardAura 의미를 유지한다.

## 기존 기획과 실제 적용 차이

| 항목 | 이전 기획/시험 | 현재 확인된 상태 |
|---|---|---|
| 기본 공격 Trail | `P_Terra_AnimTrail` Cascade 한 종 | 코드 경로와 입자는 동작했으나 화면에 보이지 않았다. 이후 사용자 승인·추가 에셋으로 몽타주 Timed Niagara 방식을 사용한다. |
| 최종 Niagara 참조 | 초기 Trial 01 / 사용자 Trail 4 선택 기록 | 읽기 전용 MCP에서 저장된 A–D 네 몽타주와 Q 몽타주의 `NS_Trail_14` 의존성을 확인했다. 레지스트리를 실제 참조로 정정했다. |
| Q 애니메이션 | `Q_swing_B`를 동적 몽타주로 재생 | 사용자 제작 Q 몽타주를 `Montage_Play`로 재생하고, 기존 시퀀스 설정은 폴백으로 유지한다. |
| 동기화 | 별도 VFX 패킷 없이 공격 표현 중계 | 유지. 로컬은 기존 예측 표현, 상대는 서버 중계된 행동의 몽타주/Notify로 표시한다. Notify는 피해 근거가 아니다. |

SwordTrailVFX의 포트폴리오 사용 가능 여부는 사용자가 확인했다. 구체적인 상품 출처는 미기록이다. 최종 Notify 프레임·소켓·폭·수명은 이번 인계에서 독립적으로 재측정하지 않았다.

## 변경 코드와 에셋

- `Source/S1/Combat/S1KnightCombatComponent.cpp`: Q 몽타주 직접 재생, 반환 재생 시간 사용, 실패 시 회전 상태 복구, 서버 중단 시 몽타주 정지와 타이머/상태 정리. 기본 공격 계약과 패킷은 변경하지 않았다.
- `Source/S1/Combat/S1KnightChargePresentationTests.cpp`: 저장된 양쪽 Blueprint Q 참조, 실제 재생, 반복 입력 억제, 중단 복구, 원격 중복 억제/재요청 방지, 시퀀스 폴백 자동화 검사.
- `AM_Terra_Q_AttackMontage`: `DefaultSlot`을 기존 `UpperBody`로 정합. Terra 스켈레톤, `Q_swing_B` 3.100초, 1회 재생, 자동 BlendOut 및 사용자 Niagara 설정 유지.
- `/Game/Blueprints/BP_Player`: 원격 Q 참조를 같은 몽타주로 변경하고 컴파일·저장.
- `/Game/Blueprints/BP_MyPlayer`: 사용자가 설정한 참조를 확인했으며 구현 담당은 수정·저장하지 않았다.
- A–D Niagara Notify는 사용자가 제작한 변경이다. 이번 Q 코드 수정에서 A–D 몽타주나 Niagara 에셋 설정을 변경하지 않았다.

바이너리 변경 전 `Saved/Checkpoints/QMontagePlayback_20260930/` 복사본과 SHA-256 일치를 확인했다. 저장 직전 의도한 두 패키지의 dirty 상태를 확인하고 Q 몽타주와 `BP_Player`만 저장했다. 재시작 후 저장된 참조/슬롯과 clean 상태를 MCP로 재확인했다. 서버·프로토콜·DB·설정 변경은 없다.

## 검증 근거와 한계

| 검증 | 결과 / 근거 |
|---|---|
| Game Development 빌드 | 성공. `Saved/Logs/QMontage_GameBuild_20260930.log` |
| 최종 Editor Development 빌드 | 성공. `Saved/Logs/QMontage_EditorBuild_20260930_Final.log` |
| Q 자동화 | `S1.Combat.KnightChargePresentation` 성공, 0 errors / 1 warning. `Saved/Automation/QMontage_20260930_Final/index.json`, `Saved/Logs/QMontage_Automation_20260930_Final.log`. 기존 GameplayCueNotifyPaths 미설정 경고는 미수정. |
| 실제 시각 / 1·2인 PIE | 사용자 확인 완료. MainMap Q 모션·Trail, 종료 후 이동·기본 공격 복귀, 상대 화면 Q 표시. 빌드/자동화 결과와 구분한다. |
| 새 PIE 로그 | `Saved/Logs/S1.log`: 16:55:55 KST Seq2, 16:56:01 Seq3에서 새 Q 몽타주 로컬/원격 재생(3.100초) 및 Type2 중계 기록. 이 근거만으로 전체 네트워크 회귀를 통과 처리하지 않는다. |
| 미검증 | 이번 Niagara/Q 버전 패키징·패키지 MainMap 전투, Q 중 사망/접속 종료의 실제 화면 정리, 수동 반복-Q 스트레스, Niagara 최종 수치 독립 감사. |

초기 자동화의 생산 AnimBP 실행에서 기존 Terra divide-by-zero 경고가 나타났다. 최종 하네스는 native AnimInstance로 재생 계약을 검증하며 생산 AnimBP 문제를 수정한 것은 아니다. 기존 UI 포커스 경고도 별도 항목이다. 이전 Cascade 패키징 성공을 최신 Niagara/Q 패키징 성공으로 보고하지 않는다.

## 유지 기준

외부 IOCP 서버가 행동/타격/피해/HP/사망 결과를 결정하고 Unreal/GAS/Notify는 입력·예측·표현을 담당한다. 기본 공격 수락과 명중은 구분한다. Q의 표현 중계를 완전한 서버 권위 Q로 간주하지 않는다.

Dusk 0.7 / 캡슐 28×62 / 메시 Yaw +90° / 평지 중심 Z 61.5cm 및 MainMap 정적 경로 우회·복귀를 유지한다. 기존 사용자 공격·복귀·2인 회귀 완료 기록은 보존하지만 이번 Q 검사에서 전부 재실행한 것은 아니다. 경사·계단·동적 장애물, E·방어·회피 확장은 포함하지 않는다.

## 건의 및 다음 작업 결정

1. 가장 작은 정리안: 화면에 보이지 않았던 기존 Cascade Trail 코드/타이머와 임시 진단 로그를 정리한다. 현재 Niagara 표현과 거부/사망/중단 수명 정리를 먼저 조사하고, 1·2인 표적 회귀로 검증한다. 아직 삭제하지 않았다.
2. Q 콘텐츠로 넘어간다면 별도 최소 서버 권위 명세가 필요하다: 이동 거리/시간, 조향 허용, 벽·대상 경계 정지, 타격 시점과 대상 수, 쿨다운/자원, 중단·거부 복구. 기존 GuardAura 11002를 재사용해 의미를 바꾸지 않는다. 필요한 패킷 변경은 구체안과 사용자 승인 후 구현한다.
3. 최신 Niagara/Q 패키징 확인은 기존 패키징과 구분한 좁은 검증 작업으로 잡는다. 생산 AnimBP 나눗셈·UI 포커스 경고도 콘텐츠 구현과 분리해 우선순위를 정한다.

Astra에는 이 완료 범위와 미검증을 검토하고 다음 한 작업의 명세를 제안하도록 요청한다. 이번 인계 자체는 위 후속 구현을 승인하거나 자동 시작하는 요청이 아니다.

## 갱신 문서

`TerraQMontageIntegration.md`, `TerraPrimaryTrailImplementation.md`, `CurrentAssetRegistry.md`, 이 인계 문서. 이전 설계·실패 기록은 현재 검증 결과로 덮어쓰지 않고 날짜별로 구분했다.
