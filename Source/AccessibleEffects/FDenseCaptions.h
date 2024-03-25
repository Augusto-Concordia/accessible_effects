// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FDenseCaptions.generated.h"

USTRUCT(BlueprintType)
struct FDenseCaption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Caption")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Caption")
	float Confidence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Caption")
	FVector2D Position;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Caption")
	FVector2D Size;

	bool operator <(const FDenseCaption& Other) const;
};
