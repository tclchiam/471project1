#include "stdafx.h"
#include "SubtractiveInstrument.h"
#include "Note.h"
#include "Notes.h"
#include "Envelope.h"

CSubtractiveInstrument::CSubtractiveInstrument()
{
	mDuration = 0.1;
	mResonFilter = false;
	mFilterEnvelope = false;
	mADSREnvelope = false;
}


CSubtractiveInstrument::~CSubtractiveInstrument()
{
}

void CSubtractiveInstrument::Start()
{
	mTime = 0;
	if (mWaveform == Sawtooth)
	{
		mSawtooth.SetSampleRate(GetSampleRate());
		mSawtooth.Start();
		if (mResonFilter && mFilterEnvelope)
		{
			mReson.SetSource(&mSawtooth);
			mPitchFilter.SetSource(&mReson);
			mAmplitudeFilter.SetSource(&mAmplitudeFilter);
		}
		else if (mResonFilter)
		{
			mReson.SetSource(&mSawtooth);
			mAmplitudeFilter.SetSource(&mReson);
		}
		else if (mFilterEnvelope)
		{
			mPitchFilter.SetSource(&mSawtooth);
			mAmplitudeFilter.SetSource(&mPitchFilter);
		}
		else
		{
			mAmplitudeFilter.SetSource(&mSawtooth);
		}

	}
	else if (mWaveform == Triangle)
	{
		mTriangle.SetSampleRate(GetSampleRate());
		mTriangle.Start();
		if (mResonFilter && mFilterEnvelope)
		{
			mReson.SetSource(&mTriangle);
			mPitchFilter.SetSource(&mReson);
			mAmplitudeFilter.SetSource(&mAmplitudeFilter);
		}
		else if (mResonFilter)
		{
			mReson.SetSource(&mTriangle);
			mAmplitudeFilter.SetSource(&mPitchFilter);
		}
		else if (mFilterEnvelope)
		{
			mPitchFilter.SetSource(&mTriangle);
			mAmplitudeFilter.SetSource(&mPitchFilter);
		}
		else
		{
			mAmplitudeFilter.SetSource(&mTriangle);
		}
	}
	else if (mWaveform == Square)
	{
		mSquare.SetSampleRate(GetSampleRate());
		mSquare.Start();
		if (mResonFilter && mFilterEnvelope)
		{
			mReson.SetSource(&mSquare);
			mPitchFilter.SetSource(&mReson);
			mAmplitudeFilter.SetSource(&mAmplitudeFilter);
		}
		else if (mResonFilter)
		{
			mReson.SetSource(&mSquare);
			mAmplitudeFilter.SetSource(&mReson);
		}
		else if (mFilterEnvelope)
		{
			mPitchFilter.SetSource(&mSquare);
			mAmplitudeFilter.SetSource(&mPitchFilter);
		}
		else
		{
			mAmplitudeFilter.SetSource(&mSquare);
		}
		
	}
	else
	{
		mSinewave.SetSampleRate(GetSampleRate());
		mSinewave.Start();
		if (mResonFilter && mFilterEnvelope)
		{
			mReson.SetSource(&mSinewave);
			mPitchFilter.SetSource(&mReson);
			mAmplitudeFilter.SetSource(&mAmplitudeFilter);
		}
		else if (mResonFilter)
		{
			mReson.SetSource(&mSinewave);
			mAmplitudeFilter.SetSource(&mReson);
		}
		else if (mFilterEnvelope)
		{
			mPitchFilter.SetSource(&mSinewave);
			mAmplitudeFilter.SetSource(&mPitchFilter);
		}
		else
		{
			mAmplitudeFilter.SetSource(&mSinewave);
		}
	}

	mEnvelope = new CADSREnvelope();
	
	if (mResonFilter)
	{
		ResonFilterSetup();
	}
	if (mFilterEnvelope)
	{
		mPitchFilter.SetEnvelope(mEnvelope);
		mPitchFilter.SetSampleRate(GetSampleRate());
		mPitchFilter.SetDuration(mDuration);
		mPitchFilter.Start();
	}

	mAmplitudeFilter.SetEnvelope(mEnvelope);
	mAmplitudeFilter.SetSampleRate(GetSampleRate());
	mAmplitudeFilter.SetDuration(mDuration);
	mAmplitudeFilter.Start();
	
}

