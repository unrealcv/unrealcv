// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "UE4CVCommands.h"
#include "ViewMode.h"
#include "ObjectPainter.h"
#include "CommandHandler.h"

UE4CVCommands::UE4CVCommands(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
{
	this->Character = InCharacter;
	this->CommandDispatcher = InCommandDispatcher;
	this->RegisterCommands();
}

UE4CVCommands::~UE4CVCommands()
{
}

void UE4CVCommands::RegisterCommands()
{
	FObjectCommandHandler ObjectCommandHandler(Character, CommandDispatcher);
	ObjectCommandHandler.RegisterCommands();

	FCameraCommandHandler CameraCommandHandler(Character, CommandDispatcher);
	CameraCommandHandler.RegisterCommands();

	FPluginCommandHandler PluginCommandHandler(Character, CommandDispatcher);
	PluginCommandHandler.RegisterCommands();

	// this->RegisterCommandsCamera();
	// this->RegisterCommandsPlugin();
	// First version
	// CommandDispatcher->BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
	FDispatcherDelegate Cmd;
	FString URI;
	// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
	// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp

	Cmd = FDispatcherDelegate::CreateRaw(&FViewMode::Get(), &FViewMode::SetMode);
	URI = "vset /mode/[str]";
	CommandDispatcher->BindCommand(URI, Cmd, "Set mode"); // Better to check the correctness at compile time

	Cmd = FDispatcherDelegate::CreateRaw(&FViewMode::Get(), &FViewMode::GetMode);
	CommandDispatcher->BindCommand("vget /mode", Cmd, "Get mode");


	// Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::PaintRandomColors);
	// CommandDispatcher->BindCommand(TEXT("vget /util/random_paint"), Cmd, "Paint objects with random color");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCommands);
	CommandDispatcher->BindCommand(TEXT("vget /util/get_commands"), Cmd, "Get all available commands");

	CommandDispatcher->Alias("SetDepth", "vset /mode/depth", "Set mode to depth"); // Alias for human interaction
	CommandDispatcher->Alias("VisionCamInfo", "vget /camera/0/name", "Get camera info");
	CommandDispatcher->Alias("ls", "vget /util/get_commands", "List all commands");
	CommandDispatcher->Alias("shot", "vget /camera/0/image", "Save image to disk");

}


FExecStatus UE4CVCommands::GetCommands(const TArray<FString>& Args)
{
	FString Message;

	TArray<FString> UriList;
	TMap<FString, FString> UriDescription = CommandDispatcher->GetUriDescription();
	UriDescription.GetKeys(UriList);

	for (auto Value : UriDescription)
	{
		Message += Value.Key + "\n";
		Message += Value.Value + "\n";
	}

	return FExecStatus::OK(Message);
}
