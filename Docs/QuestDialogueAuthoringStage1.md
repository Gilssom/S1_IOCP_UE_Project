# 퀘스트·대화 데이터 1차 구현 — 2026-10-19

후속: 사용자가 G 진행 수정본 확인 완료를 보고했다(클라이언트 수/패키지 범위 미지정).
B단계 방향은 마지막 타격자가 아니라 수락 후 피해 기여자 전원 인정으로 확정했다.
상세 소스 조사·패킷/DB 변경 제안은 QuestStageBContract_2026-10-19.md를 참조.
해당 계약은 승인 대기 설계안이며 현행 구현·DDL 적용 기록이 아니다.

## 사용자 인수와 G 진행 후속

사용자가 1차 버전에서 일반 대사2개→3번째 선택창→수락시험/거절 전용 대사와
닫기·Esc 후 이동/공격/카메라 복귀 테스트를 완료했고 문제없다고 보고했다.
클라이언트 수나 모든 장애 경로까지 인수했다고 확대 해석하지 않는다.

추가 요청으로 Line 대사는 다음 버튼과 G키를 모두 지원한다. 버튼 문구는
`다음 [G]`. Widget NativeOnPreviewKeyDown에서 처리해 자식 버튼 포커스에서도
일관되게 작동하도록 하며 OS 키 반복은 무시한다. Opening G를 계속 누르고 있어도
후속 대사를 연속 소모하지 않는다. Offer/End에서는 G를 소비하되 행동하지 않아
수락/거절/획득/다른 상호작용으로 전달하지 않는다. 수락·거절은 명시적 클릭,
종료는 기존 닫기/Esc. 서버·DB·Data Table/Blueprint 바이너리 변경 없음.

변경: UI/S1DialogueWidget.h/.cpp, Quest/S1DialogueSubsystem.cpp,
신규 Quest/S1DialogueInputTests.cpp와 문서. 기존 Phase::Active와 FSM 전이를
재사용하고 별도 gameplay G 바인딩을 추가하지 않는다. Line에서만 OnAdvanceLine
delegate를 연결하고 노드 변경/종료/destruct에서 해제한다.
Checkpoint DialogueGKey_20261019/SourceBefore. 수정 전 MCP PIE=false,
WBP_S1Dialogue clean.

G 수정본 검증: S1Editor/S1 Win64 Development 빌드 exit0, 자동31개
(30clean +1기존 GameplayCueNotifyPaths 경고,0실패/미실행) 통과.
DialogueGKey/Authoring/DialogueStageA 자체는 경고·오류0. 직접 preview-key 호출로
단발1회, 반복무시, Offer delegate 분리, Esc/cleanup을 검사했다. 실제 Slate의 자식
포커스→터널 입력 경로를 사람이 조작한 검증과는 구분한다.
새 에디터/MCP PIE=false, 기존 대화 WBP warnings-as-errors 컴파일 통과/dirty=false.
시작 로그 Error/Fatal/Ensure/Blueprint Runtime Error 스캔 결과 없음. 저장한 애셋 없음.
증거 checkpoint DialogueGKey_20261019의 빌드·자동 로그와
Saved/Automation/DialogueGKey_20261019/index.json. G 단발/홀드/버튼포커스/
수락창 G 무동작 실제 PIE·패키지 검증은 미검증이며 사용자 확인을 요청했다.

## 범위와 현재 상태

사용자 승인: 퀘스트 Data Table·검증·서버 Export/로더 준비에 대화 노드와
다음/수락 분기 미리보기/거절 분기를 추가. **서버 퀘스트 행동, DB, 패킷,
수락 기록·처치 집계·보상 지급은 구현하지 않는다.** ready는 정의 파일 검증 성공을
뜻하며 게임플레이 기능 활성화나 서버 행동 승인을 뜻하지 않는다.

이 문서의 초기 B/C 패킷·DB 초안은 이번 구현의 프로토콜이 아니다.
기존 전투·G 획득·인벤토리·이동 권한·카메라 복구 경로를 유지한다.
새 에셋 구매/모델/애니메이션/폰트 변경 없음. 기본 샘플 목표3/포션3은 미확정
비활성 시험값이며 콘텐츠 규칙 승인으로 취급하지 않는다.

