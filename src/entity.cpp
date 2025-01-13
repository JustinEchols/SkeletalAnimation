
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

inline void
EvaluateIdle(entity *Entity, move_info MoveInfo)
{
	Assert(Entity->MovementState == MovementState_Idle);
	if(MoveInfo.Moving)
	{
		if(MoveInfo.Jumping)
		{
			Entity->MovementState = MovementState_Jump;
		}
		else if(MoveInfo.Sprinting)
		{
			Entity->MovementState = MovementState_Sprint;
		}
		else
		{
			Entity->MovementState = MovementState_Run;
		}
	}
	else
	{
		if(MoveInfo.Crouching)
		{
			Entity->MovementState = MovementState_Crouch;
		}
	}
}

inline void
EvaluateRun(entity *Entity, move_info MoveInfo)
{
	Assert(Entity->MovementState == MovementState_Run);
	if(!MoveInfo.Moving)
	{
		Entity->MovementState = MovementState_Idle;
	}
	else
	{
		if(MoveInfo.Sprinting)
		{
			Entity->MovementState = MovementState_Sprint;
		}

		if(MoveInfo.Jumping)
		{
			Entity->MovementState = MovementState_Jump;
		}

		if(MoveInfo.Crouching)
		{
			Entity->MovementState = MovementState_Sliding;
		}
	}
}

inline void
EvaluateSprint(entity *Entity, move_info MoveInfo)
{
	Assert(Entity->MovementState == MovementState_Sprint);
	if(!MoveInfo.Moving)
	{
		Entity->MovementState = MovementState_Idle;
	}
	else
	{
		if(!MoveInfo.Sprinting)
		{
			Entity->MovementState = MovementState_Run;
		}
	}
}

inline void
EvaluateJump(entity *Entity, move_info MoveInfo)
{
	Assert(Entity->MovementState == MovementState_Jump);
	FlagClear(Entity, EntityFlag_YSupported);

	if(!MoveInfo.Jumping)
	{
		if(!MoveInfo.Moving)
		{
			Entity->MovementState = MovementState_Idle;
		}
		else if(MoveInfo.Sprinting)
		{
			Entity->MovementState = MovementState_Sprint;
		}
		else
		{
			Entity->MovementState = MovementState_Run;
		}
	}
}

inline void
EvaluateCrouch(entity *Entity, move_info MoveInfo)
{
	Assert(Entity->MovementState == MovementState_Crouch);
	if(!MoveInfo.Crouching)
	{
		Entity->MovementState = MovementState_Idle;
	}
}

inline void
EvaluateSliding(entity *Entity, move_info MoveInfo)
{
	Assert(Entity->MovementState == MovementState_Sliding);
	if(!MoveInfo.Moving)
	{
		Entity->MovementState = MovementState_Idle;
	}
	else
	{
		if(MoveInfo.Sprinting)
		{
			Entity->MovementState = MovementState_Sprint;
		}
		else
		{
			Entity->MovementState = MovementState_Run;
		}
	}
}
