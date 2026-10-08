# Terra 기본 공격 승인·거부 및 예측 복구 계약 — 구현 전 검토안

작성일: 2026-09-23  
상태: **2026-09-23 구현 승인을 받아 적용 시작. 최초 설계 제출 시점의 제안값은 아래 별도 구현 기록과 구분.**  
최신 구현·검증 상태: [TerraPrimaryApprovalImplementation.md](TerraPrimaryApprovalImplementation.md).  
적용 범위: Terra A–D와 현재 Dusk 1종. 서버 피해·HP·사망의 기존 권위를 보존한다.

## 1. 조사한 현행과 목표 계약

| 관심사 | 현재 코드 | 제안하는 최소 변경 |
| --- | --- | --- |
| 입력/예측 | `IA_Attack`의 `Started` → LocalOnly GAS → `US1KnightCombatComponent::StartComboStep()`에서 Montage 재생 후 `AS1MyPlayer::HandleKnightActionStarted()`가 시퀀스를 발급·전송. 추가 클릭은 bool 1개로 버퍼. | 클릭 즉시 재생과 1개 버퍼·80% 분기 규칙 유지. 시작한 예측 타와 요청 시퀀스·선행 타를 명시적으로 연결한다. |
| 서버 검증 | `Room::HandleCombatAction()`이 Room/생존, 증가 시퀀스, 타입·단계, 유한 yaw, 기본 공격 최소 0.55초, 다음 단계 순서·2초 제한을 검사. 불합격은 모두 무응답 `return`. | 각 기본 공격 요청을 수락/거부로 종결해 요청자에게 응답. 동일 시퀀스 중복은 이전 결정만 재통지, 새 Action·타격 없음. |
| 원격 표현 | 수락하면 `S_COMBAT_ACTION`을 요청자 **제외** Room에 방송. | 그대로 유지. 거부된 요청은 원격 재생하지 않는다. |
| Action ID/타격 | Room의 `_nextActionInstanceId++`를 수락한 행동에 부여; `500/530/450/530 ms` 타이머가 `ResolvePlayerHit()` 호출. 타격 시 플레이어 존재·생존, Dusk 상태, 거리/높이/방향/Nav 경로 검사. | 서버 승인과 적중은 별개. 수락된 헛스윙은 정상 행동이나 `S_DAMAGE_RESULT` 없음. 타이머는 동일 Player 인스턴스·Room·유효 Action·한 번 판정 여부도 확인한다. |
| 요청자 복구 | 승인/거부 패킷·요청별 상태 없음. `InterruptByServer()`는 타이머·상태만 정리하고 현재 Montage 자체를 명시적으로 정지하지 않음. | 거부된 현재 예측 타와 이에 종속된 버퍼/후속 타만 짧게 종료. 독립적인 새 승인 행동이나 사망 상태는 유지. |
| 피해/상태 | `S_DAMAGE_RESULT`가 Action ID와 최종 HP를 방송; `S_PLAYER_STATUS` 사망 시 컴포넌트를 중단. | 이 패킷들은 유지. Damage의 부재를 Reject로 해석하지 않는다. 사망이 복구보다 우선한다. |
| Q | Q도 동일 `C/S_COMBAT_ACTION`, `CombatActionSequence`, 서버 Action ID 카운터를 사용하며 별도 타격 예약은 없음. | 시퀀스 중복/수명 규칙은 공유하되 Q 이동·충돌·피해 및 Q 예측 복구 정책은 이번 작업 밖. AbilityId `11002`를 Q로 재해석하지 않는다. |

현 서버는 `lastCombatActionSequence`를 **수락 시에만** 갱신하므로 거부된 시퀀스를 다른 payload로 다시 제출할 여지가 있다. 또한 `ResolvePlayerHit()`는 Action ID를 인자로 받지만 그 ID가 여전히 해당 Player의 유효 행동인지 검증하지 않는다. 이는 이번 설계의 두 직접적인 보완점이다. `S_DAMAGE_RESULT`의 Action ID는 이미 피해 사건을 묶지만 요청 시퀀스와 서버 ID의 대응을 요청자에게 알려주지 않는다.

## 2. 콤보 시간 비교

