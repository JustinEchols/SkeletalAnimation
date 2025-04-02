
struct entity_result
{
	entity *Entity;
	u32 ID;
};

internal entity * 
EntityAdd(game_state *GameState, entity_type Type)
{
	entity_result Result = {};
	Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
	entity *Entity = GameState->Entities + GameState->EntityCount;
	Entity->Type = Type;
	Entity->ID = GameState->EntityCount;
	GameState->EntityCount++;
	return(Entity);
}

inline void
FlagAdd(entity *Entity, u32 Flag)
{
	Entity->Flags |= Flag;
}

inline void
FlagClear(entity *Entity, u32 Flag)
{
	Entity->Flags &= ~Flag;
}

inline b32 
FlagIsSet(entity *Entity, u32 Flag)
{
	b32 Result = (Entity->Flags & Flag) != 0;
	return(Result);
}

inline b32
IsMoving(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_Moving);
	return(Result);
}

inline b32
IsGrounded(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_YSupported);
	return(Result);
}

inline b32
CanMove(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_Moveable);
	return(Result);
}

inline b32
ShouldAttack(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_ShouldAttack);
	return(Result);
}

inline b32
IsAttacking(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_Attacking);
	return(Result);
}

inline b32
CanAttack(entity *Entity)
{
	b32 Result = (ShouldAttack(Entity) && !IsAttacking(Entity));
	return(Result);
}

inline b32
EntityCollided(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_Collided);
	return(Result);
}

inline b32
AnimationControlling(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_AnimationControlling);
	return(Result);
}

inline mat4
EntityTransform(entity *Entity, f32 Scale = 1.0f)
{
	mat4 Result = Mat4Identity();

	mat4 R = QuaternionToMat4(Entity->Orientation);
	Result = Mat4(Scale * Mat4ColumnGet(R, 0),
				  Scale * Mat4ColumnGet(R, 1),
				  Scale * Mat4ColumnGet(R, 2),
				  Entity->P);

	return(Result);
}

inline mat4
EntityTransform(entity *Entity, v3 Scale = V3(1.0f))
{
	mat4 Result = Mat4Identity();

	mat4 R = QuaternionToMat4(Entity->Orientation);
	Result = Mat4(Scale.x * Mat4ColumnGet(R, 0),
				  Scale.y * Mat4ColumnGet(R, 1),
				  Scale.z * Mat4ColumnGet(R, 2),
				  Entity->P);

	return(Result);
}

inline void
EntityOrientationUpdate(entity *Entity, f32 dt, f32 AngularSpeed)
{
	v3 FacingDirection = Entity->dP;

	f32 TargetAngleInRad = DirectionToEuler(FacingDirection).yaw;
	Entity->ThetaTarget = RadToDegrees(TargetAngleInRad);
	if(Entity->Theta != Entity->ThetaTarget)
	{
		// TODO(Justin): Simplify this!
		f32 TargetAngleInDegrees = RadToDegrees(TargetAngleInRad);
		quaternion QTarget;
		if((TargetAngleInDegrees == 0.0f) ||
		   (TargetAngleInDegrees == -0.0f) ||
		   (TargetAngleInDegrees == 180.0f) ||
		   (TargetAngleInDegrees == -180.0f))
		{
			QTarget	= Quaternion(V3(0.0f, 1.0f, 0.0f), TargetAngleInRad);
		}
		else
		{
			QTarget	= Quaternion(V3(0.0f, 1.0f, 0.0f), -1.0f*TargetAngleInRad);
		}

		quaternion QCurrent = Entity->Orientation;
		Entity->Orientation = RotateTowards(QCurrent, QTarget, dt, AngularSpeed);
		Entity->Theta = -1.0f*RadToDegrees(YawFromQuaternion(Entity->Orientation));
	}
}

inline void
ApplyAcceleration(entity *Entity, f32 Scale = 1.0f)
{
	f32 a = Entity->Acceleration;
	a *= Scale;
	v3 ddP = Entity->MoveInfo.ddP;
	Entity->ddP = ddP;
	Entity->MoveInfo.ddP = a * ddP;
	Entity->MoveInfo.ddP += -1.0f*Entity->Drag*Entity->dP;
}

