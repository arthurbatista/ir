#include <iostream>
#include <fstream>
#include <vector>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include <set>

#include "rapidxml/rapidxml.hpp"

using namespace rapidxml;
using namespace std;

class ProductParser
{

    public:
        /************ function alphaNumOnly *********************************\
        Description: Used to isolate the alphanumeric characters of a string

        Precondition: strToConvert is defined

        Postcondition: Returns a string
        /********************************************************************/
        static string normalizeString(string strToNormalized)
        {
          string strConverted;

          for (unsigned int i=0; i<strToNormalized.length();i++)
          {
            if (strToNormalized[i] != '.' and strToNormalized[i] != ',')
                strConverted += tolower(strToNormalized[i]);
          }

          return strConverted;
        }

        static vector<Document*> parseProducts(string path_products_desc)
        {
            vector<Document*> docs;

            set<std::string> stop_words;

            ifstream file("stop_words.txt");
            string str; 
            while (std::getline(file, str))
            {
                stop_words.insert(str);
            }

            xml_document<> doc;
            xml_node<> * root_node;
            
            // Read the xml file into a vector
            ifstream file_products(path_products_desc.c_str());
            
            vector<char> buffer((istreambuf_iterator<char>(file_products)), istreambuf_iterator<char>());
            buffer.push_back('\0');
            
            // Parse the buffer using the xml file parsing library into doc
            doc.parse<0>(&buffer[0]);
            
            // Find our root node
            root_node = doc.first_node("produtos");
            
            // Iterate over the products
            for (xml_node<> * product = root_node->first_node("produto"); product; product = product->next_sibling())
            {
                
                vector<string> doc_vocab;
                
                //Normilize title
                vector<string> xmlTitle; 
                string title = product->first_node("titulo")->value();

                istringstream iss_title(title);

                copy( istream_iterator<string>(iss_title),
                      istream_iterator<string>(),
                      back_inserter(xmlTitle) );

                for(vector<string>::iterator it = xmlTitle.begin(); it != xmlTitle.end(); ++it) 
                {
                    string item_term = normalizeString(*it);

                    if (stop_words.find(item_term) == stop_words.end())
                    {
                        doc_vocab.push_back(item_term);
                    }
                }

                //Normilize descricao
                vector<string> xmlDesc; 
                string desc = product->first_node("descricao")->value();

                istringstream iss_desc(desc);

                copy( istream_iterator<string>(iss_desc),
                      istream_iterator<string>(),
                      back_inserter(xmlDesc) );

                for(vector<string>::iterator it = xmlDesc.begin(); it != xmlDesc.end(); ++it) 
                {
                    string desc_term = normalizeString(*it);
                    if (stop_words.find(desc_term) == stop_words.end())
                    {
                        doc_vocab.push_back(desc_term);
                    }
                }

                //Read all documents and put they in memory
                Document* doc  = new Document();
                doc->img       = product->first_node("img")->value();
                doc->doc_vocab = doc_vocab;

                docs.push_back(doc);
            }

            return docs;
        }

        static vector<Document*> parseProductsImage()
        {
            vector<Document*> docs;

            for (int i = 1; i <= 7; ++i)
            {
                ostringstream chunk_file;
                chunk_file << "sdlf/chunk_" << i << ".txt";

                ifstream file(chunk_file.str().c_str());
                string str;

                while (getline(file, str))
                {
                    vector<string> img_vocab;

                    istringstream iss(str);
                        
                    copy( istream_iterator<string>( iss),
                          istream_iterator<string>(),
                          back_inserter(img_vocab) );

                    Document* doc  = new Document();
                    doc->img       = img_vocab.at(0);

                    //Remove first element since it is the image name
                    img_vocab.erase(img_vocab.begin());
                    doc->doc_vocab = img_vocab;

                    docs.push_back(doc);
                }

                file.close();
                file.clear();
            }

            return docs;
        }


        static vector<string> parseRelevants(int result_index, string folder_relevantes)
        {
            ostringstream resultFileName;
            resultFileName << folder_relevantes << result_index << "_relevante.xml";

            vector<string> results;

            xml_document<> doc;
            xml_node<> * root_node;
            
            // Read the xml file into a vector
            ifstream file(resultFileName.str().c_str());
            
            vector<char> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            buffer.push_back('\0');
            
            // Parse the buffer using the xml file parsing library into doc
            doc.parse<0>(&buffer[0]);
            
            // Find our root node
            root_node = doc.first_node("relevantes");
            
            // Iterate over the products
            for (xml_node<> * relevante = root_node->first_node("relevante"); relevante; relevante = relevante->next_sibling())
            {
                results.push_back(relevante->first_node("img")->value());
            }

            file.close();

            return results;
        }

        static vector<QueryResult*> parseQueryResult(string folder_consultas, string folder_relevantes)
        {

            vector<QueryResult*> queryResults;

            ifstream queryFile;

            for (int query_index = 1; query_index <= 50; query_index++)
            {
                ostringstream queryFileName;
                queryFileName << folder_consultas << query_index << ".txt";

                queryFile.open(queryFileName.str().c_str());

                string query;
                getline(queryFile, query);

                queryFile.close();
                queryFile.clear();

                QueryResult* queryResult = new QueryResult();
                queryResult->image = query;
                queryResult->query = query;
                queryResult->relevantResults = ProductParser::parseRelevants(query_index,folder_relevantes);

                queryResults.push_back(queryResult);
            }

            return queryResults;
        }

        static vector<QueryResult*> parseImageQueryResult(string folder_consultas, string folder_relevantes)
        {

            vector<QueryResult*> queryResults;

            ifstream queryFile("sdlf/queries.txt");
            string str;

            int query_index=1;

            ifstream queryFile_i;

            while (getline(queryFile, str))
            {
                //Retrieve image name
                ostringstream queryFileName_i;
                queryFileName_i << folder_consultas << query_index << ".txt";
                queryFile_i.open(queryFileName_i.str().c_str());
                string image;
                getline(queryFile_i, image);
                queryFile_i.close();
                queryFile_i.clear();

                //Crete QueryResult object
                QueryResult* queryResult = new QueryResult();
                queryResult->image = image;
                queryResult->query = str;
                queryResult->relevantResults = ProductParser::parseRelevants(query_index++,folder_relevantes);

                queryResults.push_back(queryResult);
            }

            queryFile.close();
            queryFile.clear();

            return queryResults;
        }
};