## 에디터 편집 위치

- `/Game/S1/Quest/Data/DT_S1QuestDefinitions`: 행 구조 `FS1QuestDefinitionRow`.
  QuestId, DefinitionVersion, bEnabled, Title, Description, AcceptNpcId,
  ReturnNpcId, MonsterDefinitionId, RequiredKills, RewardItemId, RewardCount.
- `/Game/S1/Quest/Data/DT_S1DialogueNodes`: 행 구조 `FS1DialogueNodeRow`.
  NodeId, NpcId, Kind, Speaker, Body, NextNodeId, QuestId, AcceptNodeId, DeclineNodeId.
- NPC BP/배치 액터 → NPC → Dialogue Data: QuestDefinitions/DialogueNodes,
  StartDialogueNodeId, Preview Quest Branches.

Data Table RowName은 편집용 이름이다. 실제 연결은 숫자 NodeId/QuestId로 한다.
처음에는 처치 목표1개·보상 아이템1종만 지원. 서버 현재 등록 Dusk=1,
회복 포션=20001만 검증 허용. 다른 몬스터/아이템을 추가하면 서버 실제 정의 등록과
클라이언트 Export 검증 목록도 함께 확장해야 한다. 서버가 최종 재검증한다.

| Kind | 연결 방법 |
|---|---|
| Line | NextNodeId만 양수. QuestId/AcceptNodeId/DeclineNodeId는0 |
| QuestOffer | QuestId와 두 분기 ID 양수, NextNodeId=0 |
| End | 모든 연결 ID=0. 닫기만 표시 |

예제: 1 Intro01 → 2 Intro02 → 3 QuestOffer → 수락 시험4 / 거절5.
따라서 한 번의 대화에서 3번째 대사에 선택창이 나타난다. NPC에게 말을 건 횟수를
누적하는 시스템이 아니다. 대화마다 시작 노드부터 다시 시작하고 수락/거절 기록은
저장하지 않는다. N을 바꾸려면 Line 노드와 NextNodeId 연결을 편집한다.
대사 화자는 Speaker, NPC 머리 위 이름은 기존 DisplayName을 따로 편집한다.

### 미리보기 표시

BP_S1QuestNpc에 Preview Quest Branches=true를 샘플 시험용으로 지정했다.
Offer에만 퀘스트 상세 패널을 표시하고 '[미리보기 · 저장 안 됨]' 경고,
버튼 '수락 분기 시험'을 사용한다. 수락 클릭은 로컬 NodeId만 바꾼다.
true라도 패킷/DB/처치/아이템 처리를 하지 않는다. false면 '수락 미연결'로
비활성화하며 거절/닫기는 사용할 수 있다. bEnabled를 true로 바꾸어도 실제 수락은
연결되지 않는다. 실제 구현 시 서버 수락 성공 응답 후 AcceptNodeId로 넘어가야 하며
현재 ChoosePreview를 서버 승인 함수로 재사용하면 안 된다.

기존 WBP_S1Dialogue/마루부리 디자인은 그대로 사용한다. AcceptButton/DeclineButton과
AcceptLabel/DeclineLabel을 재사용하여 다음/분기/닫기 문구를 설정한다.
QuestDetailFrame은 Offer에서만 표시. Esc는 분기 선택 없이 대화를 종료한다.
닫기/피격/사망/거리이탈/단절/월드종료 시 기존 cleanup이 세션 NodeId와 delegate도
정리한다. 늦은 응답을 처리하는 네트워크 계약은 아직 없으므로 구현했다고 주장하지 않는다.

## 검증·Export·배포

1. PIE를 끝내고 두 Data Table을 편집·저장한다.
2. MainMap NPC 액터를 선택한다. Details에서 `ExportQuestData` 버튼을 누른다.
   검색창에 Export를 입력하면 찾을 수 있다. Blueprint 기본값에서도 사용 가능.
