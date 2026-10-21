# MainMap_Test — 에디터 Nav 구성

## 범위

사용자 승인: Nav 구성 먼저 진행. /Game/Maps/MainMap_Test만 변경·선택 저장했다.
기존 MainMap/원본 L_ElderboomVillage/프로젝트 Config/서버 실행 데이터/AnimBP는 미변경.
서버 Nav 배포, 평지 높이 계약 변경, 경사/계단 대응은 이번 구현이 아니다.

## 체크포인트 / dirty 보호

`C:/Users/user/Documents/Unreal IOCP Server/checkpoints/MainMapTest_Nav_20261021`

- MainMap_Test.before.umap: DC9825FA5702D9C4CC43CE746547CBBD7DB4FB0BD91E3754D734F5A37417FA9E
- MainMap_Test.after.umap: C2C660E3AF4562AB63B7E24E3703A71D23599980C61B8810BB24FEF71652C78D

Before/After 모두 복사 해시 일치 확인. 다른 에셋을 포함하지 않는 부분 백업이다.
시작 시 MainMap_Test clean, ABP_S1Terra dirty. 변경 후 두 dirty 패키지를 보고하고
MainMap_Test만 저장했다. Build Paths 후에도 같은 정책으로 맵만 다시 저장했다.
최종 MainMap_Test/MainMap/원본마을 clean, ABP_S1Terra의 기존 dirty 상태 유지.
AnimBP0나눗셈 경고는 수정하지 않았다.

## 적용 내용

- WorldSettings.bEnableNavigationSystem=true.
- NavMeshBounds_S1Village (NavMeshBoundsVolume_0) 추가.
- 중심(2569,2569,1500), 회전0, scale(80,80,20).
  실제 bounds min(-5431,-5431,-500), max(10569,10569,3500), 마을 Landscape 범위 포함.
- 생성된 RecastNavMesh-Default: radius35cm, height144cm, slope44°,
  tile1000cm, RuntimeGeneration Static, bCanBeMainNavData=true.
  기존 서버 MainMap.nav의35/144/44 기준과 일치한다.
- 기존 NPC/스폰/PlayerStart/지형을 이동하거나 높이를 변경하지 않았다.

## 실제 검증 / 실패 이력

1. MCP로 생성/설정/실제 bounds readback, 맵 선택 저장.
2. S1ExportNavigation commandlet로 저장된 맵을 검사했으나 첫 시도는
   `RecastNavMesh contains no polygons`, Main return6/process exit1이었다.
   Bounds 배치만으로 실제 Nav가 빌드된 것으로 보고하지 않았다.
3. MCP Slate 메뉴에서 Build → Build Paths만 실행(전체 레벨/조명 빌드 아님).
   S1.log의 UNavigationSystemV1::Build 실행/완료0.32초 확인.
   bounds 확장으로 직렬화 maxTiles 재생성 경고가 있었고 build는 완료됐다.
4. 맵만 재저장 후 commandlet 재실행 성공/process exit0.
   저장된 Nav polygons3297, edges7406을 읽었고 진단 export를 생성했다.
   따라서 별도 프로세스에서 저장된 Nav 데이터를 읽는 persistence 검사까지 수행했다.

근거:
- Saved/Logs/MainMapTest_NavCheck_20261021.log (첫 실패 보존)
- Saved/Logs/MainMapTest_NavCheck_AfterBuild_20261021.log (재검사 성공)
- Saved/Diagnostics/MainMapTest_Nav_20261021.s1nav (진단 전용)

기존 commandlet의 format header는 Map 인자와 관계없이 `S1NAV 2 MainMap`이다.
이번 파일 이름/위치는 Diagnostics이며 MainMap_Test 자료다. 이 헤더를 근거로
현재 서버 MainMap용이라고 간주하거나 Tools/서버Data에 배포하면 안 된다.
노드Z 범위110..1170cm로 현재 서버 flat groundZ=-0.5와 일치하지 않는다.
폴리곤 존재는 모든 길의 연결/바닥 높이/몬스터 이동이 유효하다는 뜻이 아니다.

## 보존 / 미검증 / 다음 단계

기존 MainMap SHA681571215312D1A2196A6227E8746F56C4C1642FCA4514C32FC116204CD66CA0,
원본 마을 SHA E8589D19A1AD5962763ED0099BD80BC42726D2F6FFBA381B433DB074BA13C684 불변.
C++/Blueprint/프로토콜/DB/Config 수정이나 빌드는 하지 않았다.
실제 몬스터 추적/벽 우회/재교전·복귀, 새PIE 및 패키징은 미검증.
다음은 평지 테스트 구역/좌표·높이 계약 확정 후 실제 MainMap 교체와
Nav/spawn/장애물/NPC 데이터의 동일 revision 배포다. 자동 진행하지 않는다.
지형·충돌 변경 뒤에는 Build Paths → MainMap_Test만 저장 → 재검사 순서를 따른다.
