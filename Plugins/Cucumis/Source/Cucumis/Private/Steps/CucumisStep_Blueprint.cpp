// Fill out your copyright notice in the Description page of Project Settings.


#include "Steps/CucumisStep_Blueprint.h"

bool UCucumisStep_Blueprint::RunProceedToNextStep()
{
	if (bNextStep)
	{
		bNextStep = false;
		bRunningStep = false;
		return true;
	}
	return false;
}

bool UCucumisStep_Blueprint::StartCurrentStep()
{
	if (!bRunningStep)
	{
		bRunningStep = true;
		bNextStep = false;
		return true;
	}
	return false;
}

bool UCucumisStep_Blueprint::StepStart()
{
	if (StartCurrentStep())
	{
		EventStepStart();
	}
	return RunProceedToNextStep();
}

bool UCucumisStep_Blueprint::StepRun()
{
	if (StartCurrentStep())
	{
		EventStepRun();
	}
	return RunProceedToNextStep();
}

bool UCucumisStep_Blueprint::StepEnd()
{
	if (StartCurrentStep())
	{
		EventStepEnd();
	}
	return RunProceedToNextStep();
}

void UCucumisStep_Blueprint::EventStepRun_Implementation()
{
	ProceedToNextStep();
}

void UCucumisStep_Blueprint::EventStepEnd_Implementation()
{
	ProceedToNextStep();
}

void UCucumisStep_Blueprint::EventStepStart_Implementation()
{
	ProceedToNextStep();
}

void UCucumisStep_Blueprint::ProceedToNextStep()
{
	bNextStep = true;
}

void UCucumisStep_Blueprint::Retry()
{
	bRunningStep = true;
}
