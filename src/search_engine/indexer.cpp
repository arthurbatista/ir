#include <map>
#include <iostream>
#include <cassert>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <math.h>  

using namespace std; 


class Document
{
    public:
        string value;
};

class DocNode
{
    public:
        Document* doc;
        DocNode* next;
        int tf;

};

class Term
{
    public:
        string name;
        double idf;
        DocNode* docNode;
        int nt;

};

int main(int argc, char **argv)
{
    string readed_docs[4] = {"A A A B","A A C","A A","B B"};

    int readed_docs_len = (sizeof(readed_docs)/sizeof(*readed_docs));

    map<string, Term*> vector_space;
    vector<Document*> docs;

    //Read all documents and put they in memory
    for (int i=0; i<readed_docs_len; i++) 
    {
        Document* doc = new Document();
        doc->value = readed_docs[i];
        docs.push_back(doc);
    }

    //Iterate in docs
    for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) { 

        map<string, DocNode*> map_doc_vocab;

        //Split current document in an vector of strings
        istringstream iss((*it_doc)->value);

        vector<string> doc_vocab; 
        
        copy( istream_iterator<string>( iss),
              istream_iterator<string>(),
              back_inserter( doc_vocab ) );

        for(vector<string>::iterator it = doc_vocab.begin(); it != doc_vocab.end(); ++it) {
            
            
            if (map_doc_vocab.find(*it) == map_doc_vocab.end())
            {
                DocNode* docNode = new DocNode();
                docNode->doc = *it_doc;
                docNode->tf = 1;

                map_doc_vocab[*it] = docNode;

                //If the keyword doesn't exist, create a new term
                if (vector_space.find(*it) == vector_space.end())
                {

                    Term* term = new Term();
                    term->name = *it;
                    //Accumulate the amount of documents where the term appears
                    term->nt = 1;
                    term->docNode = docNode;

                    vector_space[*it] = term;
                }
                else 
                {
                    Term* tmpTerm = vector_space[*it];
                    tmpTerm->nt++;

                    docNode->next = tmpTerm->docNode;

                    tmpTerm->docNode = docNode;
                }
            }
            else 
            {
                //Accumulate the amount of each term for the current document = TF
                map_doc_vocab[*it]->tf++;
            }
        }
    }

    //Search a term
    Term* tmpTerm = vector_space["C"];

    DocNode* cDocNode = tmpTerm->docNode;

    while (true) {

        cout << " Doc: " << cDocNode->doc->value << '\n';
        cout << " TF: "  << cDocNode->tf << '\n';
        cout << " NT: "  << tmpTerm->nt << '\n';

        cout << " TFxIDF - "<< log((double)readed_docs_len/tmpTerm->nt) * cDocNode->tf << '\n';

        if (cDocNode->next) {
            cDocNode = cDocNode->next;
        } else {
            break;
        }

    }
  
    return 0;
}