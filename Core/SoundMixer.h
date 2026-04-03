#pragma once
#include "stdafx.h"
#include "EmulationSettings.h"
#include "../Utilities/LowPassFilter.h"
#include "../Utilities/blip_buf.h"
#include "../Utilities/SimpleLock.h"
#include "IAudioDevice.h"
#include "Snapshotable.h"
#include "StereoPanningFilter.h"
#include "StereoDelayFilter.h"
#include "StereoCombFilter.h"
#include "ReverbFilter.h"
#include "CrossFeedFilter.h"

class Console;
class WaveRecorder;
class OggMixer;

namespace orfanidis_eq {
	class freq_grid;
	class eq1;
}

class SoundMixer : public Snapshotable
{
public:
	static constexpr uint32_t CycleLength = 10000;
	static constexpr uint32_t BitsPerSample = 16;

private:
	static constexpr uint32_t MaxSampleRate = 96000;
	static constexpr uint32_t MaxSamplesPerFrame = MaxSampleRate / 60 * 4 * 2; //x4 to allow CPU overclocking up to 10x, x2 for panning stereo
	static constexpr uint32_t MaxChannelCount = 13;
	static constexpr double squareSumFactor[31] = { 1.0, 1.352456, 1.336216, 1.320361, 1.304879, 1.289755, 1.274978, 1.260535, 1.246416, 1.232610, 1.219107, 1.205896, 1.192968, 1.180314, 1.167927, 1.155796, 1.143915, 1.132276, 1.120871, 1.109693, 1.098737, 1.087994, 1.077460, 1.067127, 1.056991, 1.047046, 1.037286, 1.027706, 1.018302, 1.009068, 1.0 };

	IAudioDevice* _audioDevice;
	EmulationSettings* _settings;
	shared_ptr<WaveRecorder> _waveRecorder;
	double _fadeRatio;
	uint32_t _muteFrameCount;
	unique_ptr<OggMixer> _oggMixer;
	
	unique_ptr<orfanidis_eq::freq_grid> _eqFrequencyGrid;
	unique_ptr<orfanidis_eq::eq1> _equalizerLeft;
	unique_ptr<orfanidis_eq::eq1> _equalizerRight;
	shared_ptr<Console> _console;

	CrossFeedFilter _crossFeedFilter;
	LowPassFilter _lowPassFilter;
	StereoPanningFilter _stereoPanning;
	StereoDelayFilter _stereoDelay;
	StereoCombFilter _stereoCombFilter;
	ReverbFilter _reverbFilter;

	int16_t _previousOutputLeft = 0;
	int16_t _previousOutputRight = 0;

	double _rateAdjustment = 1.0;
	int32_t _underTarget = 0;

	vector<uint32_t> _timestamps;
	int16_t _channelOutput[MaxChannelCount][CycleLength];
	int16_t _currentOutput[MaxChannelCount];
	uint8_t _squareVolume[2];

	blip_t* _blipBufLeft;
	blip_t* _blipBufRight;
	int16_t *_outputBuffer;
	double _volumes[MaxChannelCount];
	double _panning[MaxChannelCount];

	NesModel _model;
	uint32_t _sampleRate;
	uint32_t _clockRate;

	bool _hasPanning;

	double _previousTargetRate;

	double GetChannelOutput(AudioChannel channel, bool forRightChannel);
	int16_t GetOutputVolume(bool forRightChannel);
	void EndFrame(uint32_t time);

	void UpdateRates(bool forceUpdate);
	
	void UpdateEqualizers(bool forceUpdate);
	void ApplyEqualizer(orfanidis_eq::eq1* equalizer, size_t sampleCount);
	
	double GetTargetRateAdjustment();
	void UpdateTargetSampleRate();

protected:
	virtual void StreamState(bool saving) override;

public:
	SoundMixer(shared_ptr<Console> console);
	~SoundMixer();

	void SetNesModel(NesModel model);
	void Reset();
	
	void PlayAudioBuffer(uint32_t cycle);
	void AddDelta(AudioChannel channel, uint32_t time, int16_t delta);
	void RawVolume(AudioChannel channel, uint8_t volume);

	void StartRecording(string filepath);
	void StopRecording();
	bool IsRecording();

	//For NSF/NSFe
	uint32_t GetMuteFrameCount();
	void ResetMuteFrameCount();
	void SetFadeRatio(double fadeRatio);

	void StopAudio(bool clearBuffer = false);
	void RegisterAudioDevice(IAudioDevice *audioDevice);

	OggMixer* GetOggMixer();

	AudioStatistics GetStatistics();
	void ProcessEndOfFrame();
	double GetRateAdjustment();
};
