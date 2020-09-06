#pragma once


#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>


#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dwmapi.h>


#include "Animation.h"
#include "SpacecraftSound.h"

/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/
#define SafeRelease(p) { if(p) { (p)->Release(); (p)=NULL; } }

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif

class GameMath
{
public:
	//각도를 라디안으로 변환한다
	static float DegreeToRadian(float degree);

	//라디안을 각도로 변환한다
	static float RadianToDegree(float radian);

	//(inX, inY)에서 inDistance만큼 떨어진 랜덤한 위치 (outX,outY)를 계산한다
	static void RandomPosition(float inX, float inY, float inDistance, float *outX, float *outY);

	//(x, y)에서 (targetX, targetY)의 방향을 라디안으로 계산한다
	static float TargetDirection(float x, float y, float targetX, float targetY);

	//(x1,y1)위치의 반지름 r1인 원과 (x2, y2)위치의 반지름 r2인 원이 충돌하는지 계산하고 충돌하면 true를 반환한다
	static bool IntersectionCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2);
};

class D2DUtility
{
public:
	//파일을 읽어 비트맵을 만든다
	static HRESULT LoadBitmapFromFile(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR filePath,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap
	);
};

class Vector
{
public:
	float x, y;
};

class Spacecraft
{
public:
	Spacecraft();

	//초기화 함수
	void Initalize(float x = 0, float y = 0, float mx = 0, float my = 0, float direction = 0, float rotation = 0, float acceleration = 0);
	void Update();

	//direction 방향으로 가속한다
	void Accelerate();

	//감속한다
	void Deceleration();

	//멈추게 한다. 다른 입력이 있으면 취소된다
	void StopMoving();

	//direction을 왼쪽으로 회전한다
	void Rotate(float rotate = 0.0f);


	//멤버에 대한 Get함수들
	Vector GetCoord();
	Vector GetMove();
	float GetDirection();
	float GetAcceleration();

	//멤버에 대한 Set함수들
	void SetCoord(Vector coord);
	void SetMove(Vector move);
	void SetDirection(float direction);
	void SetAcceleration(float acceleration);

private:
	// 우주선 회전을 0이 되게 만든다
	void StoppingZeroizeRotation()
	{

	}
	// 우주선 움직임을 0이 되게 만든다
	void StoppingAcceleration()
	{

	}

private:
	Vector m_coord; //우주선 위치
	Vector m_move; //우주선 움직임
	float m_direction; //우주선 방향
	float m_rotation; //우주선 회전
	float m_acceleration; //우주선 가속

	const float M_RotationForce = 0.003f;
	const float M_AccelerationForce = 0.005f;
	const float M_RotationForceLimit = M_PI / 3;
	const float M_AccelerationForceLimit = 1;

	bool m_isStopping; //우주선 정지
	int m_stoppingPhase; //정지 단계
	int m_stoppingRotationDirection; //우주선 회전방향 -1:왼쪽 1:오른쪽
	float m_stoppingStartDirection; //역방향 계산 시작 방향
	float m_stoppingHalfDirection; //역방향 계산 중간 방향
	float m_stoppingEndDirection; //역방향 계산 끝 방향
	float m_stoppingHalfSpeed; //역추진 속도의 반
};

/*class Spacecraft
{
public:
	Spacecraft();

	//초기화 함수
	void Initalize(float x = 0, float y = 0, float direction = 0, float speed = 0);
	void Update();

	//direction 방향으로 가속한다
	void Accelerate();
	
	//감속한다
	void Deceleration();

	//멈추게 한다. 다른 입력이 있으면 취소된다
	void StopMoving();

	//direction을 왼쪽으로 회전한다
	void RotateLeft();

	//direction을 오른쪽으로 회전한다
	void RotateRight();

	//멤버에 대한 Get함수들
	float GetX();
	float GetY();
	float GetDirection();
	float GetSpeed();

	//멤버에 대한 Set함수들
	void SetX(float x);
	void SetY(float y);
	void SetDirection(float direction);
	void SetSpeed(float speed);

private:
	float m_x, m_y;
	float m_direction;
	float m_speed;

	bool m_isStopping;
};*/

class Comet
{
public:
	Comet();

	void Initalize(float x = 0, float y = 0, float direction = 0, float speed = 0);
	
	void Update();

	//멤버에 대한 Get함수들
	float GetX();
	float GetY();
	float GetDirection();
	float GetSpeed();

