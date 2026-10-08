# Terra Q 제자리 2타 — 최소 구현 검토안

2026-10-01 / 상태: 소스·읽기 전용 MCP 조사와 구현 제안. 코드·프로토콜·Blueprint·설정은 변경하지 않았다. 빌드·PIE를 새로 실행하지 않았다. Astra 전달문에 기록된 사용자 확정과 기획 제안을 구분한다.

후속 상태: 사용자가 같은 반경/첫 타360°/둘째 전방180°를 확정하고 쿨다운을 연기했다. 이후 Q 중 **점프만 차단, 이동 허용** 및 HitIndex/계약2 확장을 승인했다. 아래 원래 제안의 이동 잠금/고정 중심/6초 쿨다운은 적용하지 않는다. 구현 기록은 `TerraQTwoHitImplementation.md`가 최신이다. 원래 조사 시점의 문구는 그 시점의 기록으로 보존한다.

## 1. 확정 / 승인 대기

확정: 돌진 없는 제자리 Q, 기존 `AM_Terra_Q_AttackMontage`와 Niagara 유지. 0.37초에 주변 360°·기본 공격력×1.0, 1.10초에 전방 180°·×1.5. 백분율에서 시간을 재계산하지 않는다. 넉백은 이번 구현에서 제외한다.

| 미정 정책 | 추천 | 근거 / 한계 |
|---|---|---|
| 방향 | 승인 시 Q 시작 yaw를 두 타 내내 유지 | 기존 표현의 시작 방향 고정과 일치. 카메라는 자유 회전. 둘째 타 방향 재조준은 추가하지 않는다. |
| 반경 | 두 타 모두 **중심 간 수평 거리 230cm 이하**부터 시작 | 현재 서버 기본 공격도 중심거리 230cm. Dusk 반경 28cm를 추가하지 않아 기존 단위를 유지한다. 무기 접촉 거리의 측정 완료값은 아니며 최종 시각 튜닝 필요. |
| 높이 | 서버 Actor 중심 Z 차이 160cm 이하 | 기존 기본 공격 기준 재사용. 캡슐 겹침/높이 지형 대응을 의미하지 않는다. |
| 이동 | Q 시작 즉시 속도/입력 이동 정지, 정상 종료 3.10초까지 잠금 | 제자리 동작과 판정 중심을 안정적으로 맞추는 최소안. 현재는 이동이 허용되므로 사용자 확인 필요. 점프도 잠금. 사망은 항상 우선. |
| 판정 중심 | 서버 승인 시 저장한 플레이어 중심, 두 타 공통 | 제자리 잠금을 서버에서도 강제한다. 승인 이전에 처리된 이동까지 포함하므로 예측 시작 위치와 지연에 따른 오차는 존재한다. |
| 쿨다운 / 자원 | 승인부터 6초 / 소모 없음 | 기획 제안치, 아직 사용자 확정 아님. 종료 후 약 2.9초가 남는다. 취소·사망에도 재사용 시간 유지 추천. |
| 대상 | 같은 Room의 유효 몬스터 각각, 타별 최대 1회, 별도 수 제한 없음 | 현재 서버는 `_duskMonster` 한 개체. 지금은 그 개체만 검사하고 다중 개체 레지스트리 확대는 하지 않는다. |
| 명중 효과 | ActionType Q 전용 매핑에서 기존 `P_Terra_Primary_Impact` 재사용 | 새 에셋 없이 결과를 확인할 수 있음. 기본 공격 매핑에 Q를 암묵적으로 섞지 않는다. 별도 외형 요구 시 추가 결정. |

추천값은 승인값이 아니다. 무기 궤적/캐릭터 시각 반경 샘플링은 아직 수행하지 않았다. 현재 반경 추천은 기존 서버 거리 단위와 Dusk 크기에 근거한 보수적 시작값이며, 최종 무기 범위에 맞춘 측정치라고 보고하지 않는다.

## 2. 현재 실제 소스와 차이

