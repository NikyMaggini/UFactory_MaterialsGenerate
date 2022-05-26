// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExCreateMaterialsPlugin.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AutomatedAssetImportData.h"
#include "Factories/MaterialFactoryNew.h"
#include "HAL/FileManagerGeneric.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Misc/FileHelper.h"



#define LOCTEXT_NAMESPACE "FExCreateMaterialsPluginModule"

void FExCreateMaterialsPluginModule::StartupModule()
{
}

void FExCreateMaterialsPluginModule::ShutdownModule()
{
}

bool FExCreateMaterialsPluginModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	const FString& TexturePath = FString("C:/Users/User/Desktop/Immagini");	// path textures
	TArray<FString > ListsTextures = TArray<FString>();
	FFileManagerGeneric FileManager = FFileManagerGeneric();
	FileManager.FindFilesRecursive(ListsTextures, *TexturePath, TEXT("*.png"), true, true);
	TArray<UObject*> Textures = ImportAssets(ListsTextures);

	if (FParse::Command(&Cmd, TEXT("creatematerial")))	// cmd
	{
		int32 IndexForName = 0;
		int32 Index = 0;

		TArray<FString> Lines;
		FFileHelper::LoadFileToStringArray(Lines, Cmd);

		UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

		for (const FString& Line : Lines)
		{
			TArray<FString> ContentImportedFile;
			Line.ParseIntoArray(ContentImportedFile, TEXT(" "));

			FString MaterialName = "Mat_";
			MaterialName.AppendInt(IndexForName);

			UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Game/%s"), *MaterialName));

			UObject* MaterialObj = MaterialFactory->FactoryCreateNew(MaterialFactory->SupportedClass, Package, *MaterialName,
				EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, nullptr, GWarn);

			FAssetRegistryModule::AssetCreated(MaterialObj);

			UMaterial* Material = Cast<UMaterial>(MaterialObj);
			Material->Modify();

			UMaterialExpressionVectorParameter* VectorParam = NewObject<UMaterialExpressionVectorParameter>(Material);

			if (ContentImportedFile[Index] == FString("Mat"))
			{
				++Index;
				if (ContentImportedFile[Index] == FString("Color"))
				{

					++Index;
					float ColorR = FCString::Atof(*ContentImportedFile[Index]);

					++Index;
					float ColorG = FCString::Atof(*ContentImportedFile[Index]);

					++Index;
					float ColorB = FCString::Atof(*ContentImportedFile[Index]);

					++Index;

					VectorParam->DefaultValue = FLinearColor(ColorR, ColorG, ColorB);
					VectorParam->ParameterName = "Color";
					Material->BaseColor.Expression = VectorParam;
					Material->Expressions.Add(VectorParam);
				}


				if (ContentImportedFile.Num() > Index && ContentImportedFile[Index] == FString("Texture"))
				{
					++Index;
					UMaterialExpressionTextureSample* TextureSamp = NewObject<UMaterialExpressionTextureSample>(Material);
					UMaterialExpressionAdd* AddNode = NewObject<UMaterialExpressionAdd>(Material);

					int32 IndexForTexture = FCString::Atoi(*ContentImportedFile[Index]);
					TextureSamp->Texture = Cast<UTexture>(Textures[IndexForTexture]);
					AddNode->A.Expression = TextureSamp;
					AddNode->B.Expression = VectorParam;
					Material->BaseColor.Expression = AddNode;
					Material->Expressions.Add(TextureSamp);
					Material->Expressions.Add(AddNode);
					++Index;
				}

				if (ContentImportedFile.Num() > Index && ContentImportedFile[Index] == FString("Emissive"))
				{
					UMaterialExpressionVectorParameter* EmissiveColor = NewObject<UMaterialExpressionVectorParameter>(Material);

					++Index;
					float EmissiveR = FCString::Atof(*ContentImportedFile[Index]);

					++Index;
					float EmissiveG = FCString::Atof(*ContentImportedFile[Index]);

					++Index;
					float EmissiveB = FCString::Atof(*ContentImportedFile[Index]);

					++Index;

					EmissiveColor->DefaultValue = FLinearColor(EmissiveR, EmissiveG, EmissiveB);
					EmissiveColor->ParameterName = "EmissiveColor";

					Material->EmissiveColor.Expression = EmissiveColor;
					Material->Expressions.Add(EmissiveColor);
				}

				if (ContentImportedFile.Num() > Index && ContentImportedFile[Index] == FString("Normal"))
				{
					++Index;
					UMaterialExpressionTextureSample* Normal = NewObject<UMaterialExpressionTextureSample>(Material);

					int32 IndexForTexture = FCString::Atoi(*ContentImportedFile[Index]);
					Normal->Texture = Cast<UTexture>(Textures[IndexForTexture]);
					Material->Normal.Expression = Normal;
					Material->Expressions.Add(Normal);
					++Index;
				}

				Index = 0;
				++IndexForName;

				Material->PostEditChange();
				Material->MarkPackageDirty();
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Material);
			}
		}
		return true;
	}
	return false;
}

TArray<UObject*> FExCreateMaterialsPluginModule::ImportAssets(const TArray<FString>& Files)
{
	UAutomatedAssetImportData* Texture = NewObject<UAutomatedAssetImportData>();
	Texture->bReplaceExisting = true;
	Texture->DestinationPath = TEXT("/Game/Textures");
	Texture->Filenames = Files;

	FAssetToolsModule& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	TArray<UObject*>ImportedAssets = AssetTools.Get().ImportAssetsAutomated(Texture);

	for (UObject* Asset : ImportedAssets)
	{
		UPackage* Package = Asset->GetPackage();
		FString FileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(Package, Asset, RF_Public | RF_Standalone, *FileName);
		FAssetRegistryModule::AssetCreated(Asset);
	}
	return ImportedAssets;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FExCreateMaterialsPluginModule, ExCreateMaterialsPlugin)