#include "TextureReader.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
// #include "Runtime/ShaderCore/Public/StaticBoundShaderState.h"
// #include "Runtime/ShaderCore/Public/GlobalShader.h"
#include "Runtime/Engine/Public/ScreenRendering.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"

#include "UnrealcvStats.h"
#include "UnrealcvLog.h"

DECLARE_CYCLE_STAT(TEXT("ResizeReadBufferFast"), STAT_ResizeReadBufferFast, STATGROUP_UnrealCV);

/**
ReadData from texture2D
This is the standard way in Unreal, but very slow
*/
bool ReadTextureRenderTarget(UTextureRenderTarget2D* RenderTarget, TArray<FColor>& ImageData, int& Width, int& Height)
{
	if (RenderTarget == nullptr)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The RenderTarget is nullptr"));
		return false;
	}
	// Get the RHI

	Width = RenderTarget->SizeX;
	Height = RenderTarget->SizeY;

	// Initialize the image data array
	ImageData.Empty();
	ImageData.AddZeroed(Width * Height);
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();

	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);
	{
		RenderTargetResource->ReadPixels(ImageData, ReadSurfaceDataFlags);
	}

	// Serialize and save png
	return true;
}

bool FastReadTexture2DAsync(FTexture2DRHIRef Texture2D, TFunction<void(FColor*, int32, int32)> Callback)
{
	auto RenderCommand = [=](FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SrcTexture)
	{
		if (SrcTexture == nullptr)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Input texture2D is nullptr"));
			return;
		}
		FRHIResourceCreateInfo CreateInfo;
		FTexture2DRHIRef ReadbackTexture = RHICreateTexture2D(
			SrcTexture->GetSizeX(), SrcTexture->GetSizeY(),
			EPixelFormat::PF_B8G8R8A8,
			// SrcTexture->GetFormat(),
			// EPixelFormat::PF_A32B32G32R32F,
			1, 1,
			TexCreate_CPUReadback,
			CreateInfo
		);

		if (ReadbackTexture->GetFormat() != SrcTexture->GetFormat())
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("ReadbackTexture and SrcTexture are different"));
			return;
		}
		void* ColorDataBuffer = nullptr;
		int32 Width = 0, Height = 0;

		// Debug, check the data before and after the copy operation
		// RHICmdList.MapStagingSurface(ReadbackTexture, ColorDataBuffer, Width, Height);
		// RHICmdList.UnmapStagingSurface(ReadbackTexture);

		FResolveParams ResolveParams;
		RHICmdList.CopyToResolveTarget(
			SrcTexture,
			ReadbackTexture,
			FResolveParams());

		RHICmdList.MapStagingSurface(ReadbackTexture, ColorDataBuffer, Width, Height);

		FColor* ColorBuffer = reinterpret_cast<FColor*>(ColorDataBuffer);
		Callback(ColorBuffer, Width, Height);
		RHICmdList.UnmapStagingSurface(ReadbackTexture);
	};

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FastReadBuffer,
		TFunction<void(FRHICommandListImmediate&, FTexture2DRHIRef)>, InRenderCommand, RenderCommand,
		FTexture2DRHIRef, InTexture2D, Texture2D,
		{
			InRenderCommand(RHICmdList, InTexture2D);
		});
	return true;
}

// bool ResizeFastReadTexture2DAsync(FTexture2DRHIRef Texture2D, int TargetWidth, int TargetHeight, TFunction<void(FColor*, int32, int32)> Callback)
// {
// 	auto RenderCommand = [=](FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SrcTexture)
// 	{
// 		// RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);

// 		SCOPE_CYCLE_COUNTER(STAT_ResizeReadBufferFast);

// 		FIntPoint TargetSize(TargetWidth, TargetHeight);
// 		FPooledRenderTargetDesc OutputDesc(FPooledRenderTargetDesc::Create2DDesc(
// 			TargetSize,
// 			SrcTexture->GetFormat(),
// 			FClearValueBinding::None,
// 			TexCreate_None,
// 			TexCreate_RenderTargetable, // This texture can be used as a RenderTarget
// 			false));

// 		const auto FeatureLevel = GMaxRHIFeatureLevel;
// 		TRefCountPtr<IPooledRenderTarget> ResampleTexturePooledRenderTarget;

