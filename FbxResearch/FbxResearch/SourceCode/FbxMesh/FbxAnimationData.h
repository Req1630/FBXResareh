#pragma once

#include "..\Global.h"
#include "FbxInclude\FbxInclude.h"

#include <vector>
#include <string>
#include <map>

struct stAnimationData
{
	std::string Name;	// 名前.
	double StartTime;	// スタート時間.
	double EndTime;		// 終了時間.
	double NowTime;		// 経過時間.
	double AnimSpeed;	// アニメーション速度.

	double AnimationTime;	// アニメーション時間.
	double FrameRate;

	std::vector<std::map<double, FbxMatrix>> KeyFrameMatrix;		// 各フレームの行列.
	std::vector<std::vector<std::map<double, FbxMatrix>>> KeyFrameLinkMatrix;	// 各フレームのリンク行列.
	std::vector<std::vector<double>> FrameList;	// フレームリスト.

	FbxMatrix GetFrameLinkMatrix( const int& meshNo, const int& i, const double& time )
	{
		FbxMatrix m;
		if( i >= (int)KeyFrameLinkMatrix[meshNo].size() ) return m;
		// フレームの数.
		int frameSize = static_cast<int>(FrameList[meshNo].size())-1;

		auto outFrameMatrix = [&]( const int& minIndex, const int& maxIndex, FbxMatrix* outMat )
		{
			if( minIndex > frameSize ) return;
			if( maxIndex > frameSize ) return;
			for( int j = minIndex; j < maxIndex; j++ ){
				if( FrameList[meshNo][j] <= time && time <= FrameList[meshNo][j+1] ){
					double lenght = FrameList[meshNo][j+1] - FrameList[meshNo][j];
					double ollLenght = (time - FrameList[meshNo][j]) / lenght;
					FbxMatrix tmpMatrix1 = KeyFrameLinkMatrix[meshNo][i][FrameList[meshNo][j]];
					FbxMatrix tmpMatrix2 = KeyFrameLinkMatrix[meshNo][i][FrameList[meshNo][j+1]];
					*outMat = tmpMatrix2 - tmpMatrix1;
					*outMat *= ollLenght;
					*outMat += tmpMatrix1;
					break;
				}
			}
		};
		int size = frameSize/7;
		if( time < FrameList[meshNo][size*1] ){
			outFrameMatrix( size*0, size*1, &m );
		} else if ( time < FrameList[meshNo][size*2] ){
			outFrameMatrix( size*1, size*2, &m );
		} else if ( time < FrameList[meshNo][size*3] ){
			outFrameMatrix( size*2, size*3, &m );
		} else if ( time < FrameList[meshNo][size*4] ){
			outFrameMatrix( size*3, size*4, &m );
		} else if ( time < FrameList[meshNo][size*5] ){
			outFrameMatrix( size*4, size*5, &m );
		} else if ( time < FrameList[meshNo][size*6] ){
			outFrameMatrix( size*5, size*6, &m );
		} else if ( time < FrameList[meshNo][size*7] ){
			outFrameMatrix( size*6, size*7, &m );
		} else {
			outFrameMatrix( size*7, frameSize, &m );
		}
		return m;
	}
	FbxMatrix GetFrameMatrix( const int& meshNo, const double& time )
	{
		FbxMatrix m;
		// フレームの数.
		int frameSize = static_cast<int>(FrameList.size());

		/*
		auto outFrameMatrix = [&]( const int& minIndex, const int& maxIndex, FbxMatrix* outMat )
		{
			for( int j = minIndex; j < maxIndex; j++ ){
				if( FrameList[meshNo][j] <= time && time < FrameList[meshNo][j+1] ){
					double lenght = FrameList[meshNo][j+1] - FrameList[meshNo][j];
					double ollLenght = (FrameList[meshNo][j] - time) / lenght;
					FbxMatrix tmpMatrix1 = KeyFrameMatrix[meshNo][FrameList[meshNo][j]];
					FbxMatrix tmpMatrix2 = KeyFrameMatrix[meshNo][FrameList[meshNo][j+1]];
					*outMat = tmpMatrix2 - tmpMatrix1;
					*outMat *= ollLenght;
					*outMat += tmpMatrix1;
					break;
				}
			}
		};
		*/

		/*if( time <= FrameList[meshNo][frameSize/2] ){
			outFrameMatrix( 0, frameSize/2, &m );
		} else {
			outFrameMatrix( frameSize/2, frameSize, &m );
		}*/

		/*for( int j = 0; j < frameSize-1; j++ ){
			if( FrameList[j] <= time && time < FrameList[j+1] ){
				double lenght = FrameList[j+1] - FrameList[j];
				double ollLenght = (FrameList[j] - time) / lenght;
				FbxMatrix tmpMatrix1 = KeyFrameMatrix[meshNo][FrameList[j]];
				FbxMatrix tmpMatrix2 = KeyFrameMatrix[meshNo][FrameList[j+1]];
				m = tmpMatrix2 - tmpMatrix1;
				m *= ollLenght;
				m += tmpMatrix1;
				break;
			}
		}*/
		return m = KeyFrameMatrix[meshNo][FrameList[meshNo][0]];
	}

} typedef SAnimationData;