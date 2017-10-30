#ifndef __CMDLINE_PROMPT_H__
#define __CMDLINE_PROMPT_H__
#include <vector>
#define PROMPT "sceneQuery $>  "
typedef enum action_t {
	GATHER_FILES,
	PRINT_SCENES,
	PRINT_ACTORS,
	QUERY_ACTOR,
	GET_ONLINE_BIOS,
	UPDATE_SCENE_DB,
	UPDATE_ACTOR_DB,
	NEW_SCENE_DB,
	NEW_ACTOR_DB
} action_t;

typedef struct commandBuffer_t{
private:
	std::vector<std::string>b;
	size_t i;
public:
	commandBuffer_t(void){
		b.resize(0);
		i = 0;
	}
	void add(std::string s)
	{
		b.push_back(s);
		i = b.size() - 1;
	}
	std::string prev(void)
	{
		std::string s("");
		if (b.size() == 0)	// no entries in list
		{
			return "";
		}		
		else if (i == 0) 	// at first entry in list
		{
			s = b.at(i);
			i = b.size() - 1;
		}
		else // somewhere between last entry and first
		{
			s = b.at(i--);	// return string & decrement pointer
		}
		return s;
	}
	std::string next(void)
	{
		std::string s("");
		if (b.size() == 0)
		{
			return s;
		}
		else if (i == (b.size() - 1))	// at last added entry
		{
			s = b.at(i);
			i = 0;	// reinitialize pointer to the start of the list
		}
		else
		{
			s = b.at(i++);	// return string & increment pointer
		}
		return s;
	}
	void clear(void)
	{
		b.resize(0);
		i = 0;
	}
	size_t size(void)
	{
		return b.size();
	}
	int position(void)
	{
		return (int)i;
	}
} commandBuffer_t;
typedef struct interface_t{
	bool database = false;
	bool actor = false;
	bool scene = false;
	bool clear = false;
	bool newTable = false;
	bool update = false;
	bool read = false;
	bool directory = false;
	bool search = false;

} interface_t;
void interpereter(std::string, std::vector<class Actor> &, std::vector<class Scene> &, std::vector<Scene> &);
void interactiveMode(std::vector<Scene> &, std::vector<Actor> &);

#endif
