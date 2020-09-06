#include "Spacecraft.h"

	//각도를 라디안으로 변환한다
	float GameMath::DegreeToRadian(float degree)
	{
		return (float)(degree * M_PI / 180.0f);
	}

	//라디안을 각도로 변환한다
	float GameMath::RadianToDegree(float radian)
	{
		return (float)(radian * 180.0f / M_PI);
	}

	//(inX, inY)에서 inDistance만큼 떨어진 랜덤한 위치 (outX,outY)를 계산한다
	void GameMath::RandomPosition(float inX, float inY, float inDistance, float *outX, float *outY)
	{
		float randomRadian = DegreeToRadian((rand() % 3600) / 10.0f);
		*outX = inX + cos(randomRadian) * inDistance;
		*outY = inY + sin(randomRadian) * inDistance;
	}

	//(x, y)에서 (targetX, targetY)의 방향을 라디안으로 계산한다
	float GameMath::TargetDirection(float x, float y, float targetX, float targetY)
	{
		float dx = targetX - x;
		float dy = targetY - y;

		return (float)(atan2(dy, dx) + M_PI);
	}

	//(x1,y1)위치의 반지름 r1인 원과 (x2, y2)위치의 반지름 r2인 원이 충돌하는지 계산하고 충돌하면 true를 반환한다
	bool GameMath::IntersectionCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		/*
		아래 코드에 비해 상대적으로 느릴 수 있다
		float dx = x2 - x1;
		float dy = y2 - y1;
		float distance = sqrt(dx * dx + dy * dy);

		if (distance <= r1 + r2)
		return true;
		return false;
		*/

		float dx = x2 - x1;
		float dy = y2 - y1;
		float distanceSquare = dx * dx + dy * dy;

		if (distanceSquare <= (r1 + r2) * (r1 + r2))
			return true;
		return false;
	}


	//파일을 읽어 비트맵을 만든다
	HRESULT D2DUtility::LoadBitmapFromFile(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR filePath,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap
	)
	{
		IWICBitmapDecoder *pDecoder = NULL;
		IWICBitmapFrameDecode *pSource = NULL;
		IWICStream *pStream = NULL;
		IWICFormatConverter *pConverter = NULL;
		IWICBitmapScaler *pScaler = NULL;

		HRESULT hr = pIWICFactory->CreateDecoderFromFilename(filePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);

		if (SUCCEEDED(hr))
		{
			// Create the initial frame.
			hr = pDecoder->GetFrame(0, &pSource);
		}
		if (SUCCEEDED(hr))
		{
			// Convert the image format to 32bppPBGRA (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
			hr = pIWICFactory->CreateFormatConverter(&pConverter);
		}

		if (SUCCEEDED(hr))
		{
			// If a new width or height was specified, create an IWICBitmapScaler and use it to resize the image.
			if (destinationWidth != 0 || destinationHeight != 0)
			{
				UINT originalWidth, originalHeight;
				hr = pSource->GetSize(&originalWidth, &originalHeight);
				if (SUCCEEDED(hr))
				{
					if (destinationWidth == 0)
					{
						FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
						destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
					}
					else if (destinationHeight == 0)
					{
						FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
						destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
					}

					hr = pIWICFactory->CreateBitmapScaler(&pScaler);
					if (SUCCEEDED(hr))
					{
						hr = pScaler->Initialize(pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
					}
					if (SUCCEEDED(hr))
					{
						hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
					}
				}
			}
			else // Don't scale the image.
			{
				hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
			}
		}
		if (SUCCEEDED(hr))
		{
			// Create a Direct2D bitmap from the WIC bitmap.
			hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
		}

		SafeRelease(pDecoder);
		SafeRelease(pSource);
		SafeRelease(pStream);
		SafeRelease(pConverter);
		SafeRelease(pScaler);

		return hr;
	}

	Spacecraft::Spacecraft() {}

	//초기화 함수
	void Spacecraft::Initalize(float x, float y, float mx, float my, float direction, float rotation, float acceleration)
	{
		m_coord.x = x;
		m_coord.y = y;
		m_move.x = mx;
		m_move.y = my;
		m_direction = direction;
		m_rotation = rotation;
		m_acceleration = acceleration;

		m_isStopping = false; //우주선 정지
		m_stoppingPhase = 0; //정지 단계
		m_stoppingRotationDirection = 0;
		m_stoppingStartDirection = 0;
		m_stoppingHalfDirection = 0;
		m_stoppingEndDirection = 0;
	}
	
	//플레이어 우주선을 움직인다
	void Spacecraft::Update()
	{
		//멈추려는 경우
		if (m_isStopping)
		{
			//1단계 엔진 가속 끄기
			if (m_stoppingPhase == 0)
			{
				m_acceleration = 0;

				//엔진 가속을 끄고나면 2단계로 넘어간다
				m_stoppingPhase = 1;
			}
			//2단계 회전관성 줄이기
			else if (m_stoppingPhase == 1)
			{
				//오른쪽으로 돌 때
				if (m_rotation > 0)
				{
					if (m_rotation > M_RotationForce)
						m_rotation -= M_RotationForce;
					else
						m_rotation = 0;
				}
				//왼쪽으로 돌 때
				else if (m_rotation < 0)
				{
					if (m_rotation < -M_RotationForce)
						m_rotation += M_RotationForce;
					else
						m_rotation = 0;
				}
				//회전 관성이 0이되면 3단계로 넘어간다
				else if(m_rotation == 0)
					m_stoppingPhase = 2;
			}
			//3단계 움직임 역방향 계산
			else if (m_stoppingPhase == 2)
			{
				//역방향 회전 시작 방향 저장 
				m_stoppingStartDirection = m_direction;

				//역방향 회전 끝 방향 계산
				m_stoppingEndDirection = atan2(-m_move.y, -m_move.x);

				//회전 방향 계산
				m_stoppingRotationDirection = //m_stoppingEndDirection == m_direction ? 0 : 
					sin(m_stoppingEndDirection - m_direction) >= 0 ? 1 : -1;

				//역방향 회전 중간 방향 계산
				Vector startVector = { cos(m_stoppingStartDirection), sin(m_stoppingStartDirection) };
				Vector endVector = { cos(m_stoppingEndDirection), sin(m_stoppingEndDirection) };
				Vector halfVector = { startVector.x + endVector.x, startVector.y + endVector.y };
				float halfVectorLength = sqrt(halfVector.x * halfVector.x + halfVector.y * halfVector.y);
				halfVector.x = halfVector.x / halfVectorLength;
				halfVector.y = halfVector.y / halfVectorLength;
				m_stoppingHalfDirection = atan2(halfVector.y, halfVector.x);

				//계산 후 4단계로 넘어간다
				m_stoppingPhase = 3;
			}
			//4단계 움직임 역방향 잡기
			else if (m_stoppingPhase == 3)
			{
				//역방향이 안잡힐 때
				if (abs( m_stoppingEndDirection - m_direction ) > 0)
				{
					//왼쪽으로 회전해야 할 때
					if (m_stoppingRotationDirection == -1)
					{
						bool over = sin(m_stoppingHalfDirection - m_direction) > 0 ? true : false;
						//회전을 초과했을 때
						if (m_rotation>0)
						{
							m_rotation = 0;
							m_direction = m_stoppingEndDirection;
						}
						//HalfDirection을 넘었을 때
						else if(over)
							m_rotation += M_RotationForce;
						//HalfDirection을 넘지 않았을 때
						else
							m_rotation -= M_RotationForce;
					}
					//오른쪽으로 회전해야 할 때
					else if(m_stoppingRotationDirection == 1)
					{
						bool over = sin(m_stoppingHalfDirection - m_direction) < 0 ? true : false;

						//회전을 초과했을 때
						if (m_rotation<0)
						{
							m_rotation = 0;
							m_direction = m_stoppingEndDirection;
						}
						//HalfDirection을 넘었을 때
						else if (over)
							m_rotation -= M_RotationForce;
						//HalfDirection을 넘지 않았을 때
						else
							m_rotation += M_RotationForce;
					}
				}
				//이번 업데이트에 역방향이 잡힐 때
				else
				{
					m_direction = m_stoppingEndDirection;
					m_rotation = 0;
					//움직임 역방향이 잡히면 5단계로 넘어간다
					m_stoppingPhase = 4;
				}

			}
			//움직임 속도 계산
			else if (m_stoppingPhase == 4)
			{
				m_stoppingHalfSpeed = sqrt(m_move.x * m_move.x + m_move.y * m_move.y)/2;
				m_stoppingPhase = 5;
			}
			//6단계 움직임 속도 줄이기
			else if (m_stoppingPhase == 5)
			{
				float speed = sqrt(m_move.x * m_move.x + m_move.y * m_move.y);

				//움직임 속도가 0이되면 다시 1단계로 넘어간다
				if (speed < M_AccelerationForce)
				{
					m_move.x = 0;
					m_move.y = 0;
					m_acceleration = 0;
					m_isStopping = false;
					m_stoppingPhase = 0;
				}
				else if (m_stoppingHalfSpeed < speed )
				{
					m_acceleration += M_AccelerationForce;

					//속도를 제한
					if (m_acceleration > M_AccelerationForceLimit)
					{
						m_acceleration = M_AccelerationForceLimit;
					}
				}
				else
				{
					m_acceleration -= M_AccelerationForce;

					//속도를 제한
					if (m_acceleration < 0)
					{
						m_acceleration = 0;
					}
				}
			}
		}

		//방향에 회전을 더함
		m_direction += m_rotation;

		//움직임을 계산
		m_move.x += cos(m_direction) * m_acceleration;
		m_move.y += sin(m_direction) * m_acceleration;

		//위치에 움직임을 더함
		m_coord.x += m_move.x;
		m_coord.y += m_move.y;
	}

	//direction 방향으로 가속한다
	void Spacecraft::Accelerate()
	{
		//속도를 올림
		m_acceleration += M_AccelerationForce;

		//속도를 제한
		if (m_acceleration > M_AccelerationForceLimit)
		{
			m_acceleration = M_AccelerationForceLimit;
		}

		//정지를 푼다
		m_isStopping = false;
	}

	//감속한다
	void Spacecraft::Deceleration()
	{
		//속도를 내림
		m_acceleration -= M_AccelerationForce;

		//속도를 제한
		if (m_acceleration < 0)
		{
			m_acceleration = 0;
		}

		//정지를 푼다
		m_isStopping = false;
	}

	//멈추게 한다. 다른 입력이 있으면 취소된다
	void Spacecraft::StopMoving()
	{
		m_isStopping = true;
		m_stoppingPhase = 0;
		m_stoppingRotationDirection = 0;
		m_stoppingStartDirection = 0;
		m_stoppingHalfDirection = 0;
		m_stoppingEndDirection = 0;
	}

	//direction을 rotate만큼 회전한다
	void Spacecraft::Rotate(float rotate)
	{
		m_rotation += rotate;

		//회전 제한
		if (m_rotation > M_RotationForceLimit)
			m_rotation = M_RotationForceLimit;
		else if (m_rotation < -M_RotationForceLimit)
			m_rotation = -M_RotationForceLimit;
		
		m_isStopping = false;
	}


	//멤버에 대한 Get함수들
	Vector Spacecraft::GetCoord() { return m_coord; }
	Vector Spacecraft::GetMove() { return m_move; }
	float Spacecraft::GetDirection() { return m_direction; }
	float Spacecraft::GetAcceleration() { return m_acceleration; }

	//멤버에 대한 Set함수들
	void Spacecraft::SetCoord(Vector coord) { m_coord = coord; }
	void Spacecraft::SetMove(Vector move) { m_move = move; }
	void Spacecraft::SetDirection(float direction) { m_direction = direction; }
	void Spacecraft::SetAcceleration(float acceleration) { m_acceleration = acceleration; }

	//혜성 생성자
	Comet::Comet() {}

	//혜성을 초기화한다
	void Comet::Initalize(float x, float y, float direction, float speed)
	{
		m_x = x;
		m_y = y;
		m_direction = direction;
		m_speed = speed;
	}

	//혜성을 움직인다
	void Comet::Update()
	{
		//direction 방향으로 speed만큼 움직인다
		float dx = cos(m_direction);
		float dy = sin(m_direction);

		m_x += dx * m_speed;
		m_y += dy * m_speed;
	}

	//멤버에 대한 Get함수들
	float Comet::GetX() { return m_x; }
	float Comet::GetY() { return m_y; }
	float Comet::GetDirection() { return m_direction; }
	float Comet::GetSpeed() { return m_speed; }

	//멤버에 대한 Set함수들
	void Comet::SetX(float x) { m_x = x; }
	void Comet::SetY(float y) { m_y = y; }
	void Comet::SetDirection(float direction) { m_direction = direction; }
	void Comet::SetSpeed(float speed) { m_speed = speed; }

	SpaceMine::SpaceMine() 
	{}

	void SpaceMine::Initalize(float x, float y, float direction, float speed)
	{
		m_x = x;
		m_y = y;
		m_direction = direction;
		m_speed = speed;

		m_shotState = false;
	}

	void SpaceMine::Update()
	{
		//direction 방향으로 speed만큼 움직인다
		float dx = cos(m_direction);
		float dy = sin(m_direction);

		m_x += dx * m_speed;
		m_y += dy * m_speed;
	}

	//멤버에 대한 Get함수들
	float SpaceMine::GetX()
	{
		return m_x;
	}
	float SpaceMine::GetY()
	{
		return m_y;
	}
	float SpaceMine::GetDirection()
	{
		return m_direction;
	}
	float SpaceMine::GetSpeed()
	{
		return m_speed;
	}
	bool SpaceMine::GetShotState()
	{
		return m_shotState;
	}

	//멤버에 대한 Set함수들
	void SpaceMine::SetX(float x)
	{
		m_x = x;
	}
	void SpaceMine::SetY(float y)
	{
		m_y = y;
	}
	void SpaceMine::SetDirection(float direction)
	{
		m_direction = direction;
	}
	void SpaceMine::SetSpeed(float speed)
	{
		m_speed = speed;
	}
	void SpaceMine::SetShotState(bool shotState)
	{
		m_shotState = shotState;
	}

	//생성자
	GameManager::GameManager() :
		m_spacecraftHeadPathGeometry(NULL),
		m_spacecraftBodyPathGeometry(NULL),
		m_spacecraftTailPathGeometry(NULL),
		m_spacecraftHeadFillSolidColorBrush(NULL),
		m_spacecraftBodyFillSolidColorBrush(NULL),
		m_spacecraftTailFillSolidColorBrush(NULL),
		m_spacecraftEdgeSolidColorBrush(NULL),

		m_cometPathGeometry(NULL),
		m_cometFillSolidColorBrush(NULL),
		m_cometEdgeSolidColorBrush(NULL),

		m_skillPathGeometry(NULL),
		m_skillFillSolidColorBrush(NULL),
		m_skillEdgeSolidColorBrush(NULL),

		m_spaceMinePathGeometry(NULL),
		m_spaceMineFillSolidColorBrush(NULL),
		m_spaceMineEdgeSolidColorBrush(NULL),

		m_backgroundBitmapBrush(NULL),
		m_cometBirmapBrush(NULL),

		m_engineFireColorBitmapBrush(NULL),
		m_engineFireMaskBitmap(NULL),

		m_playerTextFormat(NULL),
		m_gameOverTextFormat(NULL)
	{}
	//소멸자
	GameManager::~GameManager()
	{
		DiscardResources();
	}

	//초기화
	HRESULT GameManager::OnInitialize(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory, HWND hWnd)
	{
		HRESULT hr;

		//게임 관련 멤버를 초기화하는 함수를 호출한다
		hr = InitializeGame();
		if (SUCCEEDED(hr))
		{

		}

		//리소스 관련 멤버를 초기화하는 함수를 호출한다
		hr = InitializeResources(factory, renderTarget, dwriteFactory, wicFactory);
		if (SUCCEEDED(hr))
		{

		}

		InitializeSound(hWnd);

		return hr;
	}

	//게임 데이터 갱신
	HRESULT GameManager::OnUpdate()
	{
		//게임오버 상태일 때 F1을 누르면 게임 재시작
		if (m_gameOver)
		{
			if (GetAsyncKeyState(VK_RETURN) & 0x8000)
			{
				InitializeGame();
			}
			return 0;
		}

		//각 혜성들을 반복자를 이용해 움직인다
		for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
		{
			//각각의 혜성을 움직이게 한다
			iter->Update();

			//반복자를 다음으로 넘긴다
			++iter;
		}

		//각 우주 기뢰들을 반복자를 이용해 움직인다
		for (std::vector<SpaceMine>::iterator iter = m_spaceMines.begin(); iter != m_spaceMines.end(); )
		{
			//각각의 우주 기뢰를 움직이게 한다
			iter->Update();

			//반복자를 다음으로 넘긴다
			++iter;
		}

		//플레이어 우주선을 제어한다
		//각 키입력에 따라 우주선 좌회전, 우회전, 가속, 감속, 정지 함수를 호출한다
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			m_player.Rotate(-0.003f);
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			m_player.Rotate(0.003f);
		if (GetAsyncKeyState(VK_UP) & 0x8000)
			m_player.Accelerate();
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			m_player.Deceleration();
		if (GetAsyncKeyState('X') & 0x8000)
			m_player.StopMoving();
		if (GetAsyncKeyState('Z') & 0x8000)
			PlayerSkill();

		static bool soundToggleKeyPress = false;
		// 음소거 키가 눌러졌을 때
		if (GetAsyncKeyState('P') & 0x8000)
		{
			if (soundToggleKeyPress == false)
			{
				m_sound->ToggleMute();
				//토글 기능을 위해 soundToggleKeyPress값을 true로 바꾼다.
				soundToggleKeyPress = true;
			}
		}
		else
		{
			soundToggleKeyPress = false;
		}

		static int minusWait = 0;
		// -키가 눌러졌을 때
		if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000)
		{
			//minusWait가 0보다 크면 1감소 시킨다
			if (minusWait > 0)
			{
				--minusWait;
			}
			//minusWait가 0이하면 마스터볼륨을 줄이고 minusWait를 20으로 바꾼다
			else
			{
				m_sound->DecreaseMasterVolume();
				minusWait = 20;
			}
		}
		// -키가 눌러져있지 않으면 minusWait를 0으로 바꿔서 다시 눌렀을 때 바로 마스터 볼륨을 줄일 수 있게 한다
		else
		{
			minusWait = 0;
		}

		static int plusWait = 0;
		// +키가 눌러졌을 때
		if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000)
		{
			//plusWait가 0보다 크면 1감소 시킨다
			if (plusWait > 0)
			{
				--plusWait;
			}
			//plusWait가 0이하면 마스터볼륨을 늘리고 plusWait를 20으로 바꾼다
			else
			{
				m_sound->IncreaseMasterVolume();
				plusWait = 20;
			}
		}
		// +키가 눌러져있지 않으면 plusWait를 0으로 바꿔서 다시 눌렀을 때 바로 마스터 볼륨을 늘릴 수 있게 한다
		else
		{
			plusWait = 0;
		}

		//플레이어 우주선을 움직인다
		m_player.Update();
		//로켓 엔진 소리를 재생한다
		if (m_sound != NULL)
		{
			if (m_player.GetAcceleration() > 0)
			{
				m_sound->PlayThrusterSound();
			}
			else
			{
				m_sound->StopThrusterSound();
			}
		}

		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;

		//혜성 충돌 처리를 한다
		for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
		{
			//혜성의 위치를 얻어온다
			float cometX = iter->GetX();
			float cometY = iter->GetY();

			//혜성이 유효 범위를 벗어나면 플레이어 주위에서 다시 초기화하고 플레이어 스킬 포인트도 1 늘린다
			if (!GameMath::IntersectionCircleCircle(cometX, cometY, 13, playerX, playerY, 600))
			{
				ResetComet(&(*iter));

				//플레이어 스킬포인트를 1 늘린다
				if (m_playerSkillPoint < 100)
				{
					++m_playerSkillPoint;
					if (m_playerSkillPoint > 100)
						m_playerSkillPoint = 100;

					if(m_playerSkillPoint == 100)
						m_sound->PlayChargedSound();
				}
			}
			//혜성과 충돌했으면 게임오버 상태로 바꾼다
			else if (GameMath::IntersectionCircleCircle(cometX, cometY, 11, playerX, playerY, 9))
			{
				m_gameOver = true;
			}

			//반복자를 다음으로 넘긴다
			++iter;
		}

		//우주 기뢰 충돌 처리를 한다
		for (std::vector<SpaceMine>::iterator iter = m_spaceMines.begin(); iter != m_spaceMines.end(); )
		{
			//우주 기뢰의 위치를 얻어온다
			float spaceMineX = iter->GetX();
			float spaceMineY = iter->GetY();

			//우주 기뢰가 유효 범위를 벗어나면 다시 초기화한다
			if (!GameMath::IntersectionCircleCircle(spaceMineX, spaceMineY, 13, playerX, playerY, 1200))
			{
				ResetSpaceMine(&(*iter));
			}
			//혜성과 충돌했으면 게임오버 상태로 바꾼다
			else if (GameMath::IntersectionCircleCircle(spaceMineX, spaceMineY, 7, playerX, playerY, 9))
			{
				m_gameOver = true;
			}
			//150이내의 범위에서 감지한다.
			else if (GameMath::IntersectionCircleCircle(spaceMineX, spaceMineY, 1, playerX, playerY, 150))
			{
				if (iter->GetShotState() == false)
				{
					float direction = GameMath::TargetDirection(playerX, playerY, spaceMineX, spaceMineY);
					iter->SetDirection(direction);
					iter->SetSpeed(1.0f);
					iter->SetShotState(true);

					//타게팅 사운드를 재생한다.
					m_sound->PlayTargetingSound();
				}
			}

			//반복자를 다음으로 넘긴다
			++iter;
		}

		//게임오버가 아니면 점수를 올린다
		if (m_gameOver != true)
		{
			m_score++;

			//추가 생성되는 혜성을 500개까지 제한한다
			if (m_score < 30000)
				if (m_score % 60 == 59) // 스코어가 10 오를때마다 혜성이 1개씩 증가한다
				{
					AddComet();
				}
		}

		return 0;
	}

	//그리기
	HRESULT GameManager::OnRender(ID2D1RenderTarget *renderTarget)
	{
		HRESULT hr = 0;

		//렌더타겟의 크기로 화면 크기를 얻는다
		D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();

		//플레이어의 정보를 얻는다
		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;
		float playerDirection = m_player.GetDirection();
		float playerSpeed = m_player.GetAcceleration();
		
		//배경을 그린다
		if (m_backgroundBitmapBrush)
		{
			//플레이어 위치에 맞게 브러시의 행렬을 적용한다
			m_backgroundBitmapBrush->SetTransform(D2D1::Matrix3x2F::Translation(-playerX, -playerY));
			//플레이어 위치가 적용된 브러시로 화면을 그린다
			renderTarget->FillRectangle(D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), m_backgroundBitmapBrush);
		}

		//혜성들을 그린다
		if (m_cometPathGeometry && m_cometFillSolidColorBrush && m_cometEdgeSolidColorBrush)
		{
			m_cometBirmapBrush->SetTransform(D2D1::Matrix3x2F::Translation(100, 100));
			//각 혜성들을 반복자를 이용해 그린다
			for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
			{
				//혜성의 위치를 얻어온다
				float cometX = iter->GetX();
				float cometY = iter->GetY();

				//렌더타겟 행렬에 혜성 위치를 적용한다 
				renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2 - playerX + cometX, renderTargetSize.height / 2 - playerY + cometY));

				//혜성 그린다
				renderTarget->FillGeometry(m_cometPathGeometry, m_cometBirmapBrush);
				renderTarget->DrawGeometry(m_cometPathGeometry, m_cometEdgeSolidColorBrush);

				//반복자를 다음으로 넘긴다
				++iter;
			}
		}


		//우주 기뢰를 그린다
		if (m_spaceMinePathGeometry && m_spaceMineFillSolidColorBrush && m_spaceMineEdgeSolidColorBrush)
		{
			//각 혜성들을 반복자를 이용해 그린다
			for (std::vector<SpaceMine>::iterator iter = m_spaceMines.begin(); iter != m_spaceMines.end(); )
			{
				//우주 기뢰의 위치를 얻어온다
				float spaceMineX = iter->GetX();
				float spaceMineY = iter->GetY();

				//렌더타겟 행렬에 우주 기뢰 위치를 적용한다 
				renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2 - playerX + spaceMineX, renderTargetSize.height / 2 - playerY + spaceMineY));

				//우주기뢰를 그린다
				renderTarget->FillGeometry(m_spaceMinePathGeometry, m_spaceMineFillSolidColorBrush);
				renderTarget->DrawGeometry(m_spaceMinePathGeometry, m_spaceMineEdgeSolidColorBrush);

				//반복자를 다음으로 넘긴다
				++iter;
			}

		}

		//플레이어 우주선을 그린다
		if (m_spacecraftHeadPathGeometry && m_spacecraftBodyPathGeometry && m_spacecraftTailPathGeometry && m_spacecraftHeadFillSolidColorBrush&& m_spacecraftBodyFillSolidColorBrush&& m_spacecraftTailFillSolidColorBrush && m_spacecraftEdgeSolidColorBrush)
		{
			//플레이어의 회전행렬을 만든다
			D2D1_MATRIX_3X2_F rotationMatrix = D2D1::Matrix3x2F::Rotation(GameMath::RadianToDegree(playerDirection) + 90);
			//플레이어 우주선을 화면 중앙에 위치하게 하기위해 화면의 중앙 위치를 계산한다
			D2D1_MATRIX_3X2_F translationMatrix = D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2, renderTargetSize.height / 2);
			//최종적으로 적용될 행렬을 계산한다
			D2D1_MATRIX_3X2_F transform = rotationMatrix * translationMatrix;

			//행렬을 적용하고 플레이어 우주선을 순서대로 그린다
			renderTarget->SetTransform(transform);

			//우주선 엔진 불꽃을 먼저 그린다
			//가속 중일 때만 엔진 불꽃을 그린다
			//이전 안티알리아스 모드를 저장해두고 그린 후 되돌린다
			if (playerSpeed > 0)
			{
				//이전 안티알리아스 모드를 저장한다
				D2D1_ANTIALIAS_MODE oldAAMode = renderTarget->GetAntialiasMode();
				//마스크를 적용해서 그리기 위해 안티알리아스 모드를 설정한다
				renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

				//플레이어의 속도에 따라 이미지를 바꾼다
				int fireLevel;
				if (playerSpeed < 3)
					fireLevel = 0;
				else if (playerSpeed < 6)
					fireLevel = 1;
				else if (playerSpeed < 9)
					fireLevel = 2;
				else
					fireLevel = 3;

				//불꽃 모양을 랜덤하게 한다
				int firePhase = rand() % 4;

				//엔진 불꽃을 불투명 마스크를 이용해서 그린다
				renderTarget->FillOpacityMask(m_engineFireMaskBitmap, m_engineFireColorBitmapBrush, D2D1_OPACITY_MASK_CONTENT_GRAPHICS, D2D1::RectF(-8.0f, 10.0f, 10.0f, 42.0f), D2D1::RectF(32.0f * firePhase, 32.0f * fireLevel, 32.0f * firePhase + 32.0f, 32.0f * fireLevel + 32.0f - 1));

				//저장했던 이전 안티알리아스 모드로 설정한다
				renderTarget->SetAntialiasMode(oldAAMode);
			}

			//우주선을 그린다
			//우주선 몸체 부분을 그린다
			renderTarget->DrawGeometry(m_spacecraftBodyPathGeometry, m_spacecraftEdgeSolidColorBrush);
			renderTarget->FillGeometry(m_spacecraftBodyPathGeometry, m_spacecraftBodyFillSolidColorBrush);
			//우주선 머리 부분을 그린다
			renderTarget->DrawGeometry(m_spacecraftHeadPathGeometry, m_spacecraftEdgeSolidColorBrush);
			renderTarget->FillGeometry(m_spacecraftHeadPathGeometry, m_spacecraftHeadFillSolidColorBrush);
			//우주선 꼬리 부분을 그린다
			renderTarget->DrawGeometry(m_spacecraftTailPathGeometry, m_spacecraftEdgeSolidColorBrush);
			renderTarget->FillGeometry(m_spacecraftTailPathGeometry, m_spacecraftTailFillSolidColorBrush);
		}



		//애니메이션 값을 얻고 갱신한다
		static float float_time = 0.0f;
		float animationValue = m_gaugeBarAnimation.GetValue(float_time);

		if (float_time >= m_gaugeBarAnimation.GetDuration())
			float_time = 0.0f;
		else
			float_time += 0.016f;
		
		//스킬 UI를 그린다
		if (m_skillPathGeometry && m_skillFillSolidColorBrush && m_skillEdgeSolidColorBrush)
		{
			//변환 행렬을 적용하고 반투명하게 틀을 그린다
			renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2, renderTargetSize.height - 50));
			m_skillFillSolidColorBrush->SetOpacity(0.5f);
			renderTarget->FillGeometry(m_skillPathGeometry, m_skillFillSolidColorBrush);
			renderTarget->DrawGeometry(m_skillPathGeometry, m_skillEdgeSolidColorBrush);

			//레이어를 만든다
			ID2D1Layer* pLayer = NULL;
			hr = renderTarget->CreateLayer(NULL, &pLayer);
			if (SUCCEEDED(hr))
			{
				renderTarget->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), m_skillPathGeometry), pLayer);

				//투명를 없앤다
				m_skillFillSolidColorBrush->SetOpacity(1.0f);
				renderTarget->FillRectangle(D2D1::RectF(-55.0f, -5.0f, -55.0f + m_playerSkillPoint / 100.0f * 110.0f, 5.0f), m_skillFillSolidColorBrush);

				//플레이어 스킬 포인트가 가득차지 않았으면 애니메이션 효과를 주고 가득찼으면 애니메이션 효과를 없게한다
				if(m_playerSkillPoint < 100)
					//애니메이션 효과를 준다
					//animationValue를 곱한다
					renderTarget->FillRectangle(D2D1::RectF(-55.0f, -5.0f, -55.0f + m_playerSkillPoint / 100.0f * 110.0f * animationValue / 100.0f, 5.0f), m_skillEdgeSolidColorBrush);
				else
					renderTarget->FillRectangle(D2D1::RectF(-55.0f, -5.0f, -55.0f + m_playerSkillPoint / 100.0f * 110.0f, 5.0f), m_skillEdgeSolidColorBrush);
				renderTarget->PopLayer();

				pLayer->Release();
			}
		}
		
		
		//텍스트를 출력한다
		if (m_playerTextFormat)
		{
			//행렬을 초기화한다
			renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			WCHAR text[100];
			float top = 0;
			//플레이어 위치를 문자열로 만들고 출력한다
			swprintf_s(text, L"coord : (%.2f, %.2f)", playerX, playerY);
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//플레이어 방향을 문자열로 만들고 출력한다
			//12시 방향을 0도로 한다
			swprintf_s(text, L"direction : %.2f", GameMath::RadianToDegree(fmod(fmod(playerDirection+M_PI_2,M_PI*2)+ M_PI * 2,  M_PI * 2)));
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//플레이어 가속도를 문자열로 만들고 출력한다
			swprintf_s(text, L"acceleration : %.2f", m_player.GetAcceleration());
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//플레이어 속도를 문자열로 만들고 출력한다
			Vector playerMove = m_player.GetMove();
			swprintf_s(text, L"speed : %.2f", sqrt(playerMove.x * playerMove.x + playerMove.y * playerMove.y));
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//점수를 문자열로 만들고 출력한다
			swprintf_s(text, L"score : %d", m_score);
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//혜성 개수를 문자열로 만들고 출력한다
			swprintf_s(text, L"Num of comets : %d", m_comets.size());
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//우주 기뢰 개수를 문자열로 만들고 출력한다
			swprintf_s(text, L"Num of space mine : %d", m_spaceMines.size());
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//애니메이션 값을 문자열로 만들고 출력한다
			swprintf_s(text, L"Animation Value : %.2f%%", animationValue);
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//마스터 볼륨을 문자열로 만들고 출력한다
			if (m_sound->GetMuteState() == true)
			{
				swprintf_s(text, L"Volume : Mute (%d)", m_sound->GetMasterVolume());
			}
			else
			{
				swprintf_s(text, L"Volume : %d", m_sound->GetMasterVolume());
			}
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			if (m_gameOver)
			{
				//플레이어 상태를 문자열로 만들고 출력한다
				swprintf_s(text, L"Player is dead");
				renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
				top += 15;

				//텍스트 포맷을 중앙 정렬로 설정한다
				m_gameOverTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
				m_playerTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);

				//화면 중앙에 게임 오버를 표시한다
				swprintf_s(text, L"Game Over");
				renderTarget->DrawTextW(text, wcslen(text), m_gameOverTextFormat, D2D1::RectF(0, renderTargetSize.height / 2 - 33, renderTargetSize.width, renderTargetSize.height / 2 - 11), m_spacecraftEdgeSolidColorBrush);
				//화면 중앙에 점수를 표시한다
				swprintf_s(text, L"Score : %d", m_score);
				renderTarget->DrawTextW(text, wcslen(text), m_gameOverTextFormat, D2D1::RectF(0, renderTargetSize.width / 2 - 11, renderTargetSize.width, renderTargetSize.height / 2 + 11), m_spacecraftEdgeSolidColorBrush);
				//화면 중앙에 점수를 표시한다
				swprintf_s(text, L"Press enter to restart");
				renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, renderTargetSize.width / 2 + 11, renderTargetSize.width, renderTargetSize.height / 2 + 33), m_spacecraftEdgeSolidColorBrush);

				//플레이어 텍스트 포맷은 다시 왼쪽 정렬로 설정한다
				m_playerTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING);
			}
			else
			{
				//플레이어 상태를 문자열로 만들고 출력한다
				swprintf_s(text, L"Player is alive");
				renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
				top += 15;
			}
		}
		

		return hr;
	}

	//DiscardResources를 호출한다
	void GameManager::Release()
	{
		ReleaseSound();
		DiscardResources();
	}

	//게임 관련 멤버를 초기화한다
	HRESULT GameManager::InitializeGame()
	{
		HRESULT hr = 0;

		//게임오버 상태를 초기화한다
		m_gameOver = false;

		//스코어를 초기화한다
		m_score = 0;

		//플레이어 우주선을 초기화한다
		m_player.Initalize();

		//혜성들을 초기화한다
		m_comets.clear();
		for (int count = 0; count < 10; count++)
		{
			AddComet();
		}
		
		//우주 기뢰들을 초기화한다
		m_spaceMines.clear();
		for (int count = 0; count < 5; count++)
		{
			AddSpaceMine();
		}

		//스킬 포인트를 초기화한다
		m_playerSkillPoint = 0;

		//애니메이션을 초기화한다
		m_gaugeBarAnimation.SetStart(0);
		m_gaugeBarAnimation.SetEnd(100);
		m_gaugeBarAnimation.SetDuration(1.0f);

		return hr;
	}

	//리소스 관련 멤버를 초기화한다
	HRESULT GameManager::InitializeResources(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory)
	{
		HRESULT hr;

		//우주선 경로 기하 생성
		//우주선 머리 부분 경로 기하 생성
		hr = factory->CreatePathGeometry(&m_spacecraftHeadPathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_spacecraftHeadPathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(0, -17), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F points[2] = {
					D2D1::Point2F(-5, -12),
					D2D1::Point2F(5, -12),
				};
				pSink->AddLines(points, ARRAYSIZE(points));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				hr = pSink->Close();
				SafeRelease(pSink);
			}
		}
		//우주선 몸체 부분 경로 기하 생성
		hr = factory->CreatePathGeometry(&m_spacecraftBodyPathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_spacecraftBodyPathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(-5, -12), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F points[3] = {
					D2D1::Point2F(-5, 10),
					D2D1::Point2F(5, 10),
					D2D1::Point2F(5, -12)
				};
				pSink->AddLines(points, ARRAYSIZE(points));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				hr = pSink->Close();
				SafeRelease(pSink);
			}
		}
		//우주선 꼬리 부분 경로 기하 생성
		hr = factory->CreatePathGeometry(&m_spacecraftTailPathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_spacecraftTailPathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				//왼쪽 날개 부분
				pSink->BeginFigure(D2D1::Point2F(-5, 1), D2D1_FIGURE_BEGIN_FILLED);
				{
					D2D1_POINT_2F points[3] = {
						D2D1::Point2F(-10, 6),
						D2D1::Point2F(-10, 15),
						D2D1::Point2F(-5, 10)
					};
					pSink->AddLines(points, ARRAYSIZE(points));
				}
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				//중앙 날개 부분
				pSink->BeginFigure(D2D1::Point2F(0, 1), D2D1_FIGURE_BEGIN_FILLED);
				{
					D2D1_POINT_2F points[5] = {
						D2D1::Point2F(-1, 2),
						D2D1::Point2F(-1, 14),
						D2D1::Point2F(0, 15),
						D2D1::Point2F(1, 14),
						D2D1::Point2F(1, 2)
					};
					pSink->AddLines(points, ARRAYSIZE(points));
				}
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				//오른쪽 날개 부분
				pSink->BeginFigure(D2D1::Point2F(5, 1), D2D1_FIGURE_BEGIN_FILLED);
				{
					D2D1_POINT_2F points[3] = {
						D2D1::Point2F(10, 6),
						D2D1::Point2F(10, 15),
						D2D1::Point2F(5, 10)
					};
					pSink->AddLines(points, ARRAYSIZE(points));
				}
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				hr = pSink->Close();
				SafeRelease(pSink);
			}
		}
		//우주선 브러시 생성
		//우주선 머리 부분 채우기 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &m_spacecraftHeadFillSolidColorBrush);
		//hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_spacecraftHeadFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//우주선 몸체 부분 채우기 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), &m_spacecraftBodyFillSolidColorBrush);
		//hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_spacecraftBodyFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//우주선 꼬리 부분 채우기 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &m_spacecraftTailFillSolidColorBrush);
		//hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &m_spacecraftTailFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//우주선 외곽선 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_spacecraftEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}

		//혜성 경로기하 생성
		hr = factory->CreatePathGeometry(&m_cometPathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_cometPathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(-1, -17), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F points[16] = {
					//D2D1::Point2F(-1, -17),
					D2D1::Point2F(-4, -13),
					D2D1::Point2F(-10, -15),
					D2D1::Point2F(-12, -9),
					D2D1::Point2F(-18, -5),
					D2D1::Point2F(-15, -2),
					D2D1::Point2F(-14, 7),
					D2D1::Point2F(-10, 8),
					D2D1::Point2F(-8, 13),
					D2D1::Point2F(0, 12),
					D2D1::Point2F(6, 14),
					D2D1::Point2F(13, 11),
					D2D1::Point2F(12, 7),
					D2D1::Point2F(15, 4),
					D2D1::Point2F(13, 0),
					D2D1::Point2F(16, -5),
					D2D1::Point2F(11, -13)
				};
				pSink->AddLines(points, ARRAYSIZE(points));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				hr = pSink->Close();
				SafeRelease(pSink);
			}
		}
		//혜성 브러시 생성
		//혜성 채우기 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkOrange), &m_cometFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//혜성 외곽선 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &m_cometEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}

		//스킬 경로기하 생성
		hr = factory->CreatePathGeometry(&m_skillPathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_skillPathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(-50, -5), D2D1_FIGURE_BEGIN_FILLED);
				pSink->AddLine(D2D1::Point2F(50, -5));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(50, -5), D2D1::Point2F(55, 0), D2D1::Point2F(50, 5)));
				pSink->AddLine(D2D1::Point2F(-50, 5));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(-50, 5), D2D1::Point2F(-55, 0), D2D1::Point2F(-50, -5)));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				hr = pSink->Close();
				SafeRelease(pSink);
			}
		}
		//스킬 브러시 생성
		//스킬 채우기 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGreen), &m_skillFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//스킬 외곽선 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_skillEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}


		//우주 기뢰 경로기하 생성
		hr = factory->CreatePathGeometry(&m_spaceMinePathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_spaceMinePathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(-2, -11), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F points[] = {
					D2D1::Point2F(2, -11),
					D2D1::Point2F(2, -9),
					D2D1::Point2F(1, -9),
					D2D1::Point2F(1, -3),
					D2D1::Point2F(5, -7),
					D2D1::Point2F(4, -8),
					D2D1::Point2F(6, -10),
					D2D1::Point2F(10, -6),
					D2D1::Point2F(8, -4),
					D2D1::Point2F(7, -5),
					D2D1::Point2F(3, -1),
					D2D1::Point2F(9, -1),
					D2D1::Point2F(9, -2),
					D2D1::Point2F(11, -2),
					D2D1::Point2F(11, 2),
					D2D1::Point2F(9, 2),
					D2D1::Point2F(9, 1),
					D2D1::Point2F(3, 1),
					D2D1::Point2F(7, 5),
					D2D1::Point2F(8, 4),
					D2D1::Point2F(10, 6),
					D2D1::Point2F(6, 10),
					D2D1::Point2F(4, 8),
					D2D1::Point2F(5, 7),
					D2D1::Point2F(1, 3),
					D2D1::Point2F(1, 9),
					D2D1::Point2F(2, 9),
					D2D1::Point2F(2, 11),

					D2D1::Point2F(-2, 11),
					D2D1::Point2F(-2, 9),
					D2D1::Point2F(-1, 9),
					D2D1::Point2F(-1, 3),
					D2D1::Point2F(-5, 7),
					D2D1::Point2F(-4, 8),
					D2D1::Point2F(-6, 10),
					D2D1::Point2F(-10, 6),
					D2D1::Point2F(-8, 4),
					D2D1::Point2F(-7, 5),
					D2D1::Point2F(-3, 1),
					D2D1::Point2F(-9, 1),
					D2D1::Point2F(-9, 2),
					D2D1::Point2F(-11, 2),
					D2D1::Point2F(-11, -2),
					D2D1::Point2F(-9, -2),
					D2D1::Point2F(-9, -1),
					D2D1::Point2F(-3, -1),
					D2D1::Point2F(-7, -5),
					D2D1::Point2F(-8, -4),
					D2D1::Point2F(-10, -6),
					D2D1::Point2F(-6, -10),
					D2D1::Point2F(-4, -8),
					D2D1::Point2F(-5, -7),
					D2D1::Point2F(-1, -3),
					D2D1::Point2F(-1, -9),
					D2D1::Point2F(-2, -9)
					//D2D1::Point2F(-2, -11)
				};
				pSink->AddLines(points, ARRAYSIZE(points));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

				hr = pSink->Close();
				SafeRelease(pSink);
			}
		}
		//우주 기뢰 브러시 생성
		//우주 기뢰 채우기 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_spaceMineFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//우주 기뢰 외곽선 브러시 생성
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_spaceMineEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}

		//배경 비트맵 브러시 생성
		ID2D1Bitmap *bitmap;
		hr = D2DUtility::LoadBitmapFromFile(renderTarget, wicFactory, L"space2.gif", 200, 200, &bitmap);
		if (SUCCEEDED(hr))
		{
			D2D1_BITMAP_BRUSH_PROPERTIES properties = D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
			hr = renderTarget->CreateBitmapBrush(bitmap, properties, &m_backgroundBitmapBrush);
			if (SUCCEEDED(hr))
			{
			}
			SafeRelease(bitmap);
		}

		//혜성 비트맵 브러시 생성
		hr = D2DUtility::LoadBitmapFromFile(renderTarget, wicFactory, L"comet.png", 400, 300, &bitmap);
		if (SUCCEEDED(hr))
		{
			D2D1_BITMAP_BRUSH_PROPERTIES properties = D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
			hr = renderTarget->CreateBitmapBrush(bitmap, properties, &m_cometBirmapBrush);
			if (SUCCEEDED(hr))
			{
			}
			SafeRelease(bitmap);
		}

		//엔진 불꽃 컬러 비트맵 브러시 생성
		hr = D2DUtility::LoadBitmapFromFile(renderTarget, wicFactory, L"firecolor.jpg", 128, 128, &bitmap);
		if (SUCCEEDED(hr))
		{
			D2D1_BITMAP_BRUSH_PROPERTIES properties = D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
			hr = renderTarget->CreateBitmapBrush(bitmap, properties, &m_engineFireColorBitmapBrush);
			if (SUCCEEDED(hr))
			{
			}
			SafeRelease(bitmap);
		}
		//엔진 불꽃 마스크 비트맵 생성
		hr = D2DUtility::LoadBitmapFromFile(renderTarget, wicFactory, L"firemask.png", 128, 128, &bitmap);
		if (SUCCEEDED(hr))
		{
			m_engineFireMaskBitmap = bitmap;
			bitmap = 0;
		}

		//플레이어 텍스트 포맷 생성
		hr = dwriteFactory->CreateTextFormat(
			L"arial",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			12,
			L"",
			&m_playerTextFormat
		);
		if (SUCCEEDED(hr))
		{

		}

		//게임오버 텍스트 포맷 생성
		hr = dwriteFactory->CreateTextFormat(
			L"arial",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			20,
			L"",
			&m_gameOverTextFormat
		);
		if (SUCCEEDED(hr))
		{

		}

		return hr;
	}

	//리소스들을 해제한다
	void GameManager::DiscardResources()
	{
		SafeRelease(m_spacecraftHeadPathGeometry);
		SafeRelease(m_spacecraftBodyPathGeometry);
		SafeRelease(m_spacecraftTailPathGeometry);
		SafeRelease(m_spacecraftHeadFillSolidColorBrush);
		SafeRelease(m_spacecraftBodyFillSolidColorBrush);
		SafeRelease(m_spacecraftTailFillSolidColorBrush);
		SafeRelease(m_spacecraftEdgeSolidColorBrush);

		SafeRelease(m_cometPathGeometry);
		SafeRelease(m_cometFillSolidColorBrush);
		SafeRelease(m_cometEdgeSolidColorBrush);

		SafeRelease(m_skillPathGeometry);
		SafeRelease(m_skillFillSolidColorBrush);
		SafeRelease(m_skillEdgeSolidColorBrush);

		SafeRelease(m_spaceMinePathGeometry);
		SafeRelease(m_spaceMineFillSolidColorBrush);
		SafeRelease(m_spaceMineEdgeSolidColorBrush);

		SafeRelease(m_backgroundBitmapBrush);
		SafeRelease(m_cometBirmapBrush);

		SafeRelease(m_engineFireColorBitmapBrush);
		SafeRelease(m_engineFireMaskBitmap);

		SafeRelease(m_playerTextFormat);
		SafeRelease(m_gameOverTextFormat);
	}

	//사운드를 초기화한다
	void GameManager::InitializeSound(HWND hWnd)
	{
		m_sound = new SpacecraftSount(hWnd);
	}

	//사운드를 해제한다
	void GameManager::ReleaseSound()
	{
		delete m_sound;
		m_sound = NULL;
	}

	//혜성을 추가한다.
	void GameManager::AddComet()
	{
		//새 혜성을 추가하고 초기화한다
		m_comets.push_back(Comet());
		Comet* comet = &(m_comets.back());

		ResetComet(comet);
	}
	//혜성을 재설정한다
	void GameManager::ResetComet(Comet *comet)
	{
		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;

		//랜덤한 위치와 플레이어로의 방향을 계산한다
		float cometX, cometY;
		GameMath::RandomPosition(playerX, playerY, rand() % 30 * 10 + 300.0f, &cometX, &cometY);
		float cometDirection = GameMath::TargetDirection(playerX, playerY, cometX, cometY);

		comet->Initalize(cometX, cometY, cometDirection, 0.3f);
	}

	//우주 기뢰를 추가한다.
	void GameManager::AddSpaceMine()
	{
		//새 우주 기뢰을 추가하고 초기화한다
		m_spaceMines.push_back(SpaceMine());
		SpaceMine* spaceMine = &(m_spaceMines.back());

		ResetSpaceMine(spaceMine);
	}
	//우주 기뢰를 재설정한다.
	void GameManager::ResetSpaceMine(SpaceMine *spaceMine)
	{
		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;

		//랜덤한 위치와 플레이어로의 방향을 계산한다
		float spaceMineX, spaceMineY;
		GameMath::RandomPosition(playerX, playerY, rand() % 30 * 10 + 300.0f, &spaceMineX, &spaceMineY);
		float spaceMineDirection = GameMath::TargetDirection(spaceMineX, spaceMineY, playerX, playerY);

		spaceMine->Initalize(spaceMineX, spaceMineY, spaceMineDirection, 0.0f);
	}


	//플레이어 스킬을 사용한다
	void GameManager::PlayerSkill()
	{
		//플레이어가 스킬 사용 가능할 때 주위의 혜성을 제거한다
		if (m_playerSkillPoint >= 100)
		{
			//플레이어의 위치를 얻는다
			Vector playerCoord = m_player.GetCoord();
			float playerX = playerCoord.x;
			float playerY = playerCoord.y;
			float cometX, cometY;

			for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
			{
				//혜성의 위치를 얻는다
				cometX = iter->GetX();
				cometY = iter->GetY();

				//혜성이 스킬 범위에 있으면 제거한다
				if (GameMath::IntersectionCircleCircle(playerX, playerY, 100, cometX, cometY, 0))
				{
					iter = m_comets.erase(iter);
				}
				//반복자를 다음으로 넘긴다
				else
					++iter;
			}

			//스킬 포인트를 소모한다
			m_playerSkillPoint = 0;

			//스킬 사운드를 재생한다.
			m_sound->PlaySkillSound();
		}
	}


	void GameManager::PlayerAttack()
	{

	}