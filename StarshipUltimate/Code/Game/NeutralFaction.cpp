#include "Game/NeutralFaction.hpp"
#include "Game/Game.hpp"
#include "Game/Room.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Item.hpp"
#include "Game/App.hpp"
#include "Game/Projectile.hpp"

Chest::Chest( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
	,m_sprite( SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Chests.png"), IntVec2(4, 1)) )
{
	m_player = g_theGame->GetPlayerObject();
	m_restrictIntoRoom = false;
}

Chest::~Chest()
{

}

void Chest::BeginPlay()
{

}

void Chest::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (GetDistanceSquared2D( m_position, m_player->m_position ) < 100.f) {
		m_isNearPlayer = true;
	}
	else {
		m_isNearPlayer = false;
	}

	if (m_isNearPlayer && g_theInput->WasKeyJustPressed( PLAYER_INTERACT_KEYCODE )) {
		m_hasReward = false;
		Die();
		if (m_isMimic) {
			m_player->BeAttacked( 1.f, Vec2() );
			MimicChest* mimic = (MimicChest*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "MimicChest" ), m_position, m_orientationDegrees );
			mimic->m_maxHealth += (m_level * 20.f);
			mimic->m_level = m_level;
		}
		else {
			g_theGame->RoomClearCallBack( g_theGame->m_curRoom );
			if (g_theApp->m_bossRushFlag) {
				for (int i = 0; i < 30; i++) {
					g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
				}
			}
		}
	}
}

void Chest::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( Vec2( -m_cosmeticRadius, -m_cosmeticRadius * 0.8f ), Vec2( m_cosmeticRadius, m_cosmeticRadius * 0.8f ) ), Rgba8::WHITE, m_sprite.GetSpriteUVs( m_level - 1 ) );
	g_theRenderer->BindTexture( &m_sprite.GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void Chest::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
		m_hasReward = false;
	}
}

void Chest::RenderUI() const
{
	if (!m_isNearPlayer) {
		return;
	}
	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Collect Item", PLAYER_INTERACT_KEYCODE ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void Chest::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	if (m_isMimic) {
		m_hasReward = false;
		Die();
		MimicChest* mimic = (MimicChest*)g_theGame->SpawnEntityToGame( EntityDefinition::GetDefinition( "MimicChest" ), m_position, m_orientationDegrees );
		mimic->m_maxHealth += (m_level * 20.f);
		mimic->m_level = m_level;
		return;
	}
	Entity::BeAttacked( hit, hitNormal, directDamage, projectileVelocity );
}

MimicChest::MimicChest( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity( def, startPos, startOrientation, startVelocity )
	, m_sprite( SpriteSheet( *g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Chests.png" ), IntVec2( 4, 1 ) ) )
{
	m_player = g_theGame->GetPlayerObject();
	m_stateTimer = new Timer( 2.f, GetGameClock() );
	m_stateTimer->Start();
	m_attackTimer = new Timer( 0.5f, GetGameClock() );
	m_targetPosIndex = GetRandGen()->RollRandomIntInRange( 0, 3 );
	m_recruitTimer = new Timer( 5.f, GetGameClock() );
	m_color = Rgba8::WHITE;
	for (int i = 0; i < 4; i++) {
		m_targetPos[i] = m_targetPos[i] + g_theGame->m_curRoom->m_bounds.m_mins;
	}
}

MimicChest::~MimicChest()
{
	delete m_stateTimer;
	delete m_attackTimer;
	delete m_recruitTimer;
}

void MimicChest::BeginPlay()
{
	ParticleSystem2DAddEmitter( 1000, 0.05f,
		AABB2( m_position - Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ), m_position + Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ) ),
		FloatRange( m_cosmeticRadius * 0.4f, m_cosmeticRadius * 0.8f ),
		AABB2( Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ), Vec2( m_cosmeticRadius * 5.f, m_cosmeticRadius * 5.f ) ),
		FloatRange( 0.3f, 0.6f ), Rgba8( 255, 255, 51, 100 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 0.f ),
		FloatRange( 0.f, 0.f ), nullptr, Rgba8( 255, 255, 51, 255 ), 0.f, 0.f );
}

void MimicChest::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_isDead) {
		return;
	}

	if (m_stateTimer->DecrementPeriodIfElapsed()) {
		if (m_state == 0) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 6.f );
			m_attackTimer->Start();
		}
		else if (m_state == 1) {
			m_state = 2;
			m_stateTimer->SetPeriodSeconds( 8.f );
			m_recruitTimer->Start();
		}
		else if (m_state == 2) {
			m_state = 1;
			m_stateTimer->SetPeriodSeconds( 6.f );
			m_attackTimer->Start();
			m_renderMoney = false;
		}
	}

	if (GetDistanceSquared2D( m_position, m_targetPos[m_targetPosIndex] ) < 100.f) {
		m_targetPosIndex = GetRandGen()->RollRandomIntInRange( 0, 3 );
	}
	else {
		AddForce( (m_targetPos[m_targetPosIndex] - m_position).GetNormalized() * m_def.m_flySpeed, false );
	}

	if (m_state == 1) {
		if (m_attackTimer->DecrementPeriodIfElapsed()) {
			float orientationDegrees = (m_player->m_position - m_position).GetOrientationDegrees();
			Vec2 fwd = Vec2::MakeFromPolarDegrees( orientationDegrees + GetRandGen()->RollRandomFloatInRange( -15.f, 15.f ) );
			m_mainWeapon->Fire( fwd, m_position );
		}
	}
	else if (m_state == 2) {
		if (m_recruitTimer->DecrementPeriodIfElapsed()) {
			EntityDefinition const& def = EntityDefinition::GetRandomDefinition( g_theGame->m_levelSequence[g_theGame->m_curLevel], -1 );
			m_renderMoney = true;
			m_numOfMoney = def.m_enemyLevel * 50;
			Entity* e = g_theGame->SpawnEntityToGame( def, GetRandomOffRoomPos(), GetRandGen()->RollRandomFloatInRange( 0.f, 360.f ) );
			e->m_hasReward = false;
		}
	}

}

