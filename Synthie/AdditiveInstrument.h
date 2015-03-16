#pragma once
#include "Instrument.h"
#include "CustomWave.h"

using namespace std;

class CAdditiveInstrument :
	public CInstrument
{
public:
	CAdditiveInstrument();
	virtual ~CAdditiveInstrument();

	virtual void Start() override;
	virtual bool Generate() override;
	virtual void SetNote(CNote *note, double secPerBeat) override;

	void SetFreq(double f) { mCustomWave.SetFreq(f); }

	void SetAmplitude(double a) { mCustomWave.SetAmplitude(a); }

	void SetDuration(double d) { mDuration = d; }

	void AddHarmonics(std::wstring harmonics);

private:
	double mDuration;
	double mTime;

	CCustomWave mCustomWave;
};