| 현재 근거 | 확인된 동작 | 필요한 최소 확장 |
|---|---|---|
| `Room.cpp::HandleCombatAction` | Q Step0·방향·생존·시퀀스 검증/중계. `wantsResult`는 Primary/version1 전용. Q 피해 예약 없음 | Q 승인/거부, 쿨다운·상호배제, 같은 ActionId에 두 Hit 예약 |
| `Player.h` | 공유 요청 시퀀스/최근 32건 캐시. `activePrimaryActions`는 일회 예약 집합 | Q 인스턴스·시작 위치/yaw·완료 Hit mask·피해 대상·만료/쿨다운 보관 |
| `ResolvePlayerHit` | 기본 공격 230cm/160cm, 방향 dot≥0.34, Nav 경로 거리≤230, Dusk 한 개체. 피해 고정 25 | Q 독립 360°/180° 검사·계수 적용. 기존 기본 공격 규칙은 유지 |
| `S1KnightCombatComponent` | Q 시작 회전 고정·직접 Montage_Play, 반복 Q 및 기본 공격과 로컬 상호배제. Q 이동 속도는 낮추거나 막지 않음 | 로컬 이동 정지/잠금, 요청별 거부 복구 및 정리 |
| `S1MyPlayer` | Q와 Primary 시퀀스 공유. `PredictedPrimaryActions` 및 응답 수용은 Primary 전용 | 좁은 Q 예측 레코드 추가. Primary 계보/80% 버퍼는 그대로 유지 |
| `S_DAMAGE_RESULT` | ActionId/Source/Target/피해/HP/EventSeq, HitIndex 없음 | `hit_index` 추가 제안 |
| WorldEntity impact | 승인 ActionId로 ActionType 매핑, EventSeq(최대256건)로 FX 중복 억제 | Q Action 매핑 추가, ActionId+HitIndex+Target으로 타별 식별. 둘째 정상 이벤트를 지우지 않음 |

읽기 전용 MCP(2026-10-01): Q 몽타주 `UpperBody`, segment 0–3.0999999초, 배율1/loop1, RateScale1, 자동 BlendOut, BlendIn/Out0.25초. 기반 `Q_swing_B` RateScale1·RootMotion=false. 코드 Montage_Play 배율1. 따라서 최초 구현은 승인+370ms / 승인+1100ms 예약, 종료+3100ms. 재생률 변경을 허용하지 않는 좁은 계약을 권장하며 에셋이 변경되면 검증 실패/재승인하도록 한다. Notify는 Trail 표현만 담당한다.

현재 서버에는 DB 기반 AttackPower를 참조하는 이 경로가 없다. 최소안은 서버 전용 정의의 `BaseAttackDamage=25`를 기존 고정 피해의 명시적 원천으로 사용한다. 정수 계산은 `(uint64(base)*numerator + denominator/2)/denominator`의 양수 반올림(half-up)을 제안: 25×1.0=25, 25×1.5=38. 피해 이벤트는 실제 HP 감소량으로 제한하므로 HP20에 둘째 타가 들어오면 damage20. 이를 장비·DB 공격력 구현으로 표현하지 않는다.

## 3. 요청 → 예측 → 서버 결과 → 복구

