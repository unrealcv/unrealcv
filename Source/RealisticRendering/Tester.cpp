#include "RealisticRendering.h"
#include "Tester.h"


NetworkManagerTester::NetworkManagerTester()
{
}

void NetworkManagerTester::Init()
{
	NetworkManager = NewObject<UNetworkManager>();
	NetworkManager->AddToRoot();
	NetworkManager->OnReceived().AddRaw(this, &NetworkManagerTester::LogMessage);
	NetworkManager->Start();
}

void NetworkManagerTester::Run()
{
	NetworkManager->SendMessage(TEXT("Hello from server"));
}

void NetworkManagerTester::LogMessage(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

UE4CVServerTester::UE4CVServerTester(FCommandDispatcher *InCommandDispatcher)
{
	CommandDispatcher = InCommandDispatcher;
}

void UE4CVServerTester::Init()
{
	Server = new FUE4CVServer(CommandDispatcher);
	Server->Start();
}

void UE4CVServerTester::Run()
{
	Server->SendClientMessage(TEXT("Hello from server"));
}


FString ConvertToSandboxPath(const TCHAR* Filename);
void FilePathTester::Run()
{
	FString LaunchDir = *FPaths::LaunchDir();
	FString EngineDir = FPaths::EngineDir();
	FString EngineUserDir = FPaths::EngineUserDir();
	FString GameSavedDir = FPaths::GameSavedDir();
	FString RootDir = FPaths::RootDir();
	FString GameUserDir = FPaths::GameUserDir();
	FString ScreenShotDir = FPaths::ScreenShotDir();
	FString NormalizedScreenShotDir = ScreenShotDir;
	FPaths::NormalizeFilename(NormalizedScreenShotDir);
	FString FullScreenshot = FPaths::ConvertRelativePathToFull(ScreenShotDir);
	// FPaths::MakePlatformFilename(FullScreenshot);
	// IFileManager& FileManager = IFileManager::Get();
	// FString DiskFilename = FileManager.GetFilenameOnDisk(*FullScreenshot);
	FString TestFolder = FPaths::ConvertRelativePathToFull("Test");
	// FString EngineLocalizationPaths = FPaths::GetEngineLocalizationPaths();
	FString ProjectFilePath = FPaths::GetProjectFilePath();
	FString PlatformProcessBase = FPlatformProcess::BaseDir();


	FString Filename = FString::Printf(TEXT("%04d.png"), 0);
	const FString Dir = FPlatformProcess::BaseDir(); // TODO: Change this to screen capture folder
	FString FullFilename = FPaths::Combine(*Dir, *Filename);
	IFileManager& FileManager = IFileManager::Get();
	FString DiskFilename = FileManager.GetFilenameOnDisk(*FullFilename);

	int32 Break = 0; // If I set a breakpoint to the last line, all the local variables will be GCed.
}