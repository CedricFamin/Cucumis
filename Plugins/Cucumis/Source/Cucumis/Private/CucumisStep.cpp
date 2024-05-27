// Fill out your copyright notice in the Description page of Project Settings.

#include "CucumisStep.h"
#include "HttpServerRequest.h"
#include "ControlFlow.h"

ACucumisStep::ACucumisStep()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

bool ACucumisStep::ParsePostParams(const FHttpServerRequest& Request)
{
	const FString BodyAsString = FString(Request.Body.Num(), reinterpret_cast<const char*>(Request.Body.GetData()));
	if (BodyAsString.IsEmpty())
	{
		return true;
	}
	
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyAsString);
	
	if (FJsonSerializer::Deserialize(Reader, PostParamsRaw))
	{
		return true;
	}

	return false;
}

void ACucumisStep::Flow_SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	World = InWorld;
	PostParamsRaw = MakeShared<FJsonObject>();
	JsonReponseRaw = MakeShared<FJsonObject>();
	Response.OnCompleteCallback = OnComplete;

	if (!ParsePostParams(Request))
	{
		SetResponseError("Can't parse body.");
	}
}

void ACucumisStep::Flow_SendResponse()
{
	Response.AddStringField("State", bHasError ? "Failed" : "Succeeded");
	Response.AddDataField("Data", this);
	Response.SendResponse();
	bResponseSent = true;
}

void ACucumisStep::SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	SetActorTickEnabled(true);
	StepFlow = MakeShared<FControlFlow>();
	StepFlow->QueueFunction(TEXT("Setup Step")).BindLambda([&]{ Flow_SetupStep(InWorld, Request, OnComplete);});
	StepFlow->QueueWait("Wait until step start").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (IsRunning() || bHasError)
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueWait("Wait Start End").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (bHasError || StepStart())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueWait("Wait Run End").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (bHasError || StepRun())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueWait("Wait End End").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (bHasError || StepEnd())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueFunction(TEXT("DeInit")).BindLambda([this]{ RunCurrentStep.Unbind(); });
	StepFlow->QueueStep(TEXT("Send response"), this, &ACucumisStep::Flow_SendResponse);
	StepFlow->ExecuteFlow();
}

void ACucumisStep::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (RunCurrentStep.IsBound())
		RunCurrentStep.Execute();
}

void ACucumisStep::SetResponseError(const FString& Error)
{
	JsonReponseRaw->SetStringField("Error", Error);
	bHasError = true;
}
