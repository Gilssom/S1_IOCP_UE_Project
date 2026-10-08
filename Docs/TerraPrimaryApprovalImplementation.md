# Terra A–D 승인·거부 및 예측 복구 — 2026-09-23 구현 기록

이 문서는 실제 변경과 검증 범위를 기록한다. [계약 초안](TerraPrimaryApprovalContractDraft.md)의 모든 인수 항목을 완료했다는 뜻이 아니다.

## 구현된 동작

- 기존 `C_COMBAT_ACTION`에 계약 버전 필드를 더하고, 성공한 `S_ENTER_GAME`에서 서버 버전 1을 광고한다. 새 클라이언트는 버전 1을 받지 못하면 Terra 기본 공격 입력을 시작하지 않는다. Q의 기존 연출 요청은 유지한다.
- 서버는 기본 공격의 수락/거부를 요청자 전용 `S_COMBAT_ACTION_RESULT`(패킷 ID 1022)로 통보한다. 기존 1000–1021 ID는 유지한다. 수락한 공격만 기존 `S_COMBAT_ACTION`으로 요청자를 제외한 Room에 중계한다. 수락은 명중과 다르며 피해는 기존 `S_DAMAGE_RESULT`에서만 확정한다.
- 요청 시퀀스는 Q와 공유한다. 서버는 최근 32건을 Player별로 보관해 동일 요청 재수신 시 동일 결과만 돌려주고, 같은 시퀀스의 다른 내용·오래된 시퀀스는 새 Action 없이 거부한다. 사망·Room 퇴장·세션 변경 시 기존 행동의 타격 예약이 유효하지 않도록 활성 Action과 Player/Session 소속을 확인한다.
- 클라이언트는 Montage를 즉시 시작하되 시퀀스와 선행 예측 타를 기록한다. 거부가 현재 타 또는 미승인 후속 타의 계보에 해당하면 0.10초 블렌드아웃과 함께 버퍼·타이머·이동 제한을 정리한다. 이미 수락된 독립적인 최신 타는 늦은 이전 거부로 끊지 않는다. 2초 무응답은 `결과 미확인` 로그일 뿐 거부·명중 실패나 자동 재전송으로 처리하지 않는다.
- 사망, 월드 레지스트리 초기화, 연결 종료 시 로컬 예측을 정리한다. 세션이 바뀔 때만 로컬 시퀀스를 초기화한다. Blueprint, Montage, MainMap, Dusk 외형/캡슐/Nav 데이터는 변경하지 않았다.

## 변경 대상

- 서버 원본: `E:\Git Fork\GameServer\GameServer\Common\Protobuf\bin\Protocol.proto`, `Enum.proto`.
- 서버 로직: `E:\Git Fork\GameServer\GameServer\GameServer\Room.cpp`, `Player.h`, `ClientPacketHandler.cpp`.
- Unreal 로직: `Source/S1/Combat/S1KnightCombatComponent.h/.cpp`, `Source/S1/Game/S1MyPlayer.h/.cpp`, `Source/S1/ClientPacketHandler.cpp`, `Source/S1/Network/S1NetworkSubsystem.h/.cpp`, `Source/S1/S1GameInstance.h/.cpp`, `Source/S1/World/S1WorldEntitySubsystem.h/.cpp`.
- 기존 생성 스크립트로 서버·DummyClient·Unreal의 `Enum.pb.*`, `Protocol.pb.*`, `Struct.pb.*`와 양측 PacketHandler 헤더를 재생성했다. 생성물 직접 편집은 하지 않았다.
- 변경 전 원본·생성물 체크포인트 두 개는 `C:\Users\user\Documents\Unreal IOCP Server\checkpoints\`에 있다. 서버 저장소의 기존 광범위한 변경은 보존했다. S1 프로젝트에는 Git 저장소가 없다.

## 이번에 실제 수행한 검증

| 검사 | 결과 |
| --- | --- |
| 서버·Unreal Protobuf 헤더 SHA-256 비교 | 각각 동일. 기존 Packet ID 1018–1021 유지, 신규 1022 확인. |
| `GameServer Release x64` 대상 빌드 | 성공, `Binary/Release/GameServer.exe` 링크 완료. |
| `GameServer Debug x64` 대상 빌드 | 소스 컴파일 완료, 실행 중인 `Binary/Debug/GameServer.exe` 잠금으로 링크 실패. 새 Debug 서버는 실행되지 않음. |
| `S1 Win64 Development` 게임 타깃 빌드 | 성공, `Binaries/Win64/S1.exe` 링크 완료. |
| `S1Editor Win64 Development` 빌드 | 모든 변경 소스 컴파일 완료. 열린 UnrealEditor의 DLL 및 Visual Studio/디버거의 PDB 잠금으로 최종 링크 실패. 최초 빌드에서 발견한 float 축소 변환 컴파일 오류는 수정하고 재빌드했다. |

## 미검증·다음 순서

1. 사용자가 에디터와 Visual Studio의 미저장 작업을 확인하고 종료한 후 `S1Editor` 전체 링크를 다시 실행한다. 실행 중인 Debug GameServer도 세션을 안전하게 종료한 뒤 새 바이너리로 재시작해야 이번 계약의 PIE가 의미 있다. 현재 서버/에디터를 강제 종료하지 않았다.
2. 개발 빌드 전용 제한 테스트로 정상 수락/헛스윙, TOO_FAST·콤보 순서·잘못된 yaw 거부, 중복/충돌 시퀀스, 늦은 거부·무응답, 사망·퇴장 후 예약 타격 무효화를 확인한다. 이 주입 테스트와 혼합 버전 테스트는 아직 실행하지 않았다.
3. 인증된 1인/2인 MainMap PIE에서 A–D 입력 감각, 상대에게 승인된 공격만 보이는지, 양쪽 HP/사망, 벽 앞뒤 전투·Dusk 우회 추적/복귀, 접속 해제·재접속, 로그 오류를 검사한다. **이번 변경본의 PIE는 아직 미실행**이다.
4. 시각적인 거부 블렌드아웃 자연스러움은 사용자가 확인한다. 실제 타격 접촉 시각 조정, Q 권위 이동/피해, 경사·동적 장애물은 이번 범위 밖이다.

주의: 이전 2026-09-23 두 클라이언트 Nav/HP 테스트 성공은 **변경 전 버전의 사용자 확인**이다. 현재 새 프로토콜이 적용된 PIE 성공으로 보고하지 않는다.
