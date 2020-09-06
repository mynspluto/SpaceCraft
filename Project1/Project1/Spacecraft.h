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
	//������ �������� ��ȯ�Ѵ�
	static float DegreeToRadian(float degree);

	//������ ������ ��ȯ�Ѵ�
	static float RadianToDegree(float radian);

	//(inX, inY)���� inDistance��ŭ ������ ������ ��ġ (outX,outY)�� ����Ѵ�
	static void RandomPosition(float inX, float inY, float inDistance, float *outX, float *outY);

	//(x, y)���� (targetX, targetY)�� ������ �������� ����Ѵ�
	static float TargetDirection(float x, float y, float targetX, float targetY);

	//(x1,y1)��ġ�� ������ r1�� ���� (x2, y2)��ġ�� ������ r2�� ���� �浹�ϴ��� ����ϰ� �浹�ϸ� true�� ��ȯ�Ѵ�
	static bool IntersectionCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2);
};

class D2DUtility
{
public:
	//������ �о� ��Ʈ���� �����
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

	//�ʱ�ȭ �Լ�
	void Initalize(float x = 0, float y = 0, float mx = 0, float my = 0, float direction = 0, float rotation = 0, float acceleration = 0);
	void Update();

	//direction �������� �����Ѵ�
	void Accelerate();

	//�����Ѵ�
	void Deceleration();

	//���߰� �Ѵ�. �ٸ� �Է��� ������ ��ҵȴ�
	void StopMoving();

	//direction�� �������� ȸ���Ѵ�
	void Rotate(float rotate = 0.0f);


	//����� ���� Get�Լ���
	Vector GetCoord();
	Vector GetMove();
	float GetDirection();
	float GetAcceleration();

	//����� ���� Set�Լ���
	void SetCoord(Vector coord);
	void SetMove(Vector move);
	void SetDirection(float direction);
	void SetAcceleration(float acceleration);

private:
	// ���ּ� ȸ���� 0�� �ǰ� �����
	void StoppingZeroizeRotation()
	{

	}
	// ���ּ� �������� 0�� �ǰ� �����
	void StoppingAcceleration()
	{

	}

private:
	Vector m_coord; //���ּ� ��ġ
	Vector m_move; //���ּ� ������
	float m_direction; //���ּ� ����
	float m_rotation; //���ּ� ȸ��
	float m_acceleration; //���ּ� ����

	const float M_RotationForce = 0.003f;
	const float M_AccelerationForce = 0.005f;
	const float M_RotationForceLimit = M_PI / 3;
	const float M_AccelerationForceLimit = 1;

	bool m_isStopping; //���ּ� ����
	int m_stoppingPhase; //���� �ܰ�
	int m_stoppingRotationDirection; //���ּ� ȸ������ -1:���� 1:������
	float m_stoppingStartDirection; //������ ��� ���� ����
	float m_stoppingHalfDirection; //������ ��� �߰� ����
	float m_stoppingEndDirection; //������ ��� �� ����
	float m_stoppingHalfSpeed; //������ �ӵ��� ��
};