	//멤버에 대한 Set함수들
	void SetX(float x);
	void SetY(float y);
	void SetDirection(float direction);
	void SetSpeed(float speed);

private:
	float m_x, m_y;
	float m_direction;
	float m_speed;
};

class SpaceMine
{
public:
	SpaceMine();

	void Initalize(float x = 0, float y = 0, float direction = 0, float speed = 0);

	void Update();

	//멤버에 대한 Get함수들
	float GetX();
	float GetY();
	float GetDirection();
	float GetSpeed();
	bool GetShotState();

	//멤버에 대한 Set함수들
	void SetX(float x);
	void SetY(float y);
	void SetDirection(float direction);
	void SetSpeed(float speed);
	void SetShotState(bool shotState);

private:
	float m_x, m_y;
	float m_direction;
	float m_speed;

	bool m_shotState;
};

class GameManager
{
public:
	//생성자
	GameManager();
	//소멸자
	~GameManager();

	//초기화
	HRESULT OnInitialize(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory, HWND hWnd);
	
	//게임 데이터 갱신
	HRESULT OnUpdate();
	
	//그리기
	HRESULT OnRender(ID2D1RenderTarget *renderTarget);

	//DiscardResources를 호출한다
	void Release();

private:
	//게임 관련 멤버를 초기화한다
	HRESULT InitializeGame();

	//리소스 관련 멤버를 초기화한다
	HRESULT InitializeResources(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory);
	
	//리소스들을 해제한다
	void DiscardResources();

	//사운드를 초기화한다
	void InitializeSound(HWND hWnd);
	
	//사운드를 해제한다
	void ReleaseSound();

	//혜성을 추가한다.
	void AddComet();
	//혜성을 재설정한다
	void ResetComet(Comet *comet);

	//우주 기뢰를 추가한다.
	void AddSpaceMine();
	//우주 기뢰를 재설정한다.
	void ResetSpaceMine(SpaceMine *spaceMine);

	//플레이어 스킬을 사용한다
	void PlayerSkill();
	//플레이어 기본공격
	void PlayerAttack();

private:
	//게임 데이터
	bool m_gameOver = false;
	int m_score = 0;
	Spacecraft m_player;
	int m_playerSkillPoint;
	std::vector<Comet> m_comets;
	std::vector<SpaceMine> m_spaceMines;
	EaseOutExponentialAnimation <double> m_gaugeBarAnimation;

	//D2D 리소스
	//우주선 경로 기하, 우주선 머리,몸체,꼬리 부분 각각의 채우기 브러시, 외곽선 브러시
	ID2D1PathGeometry *m_spacecraftHeadPathGeometry;
	ID2D1PathGeometry *m_spacecraftBodyPathGeometry;
	ID2D1PathGeometry *m_spacecraftTailPathGeometry;
	ID2D1SolidColorBrush *m_spacecraftHeadFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spacecraftBodyFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spacecraftTailFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spacecraftEdgeSolidColorBrush;

	//혜성 경로 기하, 채우기 브러시, 외곽선 브러시
	ID2D1PathGeometry *m_cometPathGeometry;
	ID2D1SolidColorBrush *m_cometFillSolidColorBrush;
	ID2D1SolidColorBrush *m_cometEdgeSolidColorBrush;

	//스킬 경로 기하, 채우기 브러시, 외곽선 브러시
	ID2D1PathGeometry *m_skillPathGeometry;
	ID2D1SolidColorBrush *m_skillFillSolidColorBrush;
	ID2D1SolidColorBrush *m_skillEdgeSolidColorBrush;

	//우주 기뢰 경로 기하, 채우기 브러시, 외곽선 브러시
	ID2D1PathGeometry *m_spaceMinePathGeometry;
	ID2D1SolidColorBrush *m_spaceMineFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spaceMineEdgeSolidColorBrush;

	//배경 비트맵브러시
	ID2D1BitmapBrush *m_backgroundBitmapBrush;
	ID2D1BitmapBrush *m_cometBirmapBrush;

	//우주선 엔진 불꽃의 컬러 비트맵 브러시, 마스크 비트맵
	ID2D1BitmapBrush *m_engineFireColorBitmapBrush;
	ID2D1Bitmap *m_engineFireMaskBitmap;

	//텍스트 포맷
	IDWriteTextFormat *m_playerTextFormat;
	IDWriteTextFormat *m_gameOverTextFormat;

	//사운드
	SpacecraftSount *m_sound;
};