inline void
EvaluateIdle(entity *Entity, move_info MoveInfo)
{
	// No velocity and no action
	if(MoveInfo.StandingStill)
	{
		FlagClear(Entity, EntityFlag_Moving);
		return;
	}

	FlagAdd(Entity, EntityFlag_Moving);

	if(MoveInfo.Attacking)
	{
		Entity->MovementState = MovementState_Attack;
		FlagAdd(Entity, EntityFlag_Attacking);
		FlagAdd(Entity, EntityFlag_ShouldAttack);

		if(!MoveInfo.Accelerating)
		{
			Entity->AttackType = AttackType_Neutral1;
		}
		else
		{
			Entity->AttackType = AttackType_Strong;
		}

		return;
	}

	// If the distance from the ground is not big enough dont switch to InAir 
	if(!IsGrounded(Entity) && (Entity->DistanceFromGround >= 3.0f))
	{
		Entity->MovementState = MovementState_InAir;
		return;
	}

	// Grounded and jump pressed
	if(MoveInfo.CanJump)
	{
		Entity->MovementState = MovementState_Jump;
		FlagAdd(Entity, EntityFlag_ShouldJump);
		return;
	}

	// Accelerating and sprinting 
	if(MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Sprint;
		ApplyAcceleration(Entity, 1.5f);
		return;
	}

	// Accelerating and not sprinting 
	if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
	{
		Entity->MovementState = MovementState_Run;
		ApplyAcceleration(Entity);
		return;
	}

	// Grounded and crouch pressed
	if(MoveInfo.Crouching)
	{
		Entity->MovementState = MovementState_Crouch;
		return;
	}

	// While in previous state there was no input during the frame however
	// velocity remains so still do acceleration which applies drag
	if(!MoveInfo.NoVelocity)
	{
		ApplyAcceleration(Entity);
	}
}

inline void
EvaluateRun(entity *Entity, move_info MoveInfo)
{
	if(!MoveInfo.AnyAction)
	{
		Entity->MovementState = MovementState_Idle;
		return;
	}

	if(MoveInfo.Attacking)
	{
		Entity->MovementState = MovementState_Attack;
		FlagAdd(Entity, EntityFlag_ShouldAttack);
		FlagAdd(Entity, EntityFlag_Attacking);
		Entity->AttackType = AttackType_Forward;
		return;
	}

	if(!IsGrounded(Entity) && (Entity->DistanceFromGround >= 3.0f))
	{
		Entity->MovementState = MovementState_InAir;
		return;
	}

	if(MoveInfo.CanJump)
	{
		Entity->MovementState = MovementState_Jump;
		FlagAdd(Entity, EntityFlag_ShouldJump);
		return;
	}

	if(MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Sprint;
		ApplyAcceleration(Entity, 1.5f);
		return;
	}

	if(MoveInfo.Crouching)
	{
		Entity->MovementState = MovementState_Sliding;
		return;
	}

	// Still running
	ApplyAcceleration(Entity);
}

inline void
EvaluateSprint(entity *Entity, move_info MoveInfo)
{
	if(CanAttack(Entity))
	{
		Entity->MovementState = MovementState_Attack;
		FlagAdd(Entity, EntityFlag_Attacking);
		FlagAdd(Entity, EntityFlag_ShouldAttack);
		Entity->AttackType = AttackType_Sprint;
		return;
	}

	if(MoveInfo.Attacking)
	{
		Entity->MovementState = MovementState_Attack;
		FlagAdd(Entity, EntityFlag_ShouldAttack);
		FlagAdd(Entity, EntityFlag_Attacking);
		Entity->AttackType = AttackType_Sprint;
		return;
	}

	// Sprinted off something
	if(!IsGrounded(Entity) && (Entity->DistanceFromGround >= 3.0f))
	{
		Entity->MovementState = MovementState_InAir;
		return;
	}

	if(MoveInfo.CanJump)
	{
		Entity->MovementState = MovementState_Jump;
		FlagAdd(Entity, EntityFlag_ShouldJump);
		return;
	}

	if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
	{
		Entity->MovementState = MovementState_Run;
		ApplyAcceleration(Entity);
		return;
	}

	if(!MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Idle;
		return;
	}

	// Still sprinting
	ApplyAcceleration(Entity, 1.5f);
}

inline void
EvaluateJump(entity *Entity, move_info MoveInfo, f32 dt)
{
	Entity->JumpDelay += dt;
	if(Entity->JumpDelay >= 0.2f)
	{
		Entity->MovementState = MovementState_InAir;

		FlagAdd(Entity, EntityFlag_Moving);
		FlagAdd(Entity, EntityFlag_Jumping);
		FlagClear(Entity, EntityFlag_ShouldJump);
		FlagClear(Entity, EntityFlag_YSupported);

		Entity->JumpDelay = 0.0f;
		Entity->MoveInfo.ddP.y += 1.0f;
		Entity->dP.y = 30.0f;

		if(MoveInfo.CanSprint)
		{
			ApplyAcceleration(Entity, 1.5f);
		}
		else
		{
			ApplyAcceleration(Entity);
		}

		return;
	}

	if(MoveInfo.CanSprint)
	{
		ApplyAcceleration(Entity, 1.5f);
	}
	else
	{
		ApplyAcceleration(Entity);
	}
}

