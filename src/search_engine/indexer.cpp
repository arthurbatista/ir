#include <map>
#include <iostream>
#include <cassert>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <math.h>

#include "document.h"
#include "docnode.h"
#include "term.h"

using namespace std;

map<string, Term*> vector_space;
vector<Document*> docs;
int readed_docs_len;

int readDocuments()
{
    string readed_docs[4] = {"A A A B","A A C","A A","B B"};

    int readed_docs_len = (sizeof(readed_docs)/sizeof(*readed_docs));

    //Read all documents and put they in memory
    for (int i=0; i<readed_docs_len; i++) 
    {
        Document* doc = new Document();
        doc->value = readed_docs[i];
        docs.push_back(doc);
    }

    return readed_docs_len;
}

void indexDocuments()
{
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
}

//Calculate the TFIDF vector of all documents for each term
//The documents vector should be sparse
void processDocuments()
{

    //Create a sparse vectort for each document
    for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) 
    {
        (*it_doc)->vectorTFIDF = new double[vector_space.size()];
    }

    int i = 0;

    //Fill the tfidf of each document regard the idf of the term
    for (map<string, Term*>::iterator it_term=vector_space.begin(); it_term!=vector_space.end(); ++it_term)
    {

        Term* term = it_term->second;

        double idf = log((double)readed_docs_len/term->nt);

        DocNode* currentDocNode = term->docNode;

        while (true) {

            double tfidf = idf * currentDocNode->tf;

            currentDocNode->doc->vectorTFIDF[i] = tfidf;

            if (currentDocNode->next) {
                currentDocNode = currentDocNode->next;
            } else {
                break;
            }

        }

        i++;
    }

    for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) 
    {
        for(int j=0; j<= vector_space.size(); j++) 
        {
            (*it_doc)->norma += pow((*it_doc)->vectorTFIDF[j],2);
        }

        (*it_doc)->norma = sqrt((*it_doc)->norma);

        // cout << " Doc: " << (*it_doc)->value << " = " <<  (*it_doc)->norma << "\n";
        
    }


    // for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) 
    // {

    //     cout << " Doc: " << (*it_doc)->value << " = ";
           
    //     for(int i=0;i< vector_space.size();i++)
    //     {
    //         cout << (*it_doc)->vectorTFIDF[i] << ",";
    //     }

    //     cout << '\n';
    // }
}

//This method returns the tfidf vector from the query
double * processQuery(const string query)
{
    map<string, int> map_query_vocab;

    vector<string> query_vocab;

    double* vectorTFIDF = new double[vector_space.size()];

    istringstream iss(query);
        
    copy( istream_iterator<string>( iss),
          istream_iterator<string>(),
          back_inserter(query_vocab) );

    for(vector<string>::iterator it = query_vocab.begin(); it != query_vocab.end(); ++it) 
    {

        if (map_query_vocab.find(*it) == map_query_vocab.end()) 
        {
            map_query_vocab[*it] = 1;
        }
        else
        {
            map_query_vocab[*it]++;
        }
    }

    int i = 0;

    for (map<string, Term*>::iterator it_term=vector_space.begin(); it_term!=vector_space.end(); ++it_term)
    {

        Term* term = it_term->second;

        double idf = log((double)readed_docs_len/term->nt);

        if(map_query_vocab.find(term->name) !=  map_query_vocab.end())
        {
            vectorTFIDF[i] = idf * map_query_vocab[term->name];
        }

        i++;
    }

    //TODO - Calculate similarity (cos distance) between the query vector and the vector of each document of term invertd list

    return vectorTFIDF;
}

void searchDocs(const string &query)
{

    double* queryVectorTFIDF = processQuery(query);

    vector<string> query_vocab;

    istringstream iss(query);
        
    copy( istream_iterator<string>( iss),
          istream_iterator<string>(),
          back_inserter(query_vocab) );

    //calculate Cos Distance between query and docs
    for(vector<string>::iterator it = query_vocab.begin(); it != query_vocab.end(); ++it) 
    {
        if (vector_space.find(*it) != vector_space.end())
        {

            Term* term = vector_space[*it];

            DocNode* currentDocNode = term->docNode;

            while (true) {

                double accum = 0;

                for(int i=0;i< vector_space.size();i++)
                {   
                    accum += queryVectorTFIDF[i] * currentDocNode->doc->vectorTFIDF[i];
                }

                currentDocNode->doc->tempCosDistance = accum / currentDocNode->doc->norma;

                // cout << currentDocNode->doc->value << " - " << accum << "/" << currentDocNode->doc->norma << " - " << currentDocNode->doc->tempCosDistance << "\n";
                
                if (currentDocNode->next) 
                {
                    currentDocNode = currentDocNode->next;
                } else {
                    break;
                }
            }
        }        
    }
}

int main(int argc, char **argv)
{
    
    readed_docs_len = readDocuments();

    indexDocuments();

    //TODO - call it inside indexDocuments()
    processDocuments();

    searchDocs("A B");
    
    return 0;
}