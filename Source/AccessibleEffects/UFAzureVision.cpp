// Fill out your copyright notice in the Description page of Project Settings.


#include "UFAzureVision.h"

#include "FDenseCaptions.h"
#include "Http.h"
#include "UFUtilities.h"


void UFAzureVision::GetImageDenseCaptions(const FAzureVisionSettings& Settings,
                                         const TArray<uint8>& ImageData,
                                         const FOnSuccess OnSuccess,
                                         const FOnFailure OnFailure)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = HttpModule.CreateRequest();

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

			ProcessAzureVisionContent(Response->GetContentAsString(), &OnSuccess, &OnFailure);
		});

	// Submit the request
	Request->ProcessRequest();
}

void UFAzureVision::ProcessAzureVisionContent(const FString& Content,
                                              const FOnSuccess* OnSuccess,
                                              const FOnFailure* OnFailure)
{
	UE_LOG(LogTemp, Display, TEXT("Received: %s"), *Content);

	TArray<FDenseCaption> DenseCaptions;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonObject;

	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		auto _ = OnFailure->ExecuteIfBound(TEXT("Failed to parse JSON response"));
		return;
	}

	const TSharedPtr<FJsonObject>* DenseCaptionsResult;

	if (!JsonObject->TryGetObjectField(TEXT("denseCaptionsResult"), DenseCaptionsResult))
	{
		auto _ = OnFailure->ExecuteIfBound(TEXT("Failed to find denseCaptionsResult in JSON response"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* Values;

	if (!(*DenseCaptionsResult)->TryGetArrayField(TEXT("values"), Values))
	{
		auto _ = OnFailure->ExecuteIfBound(TEXT("Failed to find values in denseCaptionsResult"));
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Found %d dense captions in response"), Values->Num());

	for (const auto &JsonCaption : *Values)
	{
		const TSharedPtr<FJsonObject> JsonCaptionObject = JsonCaption->AsObject();

		FDenseCaption Caption;
		Caption.Text = JsonCaptionObject->GetStringField("text");
		Caption.Confidence = JsonCaptionObject->GetNumberField("confidence");

		const TSharedPtr<FJsonObject> JsonBoundingBoxObject = JsonCaptionObject->GetObjectField("boundingBox");
		Caption.Position = FVector2D(JsonBoundingBoxObject->GetNumberField("x"), JsonBoundingBoxObject->GetNumberField("y"));
		Caption.Size = FVector2D(JsonBoundingBoxObject->GetNumberField("w"), JsonBoundingBoxObject->GetNumberField("h"));

		DenseCaptions.Add(Caption);
	}

	DenseCaptions.Sort();

	//auto _ = OnSuccess.ExecuteIfBound(DenseCaptions);
}
