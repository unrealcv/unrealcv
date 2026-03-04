// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "UnrealcvServer.h"
#include "UnixTcpServer.h"
#include "Runtime/Engine/Classes/Engine/GameEngine.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

#include "ConsoleHelper.h"
#include "Commands/ObjectHandler.h"
#include "Commands/PluginHandler.h"
#include "Commands/ActionHandler.h"
#include "Commands/AliasHandler.h"
#include "Commands/CameraHandler.h"
#include "WorldController.h"
#include "UnrealcvLog.h"
#include "UnrealcvStats.h"

DECLARE_CYCLE_STAT(TEXT("FUnrealcvServer::Tick"),           STAT_Tick,           STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("FUnrealcvServer::ProcessRequest"), STAT_ProcessRequest, STATGROUP_UnrealCV);

const FString FUnrealcvServer::MessageFormat = TEXT("(\\d{1,}):(.*)");

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void FUnrealcvServer::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Tick);
	InitWorldController();
	ProcessPendingRequest();
}

void FUnrealcvServer::InitWorldController()
{
	UWorld* GameWorld = GetGameWorld();
	if (!IsValid(GameWorld) || WorldController.IsValid())
	{
		return;
	}

	UE_LOG(LogUnrealCV, Display, TEXT("Spawning WorldController."));
	WorldController = Cast<AUnrealcvWorldController>(
		GameWorld->SpawnActor(AUnrealcvWorldController::StaticClass()));

	if (WorldController.IsValid())
	{
		WorldController->InitWorld();
	}
	else
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to spawn WorldController."));
	}
}

// ---------------------------------------------------------------------------
// World access
// ---------------------------------------------------------------------------

APawn* FUnrealcvServer::GetPawn()
{
	// Re-fetch each call; the cached weak pointer is used to detect staleness.
	if (CachedPawn.IsValid())
	{
		return CachedPawn.Get();
	}

	UWorld* World = GetWorld();
	if (!IsValid(World)) { return nullptr; }

	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC)) { return nullptr; }

	CachedPawn = PC->GetPawn();
	return CachedPawn.Get();
}

UWorld* FUnrealcvServer::GetWorld() const
{
	UWorld* WorldPtr = nullptr;

#if WITH_EDITOR
	if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
	{
		WorldPtr = EditorEngine->GetPIEWorldContext()
			? EditorEngine->GetPIEWorldContext()->World()
			: EditorEngine->GetEditorWorldContext().World();
	}
#endif

	if (!IsValid(WorldPtr))
	{
		if (UGameEngine* GameEngine = Cast<UGameEngine>(GEngine))
		{
			WorldPtr = GameEngine->GetGameWorld();
		}
		else
		{
			UE_LOG(LogUnrealCV, Error, TEXT("GEngine is neither EditorEngine nor GameEngine."));
		}
	}

	if (!IsValid(WorldPtr))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UWorld pointer is invalid (%p)."), WorldPtr);
		return nullptr;
	}
	return WorldPtr;
}

