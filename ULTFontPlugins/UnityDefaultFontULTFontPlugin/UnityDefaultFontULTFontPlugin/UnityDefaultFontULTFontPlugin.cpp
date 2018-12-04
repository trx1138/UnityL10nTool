// UnityDefaultFontULTFontPlugin.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include <string>
#include <set>
using namespace std;

#include "AssetsTools/AssetsFileReader.h"
#include "AssetsTools/AssetsFileFormat.h"
#include "AssetsTools/ClassDatabaseFile.h"
#include "AssetsTools/AssetsFileTable.h"
#include "AssetsTools/ResourceManagerFile.h"
#include "AssetsTools/AssetTypeClass.h"
#include "IULTPluginCommonInterface.h"
#include "IULTFontPluginInterface.h"
#include "GeneralPurposeFunctions.h"
#include "json/json.h"

UnityL10nToolAPI* UnityL10nToolAPIGlobal;
FontPluginInfo* FontPluginInfoGlobal;

FontAssetMaps fontAssetMapsGlobal;
vector<AssetMapOption> OptionsList;
Json::Value OptionsJson;
Json::Value ProjectConfig;

class UnityDefaultFontResource {
public:
	enum class ResourceTypeEnum {
		NONE,
		TTF,
		JSON,
		BITMAP
	};
	wstring FileName;
	wstring FontFamilyName;
	ResourceTypeEnum ResourceType;
	UnityDefaultFontResource() {}
	UnityDefaultFontResource(wstring fileName) {
		this->FileName = fileName;
		if (this->FileName.find(L".ttf") != wstring::npos) {
			size_t ttfFound = this->FileName.find(L".ttf");
			this->FontFamilyName = this->FileName.substr(0, ttfFound - 1);
			this->ResourceType = ResourceTypeEnum::TTF;
		}
		else if (this->FileName.find(L".json") != wstring::npos) {
			size_t jsonFound = this->FileName.find(L".json");
			this->FontFamilyName = this->FileName.substr(0, jsonFound - 1);
			this->ResourceType = ResourceTypeEnum::JSON;
		}
		else if (this->FileName.find(L".fnt") != wstring::npos) {
			size_t fntFound = this->FileName.find(L".fnt");
			this->FontFamilyName = this->FileName.substr(0, fntFound - 1);
			this->ResourceType = ResourceTypeEnum::BITMAP;
		}
		else {
			this->ResourceType = ResourceTypeEnum::NONE;
		}
	}
};



vector<FontAssetMap> GetFontAssetMapListFromUnityDefaultFont(UnityL10nToolAPI* unityL10nToolAPI, string assetsName) {
	vector<FontAssetMap> fontAssetMapList;
	const string FontClassName = "Font";
	map<string, UINT32>::const_iterator FontClassIndexItr = unityL10nToolAPI->FindBasicClassIndexFromClassName->find(FontClassName);
	if (FontClassIndexItr != unityL10nToolAPI->FindBasicClassIndexFromClassName->end()) {
		UINT32 FontClassIndex = FontClassIndexItr->second;
		ClassDatabaseType FontAssetCDT = unityL10nToolAPI->BasicClassDatabaseFile->classes[FontClassIndex];
		const int FontAssetClassId = FontAssetCDT.classId;

		map<string, AssetsFileTable*>::const_iterator assetsFileTableIterator = unityL10nToolAPI->FindAssetsFileTablesFromAssetsName->find(assetsName);
		if (assetsFileTableIterator != unityL10nToolAPI->FindAssetsFileTablesFromAssetsName->end()) {
			AssetsFileTable* assetsFileTable = assetsFileTableIterator->second;
			AssetsFile* assetsFile = assetsFileTable->getAssetsFile();
			for (unsigned int i = 0; i < assetsFileTable->assetFileInfoCount; i++) {
				AssetFileInfoEx* assetFileInfoEx = &assetsFileTable->pAssetFileInfo[i];
				int classId = 0;
				UINT16 monoClassId = 0;
				unityL10nToolAPI->GetClassIdFromAssetFileInfoEx(assetsFileTable, assetFileInfoEx, classId, monoClassId);
				if (classId == FontAssetClassId) {
					string assetName = unityL10nToolAPI->GetAssetNameA(assetsFile, assetFileInfoEx);
					INT64 PathId = assetFileInfoEx->index;
					string containerPath;
					map<string, INT32>::const_iterator FileIDIterator = unityL10nToolAPI->FindPathIDOfContainerPathFromAssetsName->find(assetsName);
					if (FileIDIterator != unityL10nToolAPI->FindPathIDOfContainerPathFromAssetsName->end()) {
						map<pair<INT32, INT64>, string>::const_iterator containerPathItr = unityL10nToolAPI->FindContainerPathFromFileIDPathID->find(pair<INT32, INT64>(FileIDIterator->second, PathId));
						if (containerPathItr != unityL10nToolAPI->FindContainerPathFromFileIDPathID->end()) {
							containerPath = containerPathItr->second;
						}
					}
					FontAssetMap fontAssetMap = FontAssetMap(
						L"",
						assetsName,
						assetName,
						containerPath,
						false,
						std::vector<AssetMapOption>()
					);
					fontAssetMap._m_PathID = assetFileInfoEx->index;
					fontAssetMapList.push_back(fontAssetMap);
				}
			}
		}
	}
	return fontAssetMapList;
}