현재 컴포넌트의 `ComboBranchRatio=0.80`; 길이는 `KnightCombatDesign.md`의 Terra 소스 클립 값이다. **실제 런타임 `Montage_Play()` 반환 길이·PlayRate·타이머 드리프트는 별도 측정 전**이므로 아래는 기준값 계산이다.

| 타 | 문서상 클립 길이 | 80% 분기 | 분기 후 종료 | 서버 최소 간격 0.55초 대비 | 서버 2초 연결 상한 대비 |
| --- | ---: | ---: | ---: | --- | --- |
| A | 1.267초 | 1.014초 | 1.267초 | 가장 빠른 다음 B도 +0.464초 여유 | +0.733초 여유 |
| B | 1.333초 | 1.066초 | 1.333초 | 다음 C +0.516초 여유 | +0.667초 여유 |
| C | 1.133초 | 0.906초 | 1.133초 | 다음 D +0.356초 여유 | +0.867초 여유 |
| D | 1.333초 | 1.066초 | 1.333초 | 다음 A +0.516초 여유 | 서버는 A를 새 시작으로 허용 |

현재 기준값만 보면 0.55초·2초 제한이 정상 80% 또는 종료 시 연결과 **직접 충돌하지 않는다**. 이번 명세에서 숫자를 변경하지 않는다. 다만 서버는 패킷 **도착 간격**, 클라이언트는 로컬 Montage **시작 간격**으로 판단하므로 지연 변동이 큰 경우 C의 최소 0.356초 여유가 줄 수 있다. 실제 `Montage_Play()` 길이, 두 클라이언트의 송수신 시각 및 강제 지연 테스트를 기록하고, 정상 입력이 거부될 때만 최소 조정안을 별도 제출한다. 0.55초를 무작정 낮추거나 서버가 클라이언트 시각을 신뢰하게 하지 않는다. 클립 완료 후 새 클릭은 기존처럼 A로 시작하며, 서버의 2초 제한은 B/C/D 요청의 이전 **수락 타**와의 간격에만 적용한다.

## 3. 최소 와이어 계약 제안 — 아직 미구현

정식 설계 문서의 범용 `C_USE_SKILL`/`S_ACTION_*` 전체 도입 대신 현행 패킷을 확장한다. 아래 번호는 현재 최대 `1021` 다음의 **제안값**이며 구현 승인 시 충돌을 재확인한다.

```proto
// 기존 메시지에 필드 추가. 기존 1~4 번호와 의미를 유지.
message C_COMBAT_ACTION {
  // ... action_type=1, combo_step=2, sequence=3, facing_yaw=4
  uint32 combat_contract_version = 5; // 0=레거시, 1=승인 응답 지원
}

// 기존 S_ENTER_GAME의 success=1, player=2 유지.
message S_ENTER_GAME {
  // ...
  uint32 combat_contract_version = 3; // 서버 지원 버전, 없으면 0
}

enum CombatActionDecision {
  COMBAT_DECISION_UNSPECIFIED = 0;
  COMBAT_DECISION_ACCEPTED = 1;
  COMBAT_DECISION_REJECTED = 2;
}
enum CombatActionRejectReason {
  COMBAT_REJECT_NONE = 0;
  COMBAT_REJECT_NOT_IN_ROOM = 1;
  COMBAT_REJECT_DEAD = 2;
  COMBAT_REJECT_INVALID_TYPE_OR_STEP = 3;
  COMBAT_REJECT_INVALID_FACING = 4;
  COMBAT_REJECT_TOO_FAST = 5;
  COMBAT_REJECT_COMBO_ORDER = 6;
  COMBAT_REJECT_COMBO_EXPIRED = 7;
  COMBAT_REJECT_STALE_SEQUENCE = 8;
  COMBAT_REJECT_DUPLICATE_CONFLICT = 9;
  COMBAT_REJECT_ACTION_BLOCKED = 10; // 향후 상태 중단, 현 코드에는 아직 별도 stun 상태 없음
}

// 새 S_COMBAT_ACTION_RESULT, 제안 Packet ID 1022. 요청자에게만 전송.
message S_COMBAT_ACTION_RESULT {
  uint64 source_object_id = 1;
  uint32 sequence = 2;
  CombatActionType action_type = 3;
  uint32 combo_step = 4;
  CombatActionDecision decision = 5;
  uint64 action_instance_id = 6; // 수락 시 >0, 거부 시 0
  CombatActionRejectReason reason = 7; // 수락 시 NONE
  uint64 server_tick_ms = 8; // 진단용 monotonic 시간. UE 판정 시각 아님
}
```