UWorld* FUnrealcvServer::GetGameWorld() const
{
#if WITH_EDITOR
	if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
	{
		UWorld* World = EditorEngine->PlayWorld;
		return (IsValid(World) && World->IsGameWorld()) ? World : nullptr;
	}
#endif

	if (UGameEngine* GameEngine = Cast<UGameEngine>(GEngine))
	{
		UWorld* World = GameEngine->GetGameWorld();
		return IsValid(World) ? World : nullptr;
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// Singleton & construction
// ---------------------------------------------------------------------------

FUnrealcvServer& FUnrealcvServer::Get()
{
	static FUnrealcvServer Singleton;
	return Singleton;
}

void FUnrealcvServer::RegisterCommandHandlers()
{
	if (CommandHandlers.Num() > 0)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Command handlers already registered — skipping."));
		return;
	}

	CommandHandlers.Emplace(MakeUnique<FObjectHandler>());
	CommandHandlers.Emplace(MakeUnique<FPluginHandler>());
	CommandHandlers.Emplace(MakeUnique<FActionHandler>());
	CommandHandlers.Emplace(MakeUnique<FAliasHandler>());
	CommandHandlers.Emplace(MakeUnique<FCameraHandler>());

	for (const TUniquePtr<FCommandHandler>& Handler : CommandHandlers)
	{
		Handler->SetCommandDispatcher(CommandDispatcher);
		Handler->RegisterCommands();
	}
}

FUnrealcvServer::FUnrealcvServer()
	: MessageRegexPattern(MessageFormat)
{
	TcpServer = NewObject<UUnixTcpServer>();
	CommandDispatcher = MakeShared<FCommandDispatcher>();
	FConsoleHelper::Get().SetCommandDispatcher(CommandDispatcher);

	TcpServer->AddToRoot(); // Prevent GC
	TcpServer->OnReceived().AddRaw(this, &FUnrealcvServer::HandleRawMessage);
	TcpServer->OnError().AddRaw(this, &FUnrealcvServer::HandleError);
}

FUnrealcvServer::~FUnrealcvServer()
{
	CommandHandlers.Empty(); // TUniquePtr handles cleanup

	if (TcpServer)
	{
		TcpServer->RemoveFromRoot();
		TcpServer = nullptr;
	}
}

// ---------------------------------------------------------------------------
// Request processing
// ---------------------------------------------------------------------------

void FUnrealcvServer::ProcessRequest(const FRequest& Request)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessRequest);

	const FExecStatus ExecStatus = CommandDispatcher->Exec(Request.Message);

	UE_LOG(LogUnrealCV, Verbose, TEXT("Response id=%d"), Request.RequestId);

	const FString Header = FString::Printf(TEXT("%d:"), Request.RequestId);
	TArray<uint8> ReplyData;
	FExecStatus::BinaryArrayFromString(Header, ReplyData);
	ReplyData += ExecStatus.GetData();
	TcpServer->SendData(ReplyData);
}

void FUnrealcvServer::ProcessPendingRequest()
{
	constexpr int32 MaxRequestsPerTick = 32;
	int32 ProcessedCount = 0;

	while (!PendingRequest.IsEmpty())
	{
		if (ProcessedCount >= MaxRequestsPerTick)
		{
			break;
		}

		FRequest Request;
		if (!PendingRequest.Dequeue(Request))
		{
			break;
		}
		++ProcessedCount;

		// Batch mode: "vbatch <N>" collects subsequent requests into one frame.
		if (Request.Message.StartsWith(TEXT("vbatch")))
		{
			BatchNum = FCString::Atoi(*Request.Message.Replace(TEXT("vbatch"), TEXT("")));
			if (BatchNum < 1)
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("Batch size must be >= 1, got %d."), BatchNum);
			}

			const FString Header = FString::Printf(TEXT("%d:"), Request.RequestId);
			TArray<uint8> ReplyData;
			FExecStatus::BinaryArrayFromString(Header, ReplyData);
			ReplyData += FExecStatus::OK().GetData();
			TcpServer->SendData(ReplyData);
			continue;
		}

		if (BatchNum <= 0)
		{
			// Normal (non-batch) mode.
			BatchNum = 1;
		}

		if (BatchNum > 0)
		{
			Batch.Add(Request);
			--BatchNum;
		}

		if (BatchNum == 0)
		{
			for (const FRequest& BatchRequest : Batch)
			{
				ProcessRequest(BatchRequest);
			}
			Batch.Empty();
		}
	}
}

// ---------------------------------------------------------------------------
// Network event handlers
// ---------------------------------------------------------------------------

void FUnrealcvServer::HandleRawMessage(const FString& Endpoint, const FString& InRawMessage)
{
	UE_LOG(LogUnrealCV, Verbose, TEXT("Request: %s"), *InRawMessage);

	FRegexMatcher Matcher(MessageRegexPattern, InRawMessage);
	if (Matcher.FindNext())
	{
		const int32 ParsedId  = FCString::Atoi(*Matcher.GetCaptureGroup(1));
		const uint32 RequestId = (ParsedId >= 0) ? static_cast<uint32>(ParsedId) : 0u;
		const FString Message  = Matcher.GetCaptureGroup(2);
		PendingRequest.Enqueue(FRequest(Endpoint, Message, RequestId));
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Malformed raw message: '%s'"), *InRawMessage);
	}
}

void FUnrealcvServer::HandleError(const FString& InErrorMessage)
{
	BatchNum = 0;
	Batch.Empty();

	if (Config.bExitOnFailure)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Server error (exit-on-failure enabled): %s"), *InErrorMessage);
		FGenericPlatformMisc::RequestExit(false);
	}
}