map<wstring, UnityDefaultFontResource> UnityDefaultFontResources;

FontAssetMaps _cdecl GetPluginSupportAssetMap() {
	for (vector<string>::iterator assetsNameItr = UnityL10nToolAPIGlobal->AssetsFileNames->begin(); assetsNameItr != UnityL10nToolAPIGlobal->AssetsFileNames->end(); assetsNameItr++) {
		size_t sharedFound = assetsNameItr->find("sharedassets");
		size_t resourcesFound = assetsNameItr->find("resources");
		if (sharedFound != string::npos || resourcesFound != string::npos) {
			string assetsName = *assetsNameItr;
			vector<FontAssetMap> foundFontAssetMapList = GetFontAssetMapListFromUnityDefaultFont(UnityL10nToolAPIGlobal, assetsName);
			for (vector<FontAssetMap>::iterator FAMItr = foundFontAssetMapList.begin(); FAMItr != foundFontAssetMapList.end(); FAMItr++) {
				FAMItr->options = OptionsList;
				fontAssetMapsGlobal.News.push_back(*FAMItr);
			}
		}
	}
	if (ProjectConfig.isMember("Saveds")) {
		Json::Value savedJsonArray = ProjectConfig["Saveds"];
		if (savedJsonArray.isArray()) {
			for (Json::ArrayIndex i = 0; i < savedJsonArray.size(); i++) {
				FontAssetMap savedFAM((Json::Value)savedJsonArray[i]);
				for (vector<FontAssetMap>::iterator FAMItr = fontAssetMapsGlobal.News.begin(); FAMItr != fontAssetMapsGlobal.News.end(); FAMItr++) {
					if (FAMItr->LooseEquals(savedFAM)) {
						wstring savedFontFamilyName = *(wstring*)savedFAM.options[0].Value;
						if (UnityDefaultFontResources.find(savedFontFamilyName) != UnityDefaultFontResources.end()) {
							FAMItr->options[0].Value = new wstring(savedFontFamilyName);
							FAMItr->useContainerPath = savedFAM.useContainerPath;
							fontAssetMapsGlobal.Saveds.push_back(*FAMItr);
						}
						break;
					}
				}
			}
		}
	}
	return fontAssetMapsGlobal;
}

Json::Value _cdecl GetProjectConfigJson() {
	Json::Value exportJson;

	Json::Value SavedsJsonArray(Json::arrayValue);
	for (vector<FontAssetMap>::iterator FAMItr = fontAssetMapsGlobal.Saveds.begin(); FAMItr != fontAssetMapsGlobal.Saveds.end(); FAMItr++) {
		SavedsJsonArray.append(FAMItr->ToJson());
	}
	exportJson["Saveds"] = SavedsJsonArray;
	return exportJson;
}

Json::Value _cdecl GetPatcherConfigJson() {
	return GetProjectConfigJson();
}

Json::Value PatcherConfigGlobal;
bool _cdecl SetPatcherConfigJson(Json::Value patcherConfig) {
	PatcherConfigGlobal = patcherConfig;
	return true;
}

bool _cdecl SetPluginSupportAssetMap(FontAssetMaps supportAssetMaps) {
	fontAssetMapsGlobal = supportAssetMaps;
	return true;
}

