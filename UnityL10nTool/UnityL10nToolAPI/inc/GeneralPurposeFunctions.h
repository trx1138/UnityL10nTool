#pragma once
#include <Windows.h>
#include <shellapi.h>
#include <codecvt> // for std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
#include <fstream>
#include "json/json.h"

// http://faithlife.codes/blog/2008/04/exception_0xc0020001_in_ccli_assembly/ Due to static value 0xc0020001 occur
#ifndef UnityL10nToolCppCLIDEFINE
#ifndef GeneralPurposeGLOBAL
#define GeneralPurposeGLOBAL
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>* WideMultiStringConverter = new std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>(); // #include <codecvt>
//Json::Reader* JsonReader = new Json::Reader();
Json::CharReaderBuilder* builder = new Json::CharReaderBuilder();
std::unique_ptr<Json::CharReader> const JsonReader(builder->newCharReader());
Json::StreamWriterBuilder* wbuilder = new Json::StreamWriterBuilder();
#endif
#else
extern std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>* WideMultiStringConverter;
extern std::unique_ptr<Json::CharReader> const JsonReader;
extern Json::StreamWriterBuilder* wbuilder;
#endif // !UnityL10nToolCppCLIDEFINE

std::string readFile2(const std::wstring & fileName);
std::vector<std::wstring> GetAllFilesFilterWithinAllSubFolder(std::wstring directory, std::wstring filter);
std::string ReplaceAll(std::string & str, const std::string & from, const std::string & to);
bool copyFileCustom(const char *SRC, const char* DEST);
bool copyFileCustom(const wchar_t *SRC, const wchar_t* DEST);
std::vector<std::wstring> get_all_files_names_within_folder(std::wstring filter);
bool CreateProcessCustom(std::wstring commandLine);
bool CopyDirTo(const std::wstring& source_folder, const std::wstring& target_folder);

inline Json::Value JsonParseFromString(std::string str) {
	Json::Value json;
	std::string err;
	JsonReader->parse(str.c_str(), str.c_str() + str.size(), &json, &err);
	return json;
}

inline bool JsonParseFromString(std::string str, Json::Value& json) {
	std::string err;
	return JsonReader->parse(str.c_str(), str.c_str() + str.size(), &json, &err);
}

inline Json::Value JsonParseFromWString(std::wstring wstr) {
	std::string str = WideMultiStringConverter->to_bytes(wstr);
	return JsonParseFromString(str);
}

inline bool JsonParseFromWstring(std::wstring wstr, Json::Value& json) {
	std::string str = WideMultiStringConverter->to_bytes(wstr);
	return JsonParseFromString(str, json);
}

inline std::string JsonToStyleString(Json::Value& json) {
	(*wbuilder)["indentation"] = "\t";
	return Json::writeString((*wbuilder), json);
}

inline std::wstring JsonToStyleWString(Json::Value& json) {
	return WideMultiStringConverter->from_bytes(JsonToStyleString(json));
}

// https://stackoverflow.com/questions/4725115/on-windows-is-there-an-interface-for-copying-folders/4725137
inline bool CopyDirTo(const std::wstring& source_folder, const std::wstring& target_folder)
{
	std::wstring new_sf = source_folder + L"*";
	WCHAR sf[MAX_PATH + 1];
	WCHAR tf[MAX_PATH + 1];

	wcscpy_s(sf, MAX_PATH, new_sf.c_str());
	wcscpy_s(tf, MAX_PATH, target_folder.c_str());

	sf[lstrlenW(sf) + 1] = 0;
	tf[lstrlenW(tf) + 1] = 0;

	SHFILEOPSTRUCTW s = { 0 };
	s.wFunc = FO_COPY;
	s.pTo = tf;
	s.pFrom = sf;
	s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NO_UI;
	int res = SHFileOperationW(&s);

	return res == 0;
}

// https://stackoverflow.com/questions/116038/what-is-the-best-way-to-read-an-entire-file-into-a-stdstring-in-c#
inline std::string readFile2(const std::wstring &fileName)
{
	std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

	std::ifstream::pos_type fileSize = ifs.tellg();
	if (fileSize < 0)
		return std::string();

	ifs.seekg(0, std::ios::beg);

	std::vector<char> bytes(fileSize);
	ifs.read(&bytes[0], fileSize);

	return std::string(&bytes[0], fileSize);
}

