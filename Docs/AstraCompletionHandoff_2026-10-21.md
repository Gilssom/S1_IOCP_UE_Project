# Astra 기획 인계 — 2026-10-21

## 목적과 Git 범위

사용자 요청: 전체 테스트 완료 기록 → S1 커밋·푸시 → 기획 Astra에 진행사항 전달.
S1 원격은 https://github.com/Gilssom/S1_IOCP_UE_Project.git 의 main이다.
이전 커밋 d63a1d6e1535c673c857c2a8787a1ef6de01340d에 NPC·퀘스트·HUD가 포함되어 있다.
이번 몬스터 Data Table 전환의 최종 커밋 ID/푸시 확인은 실제 전달 메시지에 기록한다.
GameServer의 대응 변경은 로컬 구현·빌드 상태이며 이번에 커밋·푸시하지 않는다.
Content/Blueprint/맵/DT/이미지 원본, 빌드 결과, 체크포인트, DB 백업은 source-only Git 제외다.
따라서 S1 원격만 clone하여 현재 플레이 환경 전체를 복구할 수 있다는 의미가 아니다.

## 최근 구현 완료

| 영역 | 실제 구현과 사용자 확인 | 경계 |
|---|---|---|
| NPC 대화 | G 접근/대사 진행, N번째 대사의 수락·거절 분기, 로컬 대화 카메라와 타 플레이어 숨김/복귀, 분리한 접근 UI·거리 이름표 | 대화 연출은 다른 클라이언트의 NPC 회전을 변경하지 않음 |
| 퀘스트 B | 서버 수락·진행/DB 유지, Dusk 3마리 처치, 서버 피해 기여자 인정, 재접속 후 유지 | 마지막 타격자만 인정하는 초기 제안은 사용자 지시로 대체됨 |
| 퀘스트 C | NPC 완료/포션 3개 보상, 트랜잭션·중복 방지 기록, 완료 후 tracker 제거 | 실제 서버 결과가 권위, UI만으로 보상 지급하지 않음 |
| 퀘스트 UI | O 창, 진행/완료 탭, 왼쪽 목록과 오른쪽 상세/보상, 대화 중 일반 HUD 숨김 | 현재 퀘스트 범위 기준이며 범용 콘텐츠 확장은 별도 |
| Main HUD | HP 연결, LMB/Q 스킬 칸, 1–4 소비 아이템 수량/드래그/우클릭 해제, 레벨, 별도 미니맵·화폐 WBP, 기존 스타일 이미지/아이콘 | 레벨은 입장 snapshot, EXP/레벨업 없음. 화폐 0은 임시. 지형 지도 이미지 미연결. 쿨다운 미구현 |
| 몬스터 DT | 전투 정의와 표현 매핑 DT 분리, 스폰 ID 검증, Export/Deploy/Restart, 서버 시작 전 검증/고정 | Dusk 1종의 기존 수치/AI를 데이터화. 신규 몬스터 AI를 추가한 것이 아님 |

퀘스트 처치 조건은 현재 생명주기의 서버 확정 피해 기여다. A가 때리고 B가 마무리해도
조건을 만족한 두 캐릭터 모두 각각 진행한다. 수락 전 처치를 소급하는 규칙이 아니다.
전투 결과·피해·HP·사망·아이템·퀘스트 권위는 외부 IOCP 서버에 유지한다.
Terra A–D 4타와 Q의 기존 동작을 유지한다. E 비활성/방어 보류/회피 애니메이션 미확보,
AbilityId 11002 GuardAura의 의미를 바꾸지 않는다.

## 이번 몬스터 DT의 확정 계약

- `/Game/S1/Monsters/Data/DT_S1MonsterDefinitions`: 서버 전투 수치 작성 원본.
- `/Game/S1/Monsters/Data/DT_S1MonsterPresentations`: ID → 이름/ActorClass 매핑.
- 현재 ID1, BehaviorId1, BP_S1DuskMinion. HP200/피해10/속도220cm/s,
  스케일0.7/메시 Yaw+90°/캡슐28×62cm/평지 중심Z61.5cm 유지.
- 의도한 DT 저장 → Tools/Export Monster Definitions → DeployMonsterDefinitions.ps1
  → GameServer 재시작. Build만으로 기존 배포 파일을 덮어쓰지 않는다.
- Export는 dirty DT를 거부하고 자동 저장하지 않는다. Deploy는 서버 CLI 검증 후
  기존 Config 백업·해시 확인·교체한다. 서버는 DB/네트워크 시작 전에 검증하고,
  잘못되거나 누락된 정의는 exit2로 거부한다. 런타임 reload/생성된 actor 재튜닝은 없다.
- 클라이언트 미등록 ID/누락 표현은 Dusk fallback 없이 거부한다. native CDO hard DT
  reference와 DT의 hard BP class reference로 Cook 누락을 해결했다.
