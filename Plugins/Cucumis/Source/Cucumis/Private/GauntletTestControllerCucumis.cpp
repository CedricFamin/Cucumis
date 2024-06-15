// Fill out your copyright notice in the Description page of Project Settings.


#include "GauntletTestControllerCucumis.h"
#include "..\Public\CucumisStep.h"
#include "Steps/CucumisStep_Simple.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Cucumis.h"
#include "CucumisSettings.h"
#include "GameMapsSettings.h"
#include "HttpPath.h"
#include "HttpServerModule.h"
#include "HttpServerRequest.h"
#include "IHttpRouter.h"
#include "ObjectTools.h"
#include "Steps/CucumisStep_Blueprint.h"

FCommonCucumisStepResponse::FCommonCucumisStepResponse()
{
	JsonObject = MakeShared<FJsonObject>();
}

TSharedPtr<FJsonObject> FCommonCucumisStepResponse::AddDataField(const FString& Name, const UCucumisStep* Item)
{
	JsonObject->SetField(Name, MakeShared<FJsonValueObject>(Item->JsonVariables()));
	return JsonObject;
}

void FCommonCucumisStepResponse::SendResponse(const FHttpResultCallback& OnComplete)
{
	OnCompleteCallback = OnComplete;
	SendResponse();
}
void FCommonCucumisStepResponse::SendResponse() const
{
	TUniquePtr<FHttpServerResponse> Response = MakeUnique<FHttpServerResponse>();
	Response->Code = EHttpServerResponseCodes::Ok;
	Response->Headers.FindOrAdd("content-type", { "Application/json" });

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		Response->Body.Append(reinterpret_cast<const unsigned char*>(StringCast<UTF8CHAR>(*Body).Get()), Body.Len());
	}
	OnCompleteCallback(MoveTemp(Response));
}

void UGauntletTestControllerCucumis::OnInit()
{
	Super::OnInit();

	if (uint64 Port = 9876; FParse::Value(FCommandLine::Get(), TEXT("CucumisPort="), Port))
	{
		HttpRouter = FHttpServerModule::Get().GetHttpRouter(Port, /* bFailOnBindFailure = */ true);
		FHttpServerModule::Get().StartAllListeners();
		BindManagementRoute();
		LoadBlueprintCucumisSteps();
		LoadCucumisSteps();
	}
	else
	{
		UE_LOG(LogCucumis, Error, TEXT("Can't parse CucumisPort argument, aborting."));
		EndTest(1);
	}
}

void UGauntletTestControllerCucumis::OnPreMapChange()
{
	Super::OnPreMapChange();
}

void UGauntletTestControllerCucumis::OnPostMapChange(UWorld* World)
{
	Super::OnPostMapChange(World);
}

void UGauntletTestControllerCucumis::OnTick(float TimeDelta)
{
	Super::OnTick(TimeDelta);
	if (!StepsToRun.IsEmpty())
	{
		const TObjectPtr<UCucumisStep> CurrentStep = StepsToRun[0];
		if (CurrentStep->IsStepFinished())
		{
			FFinishedStep EndInfo;
			EndInfo.NetworkID = CurrentStep->GetNetworkID();
			EndInfo.ResponseVariables = CurrentStep->JsonVariables();
			FinishedSteps.Emplace(EndInfo);
			StepsToRun.RemoveAt(0);
		}
		else if (!CurrentStep->IsRunning())
		{
			CurrentStep->StartStep();
		}
		else
		{
			CurrentStep->Tick(GetWorld());
		}
	}
}

void UGauntletTestControllerCucumis::OnStateChange(FName OldState, FName NewState)
{
	Super::OnStateChange(OldState, NewState);
}

