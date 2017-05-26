#include "UnrealCVPrivate.h"
#include "Regex.h"
#include "CommandDispatcher.h"

// CommandDispatcher->BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp

DECLARE_CYCLE_STAT(TEXT("WaitAsync"), STAT_WaitAsync, STATGROUP_UnrealCV);
class FAsyncWatcher : public FRunnable
{
public:
	void Wait(FPromise InPromise, FCallbackDelegate InCompletedCallback) // Need to open a new thread. Can not wait on the original thread
	{
		check(InPromise.bIsValid);
		this->PendingPromise.Enqueue(InPromise);
		this->PendingCompletedCallback.Enqueue(InCompletedCallback);
	}
	static FAsyncWatcher& Get()
	{
		static FAsyncWatcher Singleton;
		return Singleton;
	}
	bool IsActive() const
	{
		return !PendingPromise.IsEmpty();
	}
	~FAsyncWatcher()
	{
		this->Stopping = true;
	}
private:
	FAsyncWatcher()
	{
		Thread = FRunnableThread::Create(this, TEXT("FAsyncWatcher"), 8 * 1024, TPri_Normal);
		Stopping = false;
	}

	TQueue<FPromise, EQueueMode::Spsc> PendingPromise;
	TQueue<FCallbackDelegate, EQueueMode::Spsc> PendingCompletedCallback;
	bool Stopping; // Whether this thread should stop?

	FRunnableThread* Thread;
	virtual uint32 Run() override
	{
		/** FIXME: Find a more efficient implementation for this */
		while (!Stopping)
		{
			if (!IsActive())
			{
				continue;
			}
			SCOPE_CYCLE_COUNTER(STAT_WaitAsync);
			FPromise Promise;
			PendingPromise.Peek(Promise);
			FExecStatus ExecStatus = Promise.CheckStatus();
			if (ExecStatus != FExecStatusType::Pending) // Job is finished
			{
				FPromise CompletedPromise;
				FCallbackDelegate CompletedCallback;
				PendingPromise.Dequeue(CompletedPromise); // Dequeue in the same thread
				PendingCompletedCallback.Dequeue(CompletedCallback);

				// This needs to be sent back to game thread
				AsyncTask(ENamedThreads::GameThread, [CompletedCallback, ExecStatus]() {
					CompletedCallback.ExecuteIfBound(ExecStatus);
					// CompletedCallback.Unbind();
				});
				// Stop the timer
			}
			if (Promise.GetRunningTime() > 5) // Kill pending task that takes too long
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("An async task failed to finish and timeout after 5 seconds"));
				// This is a failed task
				FPromise FailedPromise;
				FCallbackDelegate CompletedCallback;
				PendingPromise.Dequeue(FailedPromise); // Dequeue in the same thread
				PendingCompletedCallback.Dequeue(CompletedCallback);

				// This needs to be sent back to game thread
				AsyncTask(ENamedThreads::GameThread, [CompletedCallback]() {
					CompletedCallback.ExecuteIfBound(FExecStatus::Error("Task took too long, timeout"));
					// CompletedCallback.Unbind();
				});
			}
		}
		return 0; // This thread will exit
	}
};


FCommandDispatcher::FCommandDispatcher()
{
	FString Str = "([^ ]*)", UInt = "(\\d*)", Float = "([-+]?\\d*[.]?\\d+)"; // Each type will be considered as a group
	TypeRegexp.Emplace("str", Str);
	TypeRegexp.Emplace("uint", UInt);
	TypeRegexp.Emplace("float", Float);

	FDispatcherDelegate Cmd = FDispatcherDelegate::CreateRaw(this, &FCommandDispatcher::AliasHelper);
	FString Uri = FString::Printf(TEXT("vrun [str]"));
	BindCommand(Uri, Cmd, "Run an alias for Unreal CV plugin");
}

FCommandDispatcher::~FCommandDispatcher()
{
}

bool FCommandDispatcher::FormatUri(const FString& RawUri, FString& UriRexexp)
{
	FString Uri = "";
	bool IsTypeSpecifier = false;
	FString TypeSpecifier = "";

	// Implement a simple state-machine to parse input string
	for (int32 Index = 0; Index < RawUri.Len(); Index++)
	{
		TCHAR Ch = RawUri[Index];
		if (!IsTypeSpecifier)
		{
			if (Ch == '[')
			{
				IsTypeSpecifier = true;
				continue;
			}
			if (Ch == ']')
			{
				UE_LOG(LogUnrealCV, Error, TEXT("Unexpected ] in %d"), Index);
				check(false);
				return false;
			}
			// else
			Uri += Ch;
			continue;
		}
		if (IsTypeSpecifier)
		{
			if (Ch == '[')
			{
				UE_LOG(LogUnrealCV, Error, TEXT("Unexpected [ in %d"), Index);
				check(false);
				return false;
			}
			if (Ch == ']')
			{
				IsTypeSpecifier = false;
				if (TypeRegexp.Contains(TypeSpecifier))
				{
					Uri += TypeRegexp[TypeSpecifier];
					TypeSpecifier = "";
					continue;
				}
				else
				{
					UE_LOG(LogUnrealCV, Error, TEXT("Unknown type specifier "));
					check(false);
					return false;
				}
			}
			// else
			TypeSpecifier += Ch;
			continue;
		}
	}

	if (TypeSpecifier != "")
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Not all [ are closed by ]"));
		check(false);
		return false;
	}
	UriRexexp = Uri + "[ ]*$"; // Make sure no more parameters
	return true;
}

