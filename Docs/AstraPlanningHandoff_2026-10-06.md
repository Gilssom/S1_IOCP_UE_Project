# Astra 기획 인계 — 2026-10-06 진행사항

## 인계 목적

오늘 사용자 승인으로 완료한 구현/검증을 기획 기준에 반영한다. 구현 담당의 실제 빌드·자동 검사, 사용자 플레이 인수, 미검증 항목을 구분한다. 신규 콘텐츠나 다음 기능 구현을 자동 승인하는 문서가 아니다.

## 1. 회복 포션 사용 / 몸통 이펙트

기존 드랍→G획득→I인벤토리에서 스택 선택/사용→외부 IOCP서버의 포션1개소비·min(50,부족HP)회복→서버 공개 성공 이벤트의 양쪽FX→재접속 수량 유지 경로를 구현했다. 추가 테이블 ItemUseOperation과1030~1032 사용/결과/회복 패킷은 구체안 사용자 승인 후 구현된 것이다. UI는 직접HP/수량을 변경하지 않는다. 요청멱등·unknown조회·세션/생명검사·대상별이벤트순서·FX중복방지와 유한수명 정리가 있다.

사용자는 최신1인·2인PIE에서 회복량, 수량감소, 양쪽FX각1회, 최대HP무소비, 재접속수량유지, 몸통FX높이를 명시 확인했다. `/Game/VFX_StatusEffect01/FXS/FXS_healing000`은 회복용Active. DefaultGame.ini HealingFootOffset Z96으로 현재 half-height96을 상쇄해 몸통중심에 붙인다. 원본Niagara수정없음. 사용 사운드/버프/퀵슬롯/포션고유쿨다운/모션잠금은 미구현이며 공통 RequestUseItem API만 준비했다.

포션의 이전 자동화13개·격리DB테스트·서버/클라이언트빌드·패키징 증거는 PotionUseImplementation.md에 있다. 몸통높이변경 이후 오늘 다중스폰 전체패키징도 성공했다. 그러나 실제패키지2계정포션시각인수, 진짜COMMIT응답손실/서버crash/동시퇴장 등의 장애 주입은 별도다. HP전체DB영속화는 없고, HP적용 후 durable Applied ACK 전 crash에서 다음부팅에 포션반환 가능성이 있는 승인된 손실방지정책이다. DB+런타임HP 원자적 exactly-once를 주장하지 않는다. FX출처/공개영상권한은 확인대기.

## 2. 에디터 스폰 포인트

최초 단일Dusk 배치용 AS1MonsterSpawnPoint와 `/Game/S1/Editor/BP_S1DuskSpawnPoint`를 구현했다. 발/바닥 기준 마커이며 실제몬스터를 로컬로 생성하지 않는다. Tools→S1 Server Authoring의 Export 메뉴, Saved/ServerData/MainMap.spawn, 별도 DeployDuskSpawnPoint.ps1, 서버재시작 순서다.

사용자가 Export Complete·재시작에도 위치가 안 바뀐다고 보고했을 때 에디터출력(904.40,15.17)과 서버Data(0,600) 불일치를 확인했다. Export는 성공했지만 별도배포가 빠진 원인이었다. 서버종료 후 새좌표를 canonical/Debug/Release Data에 배포·SHA확인했다. Export Complete를 배포완료와 혼동하지 않도록 문서화했다. 자동live설정변경/서버실행중파일반영은 제공하지 않는다.

## 3. 공용 포인트 + 다중Dusk 1차

사용자가 직접1차구현을 승인하고, 기본공격/Q 모두 범위내 모든유효적타격을 확정했다. 첫 조사에서 권장했던 기본공격1명/Q전체 방식을 채택한 것이 아니다.

- 기존BP이름유지. SpawnPointId는고유양수배치ID, MonsterDefinitionId는등록종류ID, Enabled는생성여부. 포인트당살아있는개체1개, 기술적상한16, 초기시험활성3개. 종류는Dusk1만등록; 미등록ID교체로새몬스터가생기지는않는다.
- v2다중목록Export, 중복ID/미등록종류/finite·bounds/개수/평지규칙/savedNav배치검증. 서버는옛v1단일파일도지원. 이것은오프라인데이터버전이며Protobuf변경아님.
- Room에서포인트별runtime으로대상·경로·공격ID/타이밍·HP/생명·사망·복귀·재생성UUID를분리. 공통Nav그래프/Room이벤트순서/ActionID를유지한다. 초기·늦은입장snapshot은모든현재개체를담고재생성에는새ObjectId를쓴다.
- 플레이어공격승인시존재한ObjectId를캡처하고판정시현재범위/상태를검사. 같은공격의대상별1회, Q는HitIndex별1회. 이전예약이새재생성개체를맞히지않는다. 기존230cm/기본25/Q25·38/370·1100ms/A–D입력버퍼·승인복구/점프제한은유지한다.
- Dusk스케일0.7,캡슐28×62,메시Yaw+90°,평지중심Z61.5와정적Nav우회유지. HP200/속도220/탐색800/home1200/상실1000/공격170·접촉190/피해10/간격2초/420ms/사망제거3초·재생성8초유지.
- 각개체죽음에서자신의생명/사망UUID로기존LootService를호출. 배치ID와DB UUID분리. 이번확장DB/패킷수정없음.
- MainMap의사용자포인트1(904.402783,15.172481)보존, 포인트2(904.402783,215.172481),3(1104.402783,15.172481)추가. 모두feetZ=-0.5,yaw-90,scale1,Dusk1/Enabled. MainMap만의도저장; 기존BP/원본vendor/지형·벽/Nav/OBB수정없음.