- 요청 시퀀스는 기존 `C_COMBAT_ACTION.sequence`를 그대로 사용하며 Q와 공유한다. 서버 Action ID는 현 Room 카운터를 그대로 사용한다. `AuthorityAndCombatIdContract.md`의 RoomRuntimeId 상위 비트 구성은 **이번에 구현된 것으로 가정하지 않는다**. 세션을 넘는 전역 유일성을 주장하지 않고 `(로컬 세션 세대, SourceObjectId, ActionInstanceId)`로 구분한다.
- 결과 패킷은 Terra 기본 공격에만 발송한다. Q의 기존 요청·원격 중계는 유지하되 Q의 시퀀스도 서버의 중복·상한 검사에 포함한다. Q를 일반 공격 판정으로 끌어오지 않는다.
- 무인증/Player 없는 Session은 결과를 안전하게 귀속할 수 없으므로 전투 응답 대상이 아니다. 인증된 Player가 Room 밖인 경우에는 `NOT_IN_ROOM`을 돌려주도록 현재 `ClientPacketHandler.cpp`의 조기 반환 위치를 조정한다. 서버는 actor ID를 요청에서 받지 않고 Session의 Player를 사용한다.
- 유효한 패킷은 Room의 직렬 JobQueue에서 동일 순서로 결정한다. 수락 시 Action ID 발급, 상태/타격 예약, 요청자 결과 전송, 요청자 제외 `S_COMBAT_ACTION` 방송을 수행한다. **수락 응답은 명중 통보가 아니다.** 거부는 Action ID 발급·타격 예약·원격 방송을 하지 않는다.
- 수락/거부 결과를 Player별 최근 요청 캐시(초기 권장 32건, 상한 있는 TTL 8초)에 시퀀스와 payload 지문으로 보유한다. 동일 시퀀스·동일 내용은 **동일 결과만** 재전송; 동일 시퀀스·다른 내용은 `DUPLICATE_CONFLICT`, 새 Action 없음. 캐시 밖의 과거 시퀀스는 `STALE_SEQUENCE`. `lastSeenSequence`는 수락뿐 아니라 거부에도 진행시키며 0을 금지하고 uint32 wrap-safe 비교를 사용한다. 캐시·카운터는 Player/Session 생명주기와 함께 폐기한다. Q 결과를 따로 통지하지 않더라도 Q 시퀀스는 소비하고 중복 중계를 막는다.
- `S_DAMAGE_RESULT` 형식과 Dusk 패킷은 바꾸지 않는다. 중복 결과 수신은 시퀀스로 멱등 처리하고 HP 갱신은 기존 서버 값만 적용한다. 클라이언트의 시퀀스/Action ID 매핑은 원격 피해 연출 진단용이며 AnimNotify가 서버 적중을 만들지 않는다.

### 양쪽 버전 호환성

| 조합 | 동작 원칙 |
| --- | --- |
| 새 서버 + 새 클라이언트 | `S_ENTER_GAME` 버전 1 확인 후 기본 공격 요청에 버전 1, 요청자에게 `S_COMBAT_ACTION_RESULT`. 원격은 기존 `S_COMBAT_ACTION`. |
| 새 서버 + 기존 클라이언트 | 기존 클라이언트는 `S_ENTER_GAME`의 추가 Protobuf 필드를 무시하고 버전 0 요청. 서버는 **새 Packet ID 1022를 보내지 않고** 기존 중계/피해 흐름을 유지한다. 레거시 클라이언트의 거부 복구는 보장하지 않는다. |
| 기존 서버 + 새 클라이언트 | `S_ENTER_GAME` 버전 부재(0)를 감지하면 새 계약이 필요한 Terra 기본 공격 요청을 송신하지 않고 로컬 전투 입력을 비활성화해 진단 메시지를 남긴다. 로그인/이동 등 기존 흐름은 유지한다. Q는 레거시 흐름 유지 여부를 별도 테스트한다. |