3. Output Log의 `QuestExportComplete`와 출력 경로를 확인한다.
   `Saved/QuestExports/QuestDefinitions.s1quest`가 생성된다.
4. PowerShell에서 `Tools/DeployQuestDefinitions.ps1` 실행. 기본 서버 경로는 현재
   로컬 Debug 실행파일이며 다른 위치는 -ServerExecutable로 지정한다.
5. 스크립트는 **실제 서버 실행파일**의 `--validate-quest-definitions <file>` 모드로
   재검증한다. 이 모드는 main에서 DB/네트워크 시작 전에 종료한다. 성공한 파일만
   실행파일 옆 `Config/QuestDefinitions.s1quest`에 배포한다. 기존 파일은 timestamp
   백업하고 복사 SHA256을 확인한다. 실행 중 서버 데이터는 바뀌지 않는다.
6. 서버를 재시작한다. `Quest definitions loaded ... Definition-only; gameplay disabled`
   로그를 확인한다. 실패하면 `Quest definitions unavailable ... Quest feature disabled`
   와 이유를 남긴다. 기존 전투/아이템 서버는 계속 사용할 수 있다.

기본 서버 로더 경로는 작업 디렉터리가 아니라 **exe 위치 기준**이다.
별도 경로는 GAMESERVER_QUEST_DEFINITIONS_FILE 환경 변수로 지정할 수 있다.
현재 소스용 예제도 GameServer/GameServer/Data/QuestDefinitions.s1quest에 보존했다.
Release나 별도 배포 exe에는 해당 exe를 지정해 배포해야 한다. Debug만 복사했다고
Release/AWS에 반영됐다고 간주하지 않는다. 실시간 hot reload는 없다.

검증: 빈 값/중복ID/누락대사/다른NPC연결/순환(분리된 그래프 포함)/없는퀘스트·NPC/
목표·보상ID/잘못된 수량을 검사한다. Quest 1~256행, 대사1~4096행, 파일1MiB 한도.
클라이언트 제목·화자256 UTF-16코드단위, 본문8192 제한; 서버는 UTF-8 byte 한도로
1024/32768을 검사한다. 현재 번역된 FText 문자열을 Export하며 다국어 배포 계약은 별도다.
문자열은 std::quoted 호환 따옴표 형식으로 한글·따옴표·백슬래시·본문 줄바꿈을 보존한다.
JSON 파서를 새로 도입하지 않고 기존 서버 스폰 Export와 유사한 `S1QUEST 1` 텍스트를
채택했다. 이는 디스크 파일 포맷이지 네트워크 프로토콜이 아니다.
Export 검증 실패 시 기존 출력 파일을 덮어쓰지 않는다. 성공 시 임시 파일→교체.
서버 파서 실패 시 부분 정의/과거 ready 상태를 남기지 않는다.

## 변경 파일·애셋

클라이언트:
- 신규 Source/S1/Quest/S1QuestData.h/.cpp, S1QuestDataTests.cpp.
- 수정 Quest/S1QuestNpc.h/.cpp, S1DialogueSubsystem.h/.cpp,
  UI/S1DialogueWidget.h/.cpp.
- 신규 Tools/DeployQuestDefinitions.ps1.
- 신규 Data Table2개, 기존 BP_S1QuestNpc에 하드 참조/시작노드/preview 연결.
  WBP는 부모 새 binding으로 컴파일만 했고 바이너리를 저장하지 않았다.

서버:
- 신규 GameServer/GameServer/QuestDefinitionConfig.h (정의 로더와 전역 read-only 준비 캐시).
- 수정 GameServer/GameServer/GameServer.cpp (시작 로드/검증 전용 CLI).
- 신규 GameServer/Tests/QuestDefinitionConfigTests.cpp, GameServer/GameServer/Data/
  QuestDefinitions.s1quest 예제. DB/프로토콜/몬스터 규칙/Room/IOCP 권한 변경 없음.
