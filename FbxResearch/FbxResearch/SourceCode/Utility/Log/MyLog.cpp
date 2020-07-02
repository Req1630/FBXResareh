#include "MyLog.h"
#include "..\..\Global.h"

#include <iostream>
#include <fstream>
#include <sstream>

CLog::CLog()
{
}

CLog::~CLog()
{
}

//---------------------------------.
// インスタンスの取得.
//---------------------------------.
CLog* CLog::GetInstance()
{
	static std::unique_ptr<CLog> pInstance = std::make_unique<CLog>();
	return pInstance.get();
}

//---------------------------------.
// ログテキストファイルを作成.
//---------------------------------.
HRESULT CLog::OpenLogTextFile()
{
	std::ofstream logText;
	logText.open( GetInstance()->LOG_TEXT_FILE_PATH, std::ios::trunc );

	// ファイルが読み込めてなかったら終了.
	if( !logText.is_open() ){
		ERROR_MESSAGE( "log text read failed" );
		return E_FAIL;
	}

	time_t		nowTime		= time( nullptr );	// 現在の時間を獲得.
	TIME_DATA	timeData;	
	localtime_s( &timeData, &nowTime );// ローカル時間に変換.

	// 現在の時間をテキストに入力.
	logText << "[";
	logText << timeData.tm_year+1900	<< "/";	// 1900 足すことで現在の年になる.
	logText << timeData.tm_mon+1		<< "/";	// 1 足すことで現在の年になる.
	logText << timeData.tm_mday			<< " ";
	logText << timeData.tm_hour			<< ":";
	logText << timeData.tm_min;
	logText << "]" << " >> App launch"	<< std::endl << std::endl;

	// ファイルを閉じる.
	logText.close();
	return S_OK;
}

//---------------------------------.
// ログテキストファイルを閉じる.
//---------------------------------.
HRESULT CLog::CloseLogTextFile()
{
	std::ofstream logText( GetInstance()->LOG_TEXT_FILE_PATH, std::ios::app );

	// ファイルが読み込めてなかったら終了.
	if( !logText.is_open() ){
		ERROR_MESSAGE( "log text read failed" );
		return E_FAIL;
	}

	time_t		nowTime = time( nullptr );	// 現在の時間を獲得.
	TIME_DATA	timeData;
	localtime_s( &timeData, &nowTime );// ローカル時間に変換.

	// 現在の時間をテキストに入力.
	logText << std::endl		<<  "[";
	logText << timeData.tm_hour	<< ":";
	logText << timeData.tm_min	<< ":";
	logText << timeData.tm_sec;
	logText << "]" << " >> App End" << std::endl;

	// ファイルを閉じる.
	logText.close();

	return S_OK;
}

//---------------------------------.
// ログの入力.
//---------------------------------.
HRESULT CLog::Print( const std::string& message )
{
	std::ofstream logText( GetInstance()->LOG_TEXT_FILE_PATH, std::ios::app );

	// ファイルが読み込めてなかったら終了.
	if( !logText.is_open() ){
		ERROR_MESSAGE( "log text read failed" );
		return E_FAIL;
	}

	time_t		nowTime = time( nullptr );	// 現在の時間を獲得.
	TIME_DATA	timeData;
	localtime_s( &timeData, &nowTime );// ローカル時間に変換.

	// 現在の時間をテキストに入力.
	logText << "[";
	logText << timeData.tm_hour << ":";
	logText << timeData.tm_min	<< ":";
	logText << timeData.tm_sec;
	logText << "]" << " >> " << message << std::endl;	// messageの入力.

	// ファイルを閉じる.
	logText.close();

	return S_OK;
}