/*class Spacecraft
{
public:
	Spacecraft();

	//�ʱ�ȭ �Լ�
	void Initalize(float x = 0, float y = 0, float direction = 0, float speed = 0);
	void Update();

	//direction �������� �����Ѵ�
	void Accelerate();
	
	//�����Ѵ�
	void Deceleration();

	//���߰� �Ѵ�. �ٸ� �Է��� ������ ��ҵȴ�
	void StopMoving();

	//direction�� �������� ȸ���Ѵ�
	void RotateLeft();

	//direction�� ���������� ȸ���Ѵ�
	void RotateRight();

	//����� ���� Get�Լ���
	float GetX();
	float GetY();
	float GetDirection();
	float GetSpeed();

	//����� ���� Set�Լ���
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

	//����� ���� Get�Լ���
	float GetX();
	float GetY();
	float GetDirection();
	float GetSpeed();

	//����� ���� Set�Լ���
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

	//����� ���� Get�Լ���
	float GetX();
	float GetY();
	float GetDirection();
	float GetSpeed();
	bool GetShotState();

	//����� ���� Set�Լ���
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
	//������
	GameManager();
	//�Ҹ���
	~GameManager();

	//�ʱ�ȭ
	HRESULT OnInitialize(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory, HWND hWnd);
	
	//���� ������ ����
	HRESULT OnUpdate();
	
	//�׸���
	HRESULT OnRender(ID2D1RenderTarget *renderTarget);

	//DiscardResources�� ȣ���Ѵ�
	void Release();

private:
	//���� ���� ����� �ʱ�ȭ�Ѵ�
	HRESULT InitializeGame();

	//���ҽ� ���� ����� �ʱ�ȭ�Ѵ�
	HRESULT InitializeResources(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory);
	
	//���ҽ����� �����Ѵ�
	void DiscardResources();

	//���带 �ʱ�ȭ�Ѵ�
	void InitializeSound(HWND hWnd);
	
	//���带 �����Ѵ�
	void ReleaseSound();

	//������ �߰��Ѵ�.
	void AddComet();
	//������ �缳���Ѵ�
	void ResetComet(Comet *comet);

	//���� ��ڸ� �߰��Ѵ�.
	void AddSpaceMine();
	//���� ��ڸ� �缳���Ѵ�.
	void ResetSpaceMine(SpaceMine *spaceMine);

	//�÷��̾� ��ų�� ����Ѵ�
	void PlayerSkill();
	//�÷��̾� �⺻����
	void PlayerAttack();

private:
	//���� ������
	bool m_gameOver = false;
	int m_score = 0;
	Spacecraft m_player;
	int m_playerSkillPoint;
	std::vector<Comet> m_comets;
	std::vector<SpaceMine> m_spaceMines;
	EaseOutExponentialAnimation <double> m_gaugeBarAnimation;

	//D2D ���ҽ�
	//���ּ� ��� ����, ���ּ� �Ӹ�,��ü,���� �κ� ������ ä��� �귯��, �ܰ��� �귯��
	ID2D1PathGeometry *m_spacecraftHeadPathGeometry;
	ID2D1PathGeometry *m_spacecraftBodyPathGeometry;
	ID2D1PathGeometry *m_spacecraftTailPathGeometry;
	ID2D1SolidColorBrush *m_spacecraftHeadFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spacecraftBodyFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spacecraftTailFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spacecraftEdgeSolidColorBrush;

	//���� ��� ����, ä��� �귯��, �ܰ��� �귯��
	ID2D1PathGeometry *m_cometPathGeometry;
	ID2D1SolidColorBrush *m_cometFillSolidColorBrush;
	ID2D1SolidColorBrush *m_cometEdgeSolidColorBrush;

	//��ų ��� ����, ä��� �귯��, �ܰ��� �귯��
	ID2D1PathGeometry *m_skillPathGeometry;
	ID2D1SolidColorBrush *m_skillFillSolidColorBrush;
	ID2D1SolidColorBrush *m_skillEdgeSolidColorBrush;

	//���� ��� ��� ����, ä��� �귯��, �ܰ��� �귯��
	ID2D1PathGeometry *m_spaceMinePathGeometry;
	ID2D1SolidColorBrush *m_spaceMineFillSolidColorBrush;
	ID2D1SolidColorBrush *m_spaceMineEdgeSolidColorBrush;

	//��� ��Ʈ�ʺ귯��
	ID2D1BitmapBrush *m_backgroundBitmapBrush;
	ID2D1BitmapBrush *m_cometBirmapBrush;

	//���ּ� ���� �Ҳ��� �÷� ��Ʈ�� �귯��, ����ũ ��Ʈ��
	ID2D1BitmapBrush *m_engineFireColorBitmapBrush;
	ID2D1Bitmap *m_engineFireMaskBitmap;

	//�ؽ�Ʈ ����
	IDWriteTextFormat *m_playerTextFormat;
	IDWriteTextFormat *m_gameOverTextFormat;

	//����
	SpacecraftSount *m_sound;
};