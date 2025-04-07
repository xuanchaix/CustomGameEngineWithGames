#pragma once

//-----------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>

struct Vec3;

//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs


//-----------------------------------------------------------------------------------------------
class AudioSystem; //????

enum class AudioSystemSoundDimension {
	Sound2D,
	Sound3D,
};

struct AudioSystemConfig {

};


//-------------------------------------------------------------------------------------------------
class AudioSystem
{
public:
	AudioSystem(AudioSystemConfig const& aConfig);
	virtual ~AudioSystem();

public:
	void						Startup();
	void						Shutdown();
	virtual void				BeginFrame();
	virtual void				EndFrame();

	virtual SoundID				CreateOrGetSound( const std::string& soundFilePath, AudioSystemSoundDimension dimension = AudioSystemSoundDimension::Sound2D );
	virtual SoundPlaybackID		StartSound( SoundID soundID, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false );
	virtual SoundPlaybackID		StartSoundAt( SoundID soundID, const Vec3& soundPosition, bool isLooped = false, float volume = 1.0f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );

	virtual void				StopSound( SoundPlaybackID soundPlaybackID );
	virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)
	virtual void				SetSoundPosition( SoundPlaybackID soundPlaybackID, const Vec3& soundPosition );

	virtual void				ValidateResult( FMOD_RESULT result );
	virtual bool				IsSoundEnd( SoundPlaybackID soundPlaybackID );

	virtual void				SetNumListeners( int numListeners );
	virtual void				UpdateListener( int listenerIndex, const Vec3& listenerPosition, const Vec3& listenerForward, const Vec3& listenerUp );
	virtual bool				IsSoundPlaying( SoundPlaybackID soundPlaybackID );

private:
	FMOD_VECTOR					CastVec3ToFmodVec( Vec3 const& vectorToCast ) const;
	FMOD_VECTOR					CreateZeroVector() const;

protected:
	AudioSystemConfig m_config;
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
};

