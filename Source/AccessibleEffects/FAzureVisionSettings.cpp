#include "FAzureVisionSettings.h"

TMap<FString, FString> UFAzureVisionSettingsHelpers::GetParametersMap(const FAzureVisionSettings& Settings)
{
	TMap<FString, FString> Parameters;

	Parameters.Add("api-version", Settings.ApiVersion);
	Parameters.Add("features", Settings.Features);
	Parameters.Add("model-name", Settings.ModelName);
	Parameters.Add("language", Settings.Language);
	Parameters.Add("smart-crops-aspect-ratio", Settings.SmartCropsAspectRatio);
	Parameters.Add("gender-neutral-caption", Settings.GenderNeutralCaption ? "true" : "false");

	return Parameters;
}
