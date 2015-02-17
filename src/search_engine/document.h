#include <string>

using namespace std;

class Document
{
    public:
        string value;
        map<string,double> vectorTFIDF;
        double norma;
        double tempCosDistance;
        string img;
};