새 서버가 1022를 레거시 클라이언트에 보내면 현 `Handle_INVALID()`가 `false`를 반환하므로 안전한 혼합 버전이 아니다. 버전 게이트는 필수다. 필드를 추가해도 기존 번호를 재사용하지 않으며, Proto 원본 한 곳에서 서버·Unreal·DummyClient 코드를 재생성한다. 생성된 `*.pb.*`와 PacketHandler 헤더는 직접 편집하지 않는다. 서버/클라이언트 동시 빌드·패킷 ID 일치·혼합 버전 테스트 후에만 새 계약을 활성화한다.

## 4. 요청 → 예측 → 수락/거부 → 복구

```text
클릭 1회
  ├─ 로컬 GAS / CombatComponent: A 또는 버퍼 1개 (즉시 Montage)
  └─ 실제 타 시작 시 seq 발급, Pending(sessionEpoch, objectId, seq, step,
                               parentSeq, predictionGeneration) 등록 → C_COMBAT_ACTION
       ├─ 수락: seq → actionId 기록, Pending=Accepted; 로컬 Montage 재시작 금지
       │         서버는 원격에만 S_COMBAT_ACTION; 적중 여부는 나중에 별도 판정
       ├─ 거부: 동일 세션/객체/seq를 찾음
       │   ├─ 현재 예측 타 또는 아직 활성인 종속 후속 타인가?
       │   │    → 그 계보의 버퍼·분기/종료 타이머 정리,
       │   │       해당 Montage 짧은 블렌드아웃, 이동/대기 복귀
       │   └─ 더 새로운 독립·수락 타인가? → 과거 기록만 정리, 현재 타 유지
       └─ 응답 지연: Pending=Unknown 진단, 재전송·거부 추정 없음;
                    기존 모션은 자연 종료, 다음 독립 입력 가능
```

현재 `StartComboStep()`은 Montage가 정상 시작된 **뒤** delegate로 송신한다. 구현 시 `AS1MyPlayer`의 요청 발급과 CombatComponent의 예측 타 ID를 연결하는 작은 인터페이스를 추가해야 한다. GAS를 새 권위 저장소로 만들지 않는다. `CombatActionSequence`는 살아 있는 요청별 식별자로 계속 단조 증가한다. 클릭 버퍼를 넣는 시점에는 아직 새 타가 아니므로 패킷/시퀀스를 발급하지 않는다. 80% 분기 또는 종료에서 다음 타가 **실제로 시작**할 때 새 시퀀스를 발급한다. 중간에 앞 타가 거부되면 버퍼를 폐기한다.

거부 복구 권장 기본값: `Montage_Stop` 블렌드아웃 **0.10초**(편집 가능한 0.05~0.15초 검토), 컴포넌트 분기/종료 타이머 제거, 행동 상태 Ready, 이동 속도/이동 회전 복원. 현재 `FinishCurrentAction()`만 호출하면 Montage가 계속 보일 수 있으므로 명시적 Montage 중단 경로가 필요하다. 사망 통보가 먼저 왔거나 동시에 오면 Dead가 우선이고 이동을 재활성화하지 않는다. 일반 공격 거부 팝업은 없다. 상태 정리 후 **다음 새 클릭**을 즉시 받을 수 있고, 기존 클릭을 자동 재실행하지 않는다. 미확인 타임아웃은 거부와 다르므로 현재 정상 재생 중인 Montage를 중단하지 않는다. 최초 제안 타임아웃은 **2초**(네트워크 테스트로 조정), 기록은 8초까지 남겨 늦은 결과를 식별한다.

클라이언트가 결과를 적용할 때 `(현재 연결 세대, 로컬 ObjectId, sequence, action_type, combo_step)`를 모두 맞춘다. 같은 결과의 재수신은 효과가 없다. 더 늦은 시퀀스가 이미 시작됐다는 이유 **만으로** 과거 응답을 무시하지 않고, 그 후속 타가 거부된 타에 **종속하는지** 판단한다. 반대로 과거 거부가 새로운 독립 A 또는 서버 수락이 확인된 행동을 취소해서는 안 된다. `S_COMBAT_ACTION`은 지금처럼 MyPlayer에 적용하지 않아 로컬 Montage 이중 재생을 피한다.

