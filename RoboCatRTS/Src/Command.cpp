#include "RoboCatPCH.h"
#include <iostream>

shared_ptr< Command > Command::StaticReadAndCreate( InputMemoryBitStream& inInputStream )
{
	CommandPtr retVal;
	
	int type = CM_INVALID;
	inInputStream.Read( type );
	uint32_t networkId = 0;
	inInputStream.Read( networkId );
	uint32_t playerId = 0;
	inInputStream.Read( playerId );

	switch ( type )
	{
	case CM_ATTACK:
		retVal = std::make_shared< AttackCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read( inInputStream );
		break;
	case CM_MOVE:
		retVal = std::make_shared< MoveCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read( inInputStream );
		break;
	case CM_MEOW:
		retVal = std::make_shared< MeowCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read(inInputStream);
		break;
	case CM_BUILD:
		retVal = std::make_shared< BuildCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read(inInputStream);
		break;
	case CM_SWITCH:
		retVal = std::make_shared< SwitchCommand >();
		retVal->mNetworkId = networkId;
		retVal->mPlayerId = playerId;
		retVal->Read(inInputStream);
		break;
	default:
		LOG( "Read in an unknown command type??" );
		break;
	}

	return retVal;
}

void Command::Write( OutputMemoryBitStream& inOutputStream )
{
	inOutputStream.Write( mCommandType );
	inOutputStream.Write( mNetworkId );
	inOutputStream.Write( mPlayerId );
}

AttackCommandPtr AttackCommand::StaticCreate( uint32_t inMyNetId, uint32_t inTargetNetId )
{
	AttackCommandPtr retVal;
	GameObjectPtr me = NetworkManager::sInstance->GetGameObject( inMyNetId );
	GameObjectPtr target = NetworkManager::sInstance->GetGameObject( inTargetNetId );
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();
	//can only issue commands to this unit if I own it, and it's a cat,
	//and if the target is a cat that's on a different team
	if ( me && me->GetClassId() == RoboCat::kClassId &&
		me->GetPlayerId() ==  playerId &&
		target && target->GetClassId() == RoboCat::kClassId &&
		target->GetPlayerId() != me->GetPlayerId() )
	{
		retVal = std::make_shared< AttackCommand >();
		retVal->mNetworkId = inMyNetId;
		retVal->mPlayerId = playerId;
		retVal->mTargetNetId = inTargetNetId;
	}
	return retVal;
}

void AttackCommand::Write( OutputMemoryBitStream& inOutputStream )
{
	Command::Write( inOutputStream );
	inOutputStream.Write( mTargetNetId );
}

void AttackCommand::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mTargetNetId );
}

void AttackCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject( mNetworkId );
	if ( obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId )
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterAttackState( mTargetNetId );
	}
}

MoveCommandPtr MoveCommand::StaticCreate( uint32_t inNetworkId, const Vector3& inTarget )
{
	MoveCommandPtr retVal;
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject( inNetworkId );
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();

	//can only issue commands to this unit if I own it, and it's a cat
	if ( go && go->GetClassId() == RoboCat::kClassId && 
		go->GetPlayerId() == playerId )
	{
		retVal = std::make_shared< MoveCommand >();
		retVal->mNetworkId = inNetworkId;
		retVal->mPlayerId = playerId;
		retVal->mTarget = inTarget;
	}
	return retVal;
}

void MoveCommand::Write( OutputMemoryBitStream& inOutputStream )
{
	Command::Write( inOutputStream );
	inOutputStream.Write( mTarget );
}

void MoveCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject( mNetworkId );
	if ( obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId )
	{
		RoboCat* rc = obj->GetAsCat();
		rc->EnterMovingState( mTarget );
	}
}

void MoveCommand::Read( InputMemoryBitStream& inInputStream )
{
	inInputStream.Read( mTarget );
}



