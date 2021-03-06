// TestFroParse.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>
#include <regex>
using namespace std;

const wchar_t* whitespace = L" \t\n\r\f\v";

inline std::wstring& rtrim(std::wstring& s, const wchar_t* t = whitespace)
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

// trim from beginning of string (left)
inline std::wstring& ltrim(std::wstring& s, const wchar_t* t = whitespace)
{
	s.erase(0, s.find_first_not_of(t));
	return s;
}

// trim from both ends of string (left & right)
inline std::wstring& trim(std::wstring& s, const wchar_t* t = whitespace)
{
	return ltrim(rtrim(s, t), t);
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

int main()
{
    std::cout << "Hello World!\n"; 
	std::wregex regexp(L"msgctxt \"(.*)\"\\nmsgid ([\\s\\S\\n\\r\\t]*?)\\nmsgstr ((?:.|\n)*?)\"\\n\\n");
	wstring testStr = L"#: 14\nmsgctxt \"m.instructions.basics\"\nmsgid \"\"\n\"<b>W</b>: Move forward \\n\"\n\"<b>A</b>: Move left \\n\"\n\"<b>S</b>: Move backward \\n\"\n\"<b>D</b>: Move right \\n\"\n\"\\n\"\n\"<b>Shift</b>: Sprint (uses energy) \\n\"\n\"\\n\"\n\"<b>Space</b>: Jump (hold to use jetpack, if available) \\n\"\n\"\\n\"\n\"<b>E</b>: Interact (when prompted)\"\nmsgstr \"\"\n\"<b>W</b>: 앞으로 가기 \\n\"\n\"<b>A</b>: 왼쪽으로 가기 \\n\"\n\"<b>S</b>: 뒤로 가기\\n\"\n\"<b>D</b>: 오른쪽으로 가기\\n\"\n\"\\n\"\n\"<b>Shift</b>: 전력질주 (에너지 사용)\\n\"\n\"\\n\"\n\"<b>Space</b>: 뛰기 (길게 누를 시 제트팩 사용, 얻었을 때) \\n\"\n\"\\n\"\n\"<b>E</b>: 상호작용 (메시지가 나타나면)\"\n\n#: 15\nmsgctxt \"m.instructions.gamepad.basics\"\nmsgid \"\"\n\"<b>Left Stick</b>: Move\\n\"\n\"\\n\"\n\"<b>Hold Left Stick</b>: Sprint (uses energy) \\n\"\n\"\\n\"\n\"<b>A</b>: Jump (hold to use jetpack, if available) \\n\"\n\"\\n\"\n\"<b>X</b>: Interact (when prompted)\"\nmsgstr \"\"\n\"<b>왼쪽 스틱</b>: 이동\\n\"\n\"\\n\"\n\"<b>왼쪽 스틱 누르기</b>: 전력질주 (에너지를 사용함) \\n\"\n\"\\n\"\n\"<b>A</b>: 뛰기 (누르고 있으면 제트팩 사용, 사용가능할 때) \\n\"\n\"\\n\"\n\"<b>X</b>: 상호작용 (메세지가 나타나면)\"\n\n#: 16\nmsgctxt \"m.instructions.ps4.basics\"\nmsgid \"\"\n\"<b>Left Stick</b>: Move\\n\"\n\"\\n\"\n\"<b>L3</b>: Sprint (uses energy) \\n\"\n\"\\n\"\n\"<b>cross button</b>: Jump (hold to use jetpack, if available) \\n\"\n\"\\n\"\n\"<b>square button</b>: Interact (when prompted)\"\nmsgstr \"\"\n\"<b>왼쪽 스틱</b>: 움직임\\n\"\n\"\\n\"\n\"<b>L3</b>: 스프린트 (에너지 사용) \\n\"\n\"\\n\"\n\"<b>크로스 버튼</b>: 점프 (누르고 있을 시에는 제트팩 사용) \\n\"\n\"\\n\"\n\"<b>사각형 버튼</b>: 작동 (메세지가 나타나면)\"\n\n";
	const wsregex_iterator itEnd;
	wsregex_iterator it(testStr.begin(), testStr.end(), regexp);
	int i = 0;
	for (; it != itEnd; it++, i++) {
		wstring original = (*it)[0];
		wstring key = (*it)[1];
		key = ReplaceAll(key, L"\"", L"");
		wstring OriginalText = (*it)[2];
		wregex regex1(L"^\"|\"\n\"|\"$");
		OriginalText = regex_replace(OriginalText, regex1, wstring(L""));
		OriginalText = ReplaceAll(trim(OriginalText), L"\\n", L"\n");
		OriginalText = ReplaceAll(trim(OriginalText), L"\\\"", L"\"");
		wstring TranslatedText = (*it)[3];
		TranslatedText = regex_replace(TranslatedText, regex1, wstring(L""));
		TranslatedText = ReplaceAll(trim(TranslatedText), L"\\n", L"\n");
		TranslatedText = ReplaceAll(trim(TranslatedText), L"\\\"", L"\"");
	}
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
