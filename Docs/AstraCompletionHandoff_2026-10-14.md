# Astra 최신 구현 완료 인계 — 2026-10-14 (KST)

사용자 요청으로 기획 담당에게 전달. 기획안/구현/사용자검증/자동검증을
구분하며, 새로운 구현이나 커밋·푸시를 요구하지 않는다.

## 최신 기준으로 갱신할 사항

1. 교환·합치기·분할은 더 이상 제안 단계가 아니다. 사용자 승인 후 구현,
   테스트 완료 보고를 받았다. 일반 드래그 동일포션99개까지합치기/잔량유지,
   Ctrl 교환, Shift 빈칸 수량분할; 기존 빈칸 이동 유지.20사용/80표시 슬롯.
   상세/증거: InventoryStackOperationsImplementation.md.
2. 영구삭제 버리기를 승인·구현·사용자 검증한 이후, 플레이어가 버린 포션만
   공용 필드 드랍으로 전환했다. 몬스터 드랍은 기존 소유자 전용 유지.
   영구삭제1037/1038 의미를 바꾸지 않았고 새 서버 capability0/Unsupported,
   새 클라이언트 fallback 없음. 현재 기본 버리기는 영구삭제가 아니다.
3. 새1039/1040, C/S_ENTER_GAME capability field8/10, LootDropInfo.source_kind10.
   승인된 추가3InnoDB 테이블 PlayerFieldDropRequest/PlayerFieldDrop/
   PlayerFieldDropGrant를 격리 검증 후 사용자가 GameDB에 적용했다.
   기존 테이블 ALTER 없음. 차감·드랍·요청·version·outbox 원자적 commit,
   입력 fingerprint/영속 replay, 미확정은 같은 요청 readback만 수행.
   전량 획득/스택여유+빈칸 합산/동시획득1명/30초만료/boot변경만료.
   포션20001만, 현 MainMap 평지 높이/기존 거리·벽 판정 유지.
4. 처음 PIE에서 생성이 안 보이는 버그 발견: UTC DATETIME을
   UNIX_TIMESTAMP가 한국 세션 시간대로 재해석해9시간 전으로 판단.
   기존 monster 방식 TIMESTAMPDIFF UTC epoch로 수정. +09/+00 독립성과
   최초30초 수명 회귀 테스트 통과. 사용자2PIE에서 양쪽표시/타인G획득/
   양쪽제거/미획득30초소멸 전체 확인 완료. 이전 차감 임의환불 없음.
5. G 획득 안내를 /Game/S1/UI/WBP_S1LootPickupPrompt로 분리 구현.
   S1LootPickupWidget 부모,8widgets/KeyText·ItemText·StatusText3bindings.
   화면형 WidgetComponent, 로컬 선택후보1개/소유권/pending 정책 유지.
   Designer에서 배경·폰트·배치 편집, Potion Presentation Pickup UI에서
   높이70cm/화면offset 조절. 입력/focus를 받지 않음. 사용자 최신 응답
   '여기까지 테스트 완료'로 플레이 인수 보고 확인.
6. UI 디자인 후보도 제공: 기존 inventory/split/overhead 리소스와 같은
   navy/silver/gold 빈 키캡 PNG. UIResources/KeycapFantasy_v1,
   1254×1254 RGBA, 외부알파0/중앙253. 문자는별도TextBlock.
   Unreal import/키캡적용은 확인하지 않았으므로 Candidate 유지.

## 수행 증거와 남은 검증

- GameDBBeforeFieldDrop 테이블/데이터 백업 확보,9테이블/완료마커/해시
  확인. 전체 인스턴스백업/PITR/복원리허설 완료로 과장하지 않음.
- 공용 드랍 격리 DB atomic/replay/conflict/rollback/capacity/conservation/
  concurrent pickup/refund/boot 및 시간대 테스트 모두통과,테스트fixture정리.
- Debug 서버 및 S1/S1Editor 빌드 성공. UI분리 최신자동26개통과,
  실패0(기존Q GameplayCue 경고1). 영향을받은WBP 경고를오류로컴파일통과.
- Windows Development package 성공. Archived exe에서 새pickup WBP
  generated class 실제로드 확인. 기존 인증UI Non-Focusable focusError1은
  남아있으며 이번 기능 성공으로 해결됐다고 주장하지 않음.
- 새 UI만 선택저장/체크포인트해시검증,기존UI·맵·Terra 저장안함.
- 실제 packaged2계정 통합플레이, oldbinary호환/네트워크지연·중복·실패
  주입, 최종키캡UI 적용/DPI 가독성은 미검증. 현재 사용자PIE 인수와 구분.
- 후속 단계에서 우선 검토할 것은 기존 인증UI focus 오류 정리와
  패키지2계정 통합/영상용 시연 체크리스트. 추가 콘텐츠 범위는 사용자 결정.

## 참고 문서와 현재 종료 요청

PlayerFieldDropPlan_2026-10-14.md, PlayerFieldDropImplementation.md,
LootPickupUIImplementation.md, InventoryStackOperationsImplementation.md,
InventoryDiscardImplementation.md(이전 영구삭제 이력),CurrentAssetRegistry.md.
사용자가 전달 후 GameServer 종료를 요청했다. Astra 메시지 전달 성공 후
정확한 Debug 경로의 PID18680만 종료했다. 후속 조회 GameServer0개 확인.
Unreal Editor 종료 명령은 실행하지 않았고 최종 조회에서 이미0개였다.
이번 후속 작업에서 Git commit/push는 실행하지 않았으며 Content는 Git
제외 정책으로 별도 체크포인트에 보관한다. 이전 S1 commit8d6e0cc 이후
현재 변경을 이미푸시된 것으로 간주하지 않는다.
