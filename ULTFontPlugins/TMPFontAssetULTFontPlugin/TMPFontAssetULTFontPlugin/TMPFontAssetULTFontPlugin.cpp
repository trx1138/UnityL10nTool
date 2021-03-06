// TMPFontAssetULTFontPlugin.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"

#include <string>
#include <algorithm>
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
#include "TMPFontResource.h"
#include "TMPFontAssetULTFontPlugin.h"

UnityL10nToolAPI* UnityL10nToolAPIGlobal;
FontPluginInfo* FontPluginInfoGlobal;

FontAssetMaps fontAssetMapsGlobal;
vector<AssetMapOption> OptionsList;
Json::Value OptionsJson;
Json::Value ProjectConfig;

map<wstring, TMPFontResource> TMPFontResources;

bool _cdecl SetProjectConfigJson(Json::Value pluginConfig) {
	ProjectConfig = pluginConfig;
	return true;
}

FontAssetMaps _cdecl GetPluginSupportAssetMap() {
	vector<FontAssetMap> FontAssetMapListFromResourcesAssets = GetFontAssetMapListFromMonoClassName(UnityL10nToolAPIGlobal, "resources.assets", "TMPro.TMP_FontAsset");
	vector<FontAssetMap> FontAssetMapListFromShared0Assets = GetFontAssetMapListFromMonoClassName(UnityL10nToolAPIGlobal, "sharedassets0.assets", "TMPro.TMP_FontAsset");
	fontAssetMapsGlobal.News.insert(fontAssetMapsGlobal.News.end(), FontAssetMapListFromResourcesAssets.begin(), FontAssetMapListFromResourcesAssets.end());
	fontAssetMapsGlobal.News.insert(fontAssetMapsGlobal.News.end(), FontAssetMapListFromShared0Assets.begin(), FontAssetMapListFromShared0Assets.end());
	for (vector<FontAssetMap>::iterator iterator = fontAssetMapsGlobal.News.begin();
		iterator != fontAssetMapsGlobal.News.end(); iterator++) {
		iterator->options = OptionsList;
	}

	/* Load Saveds */
	if (ProjectConfig.isMember("Saveds")) {
		Json::Value selectedFontAssetMapListJson = ProjectConfig["Saveds"];
		if (selectedFontAssetMapListJson.isArray()) {
			for (Json::ArrayIndex i = 0; i < selectedFontAssetMapListJson.size(); i++) {
				Json::Value selectedFontAssetMapJson = selectedFontAssetMapListJson[i];
				FontAssetMap savedFAM(selectedFontAssetMapJson);
				for (vector<FontAssetMap>::iterator iterator = fontAssetMapsGlobal.News.begin();
					iterator != fontAssetMapsGlobal.News.end(); iterator++) {
					if (iterator->LooseEquals(savedFAM)) {
						wstring savedFontFamilyName = *(wstring*)savedFAM.options[0].Value;
						if (TMPFontResources.find(savedFontFamilyName) != TMPFontResources.end()) {
							iterator->options[0].Value = new wstring(savedFontFamilyName);
							fontAssetMapsGlobal.Saveds.push_back(*iterator);
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
	for (vector<FontAssetMap>::iterator iterator = fontAssetMapsGlobal.Saveds.begin();
		iterator != fontAssetMapsGlobal.Saveds.end();
		iterator++) {
		SavedsJsonArray.append(iterator->ToJson());
	}
	exportJson["Saveds"] = SavedsJsonArray;
	return exportJson;
}

Json::Value _cdecl GetPacherConfigJson() {
	return GetProjectConfigJson();
}

bool _cdecl CopyBuildFileToBuildFolder(wstring FontPluginRelativePath, wstring targetPath) {
	set<wstring> usedFontResources;
	for (vector<FontAssetMap>::iterator iterator = fontAssetMapsGlobal.Saveds.begin();
		iterator != fontAssetMapsGlobal.Saveds.end();
		iterator++) {
		wstring usedFontFamilyName = *(wstring*)iterator->options[0].Value;
		if (TMPFontResources.find(usedFontFamilyName) != TMPFontResources.end()) {
			usedFontResources.insert(usedFontFamilyName);
		}
	}
	CreateDirectoryW((targetPath + L"TMPFontAsset\\").c_str(), NULL);
	for (set<wstring>::iterator iterator = usedFontResources.begin();
		iterator != usedFontResources.end();
		iterator++) {
		map<wstring, TMPFontResource>::iterator TMPFontResourceItr = TMPFontResources.find(*iterator);
		if (TMPFontResourceItr != TMPFontResources.end()) {
			TMPFontResource tmpFontResource = TMPFontResourceItr->second;
			CopyFileW((FontPluginRelativePath + L"TMPFontAsset\\" + tmpFontResource.FileName).c_str(), (targetPath + L"TMPFontAsset\\" + tmpFontResource.FileName).c_str(), false);
			CopyFileW((FontPluginRelativePath + L"TMPFontAsset\\" + tmpFontResource.GetMonoFileName()).c_str(), (targetPath + L"TMPFontAsset\\" + tmpFontResource.GetMonoFileName()).c_str(), false);
			CopyFileW((FontPluginRelativePath + L"TMPFontAsset\\" + tmpFontResource.GetResSFileName()).c_str(), (targetPath + L"TMPFontAsset\\" + tmpFontResource.GetResSFileName()).c_str(), false);
		}
	}
	return true;
}

Json::Value PatcherConfigGlobal;
bool _cdecl SetPacherConfigJson(Json::Value patcherConfig) {
	PatcherConfigGlobal = patcherConfig;
	return true;
}

AssetsReplacer* ReplaceMaterial(string assetsName, AssetsFileTable* assetsFileTable, AssetFileInfoEx* assetFileInfoEx, AssetTypeInstance* assetTypeInstance, float _TextureHeight, float _TextureWidth, INT64 targetPathID, INT64 atlasPathID, INT32 atlasFileID = -1) {
	AssetTypeValueField* m_FloatsArrayATVF = assetTypeInstance->GetBaseField()->Get("m_SavedProperties")->Get("m_Floats")->Get("Array");
	AssetTypeValueField** m_FloatsATVFChildrenArray = m_FloatsArrayATVF->GetChildrenList();
	int modifyCounter = 2;
	for (unsigned int i = 0; i < m_FloatsArrayATVF->GetChildrenCount() && modifyCounter>0; i++) {
		string first = m_FloatsATVFChildrenArray[i]->Get("first")->GetValue()->AsString();
		if (first == "_TextureHeight") {
			m_FloatsATVFChildrenArray[i]->Get("second")->GetValue()->Set(new float(_TextureHeight));
			modifyCounter--;
		}
		else if (first == "_TextureWidth") {
			m_FloatsATVFChildrenArray[i]->Get("second")->GetValue()->Set(new float(_TextureWidth));
			modifyCounter--;
		}
	}

	AssetTypeValueField* m_TexEnvsATVF = assetTypeInstance->GetBaseField()->Get("m_SavedProperties")->Get("m_TexEnvs")->Get("Array");
	AssetTypeValueField** m_TextEnvsATVFChildrenArray = m_TexEnvsATVF->GetChildrenList();
	for (unsigned int i = 0; i < m_TexEnvsATVF->GetChildrenCount(); ++i) {
		string first = m_TextEnvsATVFChildrenArray[i]->Get("first")->GetValue()->AsString();
		if (first == "_MainTex") {
			if (atlasFileID != -1) {
				m_TextEnvsATVFChildrenArray[i]->Get("second")->Get("m_Texture")->Get("m_FileID")->GetValue()->Set(new INT32(atlasFileID));
			}
			m_TextEnvsATVFChildrenArray[i]->Get("second")->Get("m_Texture")->Get("m_PathID")->GetValue()->Set(new INT64(atlasPathID));
			break;
		}
	}
	return UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, assetFileInfoEx, assetTypeInstance, targetPathID);
}

AssetsReplacer* ReplaceAtlas(string assetsname, AssetsFileTable* assetsFileTable, AssetFileInfoEx* assetFileInfoEx, AssetTypeInstance* assetTypeInstance, int m_CompleteImageSize, string atlasPath, int m_Width, int m_Height, INT64 targetPathID) {
	AssetTypeValueField* assetTypeValueField = assetTypeInstance->GetBaseField();
	assetTypeInstance->GetBaseField()->Get("m_Width")->GetValue()->Set(new INT32(m_Width));
	assetTypeInstance->GetBaseField()->Get("m_Height")->GetValue()->Set(new INT32(m_Height));
	assetTypeInstance->GetBaseField()->Get("m_CompleteImageSize")->GetValue()->Set(new INT32(m_CompleteImageSize));
	assetTypeInstance->GetBaseField()->Get("m_StreamData")->Get("offset")->GetValue()->Set(new UINT32(0));
	assetTypeInstance->GetBaseField()->Get("m_StreamData")->Get("size")->GetValue()->Set(new UINT32(m_CompleteImageSize));
	assetTypeInstance->GetBaseField()->Get("m_StreamData")->Get("path")->GetValue()->Set((void*)(new string("TMPFontAsset\\"+atlasPath))->c_str());
	return UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, assetFileInfoEx, assetTypeInstance, targetPathID);
}

AssetsReplacer* ReplaceOriginalMono(AssetTypeValueField* monoOrigBaseATVF, const INT64& newMonoPathID, AssetsFileTable* assetsFileTable, AssetFileInfoEx* MonoAFIEx, AssetTypeInstance* MonoOrigATI)
{
	if (monoOrigBaseATVF->Get("fallbackFontAssets") != NULL && monoOrigBaseATVF->Get("fallbackFontAssets")->IsDummy() == false) {
		ReplaceOriginalMonoInternal(monoOrigBaseATVF, newMonoPathID, "fallbackFontAssets");
	}
	if (monoOrigBaseATVF->Get("m_FallbackFontAssetTable") != NULL && monoOrigBaseATVF->Get("m_FallbackFontAssetTable")->IsDummy() == false) {
		ReplaceOriginalMonoInternal(monoOrigBaseATVF, newMonoPathID, "m_FallbackFontAssetTable");
	}
	return UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, MonoAFIEx, MonoOrigATI);
}

void ReplaceOriginalMonoInternal(AssetTypeValueField* monoOrigBaseATVF, const INT64& newMonoPathID, const char* fallbackFontAssetFieldName)
{
	AssetTypeTemplateField* fallbackFontAssetArrayATTF = monoOrigBaseATVF->Get(fallbackFontAssetFieldName)->GetTemplateField();
	AssetTypeTemplateField* fallbackFontAssetChildATTF = &(fallbackFontAssetArrayATTF->children[1]);
	AssetTypeTemplateField* fallbackFontAssetChildFileIDATTF = &fallbackFontAssetChildATTF->children[0];
	AssetTypeTemplateField* fallbackFontAssetChildPathIDATTF = &fallbackFontAssetChildATTF->children[1];
	AssetTypeValueField* fallbackFontAssetChildFileIDATVF = new AssetTypeValueField();
	fallbackFontAssetChildFileIDATVF->Read(new AssetTypeValue(fallbackFontAssetChildFileIDATTF->valueType, new INT32(0)), fallbackFontAssetChildFileIDATTF, 0, 0);
	AssetTypeValueField* fallbackFontAssetChildPathIDATVF = new AssetTypeValueField();
	fallbackFontAssetChildPathIDATVF->Read(new AssetTypeValue(fallbackFontAssetChildPathIDATTF->valueType, new INT64(newMonoPathID)), fallbackFontAssetChildPathIDATTF, 0, 0);
	AssetTypeValueField* fallbackFontAssetChildATVF = new AssetTypeValueField();
	std::vector<AssetTypeValueField*>* fallbackFontAssetChildArray = new std::vector<AssetTypeValueField*>();
	fallbackFontAssetChildArray->push_back(fallbackFontAssetChildFileIDATVF);
	fallbackFontAssetChildArray->push_back(fallbackFontAssetChildPathIDATVF);
	fallbackFontAssetChildATVF->Read(new AssetTypeValue(ValueType_None, 0), fallbackFontAssetChildATTF, 2, fallbackFontAssetChildArray->data());
	std::vector<AssetTypeValueField*>* fallbackFontAssetArray = new std::vector<AssetTypeValueField*>();
	fallbackFontAssetArray->push_back(fallbackFontAssetChildATVF);
	AssetTypeArray* assetTypeArray = new AssetTypeArray();
	assetTypeArray->size = fallbackFontAssetArray->size();
	monoOrigBaseATVF->Get(fallbackFontAssetFieldName)->Read(new AssetTypeValue(ValueType_Array, assetTypeArray), fallbackFontAssetArrayATTF, fallbackFontAssetArray->size(), fallbackFontAssetArray->data());
}

struct AddedTMPFontMono {
	string assetsName;
	INT64 PathID;
	INT64 atlasPathID;
};

//map<string, string> copyList;
set<wstring> usedFontResourcesInPatcher;
map<wstring, AddedTMPFontMono> AddedTMPFontMonos;
map<string, vector<AssetsReplacer*>> _cdecl GetPatcherAssetReplacer() {
	int materialClassId = 0;
	map<string, INT64> lastAssetPathIDs;
	auto foundClass = UnityL10nToolAPIGlobal->FindBasicClassIndexFromClassName->find("Material");
	if (foundClass != UnityL10nToolAPIGlobal->FindBasicClassIndexFromClassName->end()) {
		ClassDatabaseType matCDT = UnityL10nToolAPIGlobal->BasicClassDatabaseFile->classes[foundClass->second];
		materialClassId = matCDT.classId;
	}
	map<string, vector<AssetsReplacer*>> replacers;
	FontAssetMaps fontAssetMaps = GetPluginSupportAssetMap();
	if (PatcherConfigGlobal.isMember("Saveds")) {
		Json::Value savedFontArrayJson = PatcherConfigGlobal["Saveds"];
		for (Json::ArrayIndex i = 0; i < savedFontArrayJson.size(); i++) {
			FontAssetMap fontAssetMap = FontAssetMap((Json::Value)savedFontArrayJson[i]);
			for (vector<FontAssetMap>::iterator FAMItr = fontAssetMaps.News.begin();
				FAMItr != fontAssetMaps.News.end();
				FAMItr++) {
				if (FAMItr->LooseEquals(fontAssetMap)) {
					wstring fontFamilyName;
					if (fontAssetMap.options[0].Value != NULL) {
						try {
							fontFamilyName = *(wstring*)fontAssetMap.options[0].Value;
						}
						catch (exception ex) {

						}
					}
					bool bFixExternalMaterial = false;
					if (fontAssetMap.options[1].Value != NULL) {
						try {
							bFixExternalMaterial = *(bool*)fontAssetMap.options[1].Value;
						}
						catch (exception ex) {

						}
					}
					bool bOverwriteOrigianlFont = false;
					if (fontAssetMap.options[2].Value != NULL) {
						try {
							bOverwriteOrigianlFont = *(bool*)fontAssetMap.options[2].Value;
						}
						catch (exception ex) {

						}
					}
					map<wstring, TMPFontResource>::iterator TMPFontResourceItr = TMPFontResources.find(fontFamilyName);
					if (TMPFontResourceItr != TMPFontResources.end()) {
						TMPFontResource localTMPFontResource = TMPFontResourceItr->second;
						map<string, AssetsFileTable*>::const_iterator assetsFileTableItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find(fontAssetMap.assetsName);
						if (assetsFileTableItr != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
							AssetsFileTable* assetsFileTable = assetsFileTableItr->second;
							AssetFileInfoEx* MonoAFIEx = assetsFileTable->getAssetInfo(FAMItr->_m_PathID);
							AssetTypeInstance* MonoOrigATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, MonoAFIEx);
							AssetTypeInstance* MonoNewATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, MonoAFIEx);
							AssetTypeValueField* monoOrigBaseATVF = MonoOrigATI->GetBaseField();
							AssetTypeValueField* monoNewBaseATVF = MonoNewATI->GetBaseField();
							INT64 lastAssetPathID = 0;
							if (lastAssetPathIDs.find(fontAssetMap.assetsName) != lastAssetPathIDs.end()) {
								lastAssetPathID = lastAssetPathIDs[fontAssetMap.assetsName];
							}
							else {
								lastAssetPathID = assetsFileTable->pAssetFileInfo[assetsFileTable->assetFileInfoCount - 1].index;
							}
							INT64 newMaterialPathID = lastAssetPathID + 1;
							INT64 newAtlasPathID = lastAssetPathID + 2;
							INT64 newMonoPathID = lastAssetPathID + 3;
							if (monoOrigBaseATVF) {
								auto foundAddedTMPFontMono = AddedTMPFontMonos.find(WideMultiStringConverter->from_bytes(fontAssetMap.assetsName) + L"_" + fontFamilyName);
								if (foundAddedTMPFontMono != AddedTMPFontMonos.end() && !bOverwriteOrigianlFont) {
									newMonoPathID = foundAddedTMPFontMono->second.PathID;
									AssetsReplacer* monoOrigAssetsReplacer = ReplaceOriginalMono(monoOrigBaseATVF, newMonoPathID, assetsFileTable, MonoAFIEx, MonoOrigATI);
									replacers[FAMItr->assetsName].push_back(monoOrigAssetsReplacer);
									if (bFixExternalMaterial) {
										string origMonoName = monoOrigBaseATVF->Get("m_Name")->GetValue()->AsString();
										string origAssetsName = foundAddedTMPFontMono->second.assetsName;
										AssetsFileTable* resAssetsFileTable = NULL;
										map<string, AssetsFileTable*>::const_iterator resAssetsFileTableItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find("resources.assets");
										if (resAssetsFileTableItr != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
											resAssetsFileTable = resAssetsFileTableItr->second;
										}
										if (resAssetsFileTable != NULL) {
											for (int i = 0; i < resAssetsFileTable->assetFileInfoCount; i++) {
												AssetFileInfoEx* matAFIEx = &(resAssetsFileTable->pAssetFileInfo[i]);
												int classId = 0;
												UINT16 monoId = 0;
												UnityL10nToolAPI::GetClassIdFromAssetFileInfoEx(resAssetsFileTable, matAFIEx, classId, monoId);
												if (classId == materialClassId) {
													AssetTypeInstance* matATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(resAssetsFileTable, matAFIEx);
													AssetTypeValueField* matATVF = matATI->GetBaseField();
													if (matATVF) {
														AssetTypeValueField* m_NameMatATVF = matATVF->Get("m_Name");
														if (m_NameMatATVF && m_NameMatATVF->IsDummy() == false) {
															string matName = m_NameMatATVF->GetValue()->AsString();
															size_t found = matName.find(origMonoName);
															
															INT64 materialPathId = monoOrigBaseATVF->Get("material")->Get("m_PathID")->GetValue()->AsInt64();
											
												
												
															if (found != std::string::npos) {
																int fixResMatAtlasFileID = 0;
																if (origAssetsName == "resources.assets") {
																	if (matAFIEx->index == materialPathId) {
																		continue;
																	}
																}
																else {
																	for (int j = 0; j < resAssetsFileTable->getAssetsFile()->dependencies.dependencyCount; ++j) {
																		if (resAssetsFileTable->getAssetsFile()->dependencies.pDependencies[j].assetPath == origAssetsName) {
																			fixResMatAtlasFileID = j + 1;
																			break;
																		}
																	}
																}
																AssetsReplacer* fixResMatReplacer = ReplaceMaterial("resources.assets", resAssetsFileTable, matAFIEx, matATI, localTMPFontResource.m_Height, localTMPFontResource.m_Width, matAFIEx->index, foundAddedTMPFontMono->second.atlasPathID, fixResMatAtlasFileID);
																replacers["resources.assets"].push_back(fixResMatReplacer);
															}
														}
													}
												}
											}
										}
									}

								}
								else if (bOverwriteOrigianlFont) {

									string originalMonoName = monoOrigBaseATVF->Get("m_Name")->GetValue()->AsString();

									INT64 materialPathId = monoOrigBaseATVF->Get("material")->Get("m_PathID")->GetValue()->AsInt64();
									AssetFileInfoEx* materialAssetFileInfoEx = assetsFileTable->getAssetInfo(materialPathId);
									INT64 atlasPathId = 0;
									atlasPathId = monoOrigBaseATVF->Get("atlas")->Get("m_PathID")->GetValue()->AsInt64();
									if (atlasPathId == 0) {
										atlasPathId = monoOrigBaseATVF->Get("m_AtlasTextures")->Get((unsigned int)0)->Get("m_PathID")->GetValue()->AsInt64();
									}
									AssetTypeInstance* materialAssetTypeInstance = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, materialAssetFileInfoEx);
									AssetsReplacer* materialAssetsReplacer = ReplaceMaterial(FAMItr->assetsName, assetsFileTable, materialAssetFileInfoEx, materialAssetTypeInstance, localTMPFontResource.m_Height, localTMPFontResource.m_Width, materialPathId, atlasPathId);

									AssetFileInfoEx* atlasAssetFileInfoEx = assetsFileTable->getAssetInfo(atlasPathId);
									AssetTypeInstance* atlasAssetTypeInstance = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, atlasAssetFileInfoEx);
									string atlasName = WideMultiStringConverter->to_bytes(localTMPFontResource.GetResSFileName());
									AssetsReplacer* atlasAssetsReplacer = ReplaceAtlas(FAMItr->assetsName, assetsFileTable, atlasAssetFileInfoEx, atlasAssetTypeInstance, localTMPFontResource.m_CompleteImageSize, atlasName, localTMPFontResource.m_Width, localTMPFontResource.m_Height, atlasPathId);

									monoNewBaseATVF->Get("material")->Get("m_PathID")->GetValue()->Set(new INT64(materialPathId));
									monoNewBaseATVF->Get("atlas")->Get("m_PathID")->GetValue()->Set(new INT64(atlasPathId));

									if (monoNewBaseATVF->Get("m_AtlasTextures") != NULL && monoNewBaseATVF->Get("m_AtlasTextures")->IsDummy() == false) {
										AssetTypeTemplateField* m_AtlasTexturesArrayATTF = monoNewBaseATVF->Get("m_AtlasTextures")->GetTemplateField();
										AssetTypeTemplateField* m_AtlasTexturesChildATTF = &(m_AtlasTexturesArrayATTF->children[1]);
										AssetTypeTemplateField* m_AtlasTexturesChildFileIDATTF = &m_AtlasTexturesChildATTF->children[0];
										AssetTypeTemplateField* m_AtlasTexturesChildPathIDATTF = &m_AtlasTexturesChildATTF->children[1];
										AssetTypeValueField* m_AtlasTexturesChildFileIDATVF = new AssetTypeValueField();
										m_AtlasTexturesChildFileIDATVF->Read(new AssetTypeValue(m_AtlasTexturesChildFileIDATTF->valueType, new INT32(0)), m_AtlasTexturesChildFileIDATTF, 0, 0);
										AssetTypeValueField* m_AtlasTexturesChildPathIDATVF = new AssetTypeValueField();
										m_AtlasTexturesChildPathIDATVF->Read(new AssetTypeValue(m_AtlasTexturesChildPathIDATTF->valueType, new INT64(atlasPathId)), m_AtlasTexturesChildPathIDATTF, 0, 0);
										AssetTypeValueField* m_AtlasTexturesChildATVF = new AssetTypeValueField();
										std::vector<AssetTypeValueField*>* m_AtlasTexturesChildArray = new std::vector<AssetTypeValueField*>();
										m_AtlasTexturesChildArray->push_back(m_AtlasTexturesChildFileIDATVF);
										m_AtlasTexturesChildArray->push_back(m_AtlasTexturesChildPathIDATVF);
										m_AtlasTexturesChildATVF->Read(new AssetTypeValue(ValueType_None, 0), m_AtlasTexturesChildATTF, 2, m_AtlasTexturesChildArray->data());
										std::vector<AssetTypeValueField*>* m_AtlasTexturesArray = new std::vector<AssetTypeValueField*>();
										m_AtlasTexturesArray->push_back(m_AtlasTexturesChildATVF);
										AssetTypeArray* assetTypeArray = new AssetTypeArray();
										assetTypeArray->size = m_AtlasTexturesArray->size();
										monoNewBaseATVF->Get("m_AtlasTextures")->Read(new AssetTypeValue(ValueType_Array, assetTypeArray), m_AtlasTexturesArrayATTF, m_AtlasTexturesArray->size(), m_AtlasTexturesArray->data());
									}

									string monoJsonStr = readFile2(FontPluginInfoGlobal->relativePluginPath + L"TMPFontAsset\\" + localTMPFontResource.GetMonoFileName());
									Json::Value monoJson;
									JsonParseFromString(monoJsonStr, monoJson);
									string tes = monoJson[monoJson.getMemberNames()[0]]["1 string m_Name"].asString();
									monoJson[monoJson.getMemberNames()[0]]["1 string m_Name"] = originalMonoName;
									AssetsReplacer* monoNewAssetsReplacer = UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, MonoAFIEx, MonoNewATI, monoJson, FAMItr->_m_PathID);

									replacers[FAMItr->assetsName].push_back(materialAssetsReplacer);
									replacers[FAMItr->assetsName].push_back(atlasAssetsReplacer);
									replacers[FAMItr->assetsName].push_back(monoNewAssetsReplacer);
								}
								else {
									AssetsReplacer* monoOrigAssetsReplacer = ReplaceOriginalMono(monoOrigBaseATVF, newMonoPathID, assetsFileTable, MonoAFIEx, MonoOrigATI);

									INT64 materialPathId = monoOrigBaseATVF->Get("material")->Get("m_PathID")->GetValue()->AsInt64();
									AssetFileInfoEx* materialAssetFileInfoEx = assetsFileTable->getAssetInfo(materialPathId);
									AssetTypeInstance* materialAssetTypeInstance = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, materialAssetFileInfoEx);
									AssetsReplacer* materialAssetsReplacer = ReplaceMaterial(FAMItr->assetsName, assetsFileTable, materialAssetFileInfoEx, materialAssetTypeInstance, localTMPFontResource.m_Height, localTMPFontResource.m_Width, newMaterialPathID, newAtlasPathID);

									INT64 atlasPathId = 0;
									atlasPathId = monoOrigBaseATVF->Get("atlas")->Get("m_PathID")->GetValue()->AsInt64();
									if (atlasPathId == 0) {
										atlasPathId = monoOrigBaseATVF->Get("m_AtlasTextures")->Get((unsigned int)0)->Get("m_PathID")->GetValue()->AsInt64();
									}
									AssetFileInfoEx* atlasAssetFileInfoEx = assetsFileTable->getAssetInfo(atlasPathId);
									AssetTypeInstance* atlasAssetTypeInstance = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(assetsFileTable, atlasAssetFileInfoEx);
									string atlasName = WideMultiStringConverter->to_bytes(localTMPFontResource.GetResSFileName());
									AssetsReplacer* atlasAssetsReplacer = ReplaceAtlas(FAMItr->assetsName, assetsFileTable, atlasAssetFileInfoEx, atlasAssetTypeInstance, localTMPFontResource.m_CompleteImageSize, atlasName, localTMPFontResource.m_Width, localTMPFontResource.m_Height, newAtlasPathID);

									monoNewBaseATVF->Get("material")->Get("m_PathID")->GetValue()->Set(new INT64(newMaterialPathID));
									monoNewBaseATVF->Get("atlas")->Get("m_PathID")->GetValue()->Set(new INT64(newAtlasPathID));

									if (monoNewBaseATVF->Get("m_AtlasTextures") != NULL && monoNewBaseATVF->Get("m_AtlasTextures")->IsDummy() == false) {
										AssetTypeTemplateField* m_AtlasTexturesArrayATTF = monoNewBaseATVF->Get("m_AtlasTextures")->GetTemplateField();
										AssetTypeTemplateField* m_AtlasTexturesChildATTF = &(m_AtlasTexturesArrayATTF->children[1]);
										AssetTypeTemplateField* m_AtlasTexturesChildFileIDATTF = &m_AtlasTexturesChildATTF->children[0];
										AssetTypeTemplateField* m_AtlasTexturesChildPathIDATTF = &m_AtlasTexturesChildATTF->children[1];
										AssetTypeValueField* m_AtlasTexturesChildFileIDATVF = new AssetTypeValueField();
										m_AtlasTexturesChildFileIDATVF->Read(new AssetTypeValue(m_AtlasTexturesChildFileIDATTF->valueType, new INT32(0)), m_AtlasTexturesChildFileIDATTF, 0, 0);
										AssetTypeValueField* m_AtlasTexturesChildPathIDATVF = new AssetTypeValueField();
										m_AtlasTexturesChildPathIDATVF->Read(new AssetTypeValue(m_AtlasTexturesChildPathIDATTF->valueType, new INT64(newAtlasPathID)), m_AtlasTexturesChildPathIDATTF, 0, 0);
										AssetTypeValueField* m_AtlasTexturesChildATVF = new AssetTypeValueField();
										std::vector<AssetTypeValueField*>* m_AtlasTexturesChildArray = new std::vector<AssetTypeValueField*>();
										m_AtlasTexturesChildArray->push_back(m_AtlasTexturesChildFileIDATVF);
										m_AtlasTexturesChildArray->push_back(m_AtlasTexturesChildPathIDATVF);
										m_AtlasTexturesChildATVF->Read(new AssetTypeValue(ValueType_None, 0), m_AtlasTexturesChildATTF, 2, m_AtlasTexturesChildArray->data());
										std::vector<AssetTypeValueField*>* m_AtlasTexturesArray = new std::vector<AssetTypeValueField*>();
										m_AtlasTexturesArray->push_back(m_AtlasTexturesChildATVF);
										AssetTypeArray* assetTypeArray = new AssetTypeArray();
										assetTypeArray->size = m_AtlasTexturesArray->size();
										monoNewBaseATVF->Get("m_AtlasTextures")->Read(new AssetTypeValue(ValueType_Array, assetTypeArray), m_AtlasTexturesArrayATTF, m_AtlasTexturesArray->size(), m_AtlasTexturesArray->data());
									}

									string monoJsonStr = readFile2(FontPluginInfoGlobal->relativePluginPath + L"TMPFontAsset\\" + localTMPFontResource.GetMonoFileName());
									Json::Value monoJson;
									JsonParseFromString(monoJsonStr, monoJson);
									AssetsReplacer* monoNewAssetsReplacer = UnityL10nToolAPIGlobal->makeAssetsReplacer(assetsFileTable, MonoAFIEx, MonoNewATI, monoJson, newMonoPathID);

									replacers[FAMItr->assetsName].push_back(monoOrigAssetsReplacer);
									replacers[FAMItr->assetsName].push_back(materialAssetsReplacer);
									replacers[FAMItr->assetsName].push_back(atlasAssetsReplacer);
									replacers[FAMItr->assetsName].push_back(monoNewAssetsReplacer);
									lastAssetPathIDs[fontAssetMap.assetsName] = lastAssetPathID + 2;

									AddedTMPFontMono addedTMPFontMono = { FAMItr->assetsName , newMonoPathID, newAtlasPathID };
									AddedTMPFontMonos[WideMultiStringConverter->from_bytes(fontAssetMap.assetsName) + L"_" + fontFamilyName] = addedTMPFontMono;


									if (bFixExternalMaterial) {
										

										string origMonoName = monoOrigBaseATVF->Get("m_Name")->GetValue()->AsString();
										string origAssetsName = fontAssetMap.assetsName;
										AssetsFileTable* resAssetsFileTable = NULL;
										if (origAssetsName == "resources.assets") {
											resAssetsFileTable = assetsFileTable;
										}
										else {
											map<string, AssetsFileTable*>::const_iterator resAssetsFileTableItr = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find("resources.assets");
											if (resAssetsFileTableItr != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
												resAssetsFileTable = resAssetsFileTableItr->second;
											}
										}
										if (resAssetsFileTable != NULL) {
											for (int i = 0; i < resAssetsFileTable->assetFileInfoCount; i++) {
												AssetFileInfoEx* matAFIEx = &(resAssetsFileTable->pAssetFileInfo[i]);
												int classId = 0;
												UINT16 monoId = 0;
												UnityL10nToolAPI::GetClassIdFromAssetFileInfoEx(resAssetsFileTable, matAFIEx, classId, monoId);
												if (classId == materialClassId) {
													AssetTypeInstance* matATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(resAssetsFileTable, matAFIEx);
													AssetTypeValueField* matATVF = matATI->GetBaseField();
													if (matATVF) {
														AssetTypeValueField* m_NameMatATVF = matATVF->Get("m_Name");
														if (m_NameMatATVF && m_NameMatATVF->IsDummy() == false) {
															string matName = m_NameMatATVF->GetValue()->AsString();
															size_t found = matName.find(origMonoName);
															if (found != std::string::npos) {
																int fixResMatAtlasFileID = 0;
																if (origAssetsName == "resources.assets") {
																	if (matAFIEx->index == materialAssetFileInfoEx->index) {
																		continue;
																	}
																}
																else {
																	for (int j = 0; j < resAssetsFileTable->getAssetsFile()->dependencies.dependencyCount; ++j) {
																		if (resAssetsFileTable->getAssetsFile()->dependencies.pDependencies[j].assetPath == origAssetsName) {
																			fixResMatAtlasFileID = j + 1;
																			break;
																		}
																	}
																}
																AssetsReplacer* fixResMatReplacer = ReplaceMaterial("resources.assets", resAssetsFileTable, matAFIEx, matATI, localTMPFontResource.m_Height, localTMPFontResource.m_Width, matAFIEx->index, newAtlasPathID, fixResMatAtlasFileID);
																replacers["resources.assets"].push_back(fixResMatReplacer);
															}
														}
													}
												}
											}
										}
									}
								}
								usedFontResourcesInPatcher.insert(fontFamilyName);
							}
						}
					}
					break;
				}
			}
		}
		{
			auto foundResourcesAssetsTable = UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->find("resources.assets");
			if (foundResourcesAssetsTable != UnityL10nToolAPIGlobal->FindAssetsFileTablesFromAssetsName->end()) {
				AssetsFileTable* resourcesAssetsFileTable = foundResourcesAssetsTable->second;
				INT64 TMPSettingsPathID = UnityL10nToolAPIGlobal->FindAssetIndexFromName(resourcesAssetsFileTable, "TMP Settings");
				if (TMPSettingsPathID != -1) {
					AssetFileInfoEx* TMPSettingsAFIEx = resourcesAssetsFileTable->getAssetInfo(TMPSettingsPathID);
					AssetTypeInstance* TMPSettingsATI = UnityL10nToolAPIGlobal->GetDetailAssetTypeInstanceFromAssetFileInfoEx(resourcesAssetsFileTable, TMPSettingsAFIEx);
					AssetTypeValueField* TMPSettingsBaseATVF = TMPSettingsATI->GetBaseField();
					if (TMPSettingsBaseATVF) {
						AssetTypeValueField* m_fallbackFontAssetsATVF = TMPSettingsBaseATVF->Get("m_fallbackFontAssets");
						if (m_fallbackFontAssetsATVF && m_fallbackFontAssetsATVF->IsDummy() == false) {
							AssetTypeTemplateField* fallbackFontAssetArrayATTF = m_fallbackFontAssetsATVF->GetTemplateField();
							AssetTypeTemplateField* fallbackFontAssetChildATTF = &(fallbackFontAssetArrayATTF->children[1]);
							AssetTypeTemplateField* fallbackFontAssetChildFileIDATTF = &fallbackFontAssetChildATTF->children[0];
							AssetTypeTemplateField* fallbackFontAssetChildPathIDATTF = &fallbackFontAssetChildATTF->children[1];

							std::vector<AssetTypeValueField*>* fallbackFontAssetArray = new std::vector<AssetTypeValueField*>();
							for (unsigned int i = 0; i < m_fallbackFontAssetsATVF->GetChildrenCount(); i++) {
								fallbackFontAssetArray->push_back(m_fallbackFontAssetsATVF->GetChildrenList()[i]);
							}

							for (auto AddedTMPFontMonoItr = AddedTMPFontMonos.begin(); AddedTMPFontMonoItr != AddedTMPFontMonos.end(); AddedTMPFontMonoItr++) {
								INT32 fileID = 0;
								if (AddedTMPFontMonoItr->second.assetsName == "resources.assets") {
									fileID = 0;
								}
								else {
									for (unsigned int i = 0; i < resourcesAssetsFileTable->getAssetsFile()->dependencies.dependencyCount; ++i) {
										if (AddedTMPFontMonoItr->second.assetsName == resourcesAssetsFileTable->getAssetsFile()->dependencies.pDependencies[i].assetPath) {
											fileID = i + 1;
										}
									}
								}
								AssetTypeValueField* fallbackFontAssetChildFileIDATVF = new AssetTypeValueField();
								fallbackFontAssetChildFileIDATVF->Read(new AssetTypeValue(fallbackFontAssetChildFileIDATTF->valueType, new INT32(fileID)), fallbackFontAssetChildFileIDATTF, 0, 0);
								AssetTypeValueField* fallbackFontAssetChildPathIDATVF = new AssetTypeValueField();
								fallbackFontAssetChildPathIDATVF->Read(new AssetTypeValue(fallbackFontAssetChildPathIDATTF->valueType, new INT64(AddedTMPFontMonoItr->second.PathID)), fallbackFontAssetChildPathIDATTF, 0, 0);

								AssetTypeValueField* fallbackFontAssetChildATVF = new AssetTypeValueField();
								std::vector<AssetTypeValueField*>* fallbackFontAssetChildArray = new std::vector<AssetTypeValueField*>();
								fallbackFontAssetChildArray->push_back(fallbackFontAssetChildFileIDATVF);
								fallbackFontAssetChildArray->push_back(fallbackFontAssetChildPathIDATVF);
								fallbackFontAssetChildATVF->Read(new AssetTypeValue(ValueType_None, 0), fallbackFontAssetChildATTF, 2, fallbackFontAssetChildArray->data());

								fallbackFontAssetArray->push_back(fallbackFontAssetChildATVF);
							}
							AssetTypeArray* assetTypeArray = new AssetTypeArray();
							assetTypeArray->size = fallbackFontAssetArray->size();
							m_fallbackFontAssetsATVF->Read(new AssetTypeValue(ValueType_Array, assetTypeArray), fallbackFontAssetArrayATTF, fallbackFontAssetArray->size(), fallbackFontAssetArray->data());

							AssetsReplacer* TMPSettingsReplacer = UnityL10nToolAPIGlobal->makeAssetsReplacer(resourcesAssetsFileTable, TMPSettingsAFIEx, TMPSettingsATI);
							replacers["resources.assets"].push_back(TMPSettingsReplacer);
						}
					}
				}
			}
		}
	}
	return replacers;
}

