#pragma once

#include "..\FbxInclude\FbxInclude.h"

#include <vector>
#include <string>
#include <map>

// �A�j���[�V�����f�[�^�̍\����.
struct stAnimationData
{
private:
	// �^���킩��ɂ����Ȃ�̂ŁA
	// �ʖ���t����.
	template <class T>
	using meshVector = std::vector<T>;	// ���b�V�����̔z��.
	template <class T>
	using boneVector = std::vector<T>;	// �{�[�����̔z��.
	template <class T>
	using keyMatrixMap = std::map<double, T>;	// �L�[�t���[������̍s��̘A�z�z��.
	using keyVector = std::vector<double>;		// �L�[�t���[�����̔z��.

public:
	std::string Name;	// ���O.
	double StartTime;	// �X�^�[�g����.
	double EndTime;		// �I������.
	double NowTime;		// �o�ߎ���.
	double AnimSpeed;	// �A�j���[�V�������x.

	double AnimationTime;	// �A�j���[�V��������.
	double FrameRate;		// �t���[�����[�g.

	meshVector<boneVector<keyMatrixMap<FbxMatrix>>> KeyFrameLinkMatrix;	// �e�t���[���̃����N�s��.
	meshVector<keyVector> KeyList;	// �L�[�t���[�����X�g.

	stAnimationData()
		: Name					()
		, StartTime				( 0.0 )
		, EndTime				( 0.0 )
		, NowTime				( 0.0 )
		, AnimSpeed				( 0.0 )
		, AnimationTime			( 0.0 )
		, FrameRate				( 0.0 )
		, KeyFrameLinkMatrix	()
		, KeyList				()
	{}

	// �L�[�t���[���s��̎擾.
	bool GetFrameLinkMatrix( const int& meshNo, const int& i, const double& time, FbxMatrix* outMatrix )
	{
		if( meshNo >= (int)KeyFrameLinkMatrix.size() ) return false;
		if( i >= (int)KeyFrameLinkMatrix[meshNo].size() ) return false;
		// �t���[���̐�.
		int frameSize = static_cast<int>(KeyList[meshNo].size())-1;

		auto outFrameMatrix = [&]( const int& minIndex, const int& maxIndex, FbxMatrix* outMat )
		{
			if( minIndex > frameSize ) return;
			if( maxIndex > frameSize ) return;
			for( int j = minIndex; j < maxIndex; j++ ){
				if( KeyList[meshNo][j] <= time && time <= KeyList[meshNo][j+1] ){
					double lenght = KeyList[meshNo][j+1] - KeyList[meshNo][j];
					*outMat = 
						KeyFrameLinkMatrix[meshNo][i][KeyList[meshNo][j+1]] - 
						KeyFrameLinkMatrix[meshNo][i][KeyList[meshNo][j]];
					*outMat *= (time - KeyList[meshNo][j]) / lenght;
					*outMat += KeyFrameLinkMatrix[meshNo][i][KeyList[meshNo][j]];
					break;
				}
			}
		};

		int size = frameSize/7;
		if( time < KeyList[meshNo][size*1] ){
			outFrameMatrix( size*0, size*1, outMatrix );
		} else if ( time < KeyList[meshNo][size*2] ){
			outFrameMatrix( size*1, size*2, outMatrix );
		} else if ( time < KeyList[meshNo][size*3] ){
			outFrameMatrix( size*2, size*3, outMatrix );
		} else if ( time < KeyList[meshNo][size*4] ){
			outFrameMatrix( size*3, size*4, outMatrix );
		} else if ( time < KeyList[meshNo][size*5] ){
			outFrameMatrix( size*4, size*5, outMatrix );
		} else if ( time < KeyList[meshNo][size*6] ){
			outFrameMatrix( size*5, size*6, outMatrix );
		} else if ( time < KeyList[meshNo][size*7] ){
			outFrameMatrix( size*6, size*7, outMatrix );
		} else {
			outFrameMatrix( size*7, frameSize, outMatrix );
		}
		return true;
	}

} typedef SAnimationData;

struct stAnimationDataList
{
	std::vector<SAnimationData> AnimList;
} typedef SAnimationDataList;