- DefinitionVersion은 메타데이터이며 패킷 버전 협상 기능이 아니다.
  서버/클라이언트를 같은 작성 revision으로 배포해야 한다.
- BehaviorId1/AS1DuskMinion 계열만 지원한다. ID 추가만으로 새로운 AI·애니메이션·드랍·퀘스트가
  생기지 않는다. 포션 드랍/DB 매핑은 기존 Dusk ID1 기준. 미등록 미래 ID 드랍은 거부한다.
- 정적 Nav 벽 우회·복귀/재교전·4초 회복 유지. 경사·계단·동적 장애물은 후순위 그대로다.
- 이번 단계는 프로토콜/DB 스키마 변경 없이 진행했다.

## 검증과 증거 구분

### 구현 단계에서 실제 수행한 자동/빌드 검사

- S1Editor/S1 Development 및 GameServer Debug 빌드 성공.
- 정의/스폰/퀘스트 정의 파서 92건, 실제 Room 오프라인 43건: 실패0.
- Unreal 자동 검사38개: 37 clean + 기존 GameplayCue 경고1, 실패0.
- 실제 DT Export/서버 CLI/배포 검증, 시작 guard2건(누락·잘못된 헤더 exit2).
- 최종 Windows Development Build/Cook/Stage/Package/Archive 성공, ExitCode0.
  IoStore 목록에서 DT2/BP_S1DuskMinion/필수맵3개 포함 확인.
  패키지 NullRHI 스모크 exit0 및 DT2/BP class 로드 확인.
- 첫 Cook의 DT/BP 누락과 중간 UBT mutex 실패는 별도 보존하고 수정/직렬 재실행했다.
  최종 성공만 기록한 것이 아니며 자세한 이력은 MonsterDataTables 문서 참조.

### 사용자 실제 플레이 인수

2026-10-21 사용자가 직전 전체 체크리스트의 1인·2인 PIE 테스트 완료를 보고했다.
다중 스폰/외형/방향/높이/HP, 벽 우회, A–D/Q 및 몬스터 타격, 벽 너머 차단,
복귀·HP 유지 재교전·4초 회복/중단, 사망·재생성, 포션 드랍·획득,
기여 처치 진행도, 양쪽 결과와 재접속 회귀를 최신 사용자 확인 완료로 분류한다.
이전 NPC/퀘스트/보상/HUD 사용자 인수는 각 구현 문서에 구분하여 보존했다.

### 아직 확인하지 않은 것

- 최신 몬스터 DT/HUD/퀘스트 조합의 패키지 2계정 실제 플레이.
  이전 패키지 인수나 이번 NullRHI 스모크를 이것의 성공으로 간주하지 않는다.
- 이번 정리·커밋 작업에서 새 빌드/PIE/독립 로그 감사/DB 복원 시험은 실행하지 않았다.
- source-only Git은 에셋/서버/DB 전체 복구 검증이 아니다.

체크포인트/로그/패키지 근거:
`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MonsterTables_20261020`,
`E:/Git Fork/S1_UE5.8/Saved/MonsterTablesPackage_20261020/Windows`.
이 로컬 산출물은 Git 원격에 포함하지 않는다.

## 다음 기획 검토 요청 / 건의

1. 이미 통과한 PIE를 다시 반복 요구하기보다, 최신 패키지 2계정 최종 통합 확인과
   포트폴리오 시연 시나리오/촬영 순서를 작은 독립 범위로 우선 검토해 달라.
2. 배경/맵 선택 뒤 정적 지형 이미지 기반 미니맵을 연결할 수 있다. 현재 미니맵은
   북쪽 고정 좌표/방향 UI + 설정 가능한 이미지 틀이며 실제 지형 지도는 아니다.
3. 다음 몬스터 확장은 모델 교체 수준인지 신규 행동인지 먼저 분리하고,
   AI/애니메이션/드랍/퀘스트 등록을 명시하라. DT 행 추가만으로 완성된다고 약속하지 말라.
4. 신규 직업/화폐/EXP/보스/다중 스킬을 동시에 확대하지 말고, 시연에 꼭 필요한
   다음 한 범위와 인수 조건을 제안해 달라. 이번 전달 자체로 신규 구현은 시작하지 않는다.

## 관련 문서

- Docs/MonsterDataTables_2026-10-20.md (최신 PIE 인수 추가)
- Docs/CurrentAssetRegistry.md (DT2를 사용자 PIE 인수 Active로 갱신)
- Docs/MainHUDExpansion_2026-10-20.md / MainHUDImplementation_2026-10-20.md
- Docs/QuestStageBImplementation_2026-10-19.md / QuestStageCRewardImplementation_2026-10-19.md
- Docs/QuestJournalTabs_2026-10-19.md / NpcDialogueStageAImplementation.md
- Docs/MaruBuriFontIntegration.md / GitManagement.md