void MimicChest::Render() const
{
	std::vector<Vertex_PCU> verts;
	std::vector<Vertex_PCU> moneyVerts;
	std::vector<Vertex_PCU> textVerts;
	AddVertsForAABB2D( verts, AABB2( Vec2( -m_cosmeticRadius, -m_cosmeticRadius * 0.8f ), Vec2( m_cosmeticRadius, m_cosmeticRadius * 0.8f ) ), Rgba8( 255, 240, 240 ), m_sprite.GetSpriteUVs( m_level - 1 ) );
	if (m_renderMoney) {
		AddVertsForDisc2D( moneyVerts, Vec2( -2.f, m_cosmeticRadius * 1.3f ), 0.6f, Rgba8( 255, 255, 0 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( -0.3f, m_cosmeticRadius * 1.3f - 0.8f ), Vec2( 2.f, m_cosmeticRadius * 1.3f + 0.8f ) ), 1.6f, Stringf( "%d", m_numOfMoney ) );
	}

	g_theRenderer->BindTexture( &m_sprite.GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( moneyVerts );

	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( textVerts );
}

void MimicChest::Die()
{
	m_isDead = true;
	if (m_hasReward) {
		SpawnReward();
	}
	g_theGame->RoomClearCallBack( g_theGame->m_curRoom );
}

void MimicChest::RenderUI() const
{

}

Vec2 MimicChest::GetRandomOffRoomPos() const
{
	AABB2 const& bounds = g_theGame->m_curRoom->m_bounds;
	float x = GetRandGen()->RollRandomFloatZeroToOne();
	float y = GetRandGen()->RollRandomFloatZeroToOne();

	if (x <= 1.f - y && x <= y) {
		x = 0.f;
	}
	else if (1.f - x <= y && x >= y) {
		x = 1.f;
	}
	else if (y <= 1.f - x && y <= x) {
		y = 0.f;
	}
	else {
		y = 1.f;
	}

	return bounds.GetPointAtUV( Vec2( x, y ) );
}

InteractableMachine::InteractableMachine( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
{
	m_player = g_theGame->GetPlayerObject();
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile( m_def.m_texturePath.c_str() );
	m_sprites = new SpriteSheet( *texture, IntVec2( 4, 2 ) );
	m_restrictIntoRoom = false;
	m_luckinessTimer = new Timer( 2.f );
	m_startPos = m_position;
}

InteractableMachine::~InteractableMachine()
{
	delete m_sprites;
	delete m_luckinessTimer;
}

void InteractableMachine::BeginPlay()
{

}

void InteractableMachine::Update( float deltaTime )
{
	Entity::Update( deltaTime );

	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	if (GetDistanceSquared2D( m_position, m_player->m_position ) < m_cosmeticRadius * m_cosmeticRadius * 6.25f) {
		m_isNearPlayer = true;
	}
	else {
		m_isNearPlayer = false;
	}

	if (m_isNearPlayer && g_theInput->WasKeyJustPressed( PLAYER_INTERACT_KEYCODE )) {
		PlayerController* controller = (PlayerController*)m_player->m_controller;
		RandomNumberGenerator* rng = GetRandGen();
		if (m_type == InteractableMachineType::Gamble) {
			if (controller->m_reward >= 10) {
				m_count++;
				controller->m_reward -= 10;
				if (m_count >= 20) {
					// give a gamble skill item
					m_isDestroyedByWeapon = false;
					Die();
				}
				else {
					float rnd = rng->RollRandomFloatZeroToOne() + m_player->m_luckiness;
					int moneyBack = 0;
					if (rnd > 0.99f) {
						// 100 coins
						moneyBack = 100;
					}
					else if (rnd > 0.96f) {
						moneyBack = rng->RollRandomIntInRange( 18, 25 );
					}
					else if (rnd > 0.93f) {
						moneyBack = rng->RollRandomIntInRange( 11, 18 );
					}
					else if (rnd > 0.75f) {
						moneyBack = rng->RollRandomIntInRange( 3, 11 );
					}
					else if (rnd > 0.4f) {
						moneyBack = rng->RollRandomIntInRange( 2, 10 );
					}
					else {
						moneyBack = rng->RollRandomIntInRange( 1, 9 );
					}
					// return money
					SpawnCoins( moneyBack );
				}
			}
		}
		else if (m_type == InteractableMachineType::GiveCoin) {
			if (controller->m_reward >= 10) {
				m_count++;
				controller->m_reward -= 10;
				if (m_count >= 20) {
					// generate a random level 3-4 item
					m_isDestroyedByWeapon = false;
					Die();
				}
				else {
					float rnd = rng->RollRandomFloatZeroToOne() + m_player->m_luckiness;
					if (rnd > 0.99f) {
						// generate a random level 2 item
						ItemDefinition* def = ItemDefinition::GetRandomDefinition( 2 );
						if (def) {
							SpawnItem( *def );
						}
					}
					else if (rnd > 0.98f) {
						// generate a random level 1 item
						ItemDefinition* def = ItemDefinition::GetRandomDefinition( 1 );
						if (def) {
							SpawnItem( *def );
						}
					}
					else if (rnd > 0.90f) {
						int rnd2 = rng->RollRandomPositiveNegative();
						Vec2 pos = m_position + GetRandomPointOnUnitCircle2D() * 25.f;
						if (rnd2 < 0) {
							// generate a armor pickup
							g_theGame->SpawnEffectToGame( EffectType::ArmorPickup, pos );
						}
						else {
							// generate a health pickup
							g_theGame->SpawnEffectToGame( EffectType::HealthPickup, pos );
						}
					}
					else if (rnd > 0.88f) {
						// return more money (13)
						SpawnCoins( 12 );
					}
					else if (rnd > 0.86f) {
						// return money (10)
						SpawnCoins( 10 );
					}
					else if (rnd > 0.8f) {
						// return money (5)
						SpawnCoins( 5 );
					}
				}
			}
		}
		else if (m_type == InteractableMachineType::Recycle) {
			if (g_theGame->m_renderItemScreen) {
				if ((int)m_player->m_itemList.size() > 0) {
					ItemDefinition& def = ItemDefinition::GetDefinition( m_player->m_itemList[g_theGame->m_insepctingItem] );
					m_player->LoseItem( m_player->m_itemList[g_theGame->m_insepctingItem] );
					def.m_charge = def.m_startCharge;
					ItemDefinition::SetStatusAndPosition( def.m_id, ItemStatus::In_Pool, Vec2() );
					SpawnCoins( def.GetBasicPrice() / 10 );
				}
			}
		}
		else if (m_type == InteractableMachineType::RefreshShop) {
			// 50% percent to refresh
			if (controller->m_reward >= 10) {
				controller->m_reward -= 10;
				float rnd = rng->RollRandomFloatZeroToOne() + m_player->m_luckiness;
				if (rnd > 0.5f) {
					m_count++;
					// re-roll items
					g_theGame->RerandomizeAllItemsInRoom( true );
				}
				if (m_count >= 10) {
					// generate a random level 3-4 item
					m_isDestroyedByWeapon = false;
					Die();
				}
			}
		}
		else if (m_type == InteractableMachineType::SaveMoney) {
			if (controller->m_reward >= 5) {
				controller->m_reward -= 5;
				g_theGame->m_savedCoins += 5;
				g_theGame->m_savedCoins = GetClamped( g_theGame->m_savedCoins, 0, 999 );
				float rnd = rng->RollRandomFloatZeroToOne();
				if (rnd > 0.98f) {
					m_isDestroyedByWeapon = false;
					Die();
				}
			}
		}
		else if (m_type == InteractableMachineType::SellHealth) {
			if ((m_player->m_health > 1.f || m_player->m_curArmor >= 1.f) && !m_player->IsInvincible()) {
				m_count++;
				m_player->BeAttacked( 1.f, Vec2() );
				int moneyBack = rng->RollRandomIntInRange( 8, 15 );
				SpawnCoins( moneyBack );
				if (m_count >= 12) {
					m_isDestroyedByWeapon = false;
					Die();
				}
			}
		}
		else if (m_type == InteractableMachineType::SellMaxHealth) {
			// gain base attack, gain speed, gain attack speed, gain items
			if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( m_cosmeticRadius, m_cosmeticRadius ), m_cosmeticRadius )) {
				if (m_player->m_maxHealth > 1.f) {
					// base attack
					m_player->GainMaxHealth( -1.f );
					m_player->m_mainWeaponDamage += 0.1f;
				}
			}
			else if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( m_cosmeticRadius, -m_cosmeticRadius ), m_cosmeticRadius )) {
				if (m_player->m_maxHealth > 1.f) {
					// speed
					m_player->GainMaxHealth( -1.f );
					m_player->m_movingSpeedModifier += 0.1f;
				}
			}
			else if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( -m_cosmeticRadius, m_cosmeticRadius ), m_cosmeticRadius )) {
				if (m_player->m_maxHealth > 1.f) {
					// attack speed
					m_player->GainMaxHealth( -1.f );
					m_player->m_attackSpeedModifier += 0.1f;
				}
			}
			else if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( -m_cosmeticRadius, -m_cosmeticRadius ), m_cosmeticRadius )) {
				if (m_player->m_maxHealth > 2.f) {
					// item
					ItemDefinition* def = ItemDefinition::GetRandomDefinitionInSpecialPool( "Demon" );
					if (def) {
						m_player->GainMaxHealth( -2.f );
						SpawnItem( *def );
					}
				}
			}
		}
		else if (m_type == InteractableMachineType::ShopOwner) {
			if (controller->m_reward >= 10) {
				controller->m_reward -= 10;
				m_player->m_luckiness += 0.01f;
				m_luckinessTimer->Start();
				float rnd = GetRandGen()->RollRandomFloatZeroToOne() + m_player->m_luckiness;
				if (rnd > 0.95f) {
					int rnd2 = rng->RollRandomPositiveNegative();
					Vec2 pos = m_position + GetRandomPointOnUnitCircle2D() * 25.f;
					if (rnd2 < 0) {
						// generate a armor pickup
						g_theGame->SpawnEffectToGame( EffectType::ArmorPickup, pos );
					}
					else {
						// generate a health pickup
						g_theGame->SpawnEffectToGame( EffectType::HealthPickup, pos );
					}
				}
			}
		}
	}
	m_position = m_startPos;
}