inline bool copyFileCustom(const char *SRC, const char* DEST)
{
	std::ifstream src(SRC, std::ios::binary);
	std::ofstream dest(DEST, std::ios::binary);
	dest << src.rdbuf();
	return src && dest;
}

inline bool copyFileCustom(const wchar_t *SRC, const wchar_t* DEST)
{
	std::ifstream src(SRC, std::ios::binary);
	std::ofstream dest(DEST, std::ios::binary);
	dest << src.rdbuf();
	return src && dest;
}

//https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
inline std::vector<std::wstring> get_all_files_names_within_folder(std::wstring filter)
{
	std::vector<std::wstring> names;
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFileW(filter.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

inline std::vector<std::wstring> GetAllFolderName(std::wstring directory) {
	std::vector<std::wstring> directories;
	WIN32_FIND_DATAW fd;
	HANDLE hFind = ::FindFirstFileW((directory + L"*").c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				directories.push_back(fd.cFileName);
			}
		} while (::FindNextFileW(hFind, &fd));
		::FindClose(hFind);
	}
	return directories;
}

inline std::vector<std::wstring> GetAllFilesFilterWithinAllSubFolderRecursive(std::wstring firstDirectory, std::wstring subDirectory, std::wstring filter) {
	std::vector<std::wstring> files;
	WIN32_FIND_DATAW fd;
	HANDLE hFind = ::FindFirstFileW((firstDirectory + subDirectory + filter).c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				files.push_back(subDirectory + fd.cFileName);
			}
		} while (::FindNextFileW(hFind, &fd));
		::FindClose(hFind);
	}
	WIN32_FIND_DATAW fdSub;
	HANDLE hFindSub = ::FindFirstFileW((firstDirectory + subDirectory + L"*").c_str(), &fdSub);
	if (hFindSub != INVALID_HANDLE_VALUE) {
		do {
			if (fdSub.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if ((!lstrcmpW(fdSub.cFileName, L".")) || (!lstrcmpW(fdSub.cFileName, L".."))) {
					continue;
				}
				std::vector<std::wstring> subFiles = GetAllFilesFilterWithinAllSubFolderRecursive(firstDirectory, subDirectory + fdSub.cFileName + L"\\", filter);
				files.insert(files.end(), subFiles.begin(), subFiles.end());
			}
		} while (FindNextFileW(hFindSub, &fdSub));
	}
	return files;
}

inline std::vector<std::wstring> GetAllFilesFilterWithinAllSubFolder(std::wstring directory, std::wstring filter) {
	return GetAllFilesFilterWithinAllSubFolderRecursive(directory, L"", filter);
}

inline bool CreateProcessCustom(std::wstring commandLine) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(NULL,   // No module name (use command line)
		(LPWSTR)commandLine.c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return false;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

inline std::wstring RemoveAll(std::wstring& str, const wchar_t from[], size_t sizeOfFrom) {
	size_t start_pos = 0;
	const size_t Wchar_tSize = sizeof(wchar_t);
	for(int i=0; i< sizeOfFrom; ++i) {
		start_pos = 0;
		while ((start_pos = str.find(from[i], start_pos)) != std::wstring::npos) {
			str.erase(start_pos, 1);
		}
	}
	return str;
}

inline std::string ReplaceAll(std::string &str, const std::string& from, const std::string& to) {
	size_t start_pos = 0; //string처음부터 검사
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)  //from을 찾을 수 없을 때까지
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
	}
	return str;
}

inline std::wstring ReplaceAll(std::wstring &str, const std::wstring& from, const std::wstring& to) {
	size_t start_pos = 0; //string처음부터 검사
	while ((start_pos = str.find(from, start_pos)) != std::wstring::npos)  //from을 찾을 수 없을 때까지
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
	}
	return str;
}

inline bool DirExists(const std::wstring& dirName_in)
{
	DWORD ftyp = GetFileAttributesW(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
//inline bool FileExist(const std::wstring& name) {
//	if (FILE *file = _wfopen(name.c_str(), L"r")) {
//		fclose(file);
//		return true;
//	}
//	else {
//		return false;
//	}
//}

inline bool FileExist(const std::wstring& name) {
	std::ifstream f(name.c_str());
	return f.good();
}



