#include "stdafx.h"
#include "ToneInstrument.h"
#include "Note.h"
#include "Notes.h"
#include "Envelope.h"
#include "ADSREnvelope.h"



CToneInstrument::CToneInstrument()
{
	mDuration = 0.1;
}


CToneInstrument::~CToneInstrument()
{
}

void CToneInstrument::Start()
{
	mSinewave.SetSampleRate(GetSampleRate());
	mSinewave.Start();
	mTime = 0;

	mEnvelope = new CADSREnvelope();

	// Tell the AR object it gets its samples from 
	// the sine wave object.
	mAmplitudeFilter.SetEnvelope(mEnvelope);
	mAmplitudeFilter.SetSource(&mSinewave);
	mAmplitudeFilter.SetSampleRate(GetSampleRate());
	mAmplitudeFilter.SetDuration(mDuration);
	mAmplitudeFilter.Start();
}


bool CToneInstrument::Generate()
{
	// Call generate on the envelope here!! Instead of in a filter
	mEnvelope->Generate();

	mSinewave.Generate();

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

void CToneInstrument::SetNote(CNote* note, double secPerBeat)
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
	}
}
