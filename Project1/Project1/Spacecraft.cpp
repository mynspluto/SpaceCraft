#include "Spacecraft.h"

	//������ �������� ��ȯ�Ѵ�
	float GameMath::DegreeToRadian(float degree)
	{
		return (float)(degree * M_PI / 180.0f);
	}

	//������ ������ ��ȯ�Ѵ�
	float GameMath::RadianToDegree(float radian)
	{
		return (float)(radian * 180.0f / M_PI);
	}

	//(inX, inY)���� inDistance��ŭ ������ ������ ��ġ (outX,outY)�� ����Ѵ�
	void GameMath::RandomPosition(float inX, float inY, float inDistance, float *outX, float *outY)
	{
		float randomRadian = DegreeToRadian((rand() % 3600) / 10.0f);
		*outX = inX + cos(randomRadian) * inDistance;
		*outY = inY + sin(randomRadian) * inDistance;
	}

	//(x, y)���� (targetX, targetY)�� ������ �������� ����Ѵ�
	float GameMath::TargetDirection(float x, float y, float targetX, float targetY)
	{
		float dx = targetX - x;
		float dy = targetY - y;

		return (float)(atan2(dy, dx) + M_PI);
	}

	//(x1,y1)��ġ�� ������ r1�� ���� (x2, y2)��ġ�� ������ r2�� ���� �浹�ϴ��� ����ϰ� �浹�ϸ� true�� ��ȯ�Ѵ�
	bool GameMath::IntersectionCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2)
	{
		/*
		�Ʒ� �ڵ忡 ���� ��������� ���� �� �ִ�
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


	//������ �о� ��Ʈ���� �����
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

	//�ʱ�ȭ �Լ�
	void Spacecraft::Initalize(float x, float y, float mx, float my, float direction, float rotation, float acceleration)
	{
		m_coord.x = x;
		m_coord.y = y;
		m_move.x = mx;
		m_move.y = my;
		m_direction = direction;
		m_rotation = rotation;
		m_acceleration = acceleration;

		m_isStopping = false; //���ּ� ����
		m_stoppingPhase = 0; //���� �ܰ�
		m_stoppingRotationDirection = 0;
		m_stoppingStartDirection = 0;
		m_stoppingHalfDirection = 0;
		m_stoppingEndDirection = 0;
	}
	
	//�÷��̾� ���ּ��� �����δ�
	void Spacecraft::Update()
	{
		//���߷��� ���
		if (m_isStopping)
		{
			//1�ܰ� ���� ���� ����
			if (m_stoppingPhase == 0)
			{
				m_acceleration = 0;

				//���� ������ ������ 2�ܰ�� �Ѿ��
				m_stoppingPhase = 1;
			}
			//2�ܰ� ȸ������ ���̱�
			else if (m_stoppingPhase == 1)
			{
				//���������� �� ��
				if (m_rotation > 0)
				{
					if (m_rotation > M_RotationForce)
						m_rotation -= M_RotationForce;
					else
						m_rotation = 0;
				}
				//�������� �� ��
				else if (m_rotation < 0)
				{
					if (m_rotation < -M_RotationForce)
						m_rotation += M_RotationForce;
					else
						m_rotation = 0;
				}
				//ȸ�� ������ 0�̵Ǹ� 3�ܰ�� �Ѿ��
				else if(m_rotation == 0)
					m_stoppingPhase = 2;
			}
			//3�ܰ� ������ ������ ���
			else if (m_stoppingPhase == 2)
			{
				//������ ȸ�� ���� ���� ���� 
				m_stoppingStartDirection = m_direction;

				//������ ȸ�� �� ���� ���
				m_stoppingEndDirection = atan2(-m_move.y, -m_move.x);

				//ȸ�� ���� ���
				m_stoppingRotationDirection = //m_stoppingEndDirection == m_direction ? 0 : 
					sin(m_stoppingEndDirection - m_direction) >= 0 ? 1 : -1;

				//������ ȸ�� �߰� ���� ���
				Vector startVector = { cos(m_stoppingStartDirection), sin(m_stoppingStartDirection) };
				Vector endVector = { cos(m_stoppingEndDirection), sin(m_stoppingEndDirection) };
				Vector halfVector = { startVector.x + endVector.x, startVector.y + endVector.y };
				float halfVectorLength = sqrt(halfVector.x * halfVector.x + halfVector.y * halfVector.y);
				halfVector.x = halfVector.x / halfVectorLength;
				halfVector.y = halfVector.y / halfVectorLength;
				m_stoppingHalfDirection = atan2(halfVector.y, halfVector.x);

				//��� �� 4�ܰ�� �Ѿ��
				m_stoppingPhase = 3;
			}
			//4�ܰ� ������ ������ ���
			else if (m_stoppingPhase == 3)
			{
				//�������� ������ ��
				if (abs( m_stoppingEndDirection - m_direction ) > 0)
				{
					//�������� ȸ���ؾ� �� ��
					if (m_stoppingRotationDirection == -1)
					{
						bool over = sin(m_stoppingHalfDirection - m_direction) > 0 ? true : false;
						//ȸ���� �ʰ����� ��
						if (m_rotation>0)
						{
							m_rotation = 0;
							m_direction = m_stoppingEndDirection;
						}
						//HalfDirection�� �Ѿ��� ��
						else if(over)
							m_rotation += M_RotationForce;
						//HalfDirection�� ���� �ʾ��� ��
						else
							m_rotation -= M_RotationForce;
					}
					//���������� ȸ���ؾ� �� ��
					else if(m_stoppingRotationDirection == 1)
					{
						bool over = sin(m_stoppingHalfDirection - m_direction) < 0 ? true : false;

						//ȸ���� �ʰ����� ��
						if (m_rotation<0)
						{
							m_rotation = 0;
							m_direction = m_stoppingEndDirection;
						}
						//HalfDirection�� �Ѿ��� ��
						else if (over)
							m_rotation -= M_RotationForce;
						//HalfDirection�� ���� �ʾ��� ��
						else
							m_rotation += M_RotationForce;
					}
				}
				//�̹� ������Ʈ�� �������� ���� ��
				else
				{
					m_direction = m_stoppingEndDirection;
					m_rotation = 0;
					//������ �������� ������ 5�ܰ�� �Ѿ��
					m_stoppingPhase = 4;
				}

			}
			//������ �ӵ� ���
			else if (m_stoppingPhase == 4)
			{
				m_stoppingHalfSpeed = sqrt(m_move.x * m_move.x + m_move.y * m_move.y)/2;
				m_stoppingPhase = 5;
			}
			//6�ܰ� ������ �ӵ� ���̱�
			else if (m_stoppingPhase == 5)
			{
				float speed = sqrt(m_move.x * m_move.x + m_move.y * m_move.y);

				//������ �ӵ��� 0�̵Ǹ� �ٽ� 1�ܰ�� �Ѿ��
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

					//�ӵ��� ����
					if (m_acceleration > M_AccelerationForceLimit)
					{
						m_acceleration = M_AccelerationForceLimit;
					}
				}
				else
				{
					m_acceleration -= M_AccelerationForce;

					//�ӵ��� ����
					if (m_acceleration < 0)
					{
						m_acceleration = 0;
					}
				}
			}
		}

		//���⿡ ȸ���� ����
		m_direction += m_rotation;

		//�������� ���
		m_move.x += cos(m_direction) * m_acceleration;
		m_move.y += sin(m_direction) * m_acceleration;

		//��ġ�� �������� ����
		m_coord.x += m_move.x;
		m_coord.y += m_move.y;
	}

	//direction �������� �����Ѵ�
	void Spacecraft::Accelerate()
	{
		//�ӵ��� �ø�
		m_acceleration += M_AccelerationForce;

		//�ӵ��� ����
		if (m_acceleration > M_AccelerationForceLimit)
		{
			m_acceleration = M_AccelerationForceLimit;
		}

		//������ Ǭ��
		m_isStopping = false;
	}

	//�����Ѵ�
	void Spacecraft::Deceleration()
	{
		//�ӵ��� ����
		m_acceleration -= M_AccelerationForce;

		//�ӵ��� ����
		if (m_acceleration < 0)
		{
			m_acceleration = 0;
		}

		//������ Ǭ��
		m_isStopping = false;
	}

	//���߰� �Ѵ�. �ٸ� �Է��� ������ ��ҵȴ�
	void Spacecraft::StopMoving()
	{
		m_isStopping = true;
		m_stoppingPhase = 0;
		m_stoppingRotationDirection = 0;
		m_stoppingStartDirection = 0;
		m_stoppingHalfDirection = 0;
		m_stoppingEndDirection = 0;
	}

	//direction�� rotate��ŭ ȸ���Ѵ�
	void Spacecraft::Rotate(float rotate)
	{
		m_rotation += rotate;

		//ȸ�� ����
		if (m_rotation > M_RotationForceLimit)
			m_rotation = M_RotationForceLimit;
		else if (m_rotation < -M_RotationForceLimit)
			m_rotation = -M_RotationForceLimit;
		
		m_isStopping = false;
	}


	//����� ���� Get�Լ���
	Vector Spacecraft::GetCoord() { return m_coord; }
	Vector Spacecraft::GetMove() { return m_move; }
	float Spacecraft::GetDirection() { return m_direction; }
	float Spacecraft::GetAcceleration() { return m_acceleration; }

	//����� ���� Set�Լ���
	void Spacecraft::SetCoord(Vector coord) { m_coord = coord; }
	void Spacecraft::SetMove(Vector move) { m_move = move; }
	void Spacecraft::SetDirection(float direction) { m_direction = direction; }
	void Spacecraft::SetAcceleration(float acceleration) { m_acceleration = acceleration; }

	//���� ������
	Comet::Comet() {}

	//������ �ʱ�ȭ�Ѵ�
	void Comet::Initalize(float x, float y, float direction, float speed)
	{
		m_x = x;
		m_y = y;
		m_direction = direction;
		m_speed = speed;
	}

	//������ �����δ�
	void Comet::Update()
	{
		//direction �������� speed��ŭ �����δ�
		float dx = cos(m_direction);
		float dy = sin(m_direction);

		m_x += dx * m_speed;
		m_y += dy * m_speed;
	}

	//����� ���� Get�Լ���
	float Comet::GetX() { return m_x; }
	float Comet::GetY() { return m_y; }
	float Comet::GetDirection() { return m_direction; }
	float Comet::GetSpeed() { return m_speed; }

	//����� ���� Set�Լ���
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
		//direction �������� speed��ŭ �����δ�
		float dx = cos(m_direction);
		float dy = sin(m_direction);

		m_x += dx * m_speed;
		m_y += dy * m_speed;
	}

	//����� ���� Get�Լ���
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

	//����� ���� Set�Լ���
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

	//������
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
	//�Ҹ���
	GameManager::~GameManager()
	{
		DiscardResources();
	}

	//�ʱ�ȭ
	HRESULT GameManager::OnInitialize(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory, HWND hWnd)
	{
		HRESULT hr;

		//���� ���� ����� �ʱ�ȭ�ϴ� �Լ��� ȣ���Ѵ�
		hr = InitializeGame();
		if (SUCCEEDED(hr))
		{

		}

		//���ҽ� ���� ����� �ʱ�ȭ�ϴ� �Լ��� ȣ���Ѵ�
		hr = InitializeResources(factory, renderTarget, dwriteFactory, wicFactory);
		if (SUCCEEDED(hr))
		{

		}

		InitializeSound(hWnd);

		return hr;
	}

	//���� ������ ����
	HRESULT GameManager::OnUpdate()
	{
		//���ӿ��� ������ �� F1�� ������ ���� �����
		if (m_gameOver)
		{
			if (GetAsyncKeyState(VK_RETURN) & 0x8000)
			{
				InitializeGame();
			}
			return 0;
		}

		//�� �������� �ݺ��ڸ� �̿��� �����δ�
		for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
		{
			//������ ������ �����̰� �Ѵ�
			iter->Update();

			//�ݺ��ڸ� �������� �ѱ��
			++iter;
		}

		//�� ���� ��ڵ��� �ݺ��ڸ� �̿��� �����δ�
		for (std::vector<SpaceMine>::iterator iter = m_spaceMines.begin(); iter != m_spaceMines.end(); )
		{
			//������ ���� ��ڸ� �����̰� �Ѵ�
			iter->Update();

			//�ݺ��ڸ� �������� �ѱ��
			++iter;
		}

		//�÷��̾� ���ּ��� �����Ѵ�
		//�� Ű�Է¿� ���� ���ּ� ��ȸ��, ��ȸ��, ����, ����, ���� �Լ��� ȣ���Ѵ�
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
		// ���Ұ� Ű�� �������� ��
		if (GetAsyncKeyState('P') & 0x8000)
		{
			if (soundToggleKeyPress == false)
			{
				m_sound->ToggleMute();
				//��� ����� ���� soundToggleKeyPress���� true�� �ٲ۴�.
				soundToggleKeyPress = true;
			}
		}
		else
		{
			soundToggleKeyPress = false;
		}

		static int minusWait = 0;
		// -Ű�� �������� ��
		if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000)
		{
			//minusWait�� 0���� ũ�� 1���� ��Ų��
			if (minusWait > 0)
			{
				--minusWait;
			}
			//minusWait�� 0���ϸ� �����ͺ����� ���̰� minusWait�� 20���� �ٲ۴�
			else
			{
				m_sound->DecreaseMasterVolume();
				minusWait = 20;
			}
		}
		// -Ű�� ���������� ������ minusWait�� 0���� �ٲ㼭 �ٽ� ������ �� �ٷ� ������ ������ ���� �� �ְ� �Ѵ�
		else
		{
			minusWait = 0;
		}

		static int plusWait = 0;
		// +Ű�� �������� ��
		if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000)
		{
			//plusWait�� 0���� ũ�� 1���� ��Ų��
			if (plusWait > 0)
			{
				--plusWait;
			}
			//plusWait�� 0���ϸ� �����ͺ����� �ø��� plusWait�� 20���� �ٲ۴�
			else
			{
				m_sound->IncreaseMasterVolume();
				plusWait = 20;
			}
		}
		// +Ű�� ���������� ������ plusWait�� 0���� �ٲ㼭 �ٽ� ������ �� �ٷ� ������ ������ �ø� �� �ְ� �Ѵ�
		else
		{
			plusWait = 0;
		}

		//�÷��̾� ���ּ��� �����δ�
		m_player.Update();
		//���� ���� �Ҹ��� ����Ѵ�
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

		//���� �浹 ó���� �Ѵ�
		for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
		{
			//������ ��ġ�� ���´�
			float cometX = iter->GetX();
			float cometY = iter->GetY();

			//������ ��ȿ ������ ����� �÷��̾� �������� �ٽ� �ʱ�ȭ�ϰ� �÷��̾� ��ų ����Ʈ�� 1 �ø���
			if (!GameMath::IntersectionCircleCircle(cometX, cometY, 13, playerX, playerY, 600))
			{
				ResetComet(&(*iter));

				//�÷��̾� ��ų����Ʈ�� 1 �ø���
				if (m_playerSkillPoint < 100)
				{
					++m_playerSkillPoint;
					if (m_playerSkillPoint > 100)
						m_playerSkillPoint = 100;

					if(m_playerSkillPoint == 100)
						m_sound->PlayChargedSound();
				}
			}
			//������ �浹������ ���ӿ��� ���·� �ٲ۴�
			else if (GameMath::IntersectionCircleCircle(cometX, cometY, 11, playerX, playerY, 9))
			{
				m_gameOver = true;
			}

			//�ݺ��ڸ� �������� �ѱ��
			++iter;
		}

		//���� ��� �浹 ó���� �Ѵ�
		for (std::vector<SpaceMine>::iterator iter = m_spaceMines.begin(); iter != m_spaceMines.end(); )
		{
			//���� ����� ��ġ�� ���´�
			float spaceMineX = iter->GetX();
			float spaceMineY = iter->GetY();

			//���� ��ڰ� ��ȿ ������ ����� �ٽ� �ʱ�ȭ�Ѵ�
			if (!GameMath::IntersectionCircleCircle(spaceMineX, spaceMineY, 13, playerX, playerY, 1200))
			{
				ResetSpaceMine(&(*iter));
			}
			//������ �浹������ ���ӿ��� ���·� �ٲ۴�
			else if (GameMath::IntersectionCircleCircle(spaceMineX, spaceMineY, 7, playerX, playerY, 9))
			{
				m_gameOver = true;
			}
			//150�̳��� �������� �����Ѵ�.
			else if (GameMath::IntersectionCircleCircle(spaceMineX, spaceMineY, 1, playerX, playerY, 150))
			{
				if (iter->GetShotState() == false)
				{
					float direction = GameMath::TargetDirection(playerX, playerY, spaceMineX, spaceMineY);
					iter->SetDirection(direction);
					iter->SetSpeed(1.0f);
					iter->SetShotState(true);

					//Ÿ���� ���带 ����Ѵ�.
					m_sound->PlayTargetingSound();
				}
			}

			//�ݺ��ڸ� �������� �ѱ��
			++iter;
		}

		//���ӿ����� �ƴϸ� ������ �ø���
		if (m_gameOver != true)
		{
			m_score++;

			//�߰� �����Ǵ� ������ 500������ �����Ѵ�
			if (m_score < 30000)
				if (m_score % 60 == 59) // ���ھ 10 ���������� ������ 1���� �����Ѵ�
				{
					AddComet();
				}
		}

		return 0;
	}

	//�׸���
	HRESULT GameManager::OnRender(ID2D1RenderTarget *renderTarget)
	{
		HRESULT hr = 0;

		//����Ÿ���� ũ��� ȭ�� ũ�⸦ ��´�
		D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();

		//�÷��̾��� ������ ��´�
		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;
		float playerDirection = m_player.GetDirection();
		float playerSpeed = m_player.GetAcceleration();
		
		//����� �׸���
		if (m_backgroundBitmapBrush)
		{
			//�÷��̾� ��ġ�� �°� �귯���� ����� �����Ѵ�
			m_backgroundBitmapBrush->SetTransform(D2D1::Matrix3x2F::Translation(-playerX, -playerY));
			//�÷��̾� ��ġ�� ����� �귯�÷� ȭ���� �׸���
			renderTarget->FillRectangle(D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), m_backgroundBitmapBrush);
		}

		//�������� �׸���
		if (m_cometPathGeometry && m_cometFillSolidColorBrush && m_cometEdgeSolidColorBrush)
		{
			m_cometBirmapBrush->SetTransform(D2D1::Matrix3x2F::Translation(100, 100));
			//�� �������� �ݺ��ڸ� �̿��� �׸���
			for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
			{
				//������ ��ġ�� ���´�
				float cometX = iter->GetX();
				float cometY = iter->GetY();

				//����Ÿ�� ��Ŀ� ���� ��ġ�� �����Ѵ� 
				renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2 - playerX + cometX, renderTargetSize.height / 2 - playerY + cometY));

				//���� �׸���
				renderTarget->FillGeometry(m_cometPathGeometry, m_cometBirmapBrush);
				renderTarget->DrawGeometry(m_cometPathGeometry, m_cometEdgeSolidColorBrush);

				//�ݺ��ڸ� �������� �ѱ��
				++iter;
			}
		}


		//���� ��ڸ� �׸���
		if (m_spaceMinePathGeometry && m_spaceMineFillSolidColorBrush && m_spaceMineEdgeSolidColorBrush)
		{
			//�� �������� �ݺ��ڸ� �̿��� �׸���
			for (std::vector<SpaceMine>::iterator iter = m_spaceMines.begin(); iter != m_spaceMines.end(); )
			{
				//���� ����� ��ġ�� ���´�
				float spaceMineX = iter->GetX();
				float spaceMineY = iter->GetY();

				//����Ÿ�� ��Ŀ� ���� ��� ��ġ�� �����Ѵ� 
				renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2 - playerX + spaceMineX, renderTargetSize.height / 2 - playerY + spaceMineY));

				//���ֱ�ڸ� �׸���
				renderTarget->FillGeometry(m_spaceMinePathGeometry, m_spaceMineFillSolidColorBrush);
				renderTarget->DrawGeometry(m_spaceMinePathGeometry, m_spaceMineEdgeSolidColorBrush);

				//�ݺ��ڸ� �������� �ѱ��
				++iter;
			}

		}

		//�÷��̾� ���ּ��� �׸���
		if (m_spacecraftHeadPathGeometry && m_spacecraftBodyPathGeometry && m_spacecraftTailPathGeometry && m_spacecraftHeadFillSolidColorBrush&& m_spacecraftBodyFillSolidColorBrush&& m_spacecraftTailFillSolidColorBrush && m_spacecraftEdgeSolidColorBrush)
		{
			//�÷��̾��� ȸ������� �����
			D2D1_MATRIX_3X2_F rotationMatrix = D2D1::Matrix3x2F::Rotation(GameMath::RadianToDegree(playerDirection) + 90);
			//�÷��̾� ���ּ��� ȭ�� �߾ӿ� ��ġ�ϰ� �ϱ����� ȭ���� �߾� ��ġ�� ����Ѵ�
			D2D1_MATRIX_3X2_F translationMatrix = D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2, renderTargetSize.height / 2);
			//���������� ����� ����� ����Ѵ�
			D2D1_MATRIX_3X2_F transform = rotationMatrix * translationMatrix;

			//����� �����ϰ� �÷��̾� ���ּ��� ������� �׸���
			renderTarget->SetTransform(transform);

			//���ּ� ���� �Ҳ��� ���� �׸���
			//���� ���� ���� ���� �Ҳ��� �׸���
			//���� ��Ƽ�˸��ƽ� ��带 �����صΰ� �׸� �� �ǵ�����
			if (playerSpeed > 0)
			{
				//���� ��Ƽ�˸��ƽ� ��带 �����Ѵ�
				D2D1_ANTIALIAS_MODE oldAAMode = renderTarget->GetAntialiasMode();
				//����ũ�� �����ؼ� �׸��� ���� ��Ƽ�˸��ƽ� ��带 �����Ѵ�
				renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

				//�÷��̾��� �ӵ��� ���� �̹����� �ٲ۴�
				int fireLevel;
				if (playerSpeed < 3)
					fireLevel = 0;
				else if (playerSpeed < 6)
					fireLevel = 1;
				else if (playerSpeed < 9)
					fireLevel = 2;
				else
					fireLevel = 3;

				//�Ҳ� ����� �����ϰ� �Ѵ�
				int firePhase = rand() % 4;

				//���� �Ҳ��� ������ ����ũ�� �̿��ؼ� �׸���
				renderTarget->FillOpacityMask(m_engineFireMaskBitmap, m_engineFireColorBitmapBrush, D2D1_OPACITY_MASK_CONTENT_GRAPHICS, D2D1::RectF(-8.0f, 10.0f, 10.0f, 42.0f), D2D1::RectF(32.0f * firePhase, 32.0f * fireLevel, 32.0f * firePhase + 32.0f, 32.0f * fireLevel + 32.0f - 1));

				//�����ߴ� ���� ��Ƽ�˸��ƽ� ���� �����Ѵ�
				renderTarget->SetAntialiasMode(oldAAMode);
			}

			//���ּ��� �׸���
			//���ּ� ��ü �κ��� �׸���
			renderTarget->DrawGeometry(m_spacecraftBodyPathGeometry, m_spacecraftEdgeSolidColorBrush);
			renderTarget->FillGeometry(m_spacecraftBodyPathGeometry, m_spacecraftBodyFillSolidColorBrush);
			//���ּ� �Ӹ� �κ��� �׸���
			renderTarget->DrawGeometry(m_spacecraftHeadPathGeometry, m_spacecraftEdgeSolidColorBrush);
			renderTarget->FillGeometry(m_spacecraftHeadPathGeometry, m_spacecraftHeadFillSolidColorBrush);
			//���ּ� ���� �κ��� �׸���
			renderTarget->DrawGeometry(m_spacecraftTailPathGeometry, m_spacecraftEdgeSolidColorBrush);
			renderTarget->FillGeometry(m_spacecraftTailPathGeometry, m_spacecraftTailFillSolidColorBrush);
		}



		//�ִϸ��̼� ���� ��� �����Ѵ�
		static float float_time = 0.0f;
		float animationValue = m_gaugeBarAnimation.GetValue(float_time);

		if (float_time >= m_gaugeBarAnimation.GetDuration())
			float_time = 0.0f;
		else
			float_time += 0.016f;
		
		//��ų UI�� �׸���
		if (m_skillPathGeometry && m_skillFillSolidColorBrush && m_skillEdgeSolidColorBrush)
		{
			//��ȯ ����� �����ϰ� �������ϰ� Ʋ�� �׸���
			renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(renderTargetSize.width / 2, renderTargetSize.height - 50));
			m_skillFillSolidColorBrush->SetOpacity(0.5f);
			renderTarget->FillGeometry(m_skillPathGeometry, m_skillFillSolidColorBrush);
			renderTarget->DrawGeometry(m_skillPathGeometry, m_skillEdgeSolidColorBrush);

			//���̾ �����
			ID2D1Layer* pLayer = NULL;
			hr = renderTarget->CreateLayer(NULL, &pLayer);
			if (SUCCEEDED(hr))
			{
				renderTarget->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), m_skillPathGeometry), pLayer);

				//���� ���ش�
				m_skillFillSolidColorBrush->SetOpacity(1.0f);
				renderTarget->FillRectangle(D2D1::RectF(-55.0f, -5.0f, -55.0f + m_playerSkillPoint / 100.0f * 110.0f, 5.0f), m_skillFillSolidColorBrush);

				//�÷��̾� ��ų ����Ʈ�� �������� �ʾ����� �ִϸ��̼� ȿ���� �ְ� ����á���� �ִϸ��̼� ȿ���� �����Ѵ�
				if(m_playerSkillPoint < 100)
					//�ִϸ��̼� ȿ���� �ش�
					//animationValue�� ���Ѵ�
					renderTarget->FillRectangle(D2D1::RectF(-55.0f, -5.0f, -55.0f + m_playerSkillPoint / 100.0f * 110.0f * animationValue / 100.0f, 5.0f), m_skillEdgeSolidColorBrush);
				else
					renderTarget->FillRectangle(D2D1::RectF(-55.0f, -5.0f, -55.0f + m_playerSkillPoint / 100.0f * 110.0f, 5.0f), m_skillEdgeSolidColorBrush);
				renderTarget->PopLayer();

				pLayer->Release();
			}
		}
		
		
		//�ؽ�Ʈ�� ����Ѵ�
		if (m_playerTextFormat)
		{
			//����� �ʱ�ȭ�Ѵ�
			renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			WCHAR text[100];
			float top = 0;
			//�÷��̾� ��ġ�� ���ڿ��� ����� ����Ѵ�
			swprintf_s(text, L"coord : (%.2f, %.2f)", playerX, playerY);
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//�÷��̾� ������ ���ڿ��� ����� ����Ѵ�
			//12�� ������ 0���� �Ѵ�
			swprintf_s(text, L"direction : %.2f", GameMath::RadianToDegree(fmod(fmod(playerDirection+M_PI_2,M_PI*2)+ M_PI * 2,  M_PI * 2)));
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//�÷��̾� ���ӵ��� ���ڿ��� ����� ����Ѵ�
			swprintf_s(text, L"acceleration : %.2f", m_player.GetAcceleration());
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//�÷��̾� �ӵ��� ���ڿ��� ����� ����Ѵ�
			Vector playerMove = m_player.GetMove();
			swprintf_s(text, L"speed : %.2f", sqrt(playerMove.x * playerMove.x + playerMove.y * playerMove.y));
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//������ ���ڿ��� ����� ����Ѵ�
			swprintf_s(text, L"score : %d", m_score);
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//���� ������ ���ڿ��� ����� ����Ѵ�
			swprintf_s(text, L"Num of comets : %d", m_comets.size());
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//���� ��� ������ ���ڿ��� ����� ����Ѵ�
			swprintf_s(text, L"Num of space mine : %d", m_spaceMines.size());
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//�ִϸ��̼� ���� ���ڿ��� ����� ����Ѵ�
			swprintf_s(text, L"Animation Value : %.2f%%", animationValue);
			renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
			top += 15;

			//������ ������ ���ڿ��� ����� ����Ѵ�
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
				//�÷��̾� ���¸� ���ڿ��� ����� ����Ѵ�
				swprintf_s(text, L"Player is dead");
				renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
				top += 15;

				//�ؽ�Ʈ ������ �߾� ���ķ� �����Ѵ�
				m_gameOverTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
				m_playerTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);

				//ȭ�� �߾ӿ� ���� ������ ǥ���Ѵ�
				swprintf_s(text, L"Game Over");
				renderTarget->DrawTextW(text, wcslen(text), m_gameOverTextFormat, D2D1::RectF(0, renderTargetSize.height / 2 - 33, renderTargetSize.width, renderTargetSize.height / 2 - 11), m_spacecraftEdgeSolidColorBrush);
				//ȭ�� �߾ӿ� ������ ǥ���Ѵ�
				swprintf_s(text, L"Score : %d", m_score);
				renderTarget->DrawTextW(text, wcslen(text), m_gameOverTextFormat, D2D1::RectF(0, renderTargetSize.width / 2 - 11, renderTargetSize.width, renderTargetSize.height / 2 + 11), m_spacecraftEdgeSolidColorBrush);
				//ȭ�� �߾ӿ� ������ ǥ���Ѵ�
				swprintf_s(text, L"Press enter to restart");
				renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, renderTargetSize.width / 2 + 11, renderTargetSize.width, renderTargetSize.height / 2 + 33), m_spacecraftEdgeSolidColorBrush);

				//�÷��̾� �ؽ�Ʈ ������ �ٽ� ���� ���ķ� �����Ѵ�
				m_playerTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING);
			}
			else
			{
				//�÷��̾� ���¸� ���ڿ��� ����� ����Ѵ�
				swprintf_s(text, L"Player is alive");
				renderTarget->DrawTextW(text, wcslen(text), m_playerTextFormat, D2D1::RectF(0, top, 300, top + 15), m_spacecraftEdgeSolidColorBrush);
				top += 15;
			}
		}
		

		return hr;
	}

	//DiscardResources�� ȣ���Ѵ�
	void GameManager::Release()
	{
		ReleaseSound();
		DiscardResources();
	}

	//���� ���� ����� �ʱ�ȭ�Ѵ�
	HRESULT GameManager::InitializeGame()
	{
		HRESULT hr = 0;

		//���ӿ��� ���¸� �ʱ�ȭ�Ѵ�
		m_gameOver = false;

		//���ھ �ʱ�ȭ�Ѵ�
		m_score = 0;

		//�÷��̾� ���ּ��� �ʱ�ȭ�Ѵ�
		m_player.Initalize();

		//�������� �ʱ�ȭ�Ѵ�
		m_comets.clear();
		for (int count = 0; count < 10; count++)
		{
			AddComet();
		}
		
		//���� ��ڵ��� �ʱ�ȭ�Ѵ�
		m_spaceMines.clear();
		for (int count = 0; count < 5; count++)
		{
			AddSpaceMine();
		}

		//��ų ����Ʈ�� �ʱ�ȭ�Ѵ�
		m_playerSkillPoint = 0;

		//�ִϸ��̼��� �ʱ�ȭ�Ѵ�
		m_gaugeBarAnimation.SetStart(0);
		m_gaugeBarAnimation.SetEnd(100);
		m_gaugeBarAnimation.SetDuration(1.0f);

		return hr;
	}

	//���ҽ� ���� ����� �ʱ�ȭ�Ѵ�
	HRESULT GameManager::InitializeResources(ID2D1Factory *factory, ID2D1RenderTarget *renderTarget, IDWriteFactory* dwriteFactory, IWICImagingFactory *wicFactory)
	{
		HRESULT hr;

		//���ּ� ��� ���� ����
		//���ּ� �Ӹ� �κ� ��� ���� ����
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
		//���ּ� ��ü �κ� ��� ���� ����
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
		//���ּ� ���� �κ� ��� ���� ����
		hr = factory->CreatePathGeometry(&m_spacecraftTailPathGeometry);
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink *pSink;
			hr = m_spacecraftTailPathGeometry->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				//���� ���� �κ�
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

				//�߾� ���� �κ�
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

				//������ ���� �κ�
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
		//���ּ� �귯�� ����
		//���ּ� �Ӹ� �κ� ä��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &m_spacecraftHeadFillSolidColorBrush);
		//hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_spacecraftHeadFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//���ּ� ��ü �κ� ä��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), &m_spacecraftBodyFillSolidColorBrush);
		//hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_spacecraftBodyFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//���ּ� ���� �κ� ä��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &m_spacecraftTailFillSolidColorBrush);
		//hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &m_spacecraftTailFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//���ּ� �ܰ��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_spacecraftEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}

		//���� ��α��� ����
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
		//���� �귯�� ����
		//���� ä��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkOrange), &m_cometFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//���� �ܰ��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &m_cometEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}

		//��ų ��α��� ����
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
		//��ų �귯�� ����
		//��ų ä��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGreen), &m_skillFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//��ų �ܰ��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &m_skillEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}


		//���� ��� ��α��� ����
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
		//���� ��� �귯�� ����
		//���� ��� ä��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_spaceMineFillSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}
		//���� ��� �ܰ��� �귯�� ����
		hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_spaceMineEdgeSolidColorBrush);
		if (SUCCEEDED(hr))
		{

		}

		//��� ��Ʈ�� �귯�� ����
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

		//���� ��Ʈ�� �귯�� ����
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

		//���� �Ҳ� �÷� ��Ʈ�� �귯�� ����
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
		//���� �Ҳ� ����ũ ��Ʈ�� ����
		hr = D2DUtility::LoadBitmapFromFile(renderTarget, wicFactory, L"firemask.png", 128, 128, &bitmap);
		if (SUCCEEDED(hr))
		{
			m_engineFireMaskBitmap = bitmap;
			bitmap = 0;
		}

		//�÷��̾� �ؽ�Ʈ ���� ����
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

		//���ӿ��� �ؽ�Ʈ ���� ����
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

	//���ҽ����� �����Ѵ�
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

	//���带 �ʱ�ȭ�Ѵ�
	void GameManager::InitializeSound(HWND hWnd)
	{
		m_sound = new SpacecraftSount(hWnd);
	}

	//���带 �����Ѵ�
	void GameManager::ReleaseSound()
	{
		delete m_sound;
		m_sound = NULL;
	}

	//������ �߰��Ѵ�.
	void GameManager::AddComet()
	{
		//�� ������ �߰��ϰ� �ʱ�ȭ�Ѵ�
		m_comets.push_back(Comet());
		Comet* comet = &(m_comets.back());

		ResetComet(comet);
	}
	//������ �缳���Ѵ�
	void GameManager::ResetComet(Comet *comet)
	{
		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;

		//������ ��ġ�� �÷��̾���� ������ ����Ѵ�
		float cometX, cometY;
		GameMath::RandomPosition(playerX, playerY, rand() % 30 * 10 + 300.0f, &cometX, &cometY);
		float cometDirection = GameMath::TargetDirection(playerX, playerY, cometX, cometY);

		comet->Initalize(cometX, cometY, cometDirection, 0.3f);
	}

	//���� ��ڸ� �߰��Ѵ�.
	void GameManager::AddSpaceMine()
	{
		//�� ���� ����� �߰��ϰ� �ʱ�ȭ�Ѵ�
		m_spaceMines.push_back(SpaceMine());
		SpaceMine* spaceMine = &(m_spaceMines.back());

		ResetSpaceMine(spaceMine);
	}
	//���� ��ڸ� �缳���Ѵ�.
	void GameManager::ResetSpaceMine(SpaceMine *spaceMine)
	{
		Vector playerCoord = m_player.GetCoord();
		float playerX = playerCoord.x;
		float playerY = playerCoord.y;

		//������ ��ġ�� �÷��̾���� ������ ����Ѵ�
		float spaceMineX, spaceMineY;
		GameMath::RandomPosition(playerX, playerY, rand() % 30 * 10 + 300.0f, &spaceMineX, &spaceMineY);
		float spaceMineDirection = GameMath::TargetDirection(spaceMineX, spaceMineY, playerX, playerY);

		spaceMine->Initalize(spaceMineX, spaceMineY, spaceMineDirection, 0.0f);
	}


	//�÷��̾� ��ų�� ����Ѵ�
	void GameManager::PlayerSkill()
	{
		//�÷��̾ ��ų ��� ������ �� ������ ������ �����Ѵ�
		if (m_playerSkillPoint >= 100)
		{
			//�÷��̾��� ��ġ�� ��´�
			Vector playerCoord = m_player.GetCoord();
			float playerX = playerCoord.x;
			float playerY = playerCoord.y;
			float cometX, cometY;

			for (std::vector<Comet>::iterator iter = m_comets.begin(); iter != m_comets.end(); )
			{
				//������ ��ġ�� ��´�
				cometX = iter->GetX();
				cometY = iter->GetY();

				//������ ��ų ������ ������ �����Ѵ�
				if (GameMath::IntersectionCircleCircle(playerX, playerY, 100, cometX, cometY, 0))
				{
					iter = m_comets.erase(iter);
				}
				//�ݺ��ڸ� �������� �ѱ��
				else
					++iter;
			}

			//��ų ����Ʈ�� �Ҹ��Ѵ�
			m_playerSkillPoint = 0;

			//��ų ���带 ����Ѵ�.
			m_sound->PlaySkillSound();
		}
	}


	void GameManager::PlayerAttack()
	{

	}