void InteractableMachine::Render() const
{
	std::vector<Vertex_PCU> verts;
	AddVertsForAABB2D( verts, AABB2( Vec2( -m_cosmeticRadius * 0.8f, -m_cosmeticRadius ), Vec2( m_cosmeticRadius * 0.8f, m_cosmeticRadius ) ), Rgba8::WHITE, m_sprites->GetSpriteDef( (int)m_type ).GetUVs() );
	
	if (m_type == InteractableMachineType::SellMaxHealth) {
		DebugDrawRing( m_position + Vec2( -m_cosmeticRadius, -m_cosmeticRadius ), m_cosmeticRadius, 0.5f, Rgba8( 255, 0, 0 ) );
		DebugDrawRing( m_position + Vec2( m_cosmeticRadius, -m_cosmeticRadius ), m_cosmeticRadius, 0.5f, Rgba8( 255, 0, 0 ) );
		DebugDrawRing( m_position + Vec2( -m_cosmeticRadius, m_cosmeticRadius ), m_cosmeticRadius, 0.5f, Rgba8( 255, 0, 0 ) );
		DebugDrawRing( m_position + Vec2( m_cosmeticRadius, m_cosmeticRadius ), m_cosmeticRadius, 0.5f, Rgba8( 255, 0, 0 ) );
	}
	
	g_theRenderer->BindTexture( &m_sprites->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );

	if (m_type == InteractableMachineType::SaveMoney) {
		std::vector<Vertex_PCU> textVerts;
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( -m_cosmeticRadius * 0.65f, -m_cosmeticRadius * 0.6f ), Vec2( -m_cosmeticRadius * 0.25f, 0.f ) ), 5.f, Stringf( "%d", (g_theGame->m_savedCoins / 100) % 10 ), Rgba8( 0, 0, 0 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( -m_cosmeticRadius * 0.15f, -m_cosmeticRadius * 0.6f ), Vec2( m_cosmeticRadius * 0.25f, 0.f ) ), 5.f, Stringf( "%d", (g_theGame->m_savedCoins / 10) % 10 ), Rgba8( 0, 0, 0 ) );
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( m_cosmeticRadius * 0.35f, -m_cosmeticRadius * 0.6f ), Vec2( m_cosmeticRadius * 0.75f, 0.f ) ), 5.f, Stringf( "%d", g_theGame->m_savedCoins % 10 ), Rgba8( 0, 0, 0 ) );

		g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
		g_theRenderer->BindShader( nullptr );
		g_theRenderer->SetBlendMode( BlendMode::ALPHA );
		g_theRenderer->SetModelConstants( GetModelMatrix() );
		g_theRenderer->DrawVertexArray( textVerts );
	}
}

