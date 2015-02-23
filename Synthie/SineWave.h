#pragma once
#include "AudioNode.h"

class CSineWave :
	public CAudioNode
{
public:
	CSineWave();
	virtual ~CSineWave();

	//! Start audio generation
	virtual void Start() override;

	//! Generate one frame of audio
	virtual bool Generate() override;

	//! Set the sine wave frequency
	void SetFreq(double f) { mFreq = f; }

	//! Set the sine wave amplitude
	void SetAmplitude(double a) { mAmp = a; }

private:
	double mFreq;
	double mAmp;
	double mPhase;
};