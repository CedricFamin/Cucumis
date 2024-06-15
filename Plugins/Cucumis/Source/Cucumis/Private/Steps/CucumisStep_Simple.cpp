// Fill out your copyright notice in the Description page of Project Settings.


#include "Steps/CucumisStep_Simple.h"

TObjectPtr<UCucumisStep_Simple> UCucumisStep_Simple::FromUFunction(const FString& StepName, UObject* Object, const FName& Func)
{
	TObjectPtr<UCucumisStep_Simple> NewStep = NewObject<UCucumisStep_Simple>();
	NewStep->SetStepName(StepName);
	NewStep->Run.BindUFunction(Object, Func);
	return NewStep;
}