MeowCommandPtr MeowCommand::StaticCreate(uint32_t inNetworkId, int time)
{
	MeowCommandPtr retVal;
	GameObjectPtr go = NetworkManager::sInstance->GetGameObject(inNetworkId);
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();

	//can only issue commands to this unit if I own it, and it's a cat
	if (go && go->GetClassId() == RoboCat::kClassId &&
		go->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< MeowCommand >();
		retVal->mNetworkId = inNetworkId;
		retVal->mPlayerId = playerId;
		retVal->mTime = time;
	}
	return retVal;
}

void MeowCommand::Write(OutputMemoryBitStream& inOutputStream)
{
	Command::Write(inOutputStream);
	inOutputStream.Write(mTime);
}

void MeowCommand::ProcessCommand()
{
	//make an offset for the meow
	Vector3 offset = Vector3(-30, -100, 0);
	float scale = 100;

	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject(mNetworkId);
	if (obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* rc = obj->GetAsCat();
		Vector3 pos = rc->GetLocation();

		//conversion to screen space
		pos += Vector3(6.5, 3.5, 0);
		pos *= scale;
		pos += offset;
		HUD::sInstance->AddMeow(pos, mTime);
	}
}

void MeowCommand::Read(InputMemoryBitStream& inInputStream)
{
	inInputStream.Read(mTime);
}


BuildCommandPtr BuildCommand::StaticCreate(const Vector3& inTarget)
{
	//no need for checks, we can spawn the cat wherever requested
	BuildCommandPtr retVal;
	retVal = std::make_shared< BuildCommand >();
	uint32_t playerId = NetworkManager::sInstance->GetMyPlayerId();
	retVal->mPlayerId = playerId;
	retVal->mTarget = inTarget;
	return retVal;
}

void BuildCommand::Write(OutputMemoryBitStream& inOutputStream)
{
	Command::Write(inOutputStream);
	inOutputStream.Write(mTarget);
}

void BuildCommand::ProcessCommand()
{
	RoboCatPtr cat = std::static_pointer_cast<RoboCat>(GameObjectRegistry::sInstance->CreateGameObject('RCAT'));
	cat->SetColor(ScoreBoardManager::sInstance->GetEntry(mPlayerId)->GetColor());
	cat->SetPlayerId(mPlayerId);
	cat->SetLocation(mTarget);
	cat->SetRotation(0);
}

void BuildCommand::Read(InputMemoryBitStream& inInputStream)
{
	inInputStream.Read(mTarget);
}

SwitchCommandPtr SwitchCommand::StaticCreate(uint32_t inNetworkId)
{
	SwitchCommandPtr retVal;
	GameObjectPtr switcher	= NetworkManager::sInstance->GetGameObject(inNetworkId);
	uint32_t playerId		= NetworkManager::sInstance->GetMyPlayerId();

	uint32_t newPlayerID = 1;

	if (playerId == 1) newPlayerID = 2;

	//can only issue commands to this unit if I own it, and it's a cat
	if (switcher && switcher->GetClassId() == RoboCat::kClassId &&
		switcher->GetPlayerId() == playerId)
	{
		retVal = std::make_shared< SwitchCommand >();
		retVal->mNetworkId = inNetworkId;
		retVal->mPlayerId = playerId;
		retVal->mNewID = newPlayerID;
	}
	return retVal;
}

void SwitchCommand::Write(OutputMemoryBitStream& inOutputStream)
{
	Command::Write(inOutputStream);
	inOutputStream.Write(mNewID);
}

void SwitchCommand::ProcessCommand()
{
	GameObjectPtr obj = NetworkManager::sInstance->GetGameObject(mNetworkId);
	if (obj && obj->GetClassId() == RoboCat::kClassId &&
		obj->GetPlayerId() == mPlayerId)
	{
		RoboCat* rc = obj->GetAsCat();
		rc->SetColor(ScoreBoardManager::sInstance->GetEntry(mNewID)->GetColor());
		rc->SetPlayerId(mNewID);
	}
}

void SwitchCommand::Read(InputMemoryBitStream& inInputStream)
{
	inInputStream.Read(mNewID);
}