void InteractableMachine::Die()
{
	m_isDead = true;
	if (m_isDestroyedByWeapon) {
		if (m_type == InteractableMachineType::SaveMoney) {
			// return some money from savings
			if (g_theGame->m_savedCoins >= 80) {
				m_hasReward = false;
				int rnd = GetRandGen()->RollRandomIntInRange( 70, std::min( g_theGame->m_savedCoins - 10, 140 ) );
				g_theGame->m_savedCoins -= (rnd + 10);
				SpawnCoins( rnd );
			}
			else {
				SpawnReward();
				m_hasReward = false;
			}
		}
		else {
			if (m_hasReward) {
				SpawnReward();
				m_hasReward = false;
			}
		}
	}
	else {
		if (m_type == InteractableMachineType::GiveCoin) {
			// give a random level 3-4 item
			float rnd = GetRandGen()->RollRandomFloatZeroToOne() + m_player->m_luckiness;
			ItemDefinition* def = nullptr;
			if (rnd > 0.80f) {
				def = ItemDefinition::GetRandomDefinition( 4 );
			}
			else {
				def = ItemDefinition::GetRandomDefinition( 3 );
			}
			if (def) {
				SpawnItem( *def );
			}
		}
		else if (m_type == InteractableMachineType::Gamble) {
			// give 1-100 coins
			int rnd = GetRandGen()->RollRandomIntInRange( 1, 100 );
			SpawnCoins( rnd );
		}
	}

	ParticleSystem2DAddEmitter( 250, 0.3f,
		AABB2( m_position, m_position ),
		FloatRange( m_cosmeticRadius * 0.1f, m_cosmeticRadius * 0.2f ),
		AABB2( -Vec2( m_cosmeticRadius, m_cosmeticRadius ) * 4.f, Vec2( m_cosmeticRadius, m_cosmeticRadius ) * 4.f ),
		FloatRange( 0.3f, 0.6f ), Rgba8( 255, 255, 51, 250 ), Particle2DShape::Disc, true, FloatRange( 0.f, 0.f ),
		FloatRange( 0.f, 0.f ), nullptr, Rgba8( 255, 255, 51, 100 ), 2.f, 0.f );
}

