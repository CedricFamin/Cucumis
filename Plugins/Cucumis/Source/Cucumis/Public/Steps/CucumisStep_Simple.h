// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CucumisStep.h"
#include "CucumisStep_Simple.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FStepFlowRunFunction, UCucumisStep_Simple*, Step);

/**
 * 
 */
UCLASS()
class CUCUMIS_API UCucumisStep_Simple : public UCucumisStep
{
	GENERATED_BODY()
public:
	static TObjectPtr<UCucumisStep_Simple> FromUFunction(const FString& StepName, UObject* Object, const FName& Func);

	UPROPERTY()
	FStepFlowRunFunction Run;

	virtual bool StepRun() override
	{
		return Run.Execute(this);
	}

};
