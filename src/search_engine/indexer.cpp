#include <map>
#include <iostream>
#include <cassert>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <set>

#include <math.h>

#include <iomanip>  

#include "query_result.h"

#include "document.h"

#include "docnode.h"
#include "term.h"

#include "parser.h"

using namespace std;

vector<QueryResult*> queryResults;
map<string, Term*> vector_space;
vector<Document*> docs;
int readed_docs_len;

int readDocuments()
{
    docs = ProductParser::parseProducts();

    return docs.size();
}

void indexDocuments()
{
    //Iterate in docs
    for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) 
    {

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

    //Create a sparse vector for each document
    for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) 
    {
        (*it_doc)->vectorTFIDF = new double[vector_space.size()];
    }

    int i = 0;

    //Fill the tfidf of each document regard the idf of the term
    for (map<string, Term*>::iterator it_term=vector_space.begin(); it_term!=vector_space.end(); ++it_term)
    {

        Term* term = it_term->second;

        double idf = log2((double)readed_docs_len/term->nt);

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
    }
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

        double idf = log2((double)readed_docs_len/term->nt);

        if(map_query_vocab.find(term->name) !=  map_query_vocab.end())
        {
            vectorTFIDF[i] = idf * map_query_vocab[term->name];
        }

        i++;
    }

    //TODO - Calculate similarity (cos distance) between the query vector and the vector of each document of term invertd list
    return vectorTFIDF;
}

DocNode* searchDocs(const string &query)
{

    string queryNormalized = ProductParser::normalizeString(query);

    double* queryVectorTFIDF = processQuery(queryNormalized);

    vector<string> query_vocab;

    vector<string> processed_terms;

    DocNode* docsResult = new DocNode();

    istringstream iss(queryNormalized);
        
    copy( istream_iterator<string>( iss),
          istream_iterator<string>(),
          back_inserter(query_vocab) );

    //calculate Cos Distance between query and docs
    for(vector<string>::iterator it = query_vocab.begin(); it != query_vocab.end(); ++it) 
    {
        bool term_aready_processed = find(processed_terms.begin(), processed_terms.end(), *it) != processed_terms.end();

        if (!term_aready_processed && vector_space.find(*it) != vector_space.end())
        {

            processed_terms.push_back(*it);

            Term* term = vector_space[*it];

            DocNode* currentDocNode = term->docNode;

            if(!docsResult->doc)
            {
                docsResult->doc = currentDocNode->doc;
            }

            while (true) {

                double accum = 0;

                for(int i=0;i< vector_space.size();i++)
                {   
                    accum += queryVectorTFIDF[i] * currentDocNode->doc->vectorTFIDF[i];
                }

                currentDocNode->doc->tempCosDistance = accum / currentDocNode->doc->norma;

                //The code below implements a linked list that contains the result docs ordered by cos distance
                DocNode* tmpDocNode = new DocNode();
                tmpDocNode->doc = currentDocNode->doc;

                DocNode* nextDocNode = docsResult;

                bool firstPosition = true; 

                while(true)
                {

                    //When a doc has the cos distance less than a doc in linked list
                    //the first doc takes the place of the second
                    if (tmpDocNode->doc->tempCosDistance > nextDocNode->doc->tempCosDistance)
                    {
                        if(nextDocNode->back)
                        {
                            nextDocNode->back->next = tmpDocNode;
                        }
                        
                        tmpDocNode->next = nextDocNode;
                        nextDocNode->back = tmpDocNode;
                        nextDocNode = tmpDocNode;

                        //This is necessary to keep the pointer pointing to the first postion of
                        //the linked list result
                        if(firstPosition)
                        {
                            docsResult = tmpDocNode;
                        }

                        break;
                    }
                    else
                    {
                        
                        firstPosition = false;

                        //If a doc has the cos distance greater than a doc in linked list
                        //check the next
                        if(nextDocNode->next)
                        {
                            if(tmpDocNode->doc == nextDocNode->doc)
                                break;

                            nextDocNode = nextDocNode->next;
                        }
                        else
                        {
                            if(tmpDocNode->doc != nextDocNode->doc)
                            {
                                nextDocNode->next = tmpDocNode;
                            }
                            break;
                        }
                    }
                }

                //Caculate the cos distance to all docs of the term inverted list
                if (currentDocNode->next) 
                {
                    currentDocNode = currentDocNode->next;
                } 
                else 
                {
                    break;
                }
            }
        }        
    }

    return docsResult;
}

int* calcPrecision(vector<string> relevantDocs, DocNode* docsResult) 
{
    int amount_relevant = 0;
    int index_result = 0;
    int* precisions = new int[10];

    while(true)
    {   
        if (docsResult->doc && find(relevantDocs.begin(), relevantDocs.end(), docsResult->doc->img) != relevantDocs.end())
        {
            // cout << docsResult->doc->img << " - " << docsResult->doc->tempCosDistance << " " << index_result << "\n";
            precisions[index_result] = ++amount_relevant;
        }
        else
        {
            precisions[index_result] = 0;   
        }

        if (docsResult->next) 
        {
            docsResult = docsResult->next;
        } 
        else 
        {
            break;
        }

        if(index_result < 9)
        {
            index_result++;
        }
        else
        {
            break;
        }
    }

    // for (int i = 0; i < 10; ++i)
    // {
    //    cout << precisions[i] << ",";
    // }

    // cout << endl;

    return precisions;
}

double calcP_10(int* precisions)
{   

    int totalRelevants = 0;

    for (int i = 0; i < 10; ++i)
    {
        if(precisions[i]>0)
            totalRelevants++;
    }

    return totalRelevants / 10.000;
}

double calcMAP(int* precisions)
{   

    double map = 0.0;

    for (int i = 0; i < 10; ++i)
    {
        if(precisions[i]>0)
        {
            map += precisions[i] / (i+1.000);
        }
    }

    return map/10.000;
}


void processQuery() 
{
    for(vector<QueryResult*>::iterator it = queryResults.begin(); it != queryResults.end(); ++it)
    {
        DocNode* docsResult = searchDocs((*it)->query);

        cout << "########## Consulta: " << (*it)->query << endl;
        
        int* precisions = calcPrecision((*it)->relevantResults,docsResult);

        cout << "P@10 - " << fixed << setprecision(3) << calcP_10(precisions) << endl;

        cout << "MAP  - " << fixed << setprecision(3) << calcMAP(precisions) << endl;

    }
}

int main(int argc, char **argv)
{

    queryResults = ProductParser::parseQueryResult();
    
    readed_docs_len = readDocuments();

    indexDocuments();

    processDocuments();

    processQuery();

    return 0;
}