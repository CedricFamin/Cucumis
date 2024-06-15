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
 * Transform cucumis step to interface, some steps will be UObject & others actors to tick & gain other functionalities.
 */
UCLASS(NotBlueprintType, NotBlueprintable)
class CUCUMIS_API UCucumisStep : public UObject
{
	GENERATED_BODY()
public:
	UCucumisStep();

	UE_DEPRECATED(5.2, "The action filter now stores a reference to the authoritative editor context. Please use the default constructor instead.")
	virtual FString GetHttpRoute() const { return GetStepName(); }
	void SetStepName(const FString& InStepName) { StepName = InStepName; }
	virtual FString GetStepName() const { return StepName; }

	virtual bool ParsePostParams(const FHttpServerRequest& Request);
	virtual void SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	virtual void Tick(UWorld* InWorld);

	virtual bool StepStart() { return true; }
	virtual bool StepRun() { return true; }
	virtual bool StepEnd() { return true; }
	
	UFUNCTION(Blueprintable)
	virtual void SetResponseError(const FString& Error);

	void StartStep() { bStepStarted = true; }
	bool IsRunning() const { return bStepStarted; }
	bool IsStepFinished() const { return bStepStarted && bResponseSent; }
	bool HasError() const { return bHasError; }
	bool ResponseSent() const { return bResponseSent; }

	void SetNetworkID(const uint64 NetID) { NetworkID = FNetworkGUID::CreateFromIndex(NetID, false);}
	const FNetworkGUID& GetNetworkID() const { return NetworkID; }
	const TSharedPtr<FJsonObject>& JsonVariables() const { return JsonResponseRaw; }
	
public:
	FCommonCucumisStepResponse Response;

protected:
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<UWorld> World;
private:
	UPROPERTY(EditAnywhere)
	FString StepName;
	
	TSharedPtr<FControlFlow> StepFlow;
	void Flow_SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	void Flow_SendResponse();

	FCucumisStep_RunStep RunCurrentStep;

	TSharedPtr<FJsonObject> PostParamsRaw;
	TSharedPtr<FJsonObject> JsonResponseRaw;

	FNetworkGUID NetworkID;
	
	bool bHasError = false;
	bool bResponseSent = false;
	bool bStepStarted = false;
};
