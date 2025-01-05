
internal entity * 
EntityAdd(game_state *GameState, entity_type Type)
{
	Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
	entity *Entity = GameState->Entities + GameState->EntityCount++;
	Entity->Type = Type;
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
EntityIsMoving(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_Moving);
	return(Result);
}

inline b32
EntityCanJump(entity *Entity)
{
	b32 Result = FlagIsSet(Entity, EntityFlag_YSupported);
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
