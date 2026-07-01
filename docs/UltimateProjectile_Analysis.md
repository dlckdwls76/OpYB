# `AOpYBUltimateProjectile` 클래스 분석 보고서 (최신 업데이트 버전)

이 문서는 현재 프로젝트에 구현된 궁극기 투사체인 `AOpYBUltimateProjectile`의 헤더 파일(`.h`)과 소스 파일(`.cpp`)의 최신 구조 및 버그 수정이 적용된 핵심 로직을 상세히 분석한 문서입니다.

## 1. 개요 (Overview)
`AOpYBUltimateProjectile`은 언리얼 엔진의 `AActor`를 상속받아 구현된 멀티플레이어 환경(Replication) 대응 궁극기 투사체입니다. 곡사(박격포) 형태로 날아가며, 지형이나 적에 닿으면 폭발하여 광역 데미지 및 넉백을 가하는 역할을 합니다.

---

## 2. 헤더 파일 구조 (`OpYBUltimateProjectile.h`)

클래스는 크게 **컴포넌트(Components)**, **전투 및 시각 효과 속성(Properties)**, **네트워크 함수(RPCs)** 세 가지로 나뉩니다.

### 2.1 주요 컴포넌트
* **`CollisionComp` (`USphereComponent`)**: 투사체의 물리적 충돌을 담당하는 구체 컴포넌트이며 렌더링 최상위 루트(RootComponent) 역할을 합니다.
* **`ProjectileMovement` (`UProjectileMovementComponent`)**: 투사체가 날아가는 속도, 궤적(중력 적용), 회전 등을 전담하는 엔진 내장 이동 컴포넌트입니다.
* **`MeshComp` (`UStaticMeshComponent`)**: 플레이어에게 보여지는 3D 외형입니다. (물리 충돌 없음)
* **`DangerZoneDecal` (`UDecalComponent`)**: 목표 지점에 투영되어 플레이어들에게 폭발 범위(경고 원)를 시각적으로 보여주는 데칼입니다.

### 2.2 전투/효과 속성 (Properties)
* **`TargetLocation`**: 캐릭터가 마우스로 조준한 **최종 목표 지점**입니다. 스폰될 때 전달받으며 네트워크를 통해 클라이언트에게 동기화(Replicated)됩니다.
* 기타 데미지(`DamageAmount`), 반경(`ExplosionRadius`), 넉백(`KnockbackStrength`) 등의 변수를 블루프린트에서 편집 가능하게 제공합니다.

### 2.3 핵심 함수 및 네트워크(RPC)
* **`OnHit` / `OnOverlapBegin`**: 지형(바닥/구조물)이나 적 캐릭터와 닿았을 때 호출되는 콜백 함수입니다.
* **`MulticastExplode` / `MulticastDestroy`**: 서버에서 판정한 폭발 및 소멸 이벤트를 모든 클라이언트 화면에서 동시에 실행되도록 만드는 `NetMulticast` 함수입니다.

---

## 3. 소스 파일 핵심 로직 (`OpYBUltimateProjectile.cpp`)

### 3.1 생성자 (Constructor) 및 지능형 커스텀 충돌 설정
투사체의 물리적 오류(땅으로 꺼지거나, 허공에 걸리는 현상)를 방지하기 위해 생성자에서 매우 정교한 커스텀 충돌(Custom Collision)이 설정되어 있습니다.

```cpp
// 1. 물리 충돌체 크기를 최소화 (스폰 즉시 바닥에 끼는 현상 방지)
CollisionComp->InitSphereRadius(30.0f);

// 2. 커스텀 충돌 채널 셋팅
CollisionComp->SetCollisionProfileName("Custom");

// 기본적으로 세상의 모든 것을 무시(통과)하게 설정
CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);

// 오직 두 가지만 예외 처리:
// 1. 바닥/벽 (WorldStatic, WorldDynamic): 멈춰서 터져야 하므로 Block 설정
CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

// 2. 캐릭터/적 (Pawn): 멈춰서 걸리는 현상을 방지하기 위해 겹침(Overlap)으로 통과시키며 터지게 함
CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
```
* **결과**: 플레이어, 카메라, 보이지 않는 투명 벽 등 불필요한 물체들은 모두 무시하고, 오로지 바닥(구조물)과 적에게만 정확하게 반응합니다.

### 3.2 게임 시작 (BeginPlay)과 블루프린트 오버라이드 방지
```cpp
// 블루프린트가 이벤트를 덮어씌워 끊어버리는 고질적인 버그 방지
if (!CollisionComp->OnComponentHit.IsAlreadyBound(this, &AOpYBUltimateProjectile::OnHit))
{
	CollisionComp->OnComponentHit.AddDynamic(this, &AOpYBUltimateProjectile::OnHit);
}
// ... OnOverlapBegin도 동일하게 런타임 강제 연결
```
기존에는 생성자에서 충돌 이벤트를 연결했으나, 언리얼 엔진 블루프린트가 이를 무시하는 버그로 인해 "구조물에 부딪혀도 터지지 않는 현상"이 발생했습니다. 이를 해결하기 위해 **게임이 시작되는 `BeginPlay` 시점에 강제로 이벤트를 메모리에 연결**하도록 수정되었습니다.

### 3.3 충돌 및 폭발 판정 (OnHit / OnOverlapBegin)
지형(Block 대상)에 부딪혀 막히면 `OnHit`이, 적 캐릭터(Overlap 대상)를 관통하면 `OnOverlapBegin`이 실행됩니다.
안전장치로써 자기 자신이나 발사자(Instigator)에게는 반응하지 않으며, 유효한 타겟이나 맵의 구조물에 부딪혔을 때 즉시 폭발을 서버에 요청합니다.

### 3.4 광역 피해 및 효과 적용 (MulticastExplode)
이 함수는 서버의 요청으로 모든 플레이어의 컴퓨터에서 동시에 실행됩니다.
1. **광역 데미지**: `ApplyRadialDamageWithFalloff`로 범위 내 데미지를 입힙니다.
2. **광역 넉백**: `SweepMultiByChannel`로 반경 내 캐릭터를 찾아 위쪽 궤적으로 띄워 날려버립니다.
3. **타이머 자동 소멸**: 폭발 파티클 재생 시, 무한 루프 이펙트를 사용할 경우를 대비하여 `SetTimer`와 약한 람다(`CreateWeakLambda`)를 사용해 **정확히 5초 뒤에 메모리에서 강제 소멸**시키는 안전망이 들어있습니다.

---

## 4. 요약
현재 `OpYBUltimateProjectile` 클래스는 다음 3가지 핵심 문제를 C++ 레벨에서 완벽하게 해결한 상태입니다.
1. **땅으로 꺼지는 현상**: 바닥을 단단하게 물리적으로 막는 `Block` 속성을 적용해 표면에서 멈추도록 개선.
2. **플레이어 몸이나 허공에 걸려 멈추는 현상**: 기본 충돌을 무시(`Ignore`)로 두고, 캐릭터(`Pawn`)는 `Overlap`으로 통과시키도록 맞춤 설정.
3. **다른 구조물에 닿았을 때 안 터지는 현상**: 블루프린트 오버라이드 버그를 피하기 위해 `BeginPlay` 런타임에 이벤트를 강제로 바인딩(Binding)하여 해결.

이제 언리얼 엔진의 멀티플레이어 환경에 맞춰 이동과 폭발 판정은 서버가 확실하게 통제하며, 시각적 효과는 클라이언트들에게 지연 없이 쾌적하게 동기화됩니다.