bool _cdecl CopyResourceFileToGameFolder(wstring FontPluginRelativePath, wstring targetPath) {
	if (usedFontResourcesInPatcher.size() != 0) {
		CreateDirectoryW((targetPath + L"TMPFontAsset\\").c_str(), NULL);
		for (set<wstring>::iterator iterator = usedFontResourcesInPatcher.begin();
			iterator != usedFontResourcesInPatcher.end();
			++iterator) {
			map<wstring, TMPFontResource>::iterator TMPFontResourceItr = TMPFontResources.find(*iterator);
			if (TMPFontResourceItr != TMPFontResources.end()) {
				TMPFontResource tmpFontResource = TMPFontResourceItr->second;
				CopyFileW((FontPluginRelativePath + L"TMPFontAsset\\" + tmpFontResource.GetResSFileName()).c_str(), (targetPath + L"TMPFontAsset\\" + tmpFontResource.GetResSFileName()).c_str(), false);
			}
		}
	}
	return true;
}

bool _cdecl SetPluginSupportAssetMap(FontAssetMaps supportAssetMaps) {
	fontAssetMapsGlobal = supportAssetMaps;
	return true;
}

bool _cdecl GetFontPluginInfo(UnityL10nToolAPI* unityL10nToolAPI, FontPluginInfo* fontPluginInfo) {
	UnityL10nToolAPIGlobal = unityL10nToolAPI;
	FontPluginInfoGlobal = fontPluginInfo;
	wcsncpy_s(fontPluginInfo->FontPluginName, L"TMPFontAsset", 12);
	fontPluginInfo->SetProjectConfigJson = SetProjectConfigJson;
	fontPluginInfo->GetPluginSupportAssetMap = GetPluginSupportAssetMap;
	fontPluginInfo->SetPluginSupportAssetMap = SetPluginSupportAssetMap;
	fontPluginInfo->GetProjectConfigJson = GetProjectConfigJson;
	fontPluginInfo->GetPacherConfigJson = GetPacherConfigJson;
	fontPluginInfo->CopyBuildFileToBuildFolder = CopyBuildFileToBuildFolder;

	fontPluginInfo->SetPacherConfigJson = SetPacherConfigJson;
	fontPluginInfo->GetPatcherAssetReplacer = GetPatcherAssetReplacer;
	fontPluginInfo->CopyResourceFileToGameFolder = CopyResourceFileToGameFolder;

	vector<wstring> TMPFontAssetNameList = get_all_files_names_within_folder(fontPluginInfo->relativePluginPath + L"TMPFontAsset\\*.Font.json");
	AssetMapOption assetMapOption = AssetMapOption(
		L"Font Family",
		L"Select Font Family Name to use",
		NULL,
		NULL,
		AssetMapOption::Type::OPTION_TYPE_WSTRING,
		AssetMapOption::Type::OPTION_TYPE_NONE,
		vector<AssetMapOption>()
	);
	for (vector<wstring>::iterator iterator = TMPFontAssetNameList.begin();
		iterator != TMPFontAssetNameList.end();
		iterator++) {
		string TMPFontJsonStr = readFile2(fontPluginInfo->relativePluginPath + L"TMPFontAsset\\" + *iterator);
		Json::Value TMPFontJson;
		JsonParseFromString(TMPFontJsonStr, TMPFontJson);
		TMPFontResource tmpFontResource(*iterator, TMPFontJson);
		if (FileExist(fontPluginInfo->relativePluginPath + L"TMPFontAsset\\" + tmpFontResource.GetMonoFileName())
			&& FileExist(fontPluginInfo->relativePluginPath + L"TMPFontAsset\\" + tmpFontResource.GetResSFileName())) {
			TMPFontResources[tmpFontResource.FontFamilyName] = tmpFontResource;
			AssetMapOption assetMapOptionFontFamilyEnum = AssetMapOption(
				L"",
				L"",
				NULL,
				new wstring(tmpFontResource.FontFamilyName),
				AssetMapOption::Type::OPTION_TYPE_NONE,
				AssetMapOption::Type::OPTION_TYPE_WSTRING,
				std::vector<AssetMapOption>()
			);
			assetMapOption.nestedOptions.push_back(assetMapOptionFontFamilyEnum);
		}
	}
	if (assetMapOption.nestedOptions.size() == 0) {
		AssetMapOption assetMapOptionFontFamilyEnum = AssetMapOption(
			L"",
			L"",
			NULL,
			new wstring(L"(Cannot load Font Resources."),
			AssetMapOption::Type::OPTION_TYPE_NONE,
			AssetMapOption::Type::OPTION_TYPE_WSTRING,
			std::vector<AssetMapOption>()
		);
	}
	OptionsList.push_back(assetMapOption);
	AssetMapOption assetMapOptionReplaceOtherMaterials = AssetMapOption(
		L"Fix mismatch shadow and outline",
		L"If there is mismatch shadow after font patch, trun on this option",
		new bool(false),
		NULL,
		AssetMapOption::OPTION_TYPE_BOOL,
		AssetMapOption::OPTION_TYPE_NONE,
		vector<AssetMapOption>()
	);
	OptionsList.push_back(assetMapOptionReplaceOtherMaterials);
	AssetMapOption assetMapOptionOverwriteOriginalFont = AssetMapOption(
		L"Overwrite original font (previous way)",
		L"For fallback unsupport game",
		new bool(false),
		NULL,
		AssetMapOption::OPTION_TYPE_BOOL,
		AssetMapOption::OPTION_TYPE_NONE,
		vector<AssetMapOption>()
	);
	OptionsList.push_back(assetMapOptionOverwriteOriginalFont);
	return true;
}