`S_PLAYER_STATUS` 사망, `S_LEAVE_GAME`/despawn, LevelFlow world reset, Disconnect/재접속, 로컬 Player 재생성 시 Pending/매핑/버퍼를 지우고 로컬 연결 세대를 증가시킨다. 서버는 Player 퇴장·사망 시 활성 기본 공격을 무효화한다. 타격 타이머는 실행 직전에 `(Player 인스턴스 또는 세션 세대, Room 소속, ActionInstanceId 활성, 미판정, 생존)`을 다시 확인하고 한 번만 소비한다. 현재 타격 콜백의 단순 objectId 조회만으로 충분하다고 보지 않는다. 예전 세션 응답은 네트워크 수신 큐/세대와 결합해 폐기한다.

## 5. 예정 파일 — 검토용, 변경하지 않음

| 소유 계층 | 실제 경로 | 예정 역할 |
| --- | --- | --- |
| 공용 프로토콜 원본 | `E:\Git Fork\GameServer\GameServer\Common\Protobuf\bin\Protocol.proto`, `Enum.proto` | 위 호환 필드·결정/사유 enum·새 결과 메시지 제안. `GenPackets.bat`로 생성물 갱신. |
| 서버 패킷 진입 | `E:\Git Fork\GameServer\GameServer\GameServer\ClientPacketHandler.cpp` | 인증 Player/Room 확인, Room 밖 거부 응답 경로. |
| 서버 행동·타이머 | `E:\Git Fork\GameServer\GameServer\GameServer\Room.cpp`, `Room.h` | 수락/거부/중복 캐시 순서, 결과 송신, 활성 Action 수명 확인, 퇴장·사망 무효화. 기존 Nav/피해 수치 유지. |
| 서버 Player 상태 | `E:\Git Fork\GameServer\GameServer\GameServer\Player.h` | 마지막 본 시퀀스, 제한된 결과 캐시, 기본 공격 활성 Action/세대. 필요 시 `Player.cpp` 초기화. |
| Unreal 패킷 진입 | `E:\Git Fork\S1_UE5.8\Source\S1\ClientPacketHandler.cpp`, `Network\S1NetworkSubsystem.h/.cpp` | 새 결과를 game thread의 typed delegate로 전달. |
| Unreal 조정/월드 | `E:\Git Fork\S1_UE5.8\Source\S1\S1GameInstance.h/.cpp`, `World\S1WorldEntitySubsystem.h/.cpp` | 버전 확인, 로컬 Player만 결과 적용, Disconnect/월드 초기화 세대 정리. 기존 HP 적용 유지. |
| Unreal 예측/입력 | `E:\Git Fork\S1_UE5.8\Source\S1\Game\S1MyPlayer.h/.cpp`, `Combat\S1KnightCombatComponent.h/.cpp` | 시퀀스와 예측 타 연결, 계보·버퍼·블렌드아웃 복구, 사망 우선. `Abilities\S1KnightGameplayAbilities.cpp`는 조사상 변경 불필요 예상. |
| 생성 산출물 | 서버·DummyClient·Unreal의 `Protocol.pb.*`, `Enum.pb.*`, `ClientPacketHandler.h` 등 | Proto 생성기로만 갱신. 파일별 차이를 확인하고 수동 편집 금지. |
| 문서 | 본 계약, `DuskVerticalSliceImplementation.md` 등 | 구현·검증을 실제 완료한 뒤에만 상태 갱신. |

Blueprint, Montage, BP_S1DuskMinion, MainMap, Nav 데이터는 이 계약 구현의 변경 대상이 아니다. 컴포넌트의 `EditDefaultsOnly` 복구 블렌드 수치는 기존 BP 파생 객체에서 조정 가능하되 바이너리 저장을 필수로 하지 않는다. 실제 구현 전 서버 저장소의 광범위한 기존 Git 변경을 보존하고, 변경 대상의 현재 diff 및 복구 체크포인트를 다시 확인한다. S1 폴더는 현재 Git 저장소가 아니다.

