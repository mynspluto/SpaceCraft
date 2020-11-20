게임 요약: 우주선으로 운석을 피하는 게임

실행 방법: 비쥬얼 스튜디오에서 sln 파일을 열고, 솔루션 플랫폼을 x86으로 지정한 후 f5


조작키 - 화살표키(이동), z(스킬), x(브레이크), -,+,p(볼륨)
                                                //스킬은 주변의 운석을 없앰



1. 어떤 상황에 소리가 어떤 소리가 나는 지
예제 10soundinputR의 SoundManager.h, SoundManager.cpp를 가져와서 사용했습니다. 
이 사운드 매니저 클래스를 우주선 게임에서 쉽게 사용하기위해 새로운 클래스를 SpacecraftSound.h, SpacecraftSound.cpp에 선언/정의 했고 SpacecraftSound클래스의 Init멤버함수에서 각 효과음 파일을 로드합니다.

1-1. 로켓 쓰러스터 효과음 : 
Thruster.wav는 로켓 쓰러스터 효과음으로 로켓엔진이 가동될 때는 계속 재생됩니다. 반대로 엔진이 가동되지 않으면 정지됩니다. SpacecraftSound클래스의 PlayThrusterSound를 이용해 재생하고, StopThrusterSound로 정지됩니다.
Spacecraft.cpp을 보시면 우주선의 추력이 0보다 큰 경우 계속해서 PlayThrusterSound()를 호출하는데 재생중에 호출되면 처음부터 재생이 아니라 재생 상태를 유지하기 때문에 정상적으로 재생될 수 있습니다.

1-2. 스킬 충전 효과음 :
Charged.wav는 스킬이 충전될 때마다 짧게 한번 재생됩니다. SpacecraftSound클래스의 PlayChargedSound를 이용해 재생하며, Thruster와 달리 반복재생이 아니고 한번만 짧게 재생되기 때문에 Stop 함수는 만들지 않았습니다. 
Spacecraft.cpp을 보시면 플레이어가 혜성으로부터 일정거리(약 600픽셀, 정확히 613픽셀) 떨어지면 혜성을 플레이어 주변에 다시 위치시키게 됩니다. 다시 위치시킴과 동시에 스킬 포인트를 1 증가시키는데 이 스킬포인트가 100이 되면 스킬 충전 효과음을 재생합니다.

1-3. 스킬 사용 효과음 : 
Skill.wav는 스킬을 사용할 때마다 짧게 한번 재생됩니다. SpacecraftSound클래스의 PlaySkillSound를 이용해 재생하며, 마찬가지로 한번만 짧게 재생되기 때문에 Stop함수는 만들지 않았습니다.
Spacecraft.cpp을 보시면 플레이어가 스킬 포인트가 100이상일 때 스킬을 사용하게 되는데 이때 스킬 효과음도 같이 재생됩니다.

1-4. 우주 기뢰 타게팅 효과음 :
Targeting.wav는 우주 기뢰 처리 부분에서 플레이어와 일정거리 (약150픽셀, 정확히 151픽셀) 이내에 위치하게 되면 최초 1회만 재생됩니다. SpacecraftSound 클래스의 PlayTargetingSound를 이용해 한번만 짧게 재생합니다.
 


2. 기뢰가 접근하는 경우 상황 설명
기뢰는 1213픽셀 넘게 떨어지면 플레이어 주변에 새로 위치하게 됩니다
16픽셀 이하로 접근하면 플레이어가 죽게됩니다.
151픽셀 이내일 때 그 순간의 플레이어위치 방향으로 1의 속도로 이동하게 됩니다. 이 동작은 초기화 이후 한번만 처리되고 다시 초기화 되기 전까지는 동작하지 않습니다. 이 동작 때 타게팅 효과음이 재생됩니다



3. 물리 구현한 부분 설명
물리 구현은 플레이어 우주선에만 적용되었습니다. 혜성이나 기뢰도 등속운동을 하지만 너무 간단한 모델이라 물리라고 하기 어렵고, 이번 플레이어의 움직임이 바뀐 것이 조금 더 물리를 적용하려고 변경한 것입니다.
우주선 클래스인 Spacecraft클래스의 Initialize 멤버함수 부분에 인자에 따라 m_coord를 (x,y)로 초기화 하고 있고, m_move는 (mx, my)으로 초기화 하는데 m_move는 움직임 벡터입니다. 이 값이 변하지 않으면 등속운동을 하게 됩니다.
m_move가 변하는 경우는 m_acceleration(가속) 값이 0이 아니면 변하게 됩니다. 가속 중에는 m_direction(우주선이 보는 방향)에 맞게 m_move에 누적됩니다.
회전도 마찬가지로 회전력이 누적되는데 그 변수가 m_rotation입니다. Rotate멤버함수가 호출되면 회전력이 변하게 됩니다.

조금 더 정리를 한다면 m_move는 매 순간마다 m_coord에 누적됩니다. m_move가 변하지 않으면 등속운동이 됩니다. m_move를 변하게하는 요소는 m_direction과 m_acceleration입니다. 특정방향으로 가속하면 누적힘이 변하고 그 힘이 매 순간 위치를 변하게 합니다.
m_rotation은 우주선의 회전에 대한 누적힘입니다. 매 순간 m_direction에 누적됩니다. 어떤 회전력을 가지고 있으면 다른 회전력이 주어지지 않을때 같은 속도로 회전하게 됩니다. 



