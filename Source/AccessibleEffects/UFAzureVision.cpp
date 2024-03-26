// Fill out your copyright notice in the Description page of Project Settings.


#include "UFAzureVision.h"

#include "FDenseCaptions.h"
#include "Http.h"
#include "UFUtilities.h"

#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

void UFAzureVision::GetImageDenseCaptions(const FAzureVisionSettings& Settings,
                                          const TArray<uint8>& ImageData,
                                          const FOnSuccess& OnSuccess,
                                          const FOnFailure& OnFailure)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	const TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();

	// Set up the request
	Request->SetVerb(TEXT("POST"));
	Request->SetURL(UFUtilities::BuildUrlParameters(Settings.BaseUrl,
	                                                UFAzureVisionSettingsHelpers::GetParametersMap(Settings)));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));
	Request->SetHeader(TEXT("Ocp-Apim-Subscription-Key"), Settings.OcpApimSubscriptionKey);

	// Set up the request body
	Request->SetContent(ImageData);

	// Setup the response callback
	Request->OnProcessRequestComplete().BindLambda(
		[OnSuccess, OnFailure](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bWasSuccessful)
		{
			if (Response->GetResponseCode() != 200 || !bWasSuccessful)
			{
				OnFailure.ExecuteIfBound(
					FString::Printf(TEXT("Request failed with code %d"), Response->GetResponseCode()));
				return;
			}

			if (!Response->GetContentType().StartsWith("application/json"))
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				OnFailure.ExecuteIfBound(TEXT("Response is not JSON"));
				return;
			}

			ProcessAzureVisionContent(Response->GetContentAsString(), OnSuccess, OnFailure);
		});

	// Submit the request
	Request->ProcessRequest();
}

void UFAzureVision::ProcessAzureVisionContent(const FString& Content,
                                              const FOnSuccess& OnSuccess,
                                              const FOnFailure& OnFailure)
{
	UE_LOG(LogTemp, Display, TEXT("Received: %s"), *Content);

	TArray<FDenseCaption> DenseCaptions;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonObject;

	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		auto _ = OnFailure.ExecuteIfBound(TEXT("Failed to parse JSON response"));
		return;
	}

	const TSharedPtr<FJsonObject>* Metadata;
	if (!JsonObject->TryGetObjectField(TEXT("metadata"), Metadata))
	{
		auto _ = OnFailure.ExecuteIfBound(TEXT("Failed to find metadata in denseCaptionsResult"));
		return;
	}

	int32 ImageWidth;
	if (!(*Metadata)->TryGetNumberField(TEXT("width"), ImageWidth))
	{
		auto _ = OnFailure.ExecuteIfBound(TEXT("Failed to find width in metadata"));
		return;
	}

	int32 ImageHeight;
	if (!(*Metadata)->TryGetNumberField(TEXT("height"), ImageHeight))
	{
		auto _ = OnFailure.ExecuteIfBound(TEXT("Failed to find height in metadata"));
		return;
	}

	const TSharedPtr<FJsonObject>* DenseCaptionsResult;
	if (!JsonObject->TryGetObjectField(TEXT("denseCaptionsResult"), DenseCaptionsResult))
	{
		auto _ = OnFailure.ExecuteIfBound(TEXT("Failed to find denseCaptionsResult in JSON response"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* Values;
	if (!(*DenseCaptionsResult)->TryGetArrayField(TEXT("values"), Values))
	{
		auto _ = OnFailure.ExecuteIfBound(TEXT("Failed to find values in denseCaptionsResult"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Found %d dense captions in response"), Values->Num());

	for (const auto& JsonCaption : *Values)
	{
		const TSharedPtr<FJsonObject> JsonCaptionObject = JsonCaption->AsObject();

		FDenseCaption Caption;
		Caption.Text = JsonCaptionObject->GetStringField("text");
		Caption.Confidence = JsonCaptionObject->GetNumberField("confidence");

		const TSharedPtr<FJsonObject> JsonBoundingBoxObject = JsonCaptionObject->GetObjectField("boundingBox");
		Caption.Position = FVector2D(JsonBoundingBoxObject->GetNumberField("x"),
		                             JsonBoundingBoxObject->GetNumberField("y"));
		Caption.Size = FVector2D(JsonBoundingBoxObject->GetNumberField("w"),
		                         JsonBoundingBoxObject->GetNumberField("h"));

		DenseCaptions.Add(Caption);
	}

	DenseCaptions.Sort();
	DenseCaptions = DenseCaptions.FilterByPredicate([](const FDenseCaption& Caption)
	{
		return Caption.Confidence > 0.75;
	});

	FString FullCaptionText;
	uint8 Flags = 0;

	for (int i = 0; i < DenseCaptions.Num(); ++i)
	{
		const auto& [Text, Confidence, Position, Size] = DenseCaptions[i];

		// If all flags are set, break
		if (!(Flags ^ 0b11111))
		{
			break;
		}

		if (DenseCaptions.Num() == 0)
		{
			FullCaptionText = FullCaptionText.Append("I'm sorry, I could not find anything in the image.");
			continue;
		}

		// top left
		if (!CHECK_BIT(Flags, 0) &&
			Position.Component(0) + Size.Component(0) / 2 < ImageWidth / 2 &&
			Position.Component(1) + Size.Component(1) / 2 < ImageHeight / 2)
		{
			Flags |= 0b00001;
			FullCaptionText = FullCaptionText.Append(Text).Append(" on the top left corner. ");
		}
		// top right
		else if (!CHECK_BIT(Flags, 1) &&
			Position.Component(0) + Size.Component(0) / 2 >= ImageWidth / 2 &&
			Position.Component(1) + Size.Component(1) < ImageHeight / 2)
		{
			Flags |= 0b00010;
			FullCaptionText = FullCaptionText.Append(Text).Append(" on the top right corner. ");
		}
		// bottom left
		else if (!CHECK_BIT(Flags, 2) &&
			Position.Component(0) + Size.Component(0) / 2 < ImageWidth / 2 &&
			Position.Component(1) + Size.Component(1) >= ImageHeight / 2)
		{
			Flags |= 0b00100;
			FullCaptionText = FullCaptionText.Append(Text).Append(" on the bottom left corner. ");
		}
		// bottom right
		else if (!CHECK_BIT(Flags, 3) &&
			Position.Component(0) + Size.Component(0) / 2 >= ImageWidth / 2 &&
			Position.Component(1) + Size.Component(1) >= ImageHeight / 2)
		{
			Flags |= 0b01000;
			FullCaptionText = FullCaptionText.Append(Text).Append(" on the bottom right corner. ");
		}
		// center
		else if (!CHECK_BIT(Flags, 4) &&
			Position.Component(0) + Size.Component(0) / 2 >= ImageWidth / 4 &&
			Position.Component(0) + Size.Component(0) / 2 <= 3 * ImageWidth / 4 &&
			Position.Component(1) + Size.Component(1) / 2 >= ImageHeight / 4 &&
			Position.Component(1) + Size.Component(1) / 2 <= 3 * ImageHeight / 4)
		{
			Flags |= 0b10000;
			FullCaptionText = FullCaptionText.Append(Text).Append(" in the center. ");
		}
	}

	auto _ = OnSuccess.ExecuteIfBound(FullCaptionText);
}
