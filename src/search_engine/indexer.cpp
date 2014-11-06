//TODO
// Verificar se o vetor armazena so os ponteiros
// Se um termo for repetido para um documento, nao adicionar no vetor
// Verificar qual a melhor estrategia: vetor vs lista encadeada
// TF e IDF

#include <map>
#include <iostream>
#include <cassert>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace std; 

class Document
{
    public:
        string value;
        double tf;
        Document* doc;

};

class Term
{
    public:
        string name;
        double idf;
        Document* doc;

};

int main(int argc, char **argv)
{
    string readed_docs[4] = {"A A A B","A A C","A A","B B"};

    int readed_docs_len = (sizeof(readed_docs)/sizeof(*readed_docs));

    map<string, Term> vector_space;
    vector<Document> docs;

    //Read the documents and put thery in memory
    for (int i=0; i<readed_docs_len; i++) 
    {
        Document doc;
        doc.value = readed_docs[i];
        docs.push_back(doc);
    }

    //iteration in docs
    for(vector<Document>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) { 

        map<string, int> map_doc_vocab;

        //Split current document in an vector of strings
        istringstream iss(it_doc->value);

        vector<string> doc_vocab; 
        
        copy( istream_iterator<string>( iss),
              istream_iterator<string>(),
              back_inserter( doc_vocab ) );

        for(vector<string>::iterator it = doc_vocab.begin(); it != doc_vocab.end(); ++it) {
            
            //Compute the amount of each term fot the current document
            if (map_doc_vocab.find(*it) == map_doc_vocab.end())
            {
                map_doc_vocab[*it] = 1;

                //If the keyword doesn't exist, create a new term
                if (vector_space.find(*it) == vector_space.end())
                {
                    Term term;
                    term.name = *it;
                    term.doc = &(*it_doc);

                    vector_space[*it] = term;
                }
                else 
                {
                    map<string, Term>::iterator it_term = vector_space.find(*it);

                    it_doc->doc = it_term->second.doc;

                    it_term->second.doc = &(*it_doc);
                }
            }
            else 
            {
                map_doc_vocab[*it]++;
            }
        }
    }

    map<string, Term>::iterator i = vector_space.find("A");
    assert(i != vector_space.end());
    cout << "Key: " << i->first << '\n';
    cout << " Value: " << i->second.doc->value << '\n';
    cout << " Value: " << i->second.doc->doc->value << '\n';
    cout << " Value: " << i->second.doc->doc->doc->value << '\n';
    
    // i = m.find("B");
    // assert(i != m.end());
    // cout << "Key: " << i->first << " Value: " << &i->second.name1 << '\n';

    // i = m.find("C");
    // assert(i != m.end());
    // cout << "Key: " << i->first << " Value: " << &i->second.name1 << '\n';
  
    return 0;
}