https://youtu.be/a3lcGnMhvsA?t=138 영화 인터스텔라에서 우주선과 스테이션의 회전속도를 맞추기위해 몇초동안 압축가스같은걸 분사하는데 한순간에 회전력이 누적되지 않아서 그렇고 게임에서의 우주선도 비슷하게 동작합니다.

Spacecraft::Accelerate() 가속
Spacecraft::Deceleration() 감속
Spacecraft::Rotate(float rotate) 회전 (rotate가 -이면 반시계방향 회전, +이면 시계방향 회전)







음소거 및 볼륨조절 과정

 SpacecraftSound 클래스는 예제의 SoundManager.h/cpp를 이용합니다.
이 SoundManager.h/cpp는 네가지 클래스를 정의하고 있는데 볼륨조절을 위해 접근하려면 CSound와 이 객체를 관리하는 CSoundManager 두 클래스만 알면됩니다.
CSoundManager는 초기화를 거쳐서 파일이름으로 사운드파일을 불러오고(add멤버함수) 재생(play멤버함수) 및 정지(stop멤버함수)가 가능합니다. 사운드파일을 불러올 때마다 그 사운드 파일에는 0부터 1씩 증가하는 id값이 매겨집니다.
로켓엔진,스킬차지,스킬사용,기뢰감지 네가지를 순서대로 불러와서 0,1,2,3의 id를 가지고 있습니다.
그리고 제가 작성한 SpacecraftSound클래스는 초기화를 거쳐 각각의 사운드를 재생하고 정지하는 등의 멤버함수를 가지고 있습니다.
여기까지가 볼륨조절이 없을 때의 작업내용입니다.


이 예제에서는 볼륨 조절을 하지 않기 때문에 그런 기능도 빠진 예제였습니다.
DirectSound도 사운드 조절을 하는 기능이 있을텐데 예제에는 없고 어디서 어떻게 해야하는지 알아본 결과 IDirectSoundBuffer에 SetVolume 멤버함수가 있습니다.
그럼 IDirectSoundBuffer가 사용되었는지 코드를 보니 예제의 CSound 클래스가 멤버변수로 가지고 있었습니다. 그렇다면 CSound의 IDirectSoundBuffer에 접근하고 SetVolume을 호출할 수만 있으면 음소거 및 볼륨조절이 가능하겠습니다.
CSoundManager는 각 사운드파일에 대해 CSound라는 클래스를 이용해 관리합니다. 그리고 CSound는 IDirectSoundBuffer를 가지고있습니다.
CSoundManager에 id를 이용해 CSound객체를 얻어내는 접근도 가능하지만 CSoundManager의 play,stop이 id를 이용해 사운드를 관리하고 있어서 비슷한 방식으로 id와 볼륨값을 인자로 받아 처리하는 SetVolume이라는 멤버함수를 CSoundManager에 추가했습니다.
구현한 함수 내용은 아래와 같습니다. id를 입력받아 if문으로 유효한 id인지 확인 후, vector로 관리되는 CSound객체를 id로 접근하고, GetFreeBuffer로 IDirectSoundBuffer에 접근하여 SetVolume을 호출합니다.
주의하실 점은 SetVolume의 인자인 볼륨값은 일반적으로 다루는 0(음소거)~100(최대)의 범위가 아닌 -10000(음소거)~0(최대)의 범위를 가지고 있습니다.
void CSoundManager::SetVolume(int id, int volume)
{
	if (id >= 0 && id < m_soundVector.size())
		m_soundVector[id]->GetFreeBuffer()->SetVolume(volume);
}

설명에 빠진 부분이 있습니다. 이 예제는 볼륨 조절 옵션이 꺼진 상태였습니다.
SoundManger에서 Volume이라는 키워드로 찾기를 한 결과 기능을 찾았습니다.
CSound::Play 정의를 보시면 아래와 같은 코드가 있습니다.
if( m_dwCreationFlags & DSBCAPS_CTRLVOLUME )
{
	pDSB->SetVolume( lVolume );
}
이 코드는 생성당시 옵션으로 DSBCAPS_CTRLVOLUME 옵션이 있어야 볼륨조절이 가능하다는 것입니다. 이 예제에서는 이런 옵션을 주고 있지 않기 때문에 호출될 수 없습니다. 그래서 생성옵션에 이 옵션을 넣어줬습니다.
생성코드는 CSoundManager::add이고 파일이름 다음에 옵션을 넣는곳이 있습니다. 여기에 옵션을 추가했습니다.


Spacecraft 클래스에서 볼륨을 쉽게 변경할 수 있게합니다. 볼륨의 값을 저장하기 위해 masterVolume이라는 멤버변수를 추가했습니다. 그리고 음소거 상태를 저장하기위해 muteState라는 멤버변수도 추가했습니다.
멤버함수는 아래 다섯가지를 추가했습니다.
SetMasterVolume : 마스터 볼륨을 인자로 받아 모든 사운드의 볼륨을 조절하는 함수
int GetMasterVolume : 저장된 마스터 볼륨 값을 반환하는 함수
IncreaseMasterVolume : 볼륨을 조금씩 키울 수 있는 함수
DecreaseMasterVolume : 볼륨을 조금씩 줄일 수있는 함수
ToggleMute : 음소거를 토글하는 함수

그 과정에 SpacecraftSound에 음소거 상태를 얻어오는 GetMuteState를 추가했습니다. 볼륨 부분은 데시벨방식이라서 변환이 필요했습니다.
https://www.gamedev.net/forums/topic/337397-sound-volume-question-directsound/ 를 참고해서 공식가져다가 SpacecraftSount의 SetMasterVolume를 수정하는데 썼습니다.

