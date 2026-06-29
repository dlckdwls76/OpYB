# 멀티플레이어 및 UI 시스템 구현 요약 (Multiplayer & UI Implementation Summary)

이 문서는 최근 추가된 멀티플레이어 환경(Listen Server/Client) 지원용 UI 및 사망/리스폰 시스템에 대한 구조와 적용 방식을 요약합니다.

## 1. 머리 위 체력바 (Overhead Healthbar)
- **C++ 클래스**: `UOpYBOverheadWidget`
- **블루프린트 위젯**: `WBP_Healthbar`
- **핵심 기능**:
  - `AOpYBCharacter`의 머리 위에 `WidgetComponent`로 부착되어 3D 월드 상에서 캐릭터를 따라다닙니다.
  - 체력이 변경될 때마다 서버에서 `OnRep_CurrentHealth()`를 호출하여 체력바의 퍼센테이지(0~1)를 동기화합니다.
  - 클라이언트 환경에서도 컴포넌트가 정상 복제되도록 설정되어 있습니다.

## 2. 사망 및 자동 부활 시스템 (Death & Respawn System)
- **사망 처리 (`Die`)**:
  - 체력이 0이 되면 서버에서 `Die()` 함수를 실행합니다.
  - `bIsDead` 변수를 `true`로 설정하여 모든 클라이언트에게 복제합니다 (`OnRep_IsDead`).
  - 캐릭터 메시를 숨기고, 충돌(Collision)과 입력(Input)을 비활성화합니다.
- **자동 부활 (`Respawn`)**:
  - 사망 시 서버 타이머(`FTimerHandle`)를 작동시켜 10초 뒤 `Respawn()`을 호출합니다.
  - 무작위 `PlayerStart` 위치로 텔레포트한 뒤, 체력과 총알을 가득 채우고 캐릭터를 다시 활성화합니다.

## 3. 부활 대기 데스 스크린 (Death Screen UI)
- **C++ 클래스**: `UOpYBDeathScreenWidget`
- **블루프린트 위젯**: `WBP_DeathScreen`
- **핵심 기능**:
  - 플레이어가 사망하면 화면 정중앙에 "남은 시간: N" 텍스트를 띄웁니다.
  - `NativeTick`을 통해 C++ 내부에서 타이머를 직접 깎아 실시간으로 UI를 업데이트합니다.
  - 부활(Respawn)과 동시에 자동으로 화면에서 삭제됩니다.

## 4. 다이내믹 에임 커서 (Dynamic Aim Cursor)
- **C++ 클래스**: `UOpYBAimCursorWidget`
- **블루프린트 위젯**: `WBP_AimCursor`
- **핵심 기능**:
  - 기존 윈도우 마우스 포인터(`EMouseCursor::None`)를 완벽히 숨기고 마우스 위치에 텍스처를 띄웁니다.
  - 사격(클릭) 시 `PlayShootAnimation()`이 호출되어 크기가 1.5배로 커졌다가 부드럽게(Lerp) 원상 복구되는 반동 애니메이션을 제공합니다.
  - 마우스 클릭 시 포커스 캡처로 인해 커서가 화면 밖(0,0)으로 튕기는 언리얼의 고질적인 버그를 C++ 코드 레벨에서 방어합니다. (`GetMousePosition` 예외 처리)

## 5. 실시간 생존자 수 표시 (Player Count UI)
- **C++ 클래스**: `UOpYBPlayerCountWidget`, `AOpYBGameState`
- **블루프린트 위젯**: `WBP_PlayerCount`
- **핵심 기능**:
  - 멀티플레이어 환경의 통합 상태 관리를 위해 커스텀 `GameState`를 도입했습니다.
  - 캐릭터가 스폰/부활할 때 `AlivePlayerCount`를 1 증가시키고, 사망할 때 1 차감합니다.
  - UI는 `NativeTick`에서 `GameState`의 생존자 수를 실시간으로 읽어와 화면 좌측 상단에 "남은 인원 : N"으로 표시합니다.

---

**[주의 사항]**
위 모든 UI는 블루프린트 노드를 최소화하고 성능을 높이기 위해 C++ 위주로 설계되었습니다. 따라서 위젯 블루프린트를 수정할 때는 항상 **클래스 세팅(Class Settings)에서 부모 클래스가 올바르게 지정되었는지**, 그리고 텍스트나 이미지의 **변수 이름(BindWidget)이 C++ 코드와 정확히 일치하는지** 주의해야 합니다.
