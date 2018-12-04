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

map<wstring, UnityDefaultFontResource> UnityDefaultFontResources;

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

FontAssetMaps _cdecl GetPluginSupportAssetMap() {
	for (vector<string>::iterator assetsNameItr = UnityL10nToolAPIGlobal->AssetsFileNames->begin(); assetsNameItr != UnityL10nToolAPIGlobal->AssetsFileNames->end(); assetsNameItr++) {
		size_t sharedFound = assetsNameItr->find("sharedassets");
		size_t resourcesFound = assetsNameItr->find("resources");
		if (sharedFound != string::npos || resourcesFound != string::npos) {
			string assetsName = *assetsNameItr;
			
		}
	}
} 