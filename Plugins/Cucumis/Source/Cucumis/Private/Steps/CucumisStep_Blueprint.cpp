// Fill out your copyright notice in the Description page of Project Settings.


#include "Steps/CucumisStep_Blueprint.h"

bool ACucumisStep_Blueprint::RunProceedToNextStep()
{
	if (bNextStep)
	{
		bNextStep = false;
		bRunningStep = false;
		return true;
	}
	return false;
}

bool ACucumisStep_Blueprint::StartCurrentStep()
{
	if (!bRunningStep)
	{
		bRunningStep = true;
		bNextStep = false;
		return true;
	}
	return false;
}

bool ACucumisStep_Blueprint::StepStart()
{
	if (!HasActorBegunPlay())
		return false;
	
	if (StartCurrentStep())
	{
		EventStepStart();
	}
	return RunProceedToNextStep();
}

bool ACucumisStep_Blueprint::StepRun()
{
	if (StartCurrentStep())
	{
		EventStepRun();
	}
	return RunProceedToNextStep();
}

bool ACucumisStep_Blueprint::StepEnd()
{
	if (StartCurrentStep())
	{
		EventStepEnd();
	}
	return RunProceedToNextStep();
}

void ACucumisStep_Blueprint::EventStepRun_Implementation()
{
	ProceedToNextStep();
}

void ACucumisStep_Blueprint::EventStepEnd_Implementation()
{
	ProceedToNextStep();
}

void ACucumisStep_Blueprint::EventStepStart_Implementation()
{
	ProceedToNextStep();
}

void ACucumisStep_Blueprint::ProceedToNextStep()
{
	bNextStep = true;
}

void ACucumisStep_Blueprint::Retry()
{
	bRunningStep = true;
}