1. 기능 버전 확인 후 Q 클릭: Ready/접지/생존/UI 검증, 현재 카메라 yaw로 즉시 몽타주·Trail 재생. 한 번의 공유 Sequence를 Q 예측 레코드에 기록하고 이동 잠금. 서버 응답을 기다렸다 시작하지 않는다.
2. 서버 Room queue: 인증 Player/Session/Room, 중복 캐시, Step0/유한 yaw, 생존, 기본 공격과 충돌, Q 쿨다운을 검사. 거부는 응답만, 다른 화면에 중계하지 않는다. 동일 요청은 캐시 응답만 반환하며 예약/피해/쿨다운을 다시 만들지 않는다.
3. 승인: 기존 ActionId 발급, 승인 중심/yaw 저장, 쿨다운 및 Q action lock 설정, requester result → remote relay → 370/1100ms 예약. 로컬 승인으로 몽타주/Trail을 재시작하지 않는다.
4. 타별 예약: 동일 Player identity·session identity/room·ActionId·생존·미실행 HitIndex를 확인하고 한 번만 처리. 현재 Dusk 위치·생존/RETURN 제외를 각 시점에 독립 검사. 첫 타 헛스윙이 둘째 타를 막지 않는다. 둘째 타는 정규화 수평 dot≥0(180° 경계 포함); 거리0은 방향 조건 통과.
5. 사망/퇴장/세션 변경 시 Q 레코드를 무효화한다. 예약 callback이 이미 queue에 있어도 실행 시 다시 검사한다. 기존 객체와 다른 재접속 Player에 이전 ActionId를 적용하지 않는다. 첫 타로 죽은 몬스터에는 둘째 피해 없음. 같은 ObjectId 재생성 가능성에는 몬스터 생명 세대/생존 스폰 시각 확인도 필요하다.
6. 거부: 현재 재생 Q의 Sequence+ActionType이 일치할 때만 0.10초 BlendOut, 버퍼/타이머/이동·회전 제한/Trail 생성 정리. 다음 정상 입력은 즉시 Ready 복구 후 허용, 사망이면 복구하지 않음. Q 입력 중 기본 공격을 버퍼링하지 않는 현재 정책 유지.
7. 늦은 거부는 해당 예측 레코드만 종료한다. 이미 끝난 Q 또는 최신 A–D/Q를 취소하지 않는다. 2초 응답 지연은 기존처럼 결과 미확인 진단이며 거부/헛스윙/재전송으로 간주하지 않는다. 세션 리셋 시 Q 레코드·타이머도 정리한다.

서버 상호배제는 `activePrimaryActions` 존재만으로 충분하지 않다(타격 시 제거되어 후딜레이가 남음). Primary의 연결 규칙을 유지하면서 Q를 기본 공격의 정상 재생 종료까지 차단하는 별도 종료 시각이 필요하다. A–D 길이 1.267/1.333/1.133/1.333초를 정의로 검증하고, 다음 Primary는 기존 0.55초·콤보 규칙 유지. Q 시작 시 콤보 진행은 리셋해 종료 후 다음 기본 입력이 A부터 시작하도록 제안한다.

서버 `HandleMove`는 현재 클라이언트 좌표를 제한 검사 후 받아들인다. Q lock 중 좌표/yaw를 승인 중심/yaw로 유지하고 서버 위치를 기존 `S_MOVE`로 회신해야 한다. 클라이언트 소유자 적용 경로까지 조사·테스트하여 보정 루프를 방지한다. 단순 클라이언트 입력 잠금만으로 서버 제자리 계약을 충족했다고 보고하지 않는다.

## 4. 벽 차폐 — 범위 내 제한

기존 `NavigationGraph`는 노드/portal 및 A* 경로를 제공하며 직접 collision/LOS API가 없다. 넓은 Q 범위에서 우회 경로도 짧을 수 있으므로 Nav거리≤반경만으로 벽 관통을 막는다고 보장할 수 없다.

권장 최소안: MainMap 테스트 정적 벽의 실제 충돌 형상을 읽기 전용 조사한 뒤, 서버용 작은 정적 blocker 데이터로 추출하고 중심→대상 선분의 교차를 검사한다. 박스 벽이면 world OBB/수직 범위로 정확히 표현, 단순 bounds만 쓰면 보수적 과차폐 가능성을 기록한다. 바닥/캐릭터/동적 장애물은 포함하지 않는다. 추적용 기존 S1NAV2와 별도 데이터로 유지해 우회 이동을 바꾸지 않는다. blocker data 없거나 불일치 시 Q 피해를 fail-closed하고 명시 진단한다. 맵 geometry 일반 Export/경사/복잡 mesh LOS는 제외한다.

이 안은 필요한 추가 파일/정적 데이터 범위이므로 구현 승인 전에 선택해야 한다. 대안은 Q 피해 개발을 평지 LOS 미완료 상태로만 검증하고 벽 인수는 차단으로 남기는 것. Nav거리만 쓰면서 벽 인수를 통과시키는 대안은 권장하지 않는다. 이번 조사에서는 벽별 좌표/형상을 측정하지 않았으므로 하드코딩값을 제시하지 않는다.

