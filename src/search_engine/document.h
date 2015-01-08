#include <string>
#include <vector>

using namespace std; 

class Document
{
    public:
        string value;
        double* vectorTFIDF;
        double norma;
        double tempCosDistance;
};