## 6. 한 번에 검증 가능한 구현 단계와 인수 절차

1. **계약·생성만**: 공동 승인 후 Proto 원본 변경, 생성물 재생성, 양측 컴파일. 기존 필드 번호와 ID 유지, 새 Packet ID·버전 게이트 확인. 새 서버/구 클라이언트, 구 서버/새 클라이언트 혼합 테스트로 Unknown Packet/전투 오동작이 없는지 확인.
2. **서버 결정과 수명**: 기본 공격의 수락/거부 결과, 요청별 멱등 캐시, 활성 Action 무효화를 구현. Debug/Release GameServer 대상 빌드 후 좁은 서버 테스트로 유효 A, TOO_FAST, 단계 오류, yaw NaN, 동일 요청 재전송, 동일 시퀀스 다른 payload, 사망/퇴장 후 예약 콜백을 검사. 각 시퀀스의 Action ID 발급 수와 피해 건수를 기록.
3. **Unreal 예측 복구**: 로컬 요청-예측 매핑, 거부된 타 계보만 취소, 사망/세션 정리. `S1Editor` 빌드 후 1인 PIE에서 단타·A–D·분기 전/후 입력·홀드·빠른 연타, 수락한 헛스윙, 강제 거부 복구를 검사한다. 시각적 복구 전환은 사용자 확인 필요.
4. **2인 회귀**: 승인 공격만 상대 화면에서 재생, 양 화면 HP/사망 일치, 벽 앞/뒤와 우회 추적·복귀, 공격 중 연결 해제·재접속 후 이전 요청/피해 미재발, 로그의 중복 피해/Fatal/Ensure/Blueprint Runtime Error/설명되지 않은 Network Failure 없음. 패키지 검증은 실제 실행한 경우에만 성공이라고 보고한다.

정상 플레이로 어려운 경우 **개발 빌드 전용 제한 테스트 훅**을 사용한다. 서버 Room 단위 테스트 또는 인증된 로컬 테스트 세션에서 결정 경로에 거부 조건을 주입하고, DummyClient/패킷 테스트로 동일 시퀀스를 중복 제출한다. Unreal 수신 dispatch 직전에 결과 패킷 1건만 지연/역순 적용하는 테스트 큐로 오래된 응답을 재현한다. Shipping 전 제거·비활성화하고 계정 비밀번호를 코드·로그·도구 인자에 남기지 않는다. 단순 TCP 패킷 재전송이나 실제 네트워크 장애를 오인할 수 있는 전역 지연 설정은 쓰지 않는다.

## 7. 공동 결정 게이트와 추천값

1. **필수 승인**: 기존 Proto 2개에 버전 필드와 결정 결과 메시지/enum을 추가하고 ID `1022`를 사용하는 최소 변경. 이는 서버·클라이언트 공동 프로토콜 변경이므로 승인 전 구현하지 않는다.
2. **기본 복구 체감**: 거부 시 0.10초 블렌드아웃, 버퍼 폐기, 상태 정리 후 다음 클릭 즉시 수락 가능을 권장한다. 사용자가 1인/2인 화면에서 튐·입력 잠김을 확인하고 필요 시 0.05~0.15초 범위만 먼저 조정한다.
3. **진단값**: 결과 대기 2초, 최근 요청 32건/8초 보존을 첫 테스트값으로 권장한다. 타임아웃은 `Unknown`이지 `Rejected`가 아니며 자동 재전송하지 않는다. 실제 RTT/지연 테스트 후 확정한다.

제외: DB, 신규 몬스터, Q 권위 구현, GuardAura ID 변경, 방어·반격·회피, 타격 프레임 조정, 신규 VFX/SFX, 경사·동적 장애물. Dusk의 현 `0.7 / 28×62 / +90° / Z 61.5` 및 MainMap 정적 Nav를 변경하지 않는다. Nav 경로 거리 검사는 현재 벽 테스트의 유효한 안전 장치지만 모든 벽/동적 장애물에 대한 일반 LOS 증명은 아니다.