void InteractableMachine::RenderUI() const
{
	if (!m_isNearPlayer) {
		return;
	}
	std::vector<Vertex_PCU> textVerts;
	if (m_type == InteractableMachineType::Recycle) {
		if (g_theGame->m_renderItemScreen) {
			if ((int)m_player->m_itemList.size() > 0) {
				g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Sell", PLAYER_INTERACT_KEYCODE ) );
			}
		}
		else {
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "Open Item Inventory to Sell" ) );
		}
		
	}
	else {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 120.f ), Vec2( 1300.f, 160.f ) ), 40.f, Stringf( "%c to Interact", PLAYER_INTERACT_KEYCODE ) );
	}
	
	if (m_luckinessTimer->HasStartedAndNotPeriodElapsed()) {
		g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 720.f ), Vec2( 1300.f, 760.f ) ), 40.f, Stringf( "You Are Blessed, Add Luckiness" ), Rgba8( 255, 255, 0 ) );
	}

	if (m_type == InteractableMachineType::SellMaxHealth) {
		if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( m_cosmeticRadius, m_cosmeticRadius ), m_cosmeticRadius )) {
			// base attack
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 80.f ), Vec2( 1300.f, 115.f ) ), 35.f, Stringf( "Damage Deal, Cost: 1 Max Health" ) );
		}
		else if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( m_cosmeticRadius, -m_cosmeticRadius ), m_cosmeticRadius )) {
			// speed
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 80.f ), Vec2( 1300.f, 115.f ) ), 35.f, Stringf( "Moving Speed Deal, Cost: 1 Max Health" ) );
		}
		else if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( -m_cosmeticRadius, m_cosmeticRadius ), m_cosmeticRadius )) {
			// attack speed
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 80.f ), Vec2( 1300.f, 115.f ) ), 35.f, Stringf( "Attack Speed Deal, Cost: 1 Max Health" ) );
		}
		else if (IsPointInsideDisc2D( m_player->m_position, m_position + Vec2( -m_cosmeticRadius, -m_cosmeticRadius ), m_cosmeticRadius )) {
			// item
			g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( 300.f, 80.f ), Vec2( 1300.f, 115.f ) ), 35.f, Stringf( "Demon Item Deal, Cost: 2 Max Health" ) );
		}
	}
	
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );
}

void InteractableMachine::SpawnItem( ItemDefinition& itemDef ) const
{
	ItemDefinition::SetItemAvailability( itemDef.m_id, false );
	ItemDefinition::SetStatusAndPosition( itemDef.m_id, ItemStatus::In_Room, m_position + GetRandomPointOnUnitCircle2D() * 30.f, false );
	itemDef.m_isThrowAwayItem = true;
	g_theGame->m_isChoosingItems = true;
	g_theGame->m_showingItems.push_back( itemDef.m_id );
}

void InteractableMachine::SpawnCoins( int numOfCoins ) const
{
	for (int i = 0; i < numOfCoins; i++) {
		g_theGame->SpawnEffectToGame( EffectType::Reward, m_position );
	}
}

BossMercyKiller::BossMercyKiller( EntityDefinition const& def, Vec2 const& startPos, float startOrientation /*= 0.f*/, Vec2 const& startVelocity /*= Vec2() */ )
	:Entity(def, startPos, startOrientation, startVelocity)
{
	m_immuneToPoison = true;
	m_immuneToSlow = true;
	m_texture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/MercyKiller.png" );
	m_state = 0;
	m_stateTimer = new Timer( m_stateTime[m_state], m_clock );
	m_stateTimer->Start();
	m_obsidianTimer = new Timer( 0.99f, m_clock );
	m_meteorTimer = new Timer( 2.55f, m_clock );
	m_spawnEnemyTimer = new Timer( 0.49f, m_clock );
	m_obsidianTimer->Start();
	m_respawnTimer = new Timer( 3.f, GetGameClock() );
	m_stealMoneyTimer = new Timer( 0.299f, m_clock );
	m_orientationDegrees = 0.f;

	m_player = g_theGame->GetPlayerObject();

	m_obsidianDef = &ProjectileDefinition::GetDefinition( "SharpenedObsidian" );

	m_stateSequence[0] = m_stateSequenceLife1;
	m_stateSequence[1] = m_stateSequenceLife2;
	m_stateSequence[2] = m_stateSequenceLife3;
	m_stateSequence[3] = m_stateSequenceLife4;
	m_stateSequence[4] = m_stateSequenceLife5;
	m_stateSequence[5] = m_stateSequenceLife6;
	m_stateSequence[6] = m_stateSequenceLife7;

	m_disableFriction = true;

	for (int i = 0; i < 4; i++) {
		m_standPos[i] += g_theGame->m_curRoom->m_bounds.m_mins;
	}

	FindANewPosition();
	m_position = m_state7End;

	m_diamondBaseDef = &EntityDefinition::GetDefinition( "DiamondWarrior" );
	m_gunBaseDef = &EntityDefinition::GetDefinition( "GunShooter" );
	m_bacteriaBaseDef = &EntityDefinition::GetDefinition( "SmallBacteria" );
}

BossMercyKiller::~BossMercyKiller()
{
	delete m_stateTimer;
	delete m_obsidianTimer;
	delete m_meteorTimer;
	delete m_spawnEnemyTimer;
	delete m_respawnTimer;
	delete m_stealMoneyTimer;
}

void BossMercyKiller::BeginPlay()
{

}

