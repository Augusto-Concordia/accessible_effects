#pragma once

#include "FAzureVisionSettings.generated.h"

USTRUCT(BlueprintType)
struct FAzureVisionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	FString BaseUrl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString ApiVersion;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString Features;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString ModelName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString Language;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	FString SmartCropsAspectRatio;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool GenderNeutralCaption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Authorization")
	FString OcpApimSubscriptionKey;
};

UCLASS()
class UFAzureVisionSettingsHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Azure Vision|Helpers")
	static TMap<FString, FString> GetParametersMap(const FAzureVisionSettings& Settings);
};