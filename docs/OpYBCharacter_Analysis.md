# OpYBCharacter 클래스 분석 보고서

이 문서는 `OpYBCharacter.h` 및 `OpYBCharacter.cpp` 파일의 구조와 핵심 기능, 그리고 멀티플레이어 환경(Replication)에서의 동작 방식을 분석한 문서입니다.

## 1. 클래스 개요 (Overview)
`AOpYBCharacter`는 언리얼 엔진의 `ACharacter`를 상속받은 탑다운(Top-Down) 시점의 플레이어블 캐릭터 클래스입니다. 
탑다운 카메라 시점 처리, 체력/탄약 스탯 관리, 구르기 애니메이션, 그리고 네트워킹(서버-클라이언트 동기화)을 고려한 사격 및 장전 시스템을 담당하고 있습니다.

---

## 2. 주요 컴포넌트 구성 (Components)
생성자(`AOpYBCharacter()`)에서 초기화되는 핵심 컴포넌트들은 다음과 같습니다.

* **`USpringArmComponent (CameraBoom)`**: 캐릭터 위에 고정된 카메라 지지대입니다. 절대 회전값(`bUseAbsoluteRotation`)을 사용하여 캐릭터가 회전해도 카메라는 돌지 않고 고정된 탑다운 시점을 유지합니다.
* **`UCameraComponent (TopDownCamera)`**: 실제 화면을 렌더링하는 카메라입니다.
* **`UWidgetComponent (OverheadUIComponent)`**: 캐릭터 머리 위(`Z + 120.0f`)에 체력바나 탄약 등을 띄우는 Screen Space(화면 고정) UI 컴포넌트입니다.

> [!TIP]
> **이동 및 회전 설정**: 마우스 커서를 바라보고 움직여야 하므로 `bOrientRotationToMovement`를 `false`로 꺼서 캐릭터가 이동 방향으로 강제 회전하는 것을 막아두었습니다.

---

## 3. 핵심 기능 분석 (Core Features)

### A. 구르기 및 애니메이션 (Rolling & Animation)
* **`PlayRollMontage()`**: 구르기 몽타주(`RollMontage`)를 재생합니다. `IsRolling()`이 `false`일 때만 실행되어 중복 구르기를 방지합니다.
* **`IsRolling()`**: 애니메이션 인스턴스(`AnimInstance`)에서 현재 구르기 몽타주가 재생 중인지(`Montage_IsPlaying`) 실시간으로 확인합니다. 별도의 타이머 없이 깔끔하게 상태를 추적합니다.

### B. 전투 시스템: 사격 (Combat: Shooting)
클라이언트의 클릭 입력부터 서버의 총알 생성까지 철저하게 분리된 멀티플레이어 구조를 따릅니다.

1. **`AttemptShoot()` (클라이언트/로컬)**
   - 플레이어가 클릭할 때 가장 먼저 호출됩니다.
   - **방어 로직**: 탄약(`CurrentAmmo`)이 없거나, 연사 쿨타임(`FireRate`, 0.1초)이 지나지 않았으면 발사하지 않고 차단합니다.
   - 통과 시 총구 위치(Z축 20 상승)를 계산하여 서버로 `ServerShoot` RPC를 보냅니다.
2. **`ServerShoot_Implementation()` (서버)**
   - 클라이언트의 요청을 받아 서버에서 실제로 `AOpYBProjectile`(총알)을 스폰합니다.
   - **지연 생성(Deferred Spawning)**: `SpawnActorDeferred`를 사용해 총알의 소유자(Owner) 등을 먼저 주입한 뒤 `FinishSpawning`으로 생성하여 안정적인 물리 충돌 처리를 유도합니다.
   - 생성 성공 시 탄약을 1 감소시키고 서버 측 UI 업데이트(`OnRep_CurrentAmmo()`)를 호출합니다.
   - 탄약이 0이 되면 `AutoReloadTimerHandle`을 통해 2초(`AutoReloadDelay`) 뒤 자동으로 장전되도록 타이머를 돌립니다.

### C. 체력과 데미지 처리 (Health & Damage)
* **`TakeDamage()`**: 언리얼 엔진 기본 데미지 시스템을 오버라이드(재정의)했습니다.
* **서버 권한(`HasAuthority()`) 확인**: 데미지 계산과 체력 감소는 오직 서버에서만 이루어집니다. 깎인 체력값은 `CurrentHealth` 변수를 통해 클라이언트들에게 자동 동기화됩니다.

---

## 4. 멀티플레이어 데이터 동기화 (Replication)
체력과 탄약 등 게임에 치명적인 데이터는 조작(핵)을 막기 위해 서버에서 클라이언트로 단방향 동기화됩니다.

* **Replicated 프로퍼티**: `CurrentHealth`, `CurrentAmmo`
* **`GetLifetimeReplicatedProps`**: 이 함수를 통해 위 두 변수를 네트워크를 통해 동기화(`DOREPLIFETIME`)하도록 등록했습니다.
* **`ReplicatedUsing (OnRep_*)`**: 
  - `OnRep_CurrentHealth()`, `OnRep_CurrentAmmo()`
  - 서버에서 체력/탄약 값이 바뀌어 클라이언트로 전달될 때 자동으로 호출되는 콜백 함수들입니다.
  - 이 함수들 내부에서 `OverheadUIComponent`의 UI 위젯에 갱신된 수치를 밀어 넣어 줌으로써 화면의 UI가 최신 상태로 유지됩니다.

---

## 5. 총평 및 구조적 장점
* **보안성(Security)**: 사격과 장전, 체력 감소 등 중요한 판정이 모두 서버(Server RPC 및 Authority Check)에서 이루어져 멀티플레이어 환경에 매우 적합하게 설계되어 있습니다.
* **최적화(Optimization)**: UI 업데이트를 매 프레임(Tick)마다 하지 않고, 값이 바뀔 때만 호출되는 `OnRep_` 함수를 활용한 이벤트 기반 설계(Event-driven)가 잘 적용되어 성능에 유리합니다.
* **직관적인 상태 관리**: 애니메이션 상태 판단(`IsRolling`)을 엔진 자체의 몽타주 시스템에 맡김으로써 코드 복잡도를 낮추고 버그 발생 확률을 줄였습니다.
