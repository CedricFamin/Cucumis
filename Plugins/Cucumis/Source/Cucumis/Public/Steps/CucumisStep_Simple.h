// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CucumisStep.h"
#include "CucumisStep_Simple.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FStepFlowRunFunction);

/**
 * 
 */
UCLASS()
class CUCUMIS_API ACucumisStep_Simple : public ACucumisStep
{
	GENERATED_BODY()
public:
	static TObjectPtr<ACucumisStep_Simple> FromUFunction(const FString& StepName, UObject* Object, const FName& Func);

	UPROPERTY()
	FStepFlowRunFunction Run;

	virtual bool StepRun() override
	{
		return Run.Execute();
	}

};
