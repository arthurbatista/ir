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

        static vector<Document*> parseProducts()
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
            // ifstream theFile ("produtos.xml");
            ifstream theFile ("textDescDafitiPosthaus.xml");
            
            vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
            buffer.push_back('\0');
            
            // Parse the buffer using the xml file parsing library into doc
            doc.parse<0>(&buffer[0]);
            
            // Find our root node
            root_node = doc.first_node("produtos");
            
            // Iterate over the products
            for (xml_node<> * product = root_node->first_node("produto"); product; product = product->next_sibling())
            {
                
                string title = product->first_node("titulo")->value();

                istringstream iss_title(title);

                vector<string> xmlTerms; 
                
                copy( istream_iterator<string>(iss_title),
                      istream_iterator<string>(),
                      back_inserter(xmlTerms) );

                string title_normalized;

                for(vector<string>::iterator it = xmlTerms.begin(); it != xmlTerms.end(); ++it) 
                {
                    string item_term = normalizeString(*it);

                    if (stop_words.find(item_term) == stop_words.end())
                    {
                        title_normalized += item_term + " ";
                    }
                }

                string desc = product->first_node("descricao")->value();

                istringstream iss_desc(desc);

                copy( istream_iterator<string>(iss_desc),
                      istream_iterator<string>(),
                      back_inserter(xmlTerms) );

                string desc_normalized;

                for(vector<string>::iterator it = xmlTerms.begin(); it != xmlTerms.end(); ++it) 
                {
                    string desc_term = normalizeString(*it);
                    if (stop_words.find(desc_term) == stop_words.end())
                        desc_normalized += desc_term + " ";
                }

                //Read all documents and put they in memory
                Document* doc = new Document();
                doc->value    = title_normalized + desc_normalized;
                doc->img      = product->first_node("img")->value();

                docs.push_back(doc);
            }

            return docs;
        }

        static vector<string> parseResults(int result_index)
        {
            ostringstream resultFileName;
            resultFileName << "relevantes/" << result_index << "_relevante.xml";

            vector<string> results;

            xml_document<> doc;
            xml_node<> * root_node;
            
            // Read the xml file into a vector
            // ifstream theFile ("produtos.xml");
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

        static vector<QueryResult*> parseQueryResult()
        {

            vector<QueryResult*> queryResults;

            ifstream queryFile;

            for (int query_index = 1; query_index <= 50; query_index++)
            {
                ostringstream queryFileName;
                queryFileName << "consultasDafiti/" << query_index << ".txt";

                queryFile.open(queryFileName.str().c_str());

                string query;
                getline(queryFile, query);

                queryFile.close();
                queryFile.clear();

                QueryResult* queryResult = new QueryResult();
                queryResult->query = query;
                queryResult->relevantResults = ProductParser::parseResults(query_index);

                queryResults.push_back(queryResult);
            }

            return queryResults;
        }
};