// Fill out your copyright notice in the Description page of Project Settings.

#include "..\Public\CucumisStep.h"
#include "HttpServerRequest.h"
#include "ControlFlow.h"

bool UCucumisStep::ParsePostParams(const FHttpServerRequest& Request)
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

void UCucumisStep::SetResponseError(const FString& Error)
{
	JsonResponseRaw->SetStringField("Error", Error);
	bHasError = true;
}

UCucumisStep::UCucumisStep()
{
}

void UCucumisStep::SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	StepFlow = MakeShared<FControlFlow>();
	StepFlow->QueueFunction(TEXT("Setup Step")).BindLambda([&]{ Flow_SetupStep(InWorld, Request, OnComplete);});
	StepFlow->QueueWait("Wait until step start").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (IsRunning() || HasError())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueWait("Wait Start End").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (HasError() || StepStart())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueWait("Wait Run End").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (HasError() || StepRun())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueWait("Wait End End").BindLambda([&](FControlFlowNodeRef FlowHandle)
	{
		RunCurrentStep.BindLambda([this, FlowHandle]
		{
			if (HasError() || StepEnd())
			{
				FlowHandle->ContinueFlow();
			}
		});
	});
	StepFlow->QueueFunction(TEXT("DeInit")).BindLambda([this]{ RunCurrentStep.Unbind(); });
	StepFlow->QueueStep(TEXT("Send response"), this, &UCucumisStep::Flow_SendResponse);
	StepFlow->ExecuteFlow();
}


void UCucumisStep::Flow_SetupStep(UWorld* InWorld, const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	PostParamsRaw = MakeShared<FJsonObject>();
	JsonResponseRaw = MakeShared<FJsonObject>();
	Response.OnCompleteCallback = OnComplete;

	if (!ParsePostParams(Request))
	{
		SetResponseError("Can't parse body.");
	}
}

void UCucumisStep::Flow_SendResponse()
{
	Response.AddStringField("State", HasError() ? "Failed" : "Succeeded");
	Response.AddDataField("Data", this);
	Response.SendResponse();
	bResponseSent = true;
}



void UCucumisStep::Tick(UWorld* InWorld)
{
	World = InWorld;
	if (RunCurrentStep.IsBound())
		RunCurrentStep.Execute();
}


