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

int readDocuments()
{
    docs = ProductParser::parseProducts();

    return docs.size();
}

int readDocumentsImage()
{
    docs = ProductParser::parseProductsImage();

    return docs.size();
}

void indexDocuments()
{
    //Iterate in docs
    for(vector<Document*>::iterator it_doc = docs.begin(); it_doc != docs.end(); ++it_doc) 
    {
        map<string, DocNode*> map_doc_vocab;

        for(vector<string>::iterator it = (*it_doc)->doc_vocab.begin(); it != (*it_doc)->doc_vocab.end(); ++it) {
            
            map<string, DocNode*>::iterator it_map_doc_vocab = map_doc_vocab.find(*it);

            if (it_map_doc_vocab == map_doc_vocab.end())
            {
                DocNode* docNode = new DocNode();
                docNode->doc = *it_doc;
                docNode->tf = 1;

                map_doc_vocab[*it] = docNode;

                //If the keyword doesn't exist, create a new term
                map<string, Term*>::iterator it_vector_space = vector_space.find(*it);
                if (it_vector_space == vector_space.end())
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
                    Term* tmpTerm = it_vector_space->second;
                    tmpTerm->nt++;

                    docNode->next = tmpTerm->docNode;

                    tmpTerm->docNode = docNode;
                }
            }
            else 
            {
                //Accumulate the amount of each term for the current document = TF
                it_map_doc_vocab->second->tf++;
            }
        }
    }
}

//Calculate the TFIDF vector of all documents for each term
//The documents vector should be sparse
void processDocuments()
{

    int i = 0;

    //Fill the tfidf of each document regard the idf of the term
    for (map<string, Term*>::iterator it_term=vector_space.begin(); it_term!=vector_space.end(); ++it_term)
    {

        Term* term = it_term->second;

        double idf = log2((double)docs.size()/term->nt);

        DocNode* currentDocNode = term->docNode;

        while (true) {

            double tfidf = idf * currentDocNode->tf;

            currentDocNode->doc->vectorTFIDF[term->name] = tfidf;

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
        for (map<string,double>::iterator it_tfidf=(*it_doc)->vectorTFIDF.begin(); it_tfidf!=(*it_doc)->vectorTFIDF.end(); ++it_tfidf)
        {
            (*it_doc)->norma += pow(it_tfidf->second,2);
        }

        (*it_doc)->norma = sqrt((*it_doc)->norma);
    }
}

//This method returns the tfidf vector from the query
map<string,double> processQuery(const string query)
{
    map<string, int> map_query_vocab;

    map<string,double> vectorTFIDF;

    vector<string> query_vocab;

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

    for(map<string, int>::iterator it_tfidf = map_query_vocab.begin(); it_tfidf != map_query_vocab.end(); ++it_tfidf)
    {
        map<string, Term*>::iterator it_term = vector_space.find(it_tfidf->first);

        if(it_term != vector_space.end())
        {
            Term* term = it_term->second;

            double idf = log2((double)docs.size()/term->nt);
            vectorTFIDF[term->name] = idf * it_tfidf->second;
        }
    }

    return vectorTFIDF;
}

DocNode* searchDocs(const string &query)
{
    string queryNormalized = query;
    // string queryNormalized = ProductParser::normalizeString(query);

    map<string,double> queryVectorTFIDF = processQuery(queryNormalized);

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

        map<string, Term*>::iterator it_term = vector_space.find(*it);

        if (!term_aready_processed && it_term != vector_space.end())
        {
            processed_terms.push_back(*it);

            Term* term = it_term->second;

            DocNode* currentDocNode = term->docNode;

            if(!docsResult->doc)
            {
                docsResult->doc = currentDocNode->doc;
            }

            while (true) 
            {
                double accum = 0;

                for(map<string, double>::iterator it_tfidf = queryVectorTFIDF.begin(); it_tfidf != queryVectorTFIDF.end(); ++it_tfidf)
                {
                    map<string,double>::iterator it_doc_tfidf = currentDocNode->doc->vectorTFIDF.find(it_tfidf->first);

                    if(it_doc_tfidf != currentDocNode->doc->vectorTFIDF.end())
                    {
                        accum += it_doc_tfidf->second * it_tfidf->second;
                    }
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

    for (int i = 0; i < 10; ++i)
    {
       precisions[i] = 0;
    }

    while(true)
    {   
        cout << docsResult->doc->img << " - " << docsResult->doc->tempCosDistance << "\n";

        if (docsResult->doc && find(relevantDocs.begin(), relevantDocs.end(), docsResult->doc->img) != relevantDocs.end())
        {
            cout << docsResult->doc->img << " - " << docsResult->doc->tempCosDistance << " " << index_result << "\n";
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

        cout << "########## Consulta: " << endl;//<< (*it)->query << endl;
        
        int* precisions = calcPrecision((*it)->relevantResults,docsResult);

        cout << "P@10 - " << fixed << setprecision(3) << calcP_10(precisions) << endl;

        cout << "MAP  - " << fixed << setprecision(3) << calcMAP(precisions) << endl;

    }
}

int main(int argc, char **argv)
{

    // queryResults = ProductParser::parseQueryResult();
    // readDocuments();

    queryResults = ProductParser::parseImageQueryResult();
    readDocumentsImage();

    indexDocuments();
    processDocuments();

    processQuery();

    return 0;
}