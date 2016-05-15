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