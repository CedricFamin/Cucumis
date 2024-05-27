// Fill out your copyright notice in the Description page of Project Settings.


#include "Steps/CucumisStep_Simple.h"

TObjectPtr<ACucumisStep_Simple> ACucumisStep_Simple::FromUFunction(const FString& StepName, UObject* Object, const FName& Func)
{
	TObjectPtr<ACucumisStep_Simple> NewStep = NewObject<ACucumisStep_Simple>();
	NewStep->StepName = StepName;
	NewStep->HttpRoute = "/Cucumis/Steps/" + StepName;
	NewStep->Run.BindUFunction(Object, Func);
	return NewStep;
}
