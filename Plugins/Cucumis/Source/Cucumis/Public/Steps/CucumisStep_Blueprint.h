// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cucumis.h"
#include "CucumisStep.h"
#include "CucumisStep_Blueprint.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class CUCUMIS_API ACucumisStep_Blueprint : public ACucumisStep
{
	GENERATED_BODY()

public:
	bool StartCurrentStep();
	virtual bool StepStart() override;
	virtual bool StepRun() override;
	virtual bool StepEnd() override;

	UFUNCTION(BlueprintCallable)
	void ProceedToNextStep();

	UFUNCTION(BlueprintCallable)
	void Retry();

	UFUNCTION(BlueprintNativeEvent)
	void EventStepStart();

	UFUNCTION(BlueprintNativeEvent)
	void EventStepRun();

	UFUNCTION(BlueprintNativeEvent)
	void EventStepEnd();

	virtual void EventStepRun_Implementation();
	virtual void EventStepEnd_Implementation();
	virtual void EventStepStart_Implementation();

private:
	bool RunProceedToNextStep();
	
	bool bNextStep = false;
	bool bRunningStep = false;
};
