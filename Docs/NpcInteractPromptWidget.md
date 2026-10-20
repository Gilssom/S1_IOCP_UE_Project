# NPC 접근 대화 안내 WBP 분리 — 2026-10-16

사용자 요청으로 기존 [G] 대화 안내를 Designer 편집 가능한 별도 위젯으로 분리.

## 변경 범위

- 신규 /Game/S1/UI/WBP_S1NpcInteractPrompt, 부모 /Script/UMG.UserWidget.
- 기존 /Game/S1/Quest/BP_S1QuestNpc의 inherited DialoguePrompt 컴포넌트
  Widget Class만 새 WBP로 변경. C++ 기본 클래스를 바꿀 필요 없이 BP override로 연결.
- 코드/서버/DB/패킷/입력/거리/카메라/대화 WBP/MainMap 변경·저장 없음.
- native US1NpcPromptWidget는 native AS1QuestNpc의 fallback으로 남아 있지만,
  현재 MainMap의 BP NPC는 새 WBP를 사용한다. 삭제/클래스 변경 없음.

## 편집 위치

WBP_S1NpcInteractPrompt의 Designer:

```text
PromptSize (SizeBox, 200×48)
└─ PromptCanvas (Canvas Panel)
   ├─ PromptBackground (배경 Border)
   ├─ KeycapBackground (키캡 Border)
   │  └─ KeyText (G)
   └─ InteractionText (대화)
```

Canvas 아래 요소는 Position/Size로 자유 배치. Label은 AutoWrap=false.
색·폰트·문구·키캡 그림을 Designer에서 변경할 수 있다. 데이터 바인딩 필수 이름은
없으며 정적 안내 위젯이다. G의 실제 입력 키는 바꾸지 않았으므로 표시만 다른 키로
바꾸면 입력과 달라진다는 점에 주의한다. 장식/루트 HitTestInvisible, focus 없음.
생성형 PNG는 임포트하지 않고 기본 Border/Text만 사용했다.

위치/화면 크기는 BP_S1QuestNpc Components→DialoguePrompt에서 조절:
Relative Location Z210, DrawSize200×48, Screen space, WindowFocusable=false.
크기 변경 시 WBP PromptSize도 함께 맞춘다. G 후보 선택/표시 가능 여부/대화 중 숨김은
기존 S1DialogueSubsystem→Npc.ShowPrompt에서 그대로 제어하며 위젯은 요청을 보내지 않는다.

## 체크포인트/저장

외부 checkpoint: C:/Users/user/Documents/Unreal IOCP Server/checkpoints/NpcInteractPrompt_20261016.
수정 전 BP SHA25621163BCFEEBDF89065FA1361CB1B280C46B8C9B1DA2B09182DDBEC54B021A1A7,
ContentBefore에 복사. 새 WBP/변경 BP 두 패키지만 dirty임을 보고한 뒤 선택 저장.
기존 대화WBP/MainMap dirty=false 확인했고 저장하지 않았다.
ContentAfter에 두 파일 보존:
- BP SHA256 C14C8CC656070A76F8F459219F4CC32A2131995D8D7225EDBDA69B531BC734C9.
- WBP SHA256 8FC2C02B93EEF6324ECC3C4D1357BDB7DFC9EB93E960BEC079294543DA726E40.

## 검증

- 두 Blueprint warnings-as-errors 컴파일 통과. 새 위젯6개, 부모 UserWidget.
- 저장 후 둘과 기존 MainMap/대화WBP dirty=false.
- MainMap 재로드 후 배치된 BP NPC의 Widget Class가 새 WBP임을 확인.
- Asset Registry BP 의존성에 새 WBP 포함: MainMap→NPC BP→Prompt WBP.
- 짧은 MainMap Simulate PIE에서 UEDPIE_0 NPC의 DialoguePrompt WidgetClass가
  WBP_S1NpcInteractPrompt_C, Screen/200×48/WindowFocusable=false임을 조회.
  StopPIE 후 새 WBP editor를 열어둠. 로그인/캐릭터 이동/실제 G 조작은 시행하지 않음.
- C++ 수정 없으므로 신규 빌드/전체 자동 테스트는 실행하지 않았다.
- 현재 리비전의 실제 접근 표시·G/loot 배타 처리 사용자 화면 및 cook/packaged
  실행은 미검증. 이전 인수를 이 새 WBP의 화면 인수로 바꾸어 기록하지 않는다.
