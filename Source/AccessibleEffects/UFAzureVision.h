// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FAzureVisionSettings.h"
#include "FDenseCaptions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UFAzureVision.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSuccess, FString, Caption);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFailure, FString, ErrorMessage);

UCLASS()
class ACCESSIBLEEFFECTS_API UFAzureVision : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Azure Vision")
	static void GetImageDenseCaptions(const FAzureVisionSettings& Settings, const TArray<uint8>& ImageData, const FOnSuccess& OnSuccess, const FOnFailure&
	                                 OnFailure);

private:
	static void ProcessAzureVisionContent(const FString& Content, const FOnSuccess& OnSuccess, const FOnFailure& OnFailure);
};
