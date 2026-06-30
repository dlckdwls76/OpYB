# 투명벽(Invisible Wall) 구현 분석 보고서

본 문서는 `OpYB` 프로젝트의 맵 경계나 특정 구역의 접근을 제어하기 위해 구현된 **투명벽(`OpYBInvisibleWall`)** 클래스의 구조와 동작 원리를 분석한 자료입니다.

## 1. 클래스 개요
* **클래스명**: `AOpYBInvisibleWall`
* **부모 클래스**: `AActor`
* **역할**: 게임 내 화면에는 보이지 않지만, 플레이어의 이동이나 투사체(총알) 등이 통과하지 못하도록 물리적으로 막는 역할을 수행합니다.

---

## 2. 헤더 파일 분석 (`OpYBInvisibleWall.h`)

헤더 파일에서는 투명벽의 형태와 충돌 처리를 담당할 핵심 컴포넌트를 선언하고 있습니다.

```cpp
public:	
    // Box collision component to block movement
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> CollisionBox;
```
* **`UBoxComponent`**: 투명벽의 형태를 잡아주는 언리얼 엔진의 기본 상자 모양(Box) 콜리전(충돌체)입니다. 복잡한 3D 메쉬(Static Mesh)를 사용하지 않고 순수 충돌 연산만 수행하므로 매우 가볍고 효율적입니다.

---

## 3. 구현 파일 분석 (`OpYBInvisibleWall.cpp`)

생성자(`AOpYBInvisibleWall()`) 내부에 투명벽을 완성하는 핵심 로직 4가지가 구현되어 있습니다.

### ① 성능 최적화 (Tick 비활성화)
```cpp
PrimaryActorTick.bCanEverTick = false;
```
* 투명벽은 한자리에 가만히 서서 충돌만 막으면 되므로, 매 프레임마다 상태를 업데이트할 필요가 없습니다. 따라서 Tick 함수를 꺼서 **게임 성능(CPU 연산)을 최적화**했습니다.

### ② 충돌 상자 크기 설정 및 루트 지정
```cpp
CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
RootComponent = CollisionBox;
CollisionBox->InitBoxExtent(FVector(100.f, 100.f, 500.f));	
```
* `UBoxComponent`를 생성하고 이 액터의 중심(Root)으로 지정합니다.
* `InitBoxExtent`를 통해 기본 크기를 설정합니다. 여기서 Z값(높이)을 `500.f`로 넉넉하게 주어 플레이어가 점프나 구르기로 벽을 넘어가지 못하도록 디자인되었습니다.

### ③ 충돌(Collision) 프로필 설정
```cpp
CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
```
* **`BlockAll` 프로필**: 플레이어 캐릭터, 몬스터, 날아가는 총알 등 물리적인 모든 것을 완벽하게 막아냅니다.
* **`QueryAndPhysics`**: 캐릭터가 걸어가다가 부딪히는 물리(Physics) 처리뿐만 아니라, 레이캐스트나 마우스 클릭 같은 탐색(Query) 처리도 모두 막아내도록 활성화했습니다.

### ④ 시각적 은폐 (투명화)
```cpp
SetActorHiddenInGame(true);
```
* 이 투명벽의 핵심입니다. 에디터(개발 화면)에서는 붉은색/파란색 선으로 상자의 크기와 위치가 보이지만, **실제 게임이 시작되면 화면(렌더링)에서 완전히 사라지게** 만듭니다. (충돌체 자체는 보이지 않더라도 물리 법칙은 계속 적용됩니다.)

---

## 4. 요약 및 활용 방안
* `OpYBInvisibleWall`은 무거운 그래픽 메쉬 없이 `UBoxComponent` 하나만으로 이루어져 있어 **메모리와 렌더링 비용이 극히 낮습니다.**
* 배틀로얄이나 탑다운 슈터의 맵 외곽선(레벨 바운더리)에 배치하여 플레이어의 이탈을 막거나, 특정 장애물을 감싸는 보이지 않는 바리케이드로 활용하기에 최적화된 설계입니다.