void BossMercyKiller::Update( float deltaTime )
{
	if (m_isDead) {
		m_isGarbage = true;
		return;
	}

	// respawning
	if (m_respawnTimer->HasStartedAndNotPeriodElapsed()) {
		m_health = Interpolate( 0.f, m_maxHealth, m_respawnTimer->GetElapsedFraction() );
		return;
	}
	if (m_respawnTimer->HasPeriodElapsed()) {
		m_respawnTimer->Stop();
		m_curStateIndex = -1;
		GoToNextState();
	}
	Entity::Update( deltaTime );

	if (m_stateTimer->HasPeriodElapsed()) {
		GoToNextState();
	}

	Vec2 forwardToPlayer = (m_player->m_position - m_position).GetNormalized();
	float degreesToPlayer = forwardToPlayer.GetOrientationDegrees();

	// shoot obsidian
	if (m_state == 0) {
		if (m_obsidianTimer->DecrementPeriodIfElapsed()) {
			float shootOrientation = GetRandGen()->RollRandomFloatInRange( -15.f, 15.f ) + degreesToPlayer;
			for (int i = 0; i < 8; i++) {
				float rndAngle = GetRandGen()->RollRandomFloatInRange( -5.f, 5.f );
				Projectile* projectile = g_theGame->SpawnProjectileToGame( *m_obsidianDef, m_position, shootOrientation + rndAngle, Vec2::MakeFromPolarDegrees( shootOrientation + rndAngle, m_obsidianDef->m_speed ) );
				projectile->m_faction = m_def.m_faction;
			}
		}
	}
	else if (m_state == 1) {
		if (m_stateTimer->GetElapsedTime() > 1.f && !m_spawnedHarmer) {
			m_spawnedHarmer = true;
			g_theGame->SpawnEffectToGame( EffectType::MercyKillerHarmer, m_position );
		}
	}
	else if (m_state == 2) {
		m_health += deltaTime * 0.5f;
		if (m_spawnEnemyTimer->DecrementPeriodIfElapsed() && m_stateTimer->GetElapsedFraction() <= 0.6f) {
			int rnd = GetRandGen()->RollRandomIntInRange( 0, 2 );
			EntityDefinition const* def = nullptr;
			if (rnd == 0) {
				def = m_bacteriaBaseDef;
			}
			else if (rnd == 1) {
				def = m_gunBaseDef;
			}
			else if (rnd == 2) {
				def = m_diamondBaseDef;
			}
			Vec2 startPos = GetRandomPosInRoomNotNearPlayer();
			float startOrientation = (m_player->m_position - startPos).GetOrientationDegrees();
			Entity* newEntity = g_theGame->SpawnEntityToGame( *def, startPos, startOrientation );
			newEntity->m_isShadowed = true;
			newEntity->m_hasReward = false;
			ParticleSystem2DAddEmitter( 1000, 0.05f,
				AABB2( startPos - Vec2( newEntity->m_cosmeticRadius * 5.f, newEntity->m_cosmeticRadius * 5.f ), startPos + Vec2( newEntity->m_cosmeticRadius * 5.f, newEntity->m_cosmeticRadius * 5.f ) ),
				FloatRange( newEntity->m_cosmeticRadius * 0.4f, newEntity->m_cosmeticRadius * 0.8f ),
				AABB2( Vec2( newEntity->m_cosmeticRadius * 5.f, newEntity->m_cosmeticRadius * 5.f ), Vec2( newEntity->m_cosmeticRadius * 5.f, newEntity->m_cosmeticRadius * 5.f ) ),
				FloatRange( 0.3f, 0.6f ), Rgba8( 160, 160, 160, 100 ), Particle2DShape::Asteroid, true, FloatRange( 0.f, 0.f ),
				FloatRange( 0.f, 0.f ), nullptr, Rgba8( 160, 160, 160, 255 ), 0.f, 0.f );
		}
	}
	else if (m_state == 3) {
		if (m_meteorTimer->DecrementPeriodIfElapsed()) {
			SpawnMeteorShower();
		}
	}
	else if (m_state == 4) {

	}
	else if (m_state == 5) {
		if (m_stealMoneyTimer->DecrementPeriodIfElapsed()) {
			int& playerMoney = ((PlayerController*)m_player->m_controller)->m_reward;
			if (playerMoney > 0)
			{
				playerMoney = playerMoney - 1;
				StarshipReward* reward = (StarshipReward*)g_theGame->SpawnEffectToGame( EffectType::Reward, m_player->m_position );
				reward->m_target = this;
				reward->m_gotoTarget = true;
			}
			else {
				float shootOrientation = GetRandGen()->RollRandomFloatInRange( -15.f, 15.f ) + degreesToPlayer;
				for (int i = 0; i < 3; i++) {
					float rndAngle = GetRandGen()->RollRandomFloatInRange( -5.f, 5.f );
					Projectile* projectile = g_theGame->SpawnProjectileToGame( *m_obsidianDef, m_position, shootOrientation + rndAngle, Vec2::MakeFromPolarDegrees( shootOrientation + rndAngle, m_obsidianDef->m_speed ) );
					projectile->m_faction = m_def.m_faction;
				}
			}
		}
	}
	else if (m_state == 6) {
		float fraction;
		if (m_stateTimer->GetElapsedFraction() < 0.5f) {
			fraction = RangeMapClamped( m_stateTimer->GetElapsedFraction(), 0.f, 0.5f, 0.f, 1.f );
			if (m_leftRay) {
				m_leftRay->m_orientationDegrees = Interpolate( m_leftRayStartOrientationDegrees, m_leftRayStartOrientationDegrees + 150.f, SmoothStart2( fraction ) );
			}
			if (m_rightRay) {
				m_rightRay->m_orientationDegrees = Interpolate( m_rightRayStartOrientationDegrees, m_rightRayStartOrientationDegrees - 150.f, SmoothStart2( fraction ) );
			}
		}
		else {
			fraction = RangeMapClamped( m_stateTimer->GetElapsedFraction(), 0.5f, 1.f, 0.f, 1.f );
			if (m_leftRay) {
				m_leftRay->m_orientationDegrees = Interpolate( m_leftRayStartOrientationDegrees + 150.f, m_leftRayStartOrientationDegrees, SmoothStart2( fraction ) );
			}
			if (m_rightRay) {
				m_rightRay->m_orientationDegrees = Interpolate( m_rightRayStartOrientationDegrees - 150.f, m_rightRayStartOrientationDegrees, SmoothStart2( fraction ) );
			}
		}
	}
	else if (m_state == 7) {
		m_position = Interpolate( m_state7Start, m_state7End, SmoothStop3( m_stateTimer->GetElapsedFraction() ) );
	}
}

