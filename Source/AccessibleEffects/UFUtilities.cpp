// Fill out your copyright notice in the Description page of Project Settings.


#include "UFUtilities.h"

#include "ImageUtils.h"

FString UFUtilities::BuildUrlParameters(const FString& BaseUrl, const TMap<FString, FString>& Parameters)
{
	FString Url = FString(BaseUrl);

	if (Parameters.Num() > 0)
	{
		Url.Append("?");
		for (auto It = Parameters.CreateConstIterator(); It; ++It)
		{
			if (It.Value().IsEmpty()) continue;

			Url.Append(It.Key());
			Url.Append("=");
			Url.Append(It.Value());
			Url.Append("&");
		}
		Url.RemoveFromEnd("&");
	}

	return Url;
}

FString UFUtilities::GetUserTempDirectory()
{
	return FString(FGenericPlatformProcess::UserTempDir()).Append("/SeeingVR/");
}

TArray<uint8> UFUtilities::GetImagePixelsInPNG(const int32 Width,
                                               const int32 Height,
                                               const TArray<FColor>& ImagePixels)
{
	TArray<uint8> ImageData;

	// ReSharper disable once CppDeprecatedEntity
	FImageUtils::CompressImageArray(Width, Height, ImagePixels, ImageData);

	return ImageData;
}