inline void
EvaluateInAir(entity *Entity, move_info MoveInfo)
{
	if(FlagIsSet(Entity, EntityFlag_Jumping))
	{
		if(Entity->DistanceFromGround > 0.1f)
		{
			FlagClear(Entity, EntityFlag_Jumping);
		}
	}

	if(IsGrounded(Entity))
	{
		Entity->MovementState = MovementState_Land;
		return;
	}

	if(MoveInfo.CanSprint)
	{
		ApplyAcceleration(Entity, 1.5f);
	}
	else
	{
		ApplyAcceleration(Entity);
	}
}

inline void
EvaluateLand(entity *Entity, move_info MoveInfo)
{
	if(!MoveInfo.AnyAction)
	{
		Entity->MovementState = MovementState_Idle;
		ApplyAcceleration(Entity);
		return;
	}

	if(MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Sprint;
		ApplyAcceleration(Entity, 1.5f);
		return;
	}

	if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
	{
		Entity->MovementState = MovementState_Run;
		ApplyAcceleration(Entity);
		return;
	}

	if(MoveInfo.CanJump)
	{
		Entity->MovementState = MovementState_Jump;
		FlagAdd(Entity, EntityFlag_ShouldJump);
		return;
	}
}

inline void
EvaluateCrouch(entity *Entity, move_info MoveInfo)
{
	if(!MoveInfo.Crouching)
	{
		Entity->MovementState = MovementState_Idle;
	}
}

inline void
EvaluateSliding(entity *Entity, move_info MoveInfo)
{
	if(!MoveInfo.AnyAction)
	{
		Entity->MovementState = MovementState_Idle;
		return;
	}

	if(MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Sprint;
	}

	if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
	{
		Entity->MovementState = MovementState_Run;
	}
}

inline void
EvaluateAttack(entity *Entity, move_info MoveInfo, f32 dt)
{
	// New attack input, register attack event
	if(MoveInfo.Attacking)
	{
		FlagAdd(Entity, EntityFlag_ShouldAttack);
	}

	attack *Attack = &Entity->Attacks[Entity->AttackType];

	// Acknowledge attack event
	if(Attack->CurrentTime == 0.0f)
	{
		FlagClear(Entity, EntityFlag_ShouldAttack);
	}

	Attack->CurrentTime += dt;

	if(Attack->CurrentTime >= Attack->Duration)
	{
		Entity->AttackType = AttackType_None;
		FlagClear(Entity, EntityFlag_Attacking);
		Attack->CurrentTime = 0.0f;

		if(ShouldAttack(Entity))
		{
			FlagAdd(Entity, EntityFlag_Attacking);

			if(Attack->Type == AttackType_Neutral1)
			{
				Entity->AttackType = Entity->Attacks[AttackType_Neutral2].Type;
			}
			if(Attack->Type == AttackType_Neutral2)
			{
				Entity->AttackType = Entity->Attacks[AttackType_Neutral3].Type;
			}
			if(Attack->Type == AttackType_Neutral3)
			{
				Entity->AttackType = Entity->Attacks[AttackType_Neutral1].Type;
			}
		}
	}

	switch(Entity->AttackType)
	{
		case AttackType_Neutral1:
		case AttackType_Neutral2:
		case AttackType_Neutral3:
		case AttackType_Forward:
		case AttackType_Strong:
		case AttackType_Sprint:
		{
			Entity->dP = {};
			FlagClear(Entity, EntityFlag_Moving);
		} break;
		case AttackType_Air:
		{
		} break;
		case AttackType_None:
		{
			FlagClear(Entity, EntityFlag_Attacking);

			if(!MoveInfo.AnyAction)
			{
				Entity->MovementState = MovementState_Idle;
				return;
			}

			FlagAdd(Entity, EntityFlag_Moving);

			if(MoveInfo.CanSprint)
			{
				Entity->MovementState = MovementState_Sprint;
				ApplyAcceleration(Entity, 1.5f);
				return;
			}

			if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
			{
				Entity->MovementState = MovementState_Run;
				ApplyAcceleration(Entity);
				return;
			}

			if(MoveInfo.CanJump)
			{
				Entity->MovementState = MovementState_Jump;
				FlagAdd(Entity, EntityFlag_ShouldJump);
				return;
			}
		}break;
	};
}
