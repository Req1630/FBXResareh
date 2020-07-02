#ifndef MY_LOG_H
#define MY_LOG_H

#include <Windows.h>
#include <memory>
#include <string>
#include <time.h>

class CLog
{
	typedef tm TIME_DATA;
	const char* LOG_TEXT_FILE_PATH = "Log.txt";	// ログテキストのファイルパス.

public:
	CLog();
	~CLog();

	// インスタンスの取得.
	static CLog* GetInstance();

	// ログテキストファイルを作成.
	// アプリ起動時に作成する.
	static HRESULT OpenLogTextFile();

	// ログテキストファイルを閉じる.
	static HRESULT CloseLogTextFile();

	// ログの入力.
	static HRESULT Print( const std::string& message );

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	CLog( const CLog & )				= delete;
	CLog& operator = ( const CLog & )	= delete;
	CLog( CLog && )						= delete;
	CLog& operator = ( CLog && )		= delete;
};

#endif	// #ifndef MY_LOG_H.