bool UGauntletTestControllerCucumis::RunSteps(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
{
	FString StepName = Request.PathParams.FindChecked(TEXT("stepname"));
	TObjectPtr<UCucumisStep>* BaseStep = Steps.Find(StepName);
	if (BaseStep == nullptr)
	{
		BaseStep = Steps.Find("StepNotRegistered");
	}
	UCucumisStep* NewStep = DuplicateObject<UCucumisStep>(*BaseStep, GetTransientPackage());

	NewStep->SetNetworkID(NetworkGuidIndex++);
	StepsToRun.Add(NewStep);
	
	NewStep->SetupStep(GetWorld(), Request, OnComplete);
	UE_LOG(LogCucumis, Display, TEXT("Add step to the queue: %s, %s"), *NewStep->GetStepName(), *NewStep->StaticClass()->GetPathName());
	
	return true;
}

void UGauntletTestControllerCucumis::BindManagementRoute()
{
	HttpRouter->BindRoute(FHttpPath(TEXT("/Cucumis/Steps/Reset")), EHttpServerRequestVerbs::VERB_POST | EHttpServerRequestVerbs::VERB_GET
		, FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
		{
			if (!StepsToRun.IsEmpty())
			{
				return false;
			}
			
			if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
			{
				GEngine->BrowseToDefaultMap(*GameInstance->GetWorldContext());
				TUniquePtr<FCommonCucumisStepResponse> Response = MakeUnique<FCommonCucumisStepResponse>();
			
				Response->AddStringField("State", "Succeeded");
				Response->SendResponse(OnComplete);
				return true;
			}
			return false;
		}));
	
	HttpRouter->BindRoute(FHttpPath(TEXT("/Cucumis/Steps/:stepname")), EHttpServerRequestVerbs::VERB_POST | EHttpServerRequestVerbs::VERB_GET
		, FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
		{
			return RunSteps(Request, OnComplete);
		}));
	
	HttpRouter->BindRoute(FHttpPath(TEXT("/Cucumis/ListSteps")), EHttpServerRequestVerbs::VERB_GET
		, FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
		{
			TUniquePtr<FCommonCucumisStepResponse> Response = MakeUnique<FCommonCucumisStepResponse>();
			
			Response->SendResponse(OnComplete);
			return true;
		}));

	HttpRouter->BindRoute(FHttpPath(TEXT("/Cucumis/Finished")), EHttpServerRequestVerbs::VERB_GET
		, FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) -> bool
		{
			TUniquePtr<FCommonCucumisStepResponse> Response = MakeUnique<FCommonCucumisStepResponse>();
			
			Response->AddField("Steps", FinishedSteps,
			[](const FFinishedStep& Info) -> TSharedPtr<FJsonValue>
			{
				return MakeShareable(new FJsonValueNumber(Info.NetworkID.ObjectId));
			});
			
			Response->SendResponse(OnComplete);
			return true;
		}));
}


void UGauntletTestControllerCucumis::RegisterNewStep(UCucumisStep* Step)
{
	Steps.Add(Step->GetStepName(), Step);
}

void UGauntletTestControllerCucumis::LoadCucumisSteps()
{
	LoadNativeSteps();
	//LoadBlueprintCucumisSteps();
}

bool UGauntletTestControllerCucumis::IsCucumisPluginInitialized(UCucumisStep_Simple* Step)
{
	if (Steps.IsEmpty())
	{
		return false;
	}
			
	return true;
}

bool UGauntletTestControllerCucumis::LoadBlueprintSteps(UCucumisStep_Simple* Step)
{
	if (!bBlueprintLoaded)
	{
		LoadBlueprintCucumisSteps();
		bBlueprintLoaded = true;
	}

	return true;
}

bool UGauntletTestControllerCucumis::QuitTest(UCucumisStep_Simple* Step)
{
	FTimerHandle UnusedHandle;
	GetWorld()->GetTimerManager().SetTimer(
		UnusedHandle, []()
		{
			EndTest(0);
		}, 5.0f, false);
	return true;
}

bool UGauntletTestControllerCucumis::StepNotRegistered(UCucumisStep_Simple* Step)
{
	Step->SetResponseError("Step not found");
	return true;
}

void UGauntletTestControllerCucumis::LoadNativeSteps()
{
	RegisterNewStep(UCucumisStep_Simple::FromUFunction("StepNotRegistered", this, "StepNotRegistered"));
	RegisterNewStep(UCucumisStep_Simple::FromUFunction("IsCucumisPluginInitialized", this, "IsCucumisPluginInitialized"));
	RegisterNewStep(UCucumisStep_Simple::FromUFunction("LoadBlueprintSteps", this, "LoadBlueprintSteps"));
	RegisterNewStep(UCucumisStep_Simple::FromUFunction("QuitTest", this, "QuitTest"));
}

void UGauntletTestControllerCucumis::LoadBlueprintCucumisSteps()
{
	const UCucumisSettings* Settings = GetDefault<UCucumisSettings>();
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetsData;
	FARFilter Filter;
	for (const FDirectoryPath& StepsDirectory : Settings->StepsDirectories)
	{
		Filter.PackagePaths.Emplace(StepsDirectory.Path);
	}
	Filter.ClassPaths.Emplace(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	AssetRegistryModule.Get().GetAssets(Filter, AssetsData);
	for (FAssetData AssetData : AssetsData)
	{
		UObject* Asset = AssetData.GetAsset();
		const UBlueprint* BlueprintAsset = Cast<UBlueprint>(Asset);

		if (!BlueprintAsset || !BlueprintAsset->GeneratedClass)
		{
			continue;
		}

		if (!BlueprintAsset->GeneratedClass->IsChildOf(UCucumisStep_Blueprint::StaticClass()))
		{
			continue;
		}

		UCucumisStep* Step = NewObject<UCucumisStep>(GetTransientPackage(), BlueprintAsset->GeneratedClass);
		if (!Step)
		{
			continue;
		}
		RegisterNewStep(Step);
	}
}
