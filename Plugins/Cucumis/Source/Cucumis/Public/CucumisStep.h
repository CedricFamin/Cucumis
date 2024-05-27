// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GauntletTestControllerCucumis.h"
#include "UObject/Object.h"
#include "HttpResultCallback.h"
#include "ControlFlowNode.h"
#include "ControlFlow.h"

#include "CucumisStep.generated.h"

struct FHttpServerRequest;

UENUM(BlueprintType)
enum class ECucumisTestStep : uint8
{
	Queued,
	Start,
	Run,
	End,
	Finished
};

DECLARE_DELEGATE(FCucumisStep_RunStep);
/**
 * 
 */
UCLASS(NotBlueprintType, NotBlueprintable)
class CUCUMIS_API ACucumisStep : public AInfo
{
	GENERATED_BODY()
public:
	ACucumisStep();

	UPROPERTY(EditAnywhere)
	FString HttpRoute;

	UPROPERTY(EditAnywhere)
	FString StepName;

public:

	virtual bool ParsePostParams(const FHttpServerRequest& Request);
	virtual void SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	virtual void Tick(float DeltaSeconds) override;
	virtual bool StepStart() { return true; }
	virtual bool StepRun() { return true; }
	virtual bool StepEnd() { return true; }
	
	void StartStep() { bStepStarted = true; }
	bool IsRunning() const { return bStepStarted; }
	bool IsStepFinished() const { return bStepStarted && bResponseSent; }

	void SetNetworkID(const uint64 NetID) { NetworkID = FNetworkGUID::CreateFromIndex(NetID, false);}
	const FNetworkGUID& GetNetworkID() const { return NetworkID; }
	const TSharedPtr<FJsonObject>& JsonVariables() const { return JsonReponseRaw; }

	FCommonCucumisStepResponse Response;
protected:
	UFUNCTION(BlueprintCallable)
	void SetResponseError(const FString& Error);

	TSharedPtr<FJsonObject> PostParamsRaw;
	TSharedPtr<FJsonObject> JsonReponseRaw;

	UPROPERTY(BlueprintReadOnly)
	UWorld* World = nullptr;

private:
	FNetworkGUID NetworkID;
	
	TSharedPtr<FControlFlow> StepFlow;
	bool bHasError = false;
	bool bResponseSent = false;
	bool bStepStarted = false;

	FCucumisStep_RunStep RunCurrentStep;
	
	void Flow_SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	void Flow_SendResponse();
};
