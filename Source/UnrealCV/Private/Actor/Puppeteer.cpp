// Weichao Qiu @ 2018
#include "Puppeteer.h"
#include "Runtime/Engine/Classes/Components/MaterialBillboardComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

// Sets default values
APuppeteer::APuppeteer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UMaterialBillboardComponent* Billboard = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("BillboardComponent"));
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Engine/EditorMaterials/HelpActorMaterial"));
	Billboard->AddElement(MaterialAsset.Object, nullptr, false, 32.0f, 32.0f, nullptr);
	// Billboard->SetupAttachment(RootComponent);
	Billboard->bIsEditorOnly = true;
	Billboard->bHiddenInGame = true;
	
	RootComponent = Billboard;
}

// Called when the game starts or when spawned
void APuppeteer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APuppeteer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