bool _cdecl CopyBuildFileToBuildFolder(wstring FontPluginRelativePath, wstring targetPath) {
	set<wstring> usedFontResources;
	for (vector<FontAssetMap>::iterator FAMItr = fontAssetMapsGlobal.Saveds.begin(); FAMItr != fontAssetMapsGlobal.Saveds.end(); FAMItr++) {
		wstring usedFontFamilyName = *(wstring*)FAMItr->options[0].Value;
		if (UnityDefaultFontResources.find(usedFontFamilyName) != UnityDefaultFontResources.end()) {
			usedFontResources.insert(usedFontFamilyName);
		}
	}
	if (usedFontResources.size() != 0) {
		CreateDirectoryW((targetPath + L"UnityDefaultFont\\").c_str(), NULL);
		for (set<wstring>::iterator usedFontResourceItr = usedFontResources.begin(); usedFontResourceItr != usedFontResources.end(); usedFontResourceItr++) {
			map<wstring, UnityDefaultFontResource>::iterator UnityDefaultFontResourceItr = UnityDefaultFontResources.find(*usedFontResourceItr);
			if (UnityDefaultFontResourceItr != UnityDefaultFontResources.end()) {
				UnityDefaultFontResource nUnityDefaultFontResource = UnityDefaultFontResourceItr->second;
				switch (nUnityDefaultFontResource.ResourceType) {
				case UnityDefaultFontResource::ResourceTypeEnum::TTF:
					CopyFileW((FontPluginRelativePath + L"UnityDefaultFont\\" + nUnityDefaultFontResource.FileName).c_str(), (targetPath + L"UnityDefaultFont\\" + nUnityDefaultFontResource.FileName).c_str(), false);
					break;
				case UnityDefaultFontResource::ResourceTypeEnum::JSON:
				case UnityDefaultFontResource::ResourceTypeEnum::BITMAP:
				case UnityDefaultFontResource::ResourceTypeEnum::NONE:
					break;
				}
			}
		}
	}
	return true;
}

set<wstring> usedFontResourcesInPatcher;
map<string, vector<AssetsReplacer*>> _cdecl GetPatcherAssetReplacer() {
	map<string, vector<AssetsReplacer*>> replacers;
	if (PatcherConfigGlobal.isMember("Saveds")) {
		Json::Value savedFontArrayJson = PatcherConfigGlobal["Saveds"];
		for (Json::ArrayIndex i = 0; i < savedFontArrayJson.size(); i++) {
			FontAssetMap fontAssetMap = FontAssetMap((Json::Value)savedFontArrayJson[i]);
			wstring usedFontFamilyName = *(wstring*)fontAssetMap.options[0].Value;
			map<wstring, UnityDefaultFontResource>::iterator foundUnityDefaultFont = UnityDefaultFontResources.find(usedFontFamilyName);
			if (foundUnityDefaultFont != UnityDefaultFontResources.end()) {
				UnityDefaultFontResource unityDefaultFontResource = foundUnityDefaultFont->second;
				string assetsName;
				AssetsFileTable* assetsFileTable = NULL;
				AssetFileInfoEx* fontAFIEx = NULL;
				AssetTypeInstance* fontATI = NULL;
				AssetTypeValueField* fontBaseATVF = NULL;
				INT64 PathID = 0;

				if (fontAssetMap.useContainerPath && !fontAssetMap.containerPath.empty()) {
					map<string, pair<INT32, INT64>>::const_iterator FileIDPathIDiterator = UnityL10nToolAPIGlobal->FindFileIDPathIDFromContainerPath->find(fontAssetMap.containerPath);
					if (FileIDPathIDiterator != UnityL10nToolAPIGlobal->FindFileIDPathIDFromContainerPath->end()) {
						INT32 FileIDOfContainerPath = FileIDPathIDiterator->second.first;
						PathID = FileIDPathIDiterator->second.second;
						map<INT32, string>::const_iterator assetsNameIterator = UnityL10nToolAPIGlobal->FindAssetsNameFromPathIDOfContainerPath->find(FileIDOfContainerPath);
						if (assetsNameIterator != UnityL10nToolAPIGlobal->FindAssetsNameFromPathIDOfContainerPath->end()) {
							assetsName = assetsNameIterator->second;
							map<string, AssetsFileTable*>::const_iterator assetsFileTableIterator = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(assetsName);
							if (assetsFileTableIterator != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
								assetsFileTable = assetsFileTableIterator->second;
								fontAFIEx = assetsFileTable->getAssetInfo(PathID);
							}
							else {
								break;
							}
						}
						else {
							break;
						}
					}
					else {
						break;
					}
				}
				else if (!fontAssetMap.assetName.empty() && !fontAssetMap.assetsName.empty()) {
					assetsName = fontAssetMap.assetsName;
					map<string, AssetsFileTable*>::const_iterator assetsFileTableIterator = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(assetsName);
					if (assetsFileTableIterator != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
						assetsFileTable = assetsFileTableIterator->second;
						PathID = UnityL10nToolAPIGlobal->FindAssetIndexFromName(assetsFileTable, fontAssetMap.assetName);
						if (PathID == -1) {
							break;
						}
						fontAFIEx = assetsFileTable->getAssetInfo(PathID);
					}
					else {
						break;
					}
				}
				else {
					break;
				}
				

				switch (unityDefaultFontResource.ResourceType)
				{
				case UnityDefaultFontResource::ResourceTypeEnum::TTF:

					

					break;
				default:
					break;
				}
			}
		}
	}
}