// 		static const FName RendererModuleName("Renderer");
// 		IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>(RendererModuleName);

// 		// IRendererModule RendererModule;
// 		// RendererModule->RenderTargetPoolFindFreeElement(RHICmdList, OutputDesc, ResampleTexturePooledRenderTarget, TEXT("ResampleTexture")); // FIXME:
// 		if (!ResampleTexturePooledRenderTarget)
// 		{
// 			UE_LOG(LogUnrealCV, Warning, TEXT("ResampleTexturePooledRenderTarget is invalid"));
// 			return;
// 		}

// 		const FSceneRenderTargetItem& DestRenderTarget = ResampleTexturePooledRenderTarget->GetRenderTargetItem();

// 		// SetRenderTarget(RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());
// 		RHICmdList.SetViewport(0, 0, 0.0f, TargetWidth, TargetHeight, 1.0f);

// 		// TODO: check these APIs
// 		// RHICmdList.SetBlendState(TStaticBlendState<>::GetRHI());
// 		// RHICmdList.SetRasterizerState(TStaticRasterizerState<>::GetRHI());
// 		// RHICmdList.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());

// 		auto ShaderMap = GetGlobalShaderMap(FeatureLevel);
// 		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
// 		TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

// 		static FGlobalBoundShaderState BoundShaderState;
// 		// SetGlobalBoundShaderState(RHICmdList, FeatureLevel, BoundShaderState, RendererModule->GetFilterVertexDeclaration().VertexDeclarationRHI, *VertexShader, *PixelShader);

// 		if (TargetSize != FIntPoint(SrcTexture->GetSizeX(), SrcTexture->GetSizeY()))
// 		{
// 			// We're scaling down the window, so use bilinear filtering
// 			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);
// 		}
// 		else
// 		{
// 			// Drawing 1:1, so no filtering needed
// 			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Point>::GetRHI(), SrcTexture);
// 		}
// 		{
// 			RendererModule->DrawRectangle(
// 				RHICmdList,
// 				0, 0,		// Dest X, Y
// 				TargetWidth, TargetHeight,	// Dest Width, Height
// 				0, 0,		// Source U, V
// 				1, 1,		// Source USize, VSize
// 				TargetSize,		// Target buffer size
// 				FIntPoint(1, 1),		// Source texture size
// 				*VertexShader,
// 				EDRF_Default);
// 		}

// 		FRHIResourceCreateInfo CreateInfo;
// 		FTexture2DRHIRef ReadbackTexture = RHICreateTexture2D(
// 			TargetWidth,
// 			TargetHeight,
// 			SrcTexture->GetFormat(),
// 			1, 1,
// 			// TexCreate_CPUReadback | TexCreate_ResolveTargetable,
// 			TexCreate_CPUReadback,
// 			CreateInfo
// 		);

// 		if (ReadbackTexture->GetFormat() != DestRenderTarget.TargetableTexture->GetFormat())
// 		{
// 			UE_LOG(LogUnrealCV, Warning, TEXT("ReadbackTexture and DestRenderTarget have different formats"));
// 			return;
// 		}
// 		// Need to be the same for the copy operation, otherwise it will fail silently

// 		FResolveParams ResolveParams;
// 		RHICmdList.CopyToResolveTarget(
// 			DestRenderTarget.TargetableTexture,
// 			ReadbackTexture,
// 			FResolveParams());

// 		void* ColorDataBuffer = nullptr;
// 		int32 Width = 0, Height = 0;
// 		RHICmdList.MapStagingSurface(ReadbackTexture, ColorDataBuffer, Width, Height);

// 		FColor* ColorBuffer = reinterpret_cast<FColor*>(ColorDataBuffer);
// 		Callback(ColorBuffer, Width, Height);
// 		// The ColorDataBuffer maps to the memory location

// 		RHICmdList.UnmapStagingSurface(ReadbackTexture);
// 	};

// 	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
// 		FastReadBufferResize,
// 		TFunction<void(FRHICommandListImmediate&, FTexture2DRHIRef)>, InRenderCommand, RenderCommand,
// 		FTexture2DRHIRef, InTexture2D, Texture2D,
// 	{
// 		InRenderCommand(RHICmdList, InTexture2D);
// 	});
// 	return true;
// }
