#include "FileManager.h"
#include "..\..\Global.h"


CFileManager::CFileManager()
{
}

CFileManager::~CFileManager()
{
}

//-----------------------------------.
// ファイルの読み込み.
//-----------------------------------.
std::vector<std::string> CFileManager::TextLoading( const std::string& fileName, char delimiter )
{
	std::vector<std::string> readList;	// 読み込みリスト.

										// ファイルを開く.
	std::fstream fileStream( fileName );
	// ファイルが開けない場合.
	if( !fileStream.is_open() ){
		ERROR_MESSAGE("The " + fileName +  " attack file could not be opened");
		return readList;
	}

	std::string line;	// 1行分を読み込むための文字.

						// 一行づつ読み込み.
	while( std::getline( fileStream, line ) ){
		// 取得した文字に '//' があればやり直し.
		if( line.find("//") != std::string::npos ) continue;

		std::string buff = "";				// 文字を一時的に格納する.
		std::istringstream stream( line );

		// デフォルト ',' 区切りで文字を取得.
		while( std::getline( stream, buff, delimiter ) ){
			// 取得した文字に '#' があればやり直し.
			if( buff.find('#') != std::string::npos ) continue;

			// 読み込みリストに追加.
			readList.emplace_back( buff );
		}
	}

	fileStream.close();

	return readList;
}