## 5. 최소 패킷 변경안 (아직 미구현)

새 패킷 종류는 불필요하다. 기존 `C_COMBAT_ACTION` Q Step0 / Sequence / FacingYaw 및 `S_COMBAT_ACTION_RESULT` 재사용.

```proto
// Protocol.proto / 기존 필드 번호 보존
// S_DAMAGE_RESULT 에 추가
uint32 hit_index = 8; // 기존 단타=0, Q=0/1
// S_COMBAT_ACTION_RESULT 에 추가
uint32 cooldown_remaining_ms = 9; // 승인/쿨다운 거부에서 UI 진단용
// Enum.proto / CombatActionRejectReason 에 추가
COMBAT_REJECT_COOLDOWN = 11;
```

ActionType은 이미 승인/중계 메타데이터에서 얻으므로 Damage에 중복 추가하지 않는 최소안. 두 Hit는 같은 ActionId, 서로 다른 HitIndex, 대상마다 별도 EventSeq. 현재 FX EventSeq 방식만으로도 두 이벤트를 표현할 수 있지만 명시 HitIndex가 없으면 의미상 중복/테스트 진단을 보장하기 어렵다. HP 적용에도 신선한 EventSeq 및 세션 상태를 확인해야 한다.

계약 버전2 광고를 제안. 새 클라이언트는 서버≥2에서만 Q 전투 예측을 시작, Primary는≥1 유지. 새 서버는 version1 Primary를 계속 지원하되 legacy Q(version0/1)는 피해/중계하지 않는 fail-closed 정책을 권장한다. 구 클라이언트의 즉시 로컬 Q만 보일 수 있으므로 두 클라이언트+서버를 함께 갱신하는 개발 배포를 권장. `wantsResult`는 Primary(v1/v2), Q(v2)로 분리한다. version2 제안은 기존 승인으로 간주하지 않는다.

정식 승인 후 canonical `E:\Git Fork\GameServer\GameServer\Common\Protobuf\bin\Protocol.proto`, `Enum.proto` 수정, 확인된 `GenPackets.bat`/protoc 재생성. Server, DummyClient, S1 `Source/S1/Network` 출력과 handler tables를 동기화한다. 생성 pb 파일은 직접 수정하지 않는다. 패킷 ID 추가 없이 기존 handler plumbing을 유지한다.

## 6. 정확한 변경 예정 범위

기존 파일(실제 경로 확인):

- 서버 `E:\Git Fork\GameServer\GameServer\GameServer\Room.h`, `Room.cpp`, `Player.h`: Q 승인/스케줄/수명/피해/이동 잠금.
- 공용 `E:\Git Fork\GameServer\GameServer\Common\Protobuf\bin\Protocol.proto`, `Enum.proto`: 위 추가 필드/enum·버전 계약. 재생성 대상은 위와 같음.
- 클라이언트 `E:\Git Fork\S1_UE5.8\Source\S1\Combat\S1KnightCombatComponent.h/.cpp`, `Game\S1MyPlayer.h/.cpp`: Q prediction/recovery/movement lock.
- 클라이언트 `Source/S1/S1GameInstance.h`: Q 계약 capability. `World/S1WorldEntitySubsystem.h/.cpp`: 타별 피해/FX 중복·수명 및 Q effect mapping 소비. `Config/DefaultGame.ini`: 명시 ActionType2 기존 impact 재사용을 승인하면 매핑 한 줄 추가.
- 클라이언트 `Source/S1/ClientPacketHandler.h` 및 `Network/*pb.h/*pb.cc`: 생성 출력 정합, 수동 직접 편집 금지.
- 정적 벽 안을 승인하면 기존 `Plugins/S1NavigationTools/Source/S1NavigationTools/Private/S1ExportNavigationCommandlet.cpp`와 `Public/S1ExportNavigationCommandlet.h`에 별도 export 옵션을 추가하는 방안(추적 데이터 무변경).

