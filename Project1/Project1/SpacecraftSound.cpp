#include "SpacecraftSound.h"



	SpacecraftSount::SpacecraftSount(HWND hWnd)
	{
		init(hWnd);
	}
	SpacecraftSount::~SpacecraftSount()
	{
		release();
	}

	void SpacecraftSount::PlayThrusterSound()
	{
		soundManager->play(SL_THRUSTER, TRUE);
	}
	void SpacecraftSount::PlayChargedSound()
	{
		soundManager->play(SL_CHARGED, FALSE);
	}
	void SpacecraftSount::PlaySkillSound()
	{
		soundManager->play(SL_SKILL, FALSE);

	}
	void SpacecraftSount::PlayTargetingSound()
	{
		soundManager->play(SL_TARGETING, FALSE);
	}

	void SpacecraftSount::StopThrusterSound()
	{
		soundManager->stop(SL_THRUSTER);
	}

	bool SpacecraftSount::init(HWND hWnd)
	{
		soundManager = new CSoundManager;

		if (!soundManager->init(hWnd))
			return FALSE;

		// 사운드 파일을 추가함.
		int id;
		wchar_t thrusterFile[] = L"Thruster.wav";
		wchar_t chargedFile[] = L"Charged.wav";
		wchar_t skillFile[] = L"Skill.wav";
		wchar_t targetingFile[] = L"Targeting.wav";
		if (!soundManager->add(thrusterFile, &id)) //id=0부터 시작함.
			return FALSE;
		if (!soundManager->add(chargedFile, &id))
			return FALSE;
		if (!soundManager->add(skillFile, &id))
			return FALSE;
		if (!soundManager->add(targetingFile, &id))
			return FALSE;

		masterVolume = 100;
		muteState = false;

		return true;
	}

	void SpacecraftSount::release()
	{
		if (soundManager != NULL)
		{
			soundManager->release();
			soundManager = NULL;
		}
	}


	void SpacecraftSount::SetMasterVolume(int volume)
	{
		//볼륨을 설정하고 유효 범위로 맞춘다
		masterVolume = volume;
		if (masterVolume < 0)
			masterVolume = 0;
		else if (masterVolume > 100)
			masterVolume = 100;

		//마스터 볼륨을 DirectSound에서의 값으로 계산한다.
		int soundManagerVolume = (int)floorf(2000.0f * log10f((float)(masterVolume) / (float)100) + 0.5f);

		//각 사운드에 대해 볼륨을 적용한다
		soundManager->SetVolume(SL_THRUSTER, soundManagerVolume);
		soundManager->SetVolume(SL_CHARGED, soundManagerVolume);
		soundManager->SetVolume(SL_SKILL, soundManagerVolume);
		soundManager->SetVolume(SL_TARGETING, soundManagerVolume);
	}
	int SpacecraftSount::GetMasterVolume()
	{
		//마스터 볼륨을 반환한다
		return masterVolume;
	}
	void SpacecraftSount::IncreaseMasterVolume(int increase)
	{
		//마스터 볼룸을 increase(=5)만큼 증가시킨다
		SetMasterVolume(masterVolume + increase);
	}
	void SpacecraftSount::DecreaseMasterVolume(int decrease)
	{
		//마스터 볼륨을 decrease(=5)만큼 감소키신다.
		SetMasterVolume(masterVolume - decrease);
	}

	void SpacecraftSount::ToggleMute()
	{
		//음소거되지 않았을 때 모든 사운드 볼륨을 0으로 바꾸고 muteState도 바꾼다 
		if (muteState == false)
		{
			//각 사운드에 대해 음소거를 처리한다
			soundManager->SetVolume(SL_THRUSTER, -10000);
			soundManager->SetVolume(SL_CHARGED, -10000);
			soundManager->SetVolume(SL_SKILL, -10000);
			soundManager->SetVolume(SL_TARGETING, -10000);

			muteState = true;
		}
		//음소거되었을 때 모든 사운드 볼륨을 마스터 볼륨으로 바꾸고 musteState도 바꾼다
		else
		{
			//masterVolume을 이용해서 간단하게 원래 볼륨으로 변경한다.
			SetMasterVolume(masterVolume);

			muteState = false;
		}
	}

	bool SpacecraftSount::GetMuteState()
	{
		return muteState;
	}