// Fill out your copyright notice in the Description page of Project Settings.


#include "UDPListener.h"

// Sets default values
AUDPListener::AUDPListener()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ListenerSocket = nullptr;
}

// Called when the game starts or when spawned
void AUDPListener::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("UDP Server started..."));

	Init("127.0.0.1", 9100);
}

void AUDPListener::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	delete UDPReceiver;
	UDPReceiver = nullptr;

	if (ListenerSocket)
	{
		ListenerSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket);
	}

	UE_LOG(LogTemp, Warning, TEXT("UDP Server ended..."));
}

// Called every frame
void AUDPListener::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUDPListener::Init(const FString & _ip, const int32 _port)
{
	Port = (uint16)_port;
	if (!FIPv4Address::Parse(_ip, IPAddress))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to convert IP."));
	}

	FIPv4Endpoint Endpoint(IPAddress, Port);

	UE_LOG(LogTemp, Warning, TEXT("IP Address: %s"), *IPAddress.ToText().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Port: %d"), Port);

	// Socket - which is a struct (F prefix)
	ListenerSocket = FUdpSocketBuilder(TEXT("udp_sock"))
		.AsReusable()
		.AsNonBlocking()
		.BoundToEndpoint(Endpoint)
		.WithBroadcast()
		.WithReceiveBufferSize(BUFFER_SIZE)
		;

	check(ListenerSocket != nullptr)

	int32 SendSize = BUFFER_SIZE;
	ListenerSocket->SetSendBufferSize(SendSize, SendSize);

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	UDPReceiver = new FUdpSocketReceiver(ListenerSocket, ThreadWaitTime, TEXT("thread_udp_receiver"));
	UDPReceiver->OnDataReceived().BindUObject(this, &AUDPListener::Recv);
	UDPReceiver->Start();
}

void AUDPListener::Recv(const FArrayReaderPtr & ArrayReaderPtr, const FIPv4Endpoint & EndPt)
{
	auto Content = BytesToStringFixed(ArrayReaderPtr->GetData(), ArrayReaderPtr->Num());
	UE_LOG(LogTemp, Warning, TEXT("Received Data Converted to FString: %s"), *Content);

	FArrayWriter Writer;
	FServerInfo NewData;
	NewData.Players = 10;

	//Create Remote Address.
	auto RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool bIsValid;
	RemoteAddr->SetIp(*EndPt.Address.ToText().ToString(), bIsValid);
	RemoteAddr->SetPort(EndPt.Port);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("IP not valid"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Send to IP: %s"), *EndPt.Address.ToText().ToString());
	UE_LOG(LogTemp, Warning, TEXT("Send to Port: %d"), EndPt.Port);

	Writer << NewData;
	int32 BytesSent = 0;
	ListenerSocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *RemoteAddr);

	if (BytesSent <= 0)
	{
		const FString Str = "Socket is valid but the receiver received 0 bytes, make sure it is listening properly!";
		UE_LOG(LogTemp, Error, TEXT("%s"), *Str);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("UDP~ Send Succcess! Bytes Sent = %d"), BytesSent);

	TArray<uint8> RawData;
	uint16 DataReceivedNum = ArrayReaderPtr->Num();
	for (uint16 i = 0; i < DataReceivedNum; i++) { RawData.Add(ArrayReaderPtr->Pop()); }

	Event_DataReceived(RawData);
}

void AUDPListener::Event_DataReceived(const TArray<uint8>& ReceivedData)
{
	/*for (auto It = ReceivedData.CreateConstIterator(); It; ++It)
	{
		auto Data = *It;
		UE_LOG(LogTemp, Warning, TEXT("Received Data: %u"), Data);
	}*/
}
