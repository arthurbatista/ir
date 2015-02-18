#include <string>

using namespace std;

class Document
{
    public:
        vector<string> doc_vocab; 
        map<string,double> vectorTFIDF;
        double norma;
        double tempCosDistance;
        string img;
};