void BossMercyKiller::Render() const
{
	std::vector<Vertex_PCU> verts;
	if (m_respawnTimer->HasStartedAndNotPeriodElapsed()) {

	}
	else {
		if (!(m_state == 4 && m_stateTimer->GetElapsedFraction() > 0.2f)) {
			AddVertsForAABB2D( verts, AABB2( Vec2( -m_cosmeticRadius * 0.625f, -m_cosmeticRadius ), Vec2( m_cosmeticRadius * 0.625f, m_cosmeticRadius ) ), Rgba8::WHITE );
		}
	}
	
	g_theRenderer->BindTexture( m_texture );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants( GetModelMatrix(), m_color );
	g_theRenderer->DrawVertexArray( verts );
}

void BossMercyKiller::Die()
{
	if (m_respawnTimer->HasStartedAndNotPeriodElapsed()) {
		return;
	}
	m_lives--;
	if (m_lives == 0) {
		m_isDead = true;
		if (m_hasReward) {
			SpawnReward();
		}
		ParticleSystem2DAddEmitter( 1000, 3.f,
			AABB2( m_position, m_position ),
			FloatRange( m_cosmeticRadius * 0.1f, m_cosmeticRadius * 0.2f ),
			AABB2( -Vec2( m_cosmeticRadius, m_cosmeticRadius ) * 4.f, Vec2( m_cosmeticRadius, m_cosmeticRadius ) * 4.f ),
			FloatRange( 0.6f, 1.2f ), Rgba8( 255, 153, 51, 250 ), Particle2DShape::Disc, true, FloatRange( 0.f, 0.f ),
			FloatRange( 0.f, 0.f ), nullptr, Rgba8( 255, 153, 51, 100 ), 2.f, 0.f );
	}
	else {
		m_respawnTimer->Start();
		m_isInvincible = true;
		if (m_state == 7) {
			m_position = m_state7End;
		}
		// re-spawn
		m_maxHealth += 50.f;
		g_theGame->SpawnEffectToGame( EffectType::MercyKillerRespawn, m_position );
	}
}

void BossMercyKiller::RenderUI() const
{
	float remainHealthRatio = (float)m_health / (float)m_maxHealth;
	remainHealthRatio = GetClamped( remainHealthRatio, 0.f, 1.f );
	Rgba8 healthColor = Rgba8( 255, 215, 0 );
	Rgba8 maxHealthColor = Rgba8( 255, 0, 0 );
	float startX = 1050.f;
	float endX = 1550.f;

	float Y = 30.f;
	float radius = 8.f;
	std::vector<Vertex_PCU> verts;
	AddVertsForCapsule2D( verts, Vec2( startX, Y ), Vec2( endX, Y ), radius, maxHealthColor );
	AddVertsForCapsule2D( verts, Vec2( endX - (endX - startX) * remainHealthRatio, Y ), Vec2( endX, Y ), radius, healthColor );

	std::vector<Vertex_PCU> textVerts;
	g_ASCIIFont->AddVertsForTextInBox2D( textVerts, AABB2( Vec2( startX, Y + radius * 2.f ), Vec2( endX, Y + radius * 2.f + 35.f ) ), 35.f, Stringf( "Mercy Killer" ), Rgba8::WHITE, 0.618f, Vec2( 0.95f, 0.5f ) );
	g_theRenderer->BindTexture( &g_ASCIIFont->GetTexture() );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( textVerts );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShader( nullptr );
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( verts );
}

