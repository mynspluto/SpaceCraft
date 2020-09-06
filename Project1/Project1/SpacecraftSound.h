#pragma once

#include "SoundManager.h"

class SpacecraftSount
{
	

public:
	enum SoundList{ SL_THRUSTER, SL_CHARGED, SL_SKILL, SL_TARGETING };
	
	SpacecraftSount(HWND hWnd);
	~SpacecraftSount();

	void PlayThrusterSound();
	void PlayChargedSound();
	void PlaySkillSound();
	void PlayTargetingSound();

	void StopThrusterSound();

	void SetMasterVolume(int volume );
	int GetMasterVolume();
	void IncreaseMasterVolume(int increase = 5 );
	void DecreaseMasterVolume(int decrease = 5 );
	void ToggleMute();
	bool GetMuteState();
private:
	bool init(HWND hWnd);

	void release();

	CSoundManager * soundManager = NULL;
	int masterVolume = 100;
	bool muteState = false;
};