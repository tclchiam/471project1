#include "stdafx.h"
#include "AdditiveInstrument.h"
#include "Note.h"
#include "Notes.h"
#include "Envelope.h"
#include <sstream>
#include "ADSREnvelope.h"

CAdditiveInstrument::CAdditiveInstrument()
{
	mDuration = 0.1;
}

CAdditiveInstrument::~CAdditiveInstrument()
{
}

void CAdditiveInstrument::Start()
{
	mCustomWave.SetSampleRate(GetSampleRate());
	mCustomWave.Start();
	mTime = 0;

	mEnvelope = new CADSREnvelope();
	static_cast<CADSREnvelope*>(mEnvelope)->SetAttack(.05);
	static_cast<CADSREnvelope*>(mEnvelope)->SetRelease(.05);

	// Tell the amplitude filter object it gets its samples from 
	// the sine wave object.
	mAmplitudeFilter.SetEnvelope(mEnvelope);
	mAmplitudeFilter.SetSource(&mCustomWave);
	mAmplitudeFilter.SetSampleRate(GetSampleRate());
	mAmplitudeFilter.SetDuration(mDuration);
	mAmplitudeFilter.Start();
}

bool CAdditiveInstrument::Generate()
{
	// Call generate on the envelope here!! Instead of in a filter
	mEnvelope->Generate();

	// Crossfading
	if (mCustomWave.GetCrossfadeFlag())
	{
		auto crossfadeEndTime = mCustomWave.GetCrossfadeBeginTime() + mCrossfadeDuration;

		if (mTime < mCustomWave.GetCrossfadeBeginTime())
		{
			// Generate a wave from several sinusoids
			mCustomWave.Generate();
		}
		else if (mTime > mCustomWave.GetCrossfadeBeginTime() && mTime < crossfadeEndTime)
		{
			// Generate using a crossfade
			mCustomWave.GenerateCrossfade(mTime, mCrossfadeDuration);
		}
	}
	else
	{
		// Generate a wave from several sinusoids
		mCustomWave.Generate();
	}


	// Tell the component to generate an audio sample
	auto valid = mAmplitudeFilter.Generate();

	// Read the component's sample and make it our resulting frame.
	mFrame[0] = mAmplitudeFilter.Frame(0);
	mFrame[1] = mAmplitudeFilter.Frame(1);

	// Update time
	mTime += GetSamplePeriod();

	// We return true until the time reaches the duration.
	return valid;
}

void CAdditiveInstrument::SetNote(CNote* note, double secPerBeat)
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
		else if (name == "note")
		{
			SetFreq(NoteToFrequency(value.bstrVal));
		}
		else if (name == "harmonics")
		{
			AddHarmonics(value.bstrVal);
		}
		else if (name == "vibrato")
		{
			value.ChangeType(VT_R8);
			mCustomWave.SetVibratoFlag(true);
			mCustomWave.SetVibrato(value.dblVal);
		}
		else if (name == "vibratoRate")
		{
			value.ChangeType(VT_R8);
			mCustomWave.SetVibratoRate(value.dblVal);
		}
		else if (name == "crossfade")
		{
			value.ChangeType(VT_R8);
			mCrossfadeDuration = value.dblVal;
			mCustomWave.SetCrossfadeFlag(true);

			auto beginTime = mDuration - mCrossfadeDuration;
			mCustomWave.SetCrossfadeBeginTime(beginTime);
		}
		else if (name == "crossfadeOverlap")	// This is how many beats our notes overlap. We use this to move the fade forward by that much.
		{
			// OVERLAP IN BEATS
			value.ChangeType(VT_R8);

			auto overlapTime = secPerBeat * value.dblVal;
			auto crossfadeTime = mCustomWave.GetCrossfadeBeginTime();

			mCustomWave.SetCrossfadeBeginTime(crossfadeTime - overlapTime);
		}
	}
}

void CAdditiveInstrument::AddNextNote(CNote* nextNote, double secPerBeat)
{
	/**
		Creates an instrument from a passed note. 
		We can use this instrument to crossfade, by getting a frame.
	**/
	mNextNoteInstrument = new CAdditiveInstrument();

	mNextNoteInstrument->SetSampleRate(GetSampleRate());
	mNextNoteInstrument->SetNote(nextNote, secPerBeat);
	mNextNoteInstrument->Start();

	// Pair the two instruments 'custom waves' so we can interpolate
	mCustomWave.SetPartnerWave(&mNextNoteInstrument->mCustomWave);
}

void CAdditiveInstrument::AddHarmonics(wstring harmonics) 
{
	// Stringstream the value
	wstringstream sstream(harmonics);
	wstring harmonicAmplitude;

	// Reading the harmonics into the harmonics wave
	while (sstream >> harmonicAmplitude) {
		// Index is the harmonic; value is the amplitude
		mCustomWave.AddHarmonic(stod(harmonicAmplitude));
	}
}