- 기존 MSBuild prebuild가 Protobuf/handler 생성을 자동 실행함. .proto 변경이나 생성물
  직접 편집은 하지 않았으며 클라이언트 generated 파일의 내용 diff는 없었다.

## 체크포인트와 검증 기록

외부 `checkpoints/QuestAuthoring_20261019`: ContentBefore(BP/WBP), SourceBefore,
ContentAfter(NPC BP/두 DT/서버 Export), 빌드·자동 테스트 로그·도우미 보존.
기존 양쪽 dirty Git 변경과 서버 스테이징은 그대로 보존했다. 커밋/푸시 없음.
저장 전 DT2개+NPC BP만 dirty임을 보고 후 선택 저장. MainMap/대화WBP clean/미저장.
의존성은 MainMap → NPC BP → DT2개로 이어진다. 새 cook 실행은 별도 확인이 필요하다.

- S1Editor, S1 Win64 Development 빌드 성공. 최초 UE 타입 별칭 컴파일 오류 수정 후 통과.
- Debug GameServer 빌드 성공; 검증 CLI 추가 후 최종 빌드 결과는 후속 기록 참조.
- NPC BP/대화 WBP warnings-as-errors 컴파일 통과. MainMap 재로드 후 배치 NPC의
  테이블2개/Start1/preview=true 확인.
- 실제 에디터 ExportQuestData 버튼을 MCP Slate로 눌러 파일 생성.
- 독립 서버 파서: 정상·중복·미등록ID·순환·누락·초과·손상·실패 후 상태정리 등21개,
  실제 Unreal Export 파일 추가 검사까지22개 성공.
- Unreal 자동30개: 29clean +1기존 GameplayCueNotifyPaths warning,0실패/미실행.
  Authoring/DialogueStageA 경고0. transient Data Table로 양 분기/preview차단/초기화/
  결정적 Export/잘못된 참조를 검증했다. 실제 로그인 후 버튼 조작 테스트와는 구분한다.
- Export와 Debug 배포본 SHA256:
  ED8F028C0234D64C38C9D2C432C93153E89D3B2F3D9F380908628A259679B13C.
- 미검증: 인증된1/2인 PIE의 실제 다음/분기/레이아웃/종료조작, 서버 정상 DB 시작로그,
  최종 패키지 cook/실행. 사용자에게 화면 확인을 요청했다. 미실행을 통과로 보고하지 않는다.

### 최종 서버 적용 후속

검증 CLI 추가 후 C++20 u8path deprecation 오류를 Windows Unicode wmain/path로 수정,
최종 Debug 서버 빌드 exit0. 실제 EXE `--validate-quest-definitions`가 Export1개/대사5개를
승인하고 종료0, 없는 파일은 종료2로 거부했다. DeployQuestDefinitions.ps1 실행도
실제 서버 검증 후 동일 SHA256 배포본을 확인해 성공했다. 초기 정상 빌드 기록과
이 최종 검증 CLI 빌드를 구분하며 실패 빌드를 성공으로 기록하지 않는다.

새 Debug 서버 PID38388을 기존 프로세스 환경의 DB 자격정보로 실행했다(자격정보
로그/문서/명령 인자에 포함하지 않음). Bootstrap admin password는 자식 프로세스에서
비워 불필요한 관리자 비밀번호 갱신을 피했다. ServerStartup.log에서
`Quest definitions loaded. Quests[1] Dialogues[5]. Definition-only; gameplay disabled.`,
Account SELECT, field-drop/loot/potion boot recovery 성공을 확인했다.
따라서 위의 '서버 정상 DB 시작로그 미검증'은 이 후속으로 해소했다. 인증된 PIE
다음/양분기/종료복구·2인 독립 대화·새 패키지 검증은 여전히 미검증이다.

기획과 차이: JSON 대신 위의 검증된 텍스트 형식. N번째는 한 대화 내 노드 순서.
샘플 수락은 명시적 preview만 제공. 다음 B단계의 서버 수락·DB·패킷 계약은 별도 승인.
갱신 문서: 본 문서, CurrentAssetRegistry.md.