/** Bind command to a function, the command on the bottom will overwrite the command on the top */
bool FCommandDispatcher::BindCommand(const FString& ReadableUriTemplate, const FDispatcherDelegate& Command, const FString& Description) // Parse URI
{
	// TODO: Build a tree structure to boost performance
	// TODO: The order should matter

	FString UriTemplate;
	if (!FormatUri(ReadableUriTemplate, UriTemplate))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("The UriTemplate %s is malformat"), *ReadableUriTemplate);
		check(false);
		return false;
	}

	if (UriMapping.Contains(UriTemplate))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The UriTemplate %s already exist, overwrited."), *UriTemplate);
	}
	UriMapping.Emplace(UriTemplate, Command);
	UriDescription.Emplace(ReadableUriTemplate, Description);
	UriList.AddUnique(UriTemplate);
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const TArray<FString>& Commands, const FString& Description)
{
	if (AliasMapping.Contains(InAlias))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Alias %s already exist, overwrite."), *InAlias);
	}
	AliasMapping.Emplace(InAlias, Commands);
	// Alias can not support arguments
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const FString& Command, const FString& Description)
{
	TArray<FString> Commands;
	Commands.Add(Command);
	Alias(InAlias, Commands, Description);
	// Alias can not support arguments
	return true;
}

FExecStatus FCommandDispatcher::AliasHelper(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString AliasName = Args[0];
		if (AliasMapping.Contains(AliasName))
		{
			TArray<FString>& Commands = AliasMapping[AliasName];
			FString Message = "";
			for (FString Command : Commands)
			{
				FExecStatus ExecStatus = Exec(Command);
				Message += ExecStatus.GetMessage(); // TODO: Ovewrite the + operator of ExecStatus
			}
			return FExecStatus::OK(Message); // TODO: Check whether one of them failed and set ExecStatus based on that.
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Unrecognized alias %s"), *AliasName));
		}
	}
	return FExecStatus::Error("Alias can not support extra parameters");
}

const TMap<FString, FString>& FCommandDispatcher::GetUriDescription()
{
	return this->UriDescription;
}

void FCommandDispatcher::ExecAsync(const FString Uri, const FCallbackDelegate Callback)
// FIXME: This is a stupid implementation to use a new thread to check whether the task is completed.
// ExecAsync can not guarantee the execuation order, this needs to be enforced in the client.
// Which means later tasks can finish earlier
{
	// If there are unfinished tasks, new async task will be pushed into a queue.
	// so even FAsyncWatcher::Get().IsActive(), you can still keep executing more async tasks
	FExecStatus ExecStatus = Exec(Uri);
	if (ExecStatus == FExecStatusType::AsyncQuery) // This is an async task
	{
		FAsyncWatcher::Get().Wait(ExecStatus.GetPromise(), Callback);
	}
	else
	{
		Callback.ExecuteIfBound(ExecStatus);
		// If this is a sync task, return the result immediately
	}
}


FExecStatus FCommandDispatcher::Exec(const FString Uri)
{
	check(IsInGameThread());
	TArray<FString> Args; // Get args from URI

	// The newly added command should overwrite previous one.
	// for (auto& Elem : UriMapping)

	// Iterate the UriList in the reverse order
	for (int UriIndex = UriList.Num() - 1; UriIndex >= 0; UriIndex--)
	{
		// FRegexPattern Pattern = FRegexPattern(Elem.Key);
		FString Key = UriList[UriIndex];
		FRegexPattern Pattern = FRegexPattern(Key);

		// FDispatcherDelegate& Cmd = Elem.Value;
		FDispatcherDelegate& Cmd = UriMapping[Key];

		FRegexMatcher Matcher(Pattern, Uri);
		if (Matcher.FindNext())
		{
			for (uint32 GroupIndex = 1; GroupIndex < NumArgsLimit + 1; GroupIndex++)
			{
				uint32 BeginIndex = Matcher.GetCaptureGroupBeginning(GroupIndex);
				if (BeginIndex == -1) break; // No more matching group to extract
				FString Match = Matcher.GetCaptureGroup(GroupIndex); // TODO: Strip empty space
				Args.Add(Match);
			}
			return Cmd.Execute(Args); // The exec status can be successful, fail or give a message back
		}

		// TODO: Regular expression mapping is slow, need to implement in a more efficient way.
		// FRegexMatcher()
	}
	return FExecStatus::Error(FString::Printf(TEXT("Can not find a handler for URI '%s'"), *Uri));
}