void BossMercyKiller::GoToNextState()
{
	m_curStateIndex = (m_curStateIndex + 1) % m_stateSequenceCount[m_lives - 1];
	m_state = m_stateSequence[m_lives-1][m_curStateIndex];
	m_stateTimer->SetPeriodSeconds( m_stateTime[m_state] );
	m_isInvincible = false;

	if (m_state == 0) {
		m_obsidianTimer->Start();
	}
	else if (m_state == 1) {
		m_spawnedHarmer = false;
	}
	else if (m_state == 2) {
		m_spawnEnemyTimer->Start();
	}
	else if (m_state == 3) {
		SpawnMeteorShower();
		m_meteorTimer->Start();
	}
	else if (m_state == 4) {
		MercyKillerCage* cage = (MercyKillerCage*)g_theGame->SpawnEffectToGame( EffectType::MercyKillerCage, m_player->m_position, 0.f, Vec2(), true );
		cage->m_bounds.m_maxs = m_player->m_position + Vec2( 22.f, 15.f );
		cage->m_bounds.m_mins = m_player->m_position - Vec2( 22.f, 15.f );
		m_isInvincible = true;
	}
	else if (m_state == 5) {
		m_stealMoneyTimer->Start();
	}
	else if (m_state == 6) {
		Vec2 leftStartPos = m_position + Vec2( m_cosmeticRadius * -0.11f, m_cosmeticRadius * 0.705f );
		m_leftRayStartOrientationDegrees = (m_player->m_position - leftStartPos).GetOrientationDegrees() - 75.f;
		float leftLength = Minf( (m_player->m_position - leftStartPos).GetLength() * 1.5f, 80.f );
		m_leftRay = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, leftStartPos, m_leftRayStartOrientationDegrees );
		m_leftRay->m_maxLength = leftLength;
		m_leftRay->m_maxWidth = 1.f;
		m_leftRay->m_minWidth = 0.5f;
		m_leftRay->m_owner = this;
		m_leftRay->m_lifeTimeSeconds = m_stateTime[6];
		m_leftRay->m_color = Rgba8( 204, 0, 0 );
		m_leftRay->m_damage = 1.f;
		m_leftRay->m_damageCoolDown = 0.5f;
		m_leftRay->m_renderBeforeEntity = false;
		m_leftRay->BeginPlay();

		Vec2 rightStartPos = m_position + Vec2( m_cosmeticRadius * 0.105f, m_cosmeticRadius * 0.705f );
		m_rightRayStartOrientationDegrees = (m_player->m_position - rightStartPos).GetOrientationDegrees() + 75.f;
		float rightLength = Minf( (m_player->m_position - rightStartPos).GetLength() * 1.5f, 80.f );
		m_rightRay = (PersistentRay*)g_theGame->SpawnEffectToGame( EffectType::PersistentRay, rightStartPos, m_rightRayStartOrientationDegrees );
		m_rightRay->m_maxLength = rightLength;
		m_rightRay->m_maxWidth = 1.f;
		m_rightRay->m_minWidth = 0.5f;
		m_rightRay->m_owner = this;
		m_rightRay->m_lifeTimeSeconds = m_stateTime[6];
		m_rightRay->m_color = Rgba8( 204, 0, 0 );
		m_rightRay->m_damage = 1.f;
		m_rightRay->m_damageCoolDown = 0.5f;
		m_rightRay->m_renderBeforeEntity = false;
		m_rightRay->BeginPlay();
	}
	else if (m_state == 7) {
		FindANewPosition();
	}
	m_stateTimer->Start();
}

// subtract 50% of the damage
void BossMercyKiller::BeAttacked( float hit, Vec2 const& hitNormal, bool directDamage /*= false*/, Vec2 const& projectileVelocity /*= Vec2() */ )
{
	if (m_respawnTimer->HasStartedAndNotPeriodElapsed()) {
		return;
	}
	if (m_isDead) {
		return;
	}
	UNUSED( directDamage );
	UNUSED( projectileVelocity );
	if (hit > 0.f && m_def.m_isShielded && hitNormal != Vec2( 0.f, 0.f ) && DotProduct2D( hitNormal, GetForwardNormal() ) > 0.1f) {
		StarshipEffect* shieldEffect = g_theGame->SpawnEffectToGame( EffectType::Shield, m_position );
		((StarshipShield*)shieldEffect)->m_owner = this;
		shieldEffect->BeginPlay();
		return;
	}
	else {
		//g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyHit.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		m_health -= hit;
	}
	if (m_health > m_maxHealth) {
		m_health = m_maxHealth;
	}
	if (m_health <= 0.f) {
		if (m_def.m_isEnemy) {
			g_theAudio->StartSound( g_theAudio->CreateOrGetSound( "Data/Sounds/EnemyDie.wav" ), false, g_theApp->m_soundVolume * 0.1f );
		}
		Die();
	}
}

void BossMercyKiller::FindANewPosition()
{
	m_state7Start = m_position;
	int targetIndex;
	do {
		targetIndex = GetRandGen()->RollRandomIntInRange( 0, 3 );
	} while (targetIndex == m_curPosIndex);
	m_curPosIndex = targetIndex;
	m_state7End = m_standPos[m_curPosIndex];
}

void BossMercyKiller::SpawnMeteorShower()
{
	Vec2 targetPos = m_player->m_position;

	Vec2 center = Vec2( targetPos.x + GetRandGen()->RollRandomFloatInRange( -20.f, 20.f ), g_theGame->m_curRoom->m_bounds.m_maxs.y + 20.f );
	float orientationDegrees = (targetPos - center).GetOrientationDegrees();
	MeteorShower* shower = (MeteorShower*)g_theGame->SpawnEffectToGame( EffectType::MeteorShower, center, orientationDegrees );
	shower->m_apertureDegrees = 45.f;
	shower->m_radius = 300.f;
}

Vec2 BossMercyKiller::GetRandomPosInRoomNotNearPlayer()
{
	Vec2 res;
	do {
		res = g_theGame->m_curRoom->m_bounds.GetRandomPointInside();
	} while (GetDistanceSquared2D( res, m_player->m_position ) < 2500.f);
	return res;
}
