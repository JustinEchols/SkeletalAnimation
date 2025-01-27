
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
EntityCollided(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_Collided);
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

// NOTE(Justin): Each frame we clear the move_info struct
// for the player and record the input for the frame. We
// then prep the physics based off of
// the input current flags of the player. After the move is evaluated
// we go to the movement state the entity is currently in and then
// update the state and flags of the entity. So, the interface is
// between the move_info and player movement state and player flags.

inline void
EvaluatePlayerMove(entity *Entity, move_info MoveInfo, v3 *ddP)
{
	f32 a = Entity->Acceleration;

	if(!IsMoving(Entity) && MoveInfo.Accelerating)
	{
		a *= 5.0f;
	}

	if((Entity->Flags & EntityFlag_Moving ) && MoveInfo.CanSprint)
	{
		a *= 1.5f;
	}

	if(MoveInfo.CanJump)
	{
		f32 X = AbsVal(Entity->dP.x);
		f32 Z = AbsVal(Entity->dP.z);
		f32 MaxComponent = Max(X, Z);

		// TODO(Justin): Handle diagonal
		if(MaxComponent == X)
		{
			Entity->dP.x += SignOf(Entity->dP.x)*10.0f;
		}
		else
		{
			Entity->dP.z += SignOf(Entity->dP.z)*10.0f;
		}

		Entity->dP.y = 30.0f;
	}

	Entity->ddP = MoveInfo.ddP;

	// TODO(Justin): Varying drag 

	*ddP = MoveInfo.ddP;
	*ddP = a * (*ddP);
	*ddP += -Entity->Drag * Entity->dP;
}

inline void
EvaluateIdle(entity *Entity, move_info MoveInfo)
{
	if(MoveInfo.StandingStill)
	{
		// No velocity and no action
		FlagClear(Entity, EntityFlag_Moving);
		return;
	}

	FlagAdd(Entity, EntityFlag_Moving);

	if(MoveInfo.Attacking)
	{
		Entity->MovementState = MovementState_Attack;
		Entity->Attack.Type = AttackType_Neutral;
		Entity->Attack.CurrentTime = 0.0f;
		Entity->Attack.Duration = 0.2f;
		return;
	}

	if(MoveInfo.CanJump)
	{
		// Grounded and jump pressed
		Entity->MovementState = MovementState_Jump;
		return;
	}

	if(MoveInfo.Crouching)
	{
		// Grounded and crouch pressed
		Entity->MovementState = MovementState_Crouch;
		return;
	}

	if(MoveInfo.CanSprint)
	{
		// Accelerating and sprinting 
		Entity->MovementState = MovementState_Sprint;
		return;
	}

	if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
	{
		// Accelerating and not sprinting 
		Entity->MovementState = MovementState_Run;
		return;
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
		Entity->Attack.Type = AttackType_Forward;
		Entity->Attack.CurrentTime = 0.0f;
		Entity->Attack.Duration = 0.2f;
		return;
	}

	if(MoveInfo.CanJump)
	{
		Entity->MovementState = MovementState_Jump;
		return;
	}

	if(MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Sprint;
		return;
	}

	if(MoveInfo.Crouching)
	{
		Entity->MovementState = MovementState_Sliding;
		return;
	}
}

inline void
EvaluateSprint(entity *Entity, move_info MoveInfo)
{
	//if(!MoveInfo.AnyAction)
	//{
	//	Entity->MovementState = MovementState_Idle;
	//	return;
	//}

	if(MoveInfo.Attacking)
	{
		Entity->MovementState = MovementState_Attack;
		Entity->Attack.Type = AttackType_Dash;
		Entity->Attack.CurrentTime = 0.0f;
		Entity->Attack.Duration = 0.2f;
		return;
	}

	if(MoveInfo.CanJump)
	{
		Entity->MovementState = MovementState_Jump;
		return;
	}

	if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
	{
		Entity->MovementState = MovementState_Run;
		return;
	}

	if(!MoveInfo.CanSprint)
	{
		Entity->MovementState = MovementState_Idle;
		return;
	}
}

inline void
EvaluateJump(entity *Entity, move_info MoveInfo)
{
	FlagClear(Entity, EntityFlag_YSupported);
	Entity->MovementState = MovementState_InAir;
}

inline void
EvaluateInAir(entity *Entity, move_info MoveInfo)
{
	if(IsGrounded(Entity))
	{
		if(!MoveInfo.AnyAction)
		{
			Entity->MovementState = MovementState_Idle;
		}

		if(MoveInfo.CanSprint)
		{
			Entity->MovementState = MovementState_Sprint;
		}

		if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
		{
			Entity->MovementState = MovementState_Run;
		}

		if(MoveInfo.CanJump)
		{
			Entity->MovementState = MovementState_Jump;
		}

		return;
	}

	if(MoveInfo.Attacking)
	{
		Entity->MovementState = MovementState_Attack;
		Entity->Attack.Type = AttackType_Air;
		Entity->Attack.CurrentTime = 0.0f;
		Entity->Attack.Duration = 0.2f;
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
	FlagAdd(Entity, EntityFlag_Attacking);

	Entity->Attack.CurrentTime += dt;
	if(Entity->Attack.CurrentTime >= Entity->Attack.Duration)
	{
		Entity->Attack.Type = AttackType_None;
		Entity->Attack.CurrentTime = Entity->Attack.Duration;
	}

	switch(Entity->Attack.Type)
	{
		case AttackType_Neutral:
		case AttackType_Forward:
		{
			FlagClear(Entity, EntityFlag_Moving);
		} break;
		case AttackType_Dash:
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

			if(MoveInfo.CanJump)
			{
				Entity->MovementState = MovementState_Jump;
				return;
			}

			if(MoveInfo.CanSprint)
			{
				Entity->MovementState = MovementState_Sprint;
				return;
			}

			if(!MoveInfo.CanSprint && MoveInfo.Accelerating)
			{
				Entity->MovementState = MovementState_Run;
				return;
			}



		}break;
	};

}