신규 제안 파일(아직 생성 안 함): 서버 `GameServer/GameServer/KnightQDefinition.h`, `StaticCombatBlockers.h/.cpp`, `Data/MainMapCombatBlockers.s1block`; 클라이언트 `Source/S1/Combat/S1KnightQContractTests.cpp`. 서버 솔루션 등록 파일 변경은 새 cpp를 추가할 때 포함. 정확한 데이터 배치 경로는 기존 navigation data 로더와 대조해 구현 직전 확정한다.

에셋: 기존 Q 몽타주/Trail/BP_Player/BP_MyPlayer/BP_S1DuskMinion 유지, 이번 안에 바이너리 편집 불필요. 검사에서 필요한 변경이 발견되면 먼저 대상과 dirty/checkpoint를 보고한다. UE5_RPG/UE5.5 원본은 수정·복사하지 않는다.

## 7. 한 번에 검증 가능한 절차

승인된 수치/패킷/벽 범위를 먼저 하나의 계약으로 고정하고 Q 하나만 구현한다. 반경/피해/타격시간/lock/cooldown은 서버 정의에서 소유, 에디터는 표현값만 조정한다. 기존 A–D 수정이나 legacy Trail 정리는 이 과제에 끼워넣지 않는다.

1. 정적 함수/예약 테스트: R±1cm, 0/±90/180° 경계, Z160±1, 25→25/38 및 HP clamp. 동일 Sequence/동일 Hit 두 번 처리해도 피해 한 번. 서로 다른 HitIndex는 두 번 정상. fake clock으로 첫 Hit 후 시전자 사망/퇴장/세션 변경 및 target death 취소를 검증.
2. 제한된 비출시 테스트 주입: Q만 강제 reject, 응답만 지연(처리는 지연하지 않음), 동일 요청 byte 재전송. 조건/Sequence/ActionId/HitIndex만 기록하고 비밀번호/계정 비밀 없음. 지연 테스트는 정상 신규 공격을 취소하지 않아야 함.
3. GameServer Debug/Release 및 S1 Game/Editor 빌드, 좁은 자동화, 기존 에셋 read-only 재확인. 미실행은 미검증.
4. 1인 PIE: Dusk 앞/뒤/좌우, 반경/높이 경계, 두 타 사이 진입·이탈, 첫 타 대상 사망, 첫 타 후 시전자 사망, 반복 Q/6초 거부, A–D↔Q, 기본 공격 A 시작 복귀, 제자리/카메라 자유 회전·거부 복구.
5. 2인 PIE: 각 Hit의 ActionId/HitIndex/Target/EventSeq·HP·FX 대응, 헛스윙 Trail만 표시, 벽 양쪽 무피해, 추적 우회/복귀, 접속 종료/재접속 후 과거 Q 재발 없음. Fatal/Ensure/Blueprint Runtime Error/설명되지 않은 네트워크 오류 점검.
6. 현재 한 Dusk만으로 다중 대상 동시 판정은 미검증 처리한다. 콘텐츠를 자동 증설하지 않는다. 패키징 여부는 변경 영향에 맞춰 별도 증거로 남긴다.

넉백 후속 난이도: 평지 한정도 서버 위치 이동/벽 정지·Dusk Chase/Nav 재계획·원격 보간·타격 중단 정책이 필요해 피해 계수 한 줄 변경보다 크다. 짧은 서버 displacement 상태로 분리하는 것이 좋고 이번에는 구현하지 않는다.

## 8. 사용자 결정 및 현재 결과

필요 결정: 위 추천 묶음(시작 yaw 유지, 두 반경230/높이160, 3.10초 이동 잠금/고정 중심, 승인부터6초·무자원, 타별 모든 유효 대상, 기존 impact Q 명시 재사용)과 최소 version2 패킷, 테스트 정적 벽 blocker 추가 범위. 어느 것도 메시지 전달만으로 승인 처리하지 않는다.

현재 완료는 조사/검토안 작성뿐이다. 이전 Q 1·2인 시각 통과는 현재 제안한 서버 2타 피해/쿨다운 검증이 아니다. 갱신 문서는 이 제안서 하나. Astra로 자동 회신하지 않았으며 구현 승인도 요청받기 전 확장하지 않는다.
