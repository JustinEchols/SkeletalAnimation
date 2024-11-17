#if !defined(STRINGS_H)

struct string
{
	u8 *Data;
	u64 Size;
};

struct string_node
{
	string_node *Next;
	string String;
};

struct string_list
{
	string_node *First;
	string_node *Last;
	u64 Count;
	u64 Size;
};

struct string_array
{
	u32 Count;
	string *Strings;
};

struct string_hash_entry
{
	s32 Index;
	string Key;
	string_hash_entry *Next;
};

struct string_hash
{
	u32 Count;
	string_hash_entry Entries[256];
	memory_arena Arena;
};

#define STRINGS_H
#endif
