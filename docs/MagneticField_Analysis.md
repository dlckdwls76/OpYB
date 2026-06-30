# 자기장(Magnetic Field) 구현 분석 보고서

본 문서는 `OpYB` 프로젝트의 핵심 배틀로얄 시스템인 **자기장(`OpYBMagneticField`)** 클래스의 구조와 동작 원리를 분석한 자료입니다.

## 1. 클래스 개요
* **클래스명**: `AOpYBMagneticField`
* **부모 클래스**: `AActor`
* **역할**: 배틀로얄 게임의 안전 구역(Safe Zone)을 담당합니다. 시간이 지남에 따라 구역이 점진적으로 축소되며, 안전 구역 바깥에 위치한 플레이어들에게 지속적인 틱 데미지(Tick Damage)를 입힙니다.

---

## 2. 헤더 파일 분석 (`OpYBMagneticField.h`)

헤더 파일에서는 자기장의 상태 수치와 네트워크 동기화를 위한 변수들이 정의되어 있습니다.

```cpp
public:
    // 자기장의 시각적 형태를 나타낼 메쉬 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MeshComp;

    // 현재 반경 (서버->클라이언트로 복제됨)
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
    float CurrentRadius = 5000.f;

    // 데미지 관련 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
    float DamagePerTick = 5.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
    float DamageInterval = 1.0f;
```
* **네트워크 동기화 (`Replicated`)**: 배틀로얄은 멀티플레이가 필수이므로, 안전 구역의 반경(`CurrentRadius`)을 서버에서 계산한 뒤 모든 클라이언트에게 실시간으로 복제(Replicate)하도록 설계되었습니다.

---

## 3. 구현 파일 분석 (`OpYBMagneticField.cpp`)

### ① 비충돌 시각화 메쉬 처리
```cpp
MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
```
* 자기장 경계 자체는 시각적인 효과일 뿐, 플레이어의 이동이나 총알을 물리적으로 막으면 안 되기 때문에 콜리전을 `NoCollision`으로 설정했습니다.

### ② 자기장 축소 로직 (`Tick` 함수)
```cpp
if (HasAuthority())
{
    if (CurrentRadius > MinRadius)
        CurrentRadius -= ShrinkRate * DeltaTime;
}

// 시각적 스케일 조절 (서버 및 클라이언트 공통)
float ScaleFactor = CurrentRadius / 50.0f;
SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
```
* **서버 권한(`HasAuthority`) 제어**: 자기장이 줄어드는 연산은 오직 **서버**에서만 수행하여 핵/어뷰징을 방지하고 상태를 일치시킵니다.
* **시각화 보간**: 서버가 계산한 `CurrentRadius` 값을 이용해, 매 프레임 자기장의 3D 메쉬 크기(`Scale`)를 줄여 부드럽게 축소되는 애니메이션을 연출합니다.

### ③ 외곽 플레이어 데미지 판정
```cpp
// 1초(DamageInterval)마다 ApplyDamageToPlayersOutside 함수를 반복 실행
GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AOpYBMagneticField::ApplyDamageToPlayersOutside, DamageInterval, true);
```
타이머를 이용해 데미지 함수를 주기적으로 호출합니다. 데미지 함수 내부의 원리는 다음과 같습니다.

```cpp
// 1. 모든 캐릭터 액터를 찾아온다.
UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOpYBCharacter::StaticClass(), AllCharacters);

// 2. 각 캐릭터와 자기장 중심 사이의 거리를 계산한다.
float Distance = FVector::Dist2D(CenterLocation, Char->GetActorLocation());

// 3. 거리가 현재 반경(CurrentRadius)보다 크면 안전 구역 밖이므로 데미지를 입힌다.
if (Distance > CurrentRadius)
{
    UGameplayStatics::ApplyDamage(Char, DamagePerTick, nullptr, this, UDamageType::StaticClass());
}
```
* **`FVector::Dist2D`**: 높이(Z축)를 무시하고 2차원(XY) 평면상의 거리만 측정하여, 플레이어가 공중에 있건 땅에 있건 원 밖으로 나가면 공평하게 데미지를 받도록 수학적으로 계산합니다.

---

## 4. 요약 및 활용 방안
* `OpYBMagneticField`는 **멀티플레이어 환경(Replication)**과 **서버 검증(HasAuthority)**을 훌륭하게 준수하고 있는 핵심 기믹 클래스입니다.
* 무거운 물리 충돌 연산 대신, 수학적 거리 연산(`Dist2D`)과 타이머(`TimerHandle`)를 활용하여 매우 효율적이고 가볍게 배틀로얄의 필수 조건인 'Time Pressure(시간 압박)'를 구현해 내었습니다.
