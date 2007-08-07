#include <string>

class MapListNode
{
	public:
		MapListNode(std::string fullName);
		~MapListNode();

		bool * pfFilters;
		bool fInCurrentFilterSet;
		std::string filename;
		short iIndex;
		short iFilteredIndex;

		bool fReadFromCache;

		bool fValid;
};

//announcerlist and musiclist are still a screwed up (a vector accessed like a vector and a list), but way better than before
class MapList
{
    public:
        MapList();
        ~MapList();

        void add(const char * name);
        bool find(const char * name);
		bool findexact(const char * name);
        bool startswith(char letter);

		const char* currentFilename(){return (*current).second->filename.c_str();};
        const char* currentShortmapname(){return (*current).first.c_str();};
        
		void prev(bool fUseFilters);
        void next(bool fUseFilters);
		void random(bool fUseFilters);

		//Sets/Gets if a map at the current map node is valid and can be loaded
		void SetValid(bool fValid) {(*current).second->fValid = fValid;}
		bool GetValid() {return (*current).second->fValid;}
        
		int GetFilteredCount() {return iFilteredMapCount;}
		int GetCount() {return maps.size();}
		
		std::map<std::string, MapListNode*>::iterator GetCurrent() {return current;}
		void SetCurrent(std::map<std::string, MapListNode*>::iterator itr) {current = itr;}

		void WriteFilters();
		void ReadFilters();
		
		bool GetFilter(short iFilter) {return (*current).second->pfFilters[iFilter];}
		bool * GetFilters() {return (*current).second->pfFilters;}
		void ToggleFilter(short iFilter) {(*current).second->pfFilters[iFilter] = !(*current).second->pfFilters[iFilter];}

		bool FindFilteredMap();
		void ApplyFilters(bool * pfFilters);
		bool MapInFilteredSet();

		std::map<std::string, MapListNode*>::iterator GetIteratorAt(unsigned short iIndex, bool fUseFilters);

		void SaveCurrent() {savedcurrent = current;}
		void ResumeCurrent() {current = savedcurrent;}

		void ReloadMapAutoFilters();
		void WriteMapSummaryCache();

    private:

        std::map<std::string, MapListNode*> maps;
        std::map<std::string, MapListNode*>::iterator current;
		std::map<std::string, MapListNode*>::iterator savedcurrent;

		short iFilteredMapCount;

		std::map<std::string, MapListNode*>::iterator * mlnFilteredMaps;
		std::map<std::string, MapListNode*>::iterator * mlnMaps;
};
