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

		// ���� ������ �߰���.
		int id;
		wchar_t thrusterFile[] = L"Thruster.wav";
		wchar_t chargedFile[] = L"Charged.wav";
		wchar_t skillFile[] = L"Skill.wav";
		wchar_t targetingFile[] = L"Targeting.wav";
		if (!soundManager->add(thrusterFile, &id)) //id=0���� ������.
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
		//������ �����ϰ� ��ȿ ������ �����
		masterVolume = volume;
		if (masterVolume < 0)
			masterVolume = 0;
		else if (masterVolume > 100)
			masterVolume = 100;

		//������ ������ DirectSound������ ������ ����Ѵ�.
		int soundManagerVolume = (int)floorf(2000.0f * log10f((float)(masterVolume) / (float)100) + 0.5f);

		//�� ���忡 ���� ������ �����Ѵ�
		soundManager->SetVolume(SL_THRUSTER, soundManagerVolume);
		soundManager->SetVolume(SL_CHARGED, soundManagerVolume);
		soundManager->SetVolume(SL_SKILL, soundManagerVolume);
		soundManager->SetVolume(SL_TARGETING, soundManagerVolume);
	}
	int SpacecraftSount::GetMasterVolume()
	{
		//������ ������ ��ȯ�Ѵ�
		return masterVolume;
	}
	void SpacecraftSount::IncreaseMasterVolume(int increase)
	{
		//������ ������ increase(=5)��ŭ ������Ų��
		SetMasterVolume(masterVolume + increase);
	}
	void SpacecraftSount::DecreaseMasterVolume(int decrease)
	{
		//������ ������ decrease(=5)��ŭ ����Ű�Ŵ�.
		SetMasterVolume(masterVolume - decrease);
	}

	void SpacecraftSount::ToggleMute()
	{
		//���Ұŵ��� �ʾ��� �� ��� ���� ������ 0���� �ٲٰ� muteState�� �ٲ۴� 
		if (muteState == false)
		{
			//�� ���忡 ���� ���ҰŸ� ó���Ѵ�
			soundManager->SetVolume(SL_THRUSTER, -10000);
			soundManager->SetVolume(SL_CHARGED, -10000);
			soundManager->SetVolume(SL_SKILL, -10000);
			soundManager->SetVolume(SL_TARGETING, -10000);

			muteState = true;
		}
		//���ҰŵǾ��� �� ��� ���� ������ ������ �������� �ٲٰ� musteState�� �ٲ۴�
		else
		{
			//masterVolume�� �̿��ؼ� �����ϰ� ���� �������� �����Ѵ�.
			SetMasterVolume(masterVolume);

			muteState = false;
		}
	}

	bool SpacecraftSount::GetMuteState()
	{
		return muteState;
	}