Unreal은이미ObjectId별WorldEntity/PendingSpawn과Source+Action+Target+HitIndex중복키를가져재사용했다. Account/Network/LevelFlow/WorldEntity경계와외부서버권위를유지한다. UE5_RPG수정/이관없음. 군집회피·플레이어몸막기·동적장애물·순찰·경사·신규몬스터·다중종류랜덤스폰·UI디자인전환은제외했다. 다른몬스터추가시서버정의+클라이언트표현+애셋/캡슐/Nav대응을별도등록해야한다.

## 4. A→B 재탐색 후속 수정 및 사용자 인수

사용자가A를쫓다가A가벗어나면가까운B를공격하지않는현상을보고했다. 소스상ATTACK종료시A무효화분기가재탐색없이RETURN으로갔고,RETURN에서는home도착까지새플레이어탐색을안했다. 사용자승인수정: **A상실시유효B먼저재탐색, 후보없을때만복귀**. 일반추적과공격종료가공통 AcquireMonsterTarget을쓴다.

유효A는B가더가까워도유지. 공격종료후전환하며A의예약피해를B로넘기지않음. 기존2초간격유지. RETURN확정뒤재탐색금지로왕복흔들림방지. B가몬스터가까이에있어도home1200밖이면여전히제외한다. 플레이어를동적장애물/가로막기대신피해대상으로처리하는기능은없다.

Debug/Release수정본빌드/적용후사용자가“검증완료”를보고했다. **직전A→B재탐색수정의사용자검증완료**로기록한다. 이번보고에서다중드랍/latejoin/재접속등전체인수항목이각각열거되지는않았으므로모든항목을일괄완료로확대하지않는다. 이인계턴에서새PIE/실시간로그감사를하지않았다.

## 5. 실행 증거와 남은 범위

- MultiDusk_20261006: 서버Debug/Release,S1Editor,게임Development및WindowsBuild/Cook/Stage/Package/Archive성공. 실제Room오프라인26개,parser실제배포본포함29개,Unreal합성14개성공/0warnings/0errors. 마지막14개에S1.World.MultiMonsterPresentation포함;첫캐시누락/테스트컴파일오류는수정후재검증했다.
- 패키지Archive: E:\Git Fork\S1_UE5.8\Saved\MultiDuskPackage_20261006\Windows. MainMapNullRHI실제로드·마커class조회0Objects·정상Quit/exit0. 서버종료상태의예상연결Error1있음. 실제인증2인패키지검증이아님.
- DuskRetarget_20261006: 서버Debug/Release최종exit0;실제Room코드오프라인37검사/0fail(기존26+재탐색11). 로그RoomRetargetTests.log. fake/disconnected세션·실제savedNav사용,DB/IOCP소켓검증아님. 글로벌풀종료경로는runner에서제외하므로서버종료안정성증거아님. 기존LNK4075경고는남음.
- 증거/checkpoint루트: C:\Users\user\Documents\Unreal IOCP Server\checkpoints\PotionUse_20261006、DuskSpawnPoint_20261006、MultiDusk_20261006、DuskRetarget_20261006. 계정비밀정보/DB백업내용공유안함.
- 남은개별확인: 실제다중드랍DB/G획득·각자우회복귀·latejoin/접속종료/재접속·전체HP/포션/Trail회귀의명시적체크리스트,두패키지계정플레이,군집겹침시각. Nav경로거리검사는직접LOS가아니므로모든벽관통해결보장아님. 포션실제장애주입과공개FX권한은계속별도.

## 6. 최신 문서 / 기획 요청

기준: MultiDuskSpawnImplementation.md(재탐색인수추가), DuskSpawnPointAuthoring.md, KnightCombatDesign.md, TerraQTwoHitImplementation.md, CurrentAssetRegistry.md, PotionUseImplementation.md. 이전단일Dusk·한포인트·Q단일대상·몸통높이재패키징대기기록보다최신후속기록이우선한다.

다음명세는이번새기준을반영해작게작성해달라. 추천은남은다중플레이인수체크를한번정리한뒤, 기존논의한1~4퀵슬롯또는인벤토리UMG디자인분리중한개를선택하는것이다. UI디자인분리는소스조사상가능하지만아직구현승인/착수하지않았다. 퀵슬롯은키충돌·슬롯참조/저장범위·사용중상태규칙을확정하고기존RequestUseItem을재사용해야한다. 어그로위협량·피해로타겟전환·몸막기/군집회피를새규칙으로추가하려면별도기획결정이필요하다. 이번전달만으로후속구현을시작하지않는다.
