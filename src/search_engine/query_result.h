#include <string>
#include <vector>

using namespace std; 

class QueryResult
{
    public:
    	string image;
        string query;
        vector<string> relevantResults;
};