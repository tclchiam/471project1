#include "stdafx.h"
#include "DrumsInstrument.h"
#include "Instrument.h"
#include "Note.h"
#include "Notes.h"
#include "Envelope.h"


/*

10 - Waveform playback from tables // Works from mWaveL and mWaveR
20 - Envelope generation ------> // Envelope, Amplitude, and Filter Generation all integrated in Start/Generate
30 - Polyphony ---> Done automatically with project structure and playback
35 - Varying pitch playback from tables ---> 
40 - At least 20 distinct drum sounds
50 - Synthesized drum

*/


CDrumsInstrument::CDrumsInstrument()
{
	mDuration = 0.25;
}


CDrumsInstrument::~CDrumsInstrument()
{
}


void CDrumsInstrument::Start()
{
	mWavPlayer.SetSampleRate(GetSampleRate());
	mWavPlayer.Start();
	mTime = 0;

	if (mResonFilter && mPitchFilter)
	{
		mReson.SetSource(&mWavPlayer);
		mPitch.SetSource(&mReson);
		mAmplitudeFilter.SetSource(&mAmplitudeFilter);
	}
	else if (mResonFilter)
	{
		mReson.SetSource(&mWavPlayer);
		mAmplitudeFilter.SetSource(&mReson);
	}
	else if (mPitchFilter)
	{
		mPitch.SetSource(&mWavPlayer);
		mAmplitudeFilter.SetSource(&mPitch);
	}
	else
	{
		mAmplitudeFilter.SetSource(&mWavPlayer);
	}


	mEnvelope = new CADSREnvelope();

	if (mResonFilter)
	{
		ResonFilterSetup();
	}

	if (mPitchFilter)
	{
		mPitch.SetEnvelope(mEnvelope);
		mPitch.SetSampleRate(GetSampleRate());
		mPitch.SetDuration(mDuration);
		mPitch.Start();
	}

	mAmplitudeFilter.SetEnvelope(mEnvelope);
	mAmplitudeFilter.SetSource(&mWavPlayer);
	mAmplitudeFilter.SetSampleRate(GetSampleRate());
	mAmplitudeFilter.SetDuration(mDuration);
	mAmplitudeFilter.Start();
}

bool CDrumsInstrument::Generate()
{
	// Call generate on the envelope here!! Instead of in a filter
	mEnvelope->Generate();	

	if (mPitchFilter)
	{
		mPitch.Generate();
	}

	if (mResonFilter)
	{
		mReson.Generate();
	}

	mWavPlayer.Generate();

	mAmplitudeFilter.Generate();

	mFrame[0] = mAmplitudeFilter.Frame(0);
	mFrame[1] = mAmplitudeFilter.Frame(1);

	// Update time
	mTime += GetSamplePeriod();
	return mTime < mDuration;
}

//! not implemented
void CDrumsInstrument::SetNote(CNote* note, double secPerBeat)
{
	// Get a list of all attribute nodes and the
	// length of that list
	CComPtr<IXMLDOMNamedNodeMap> attributes;
	note->Node()->get_attributes(&attributes);
	long len;
	attributes->get_length(&len);

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
	    // else  if (name == "note")
		//{
		//	SetFreq(NoteToFrequency(value.bstrVal));
		//}

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
			mPitchFilter = true;
		}
	}
}

void CDrumsInstrument::ResonFilterSetup()
{
	mReson.SetEnvelope(mEnvelope);
	mReson.SetBandwidth(mResonBandwidth);
	mReson.SetFrequency(mResonFrequency);
	mReson.SetSampleRate(GetSampleRate());
	mReson.SetDuration(mDuration);
	mReson.Start();
}