// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "MyHUD.h"


bool AMyHUD::bDrawCursor = false;
bool AMyHUD::bDrawLabel = false;

AMyHUD::AMyHUD()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
}


void AMyHUD::DrawHUD()
{
	Super::DrawHUD();

	if (AMyHUD::bDrawLabel)
	{
		// Draw debug texts
		FFontRenderInfo FontRenderInfo = Canvas->CreateFontRenderInfo(false, true);
		UFont* Font = GEngine->GetSmallFont();
		ULevel* Level = GetLevel();
		for (auto Actor : Level->Actors)
		{
			if (Actor && Actor->IsA(AStaticMeshActor::StaticClass())) // Only StaticMeshActor is interesting
			{
				FVector ActorLocation = Actor->GetActorLocation();

				FVector ActorLocation2 = Canvas->Project(ActorLocation);
				Canvas->DrawText(Font, Actor->GetActorLabel(), ActorLocation2.X, ActorLocation2.Y, 1, 1, FontRenderInfo);
			}
		}
	}

	if (AMyHUD::bDrawCursor)
	{
		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		const FVector2D CrosshairDrawPosition((Center.X - CrosshairTex->GetSurfaceWidth() * 0.5f),
			(Center.Y - CrosshairTex->GetSurfaceHeight() * 0.5f));

		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;

		Canvas->DrawItem(TileItem);
	}
}

void AMyHUD::ToggleLabel()
{
	bDrawLabel = !bDrawLabel;
}

void AMyHUD::ToggleCursor()
{
	bDrawCursor = !bDrawCursor;
}
