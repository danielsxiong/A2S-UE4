// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking/Public/Networking.h"
#include "UDPListener.generated.h"

#define BUFFER_SIZE 1248
#define MAX_STRING_SIZE 128

inline FString BytesToStringFixed(const uint8 *In, int32 Count)
{
	FString Broken = BytesToString(In, Count);
	FString Fixed;

	for (int i = 0; i < Broken.Len(); i++)
	{
		const TCHAR c = Broken[i] - 1;
		Fixed.AppendChar(c);
	}

	return Fixed;
}

inline void StringToBytesFixed(const FString In, int32 Count, uint8 *OutBytes)
{
	FString Fixed;
	for (int i = 0; i < In.Len(); i++)
	{
		const TCHAR c = In[i] + 1;
		Fixed.AppendChar(c);
	}
	StringToBytes(Fixed, OutBytes, Count);
}

struct FServerInfo
{
	uint8 ResponseHeader[4] = { 0xff, 0xff, 0xff, 0xff };
	uint8 Header = 0x49;
	uint8 Protocol = 0x80;
	FString ServerName = "Default";
	FString Map = "DefaultMap";
	FString Folder;
	FString GameName;
	uint16 ID;
	uint8 Players = 8;
	uint8 MaxPlayers = 20;
	uint8 Bots = 0;
	uint8 ServerType = 0;
	uint8 Environment;
	uint8 Visibility = 0;
	uint8 VAC = 0;
	FString Version;
	uint8 EDF = 0x00;
	uint16 GamePort = 9100; // EDF 0x80
	uint64 SteamID; // EDF 0x10
	uint16 SourceTVPort; // EDF 0x40
	FString SourceTVName; // EDF 0x40
	FString Keywords; // EDF 0x20
	uint64 GameID; // EDF 0x01

	FServerInfo()
	{}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FServerInfo& TheStruct)
{
	uint8 ServerNameBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.ServerName, MAX_STRING_SIZE, ServerNameBytes);

	uint8 MapBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.Map, MAX_STRING_SIZE, MapBytes);

	uint8 FolderBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.Folder, MAX_STRING_SIZE, FolderBytes);

	uint8 GameNameBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.GameName, MAX_STRING_SIZE, GameNameBytes);

	uint8 VersionBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.Version, MAX_STRING_SIZE, VersionBytes);

	uint8 SourceTVNameBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.SourceTVName, MAX_STRING_SIZE, SourceTVNameBytes);

	uint8 KeywordsBytes[MAX_STRING_SIZE];
	StringToBytesFixed(TheStruct.Keywords, MAX_STRING_SIZE, KeywordsBytes);

	Ar << TheStruct.ResponseHeader[0];
	Ar << TheStruct.ResponseHeader[1];
	Ar << TheStruct.ResponseHeader[2];
	Ar << TheStruct.ResponseHeader[3];
	Ar << TheStruct.Header;
	Ar << TheStruct.Protocol;
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << ServerNameBytes[i];
		if (ServerNameBytes[i] == 0x00) break;
	}
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << MapBytes[i];
		if (MapBytes[i] == 0x00) break;
	}
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << FolderBytes[i];
		if (FolderBytes[i] == 0x00) break;
	}
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << GameNameBytes[i];
		if (GameNameBytes[i] == 0x00) break;
	}
	Ar << TheStruct.ID;
	Ar << TheStruct.Players;
	Ar << TheStruct.MaxPlayers;
	Ar << TheStruct.Protocol;
	Ar << TheStruct.Bots;
	Ar << TheStruct.ServerType;
	Ar << TheStruct.Environment;
	Ar << TheStruct.Visibility;
	Ar << TheStruct.VAC;
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << VersionBytes[i];
		if (VersionBytes[i] == 0x00) break;
	}
	Ar << TheStruct.EDF;
	Ar << TheStruct.GamePort;
	Ar << TheStruct.SteamID;
	Ar << TheStruct.SourceTVPort;
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << SourceTVNameBytes[i];
		if (SourceTVNameBytes[i] == 0x00) break;
	}
	for (int i = 0; i < MAX_STRING_SIZE; i++)
	{
		Ar << KeywordsBytes[i];
		if (KeywordsBytes[i] == 0x00) break;
	}
	Ar << TheStruct.GameID;

	return Ar;
}



UCLASS()
class BATTERYCOLLECTOR_API AUDPListener : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUDPListener();

	/** Event firing of when data arrives. **/
	UFUNCTION()
		void Event_DataReceived(const TArray<uint8>& ReceivedData);

	UFUNCTION(BlueprintCallable)
		void Init(const FString& _ip, const int32 _port);

	void Recv(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	/** Port on which UDP server listens. **/
	UPROPERTY(VisibleAnywhere)
		uint16 Port;
	FIPv4Address IPAddress;

	FSocket* ListenerSocket;
	FUdpSocketReceiver* UDPReceiver;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
