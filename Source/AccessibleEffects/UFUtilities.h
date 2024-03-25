// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UFUtilities.generated.h"

/**
 * 
 */
UCLASS()
class ACCESSIBLEEFFECTS_API UFUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utilities|Url")
	static FString BuildUrlParameters(const FString& BaseUrl, const TMap<FString, FString>& Parameters);

	UFUNCTION(BlueprintCallable, Category= "Utilities|Directory")
	static FString GetUserTempDirectory();

	UFUNCTION(BlueprintCallable, Category = "Utilities|Image")
	static TArray<uint8> GetImagePixelsInPNG(const int32 Width, const int32 Height,
	                                         const TArray<FColor>& ImagePixels);
};
