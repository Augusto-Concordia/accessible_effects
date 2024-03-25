// Fill out your copyright notice in the Description page of Project Settings.


#include "FDenseCaptions.h"

bool FDenseCaption::operator<(const FDenseCaption& Other) const
{
	return Confidence < Other.Confidence;
}
