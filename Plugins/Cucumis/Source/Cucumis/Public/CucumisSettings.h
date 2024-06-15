// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CucumisSettings.generated.h"

/**
 * 
 */
UCLASS(config=CucumisSettings, DefaultConfig)
class CUCUMIS_API UCucumisSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, meta=(ContentDir))
	TArray<FDirectoryPath> StepsDirectories;
};