bool CSubtractiveInstrument::Generate()
{
	// Call generate on the envelope here!! Instead of in a filter
	mEnvelope->Generate();
	if (mFilterEnvelope)
	{
		mPitchFilter.Generate();

	}
	else 
	{
		if (mWaveform == Sawtooth)
		{
			mSawtooth.Generate();
		}
		else if (mWaveform == Triangle)
		{
			mTriangle.Generate();
		}
		else if (mWaveform == Square)
		{
			mSquare.Generate();
		}
		else
		{
			mSinewave.Generate();
		}

		if (mResonFilter)
		{
			mReson.Generate();
		}
	}
	auto valid = mAmplitudeFilter.Generate();
	// Read the component's sample and make it our resulting frame.
	mFrame[0] = mAmplitudeFilter.Frame(0);
	mFrame[1] = mAmplitudeFilter.Frame(1);
	// Update time
	mTime += GetSamplePeriod();
	// We return true until the time reaches the duration.
	return valid;
}

void CSubtractiveInstrument::SetNote(CNote* note, double secPerBeat)
{
	// Get a list of all attribute nodes and the
	// length of that list
	CComPtr<IXMLDOMNamedNodeMap> attributes;
	note->Node()->get_attributes(&attributes);
	long len;
	attributes->get_length(&len);
	
	StringToWaveform(note->Waveform());

	// Loop over the list of attributes
	for (int i = 0; i < len; i++)
	{
		// Get attribute i
		CComPtr<IXMLDOMNode> attrib;
		attributes->get_item(i, &attrib);

		// Get the name of the attribute
		CComBSTR name;
		attrib->get_nodeName(&name);

		// Get the value of the attribute.  A CComVariant is a variable
		// that can have any type. It loads the attribute value as a
		// string (UNICODE), but we can then change it to an integer 
		// (VT_I4) or double (VT_R8) using the ChangeType function 
		// and then read its integer or double value from a member variable.
		CComVariant value;
		attrib->get_nodeValue(&value);

		if (name == "duration")
		{
			value.ChangeType(VT_R8);
			// number of beats * seconds per beat = seconds for note
			SetDuration(value.dblVal * secPerBeat);
		}
		else if (name == "note")
		{
			SetFreq(NoteToFrequency(value.bstrVal));
		}

		if (name == "resonfrequency")
		{
			mResonFilter = true;
			value.ChangeType(VT_R8);
			mResonFrequency = value.dblVal;
		}

		if (name == "resonbandwidth")
		{
			value.ChangeType(VT_R8);
			mResonBandwidth = value.dblVal;
		}

		if (name == "filter-envelope")
		{
			mFilterEnvelope = true;
		}
	}
}

void CSubtractiveInstrument::StringToWaveform(std::wstring waveform)
{
	if (waveform == L"sawtooth")
	{
		mWaveform = Sawtooth;
	}
	else if (waveform == L"triangle")
	{
		mWaveform = Triangle;
	}
	else if (waveform == L"square")
	{
		mWaveform = Square;
	}
}

void CSubtractiveInstrument::SetFreq(double f)
{
	mSinewave.SetFreq(f);
	mSawtooth.SetFreq(f);
	mTriangle.SetFreq(f);
	mSquare.SetFreq(f);
}

void CSubtractiveInstrument::SetAmplitude(double a)
{
	mSinewave.SetAmplitude(a);
	mSawtooth.SetAmplitude(a);
	mTriangle.SetAmplitude(a);
	mSquare.SetAmplitude(a);
}

void CSubtractiveInstrument::ResonFilterSetup()
{
	mReson.SetEnvelope(mEnvelope);
	mReson.SetBandwidth(mResonBandwidth);
	mReson.SetFrequency(mResonFrequency);
	mReson.SetSampleRate(GetSampleRate());
	mReson.SetDuration(mDuration);
	mReson.Start();
}

