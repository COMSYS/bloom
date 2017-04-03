// File       fhebloom_config.cpp
// Authors    Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
//            David Hellmans (david.hellmanns (at) rwth-aachen.de)
//            Felix Schwinger (felix.schwinger (at) rwth-aachen.de)
// Brief      Config class file of FHEBLOOM approach.
// 
// Copyright  BLOOM: Bloom filter based outsourced oblivious matchings
//            Copyright (C) 2017 Communication and Distributed Systems (COMSYS), RWTH Aachen
//            
//            This program is free software: you can redistribute it and/or modify
//            it under the terms of the GNU Affero General Public License as published
//            by the Free Software Foundation, either version 3 of the License, or
//            (at your option) any later version.
//            
//            This program is distributed in the hope that it will be useful,
//            but WITHOUT ANY WARRANTY; without even the implied warranty of
//            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//            GNU Affero General Public License for more details.
//            You should have received a copy of the GNU Affero General Public License
//            along with this program. If not, see <http://www.gnu.org/licenses/>.

#include <EncryptedArray.h>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace fs = boost::filesystem;

namespace bloomLib
{

    Ctxt loadCtChunk(uint32_t chunkNo, const FHEPubKey& publicKey, string prefix)
    {
        stringstream filename;
        filename << prefix << "_ct_chunk_" << chunkNo << ".enc";
        fstream ctChunkFile(filename.str(), fstream::in);
        Ctxt ctChunk(publicKey);
        ctChunkFile >> ctChunk;
        ctChunkFile.close();
        return ctChunk;
    }

    void storeCtResult(Ctxt ctChunk, const FHEPubKey& publicKey, string prefix)
    {
        stringstream filename;
        filename << prefix << "_ct_chunk_0.enc";
        fstream ctChunkFile(filename.str(), fstream::out|fstream::trunc);
        ctChunkFile << ctChunk;
        ctChunkFile.close();
    }

    int getChunkCount(string path, string prefix)
    {
        int count = 0;
        const boost::regex my_filter(prefix + "_ct_chunk_.*\\.enc");

        fs::directory_iterator end_itr;
        for( boost::filesystem::directory_iterator i(path); i != end_itr; ++i )
        {
            // Skip if not a file
            if( !fs::is_regular_file( i->status() ) )
                continue;

            // skip if no match
            boost::smatch what;
            if( !boost::regex_match( i->path().filename().string(), what, my_filter ) )
                continue;

            // File matches, count it
            count++;
        }

        return count;
    }

    vector<string> enumerateFiles(string path, string filter)
    {
        // db-filter: "database_.*_ct_chunk_0\.enc"
        // query-filter: "query.*_ct_chunk_0\.enc"
        vector<string> filenames;
        const boost::regex my_filter(filter);

        fs::directory_iterator end_itr;
        for( boost::filesystem::directory_iterator i(path); i != end_itr; ++i )
        {
            // Skip if not a file
            if( !fs::is_regular_file( i->status() ) )
                continue;

            // skip if no match
            boost::smatch what;
            if( !boost::regex_match( i->path().filename().string(), what, my_filter ) )
                continue;

            // File matches, count it
            filenames.push_back(i->path().filename().string());
            //cout << i->path().filename().string() << endl;
        }

        std::sort(filenames.begin(),filenames.end());

        return filenames;
    }

    void execute(string name, string database_path, string query_path, string path_result, const FHEPubKey& publicKey, EncryptedArray ea)
    {
        if(fs::exists(path_result))
            fs::remove_all(path_result);
        fs::create_directory(path_result);
        
        if (!fs::exists(database_path) || !fs::exists(query_path))
        {
            cout << name << " >> No files to calculate" << endl;
            return;
        }
        
        Ctxt ctEmpty = loadCtChunk(0, publicKey, database_path + "emptyvector");
        vector<string> dbFilenames = enumerateFiles(database_path, "database_.*_ct_chunk_0\\.enc");

        // process all database entries
        #pragma omp parallel for schedule(dynamic,1)
        for (int j = 0; j < (int) dbFilenames.size(); j++)
        {
            Ctxt ctResult = ctEmpty;

            // for each chunk perform calculation
            string prefix = dbFilenames.at(j).substr(0, dbFilenames.at(j).size()-string("_ct_chunk_0.enc").length());
            int tmpPos = prefix.find("_");
            string currentBloom = prefix.substr(prefix.find("_", tmpPos+1)+1, prefix.length());
            int numberChunks = getChunkCount(database_path, prefix);

            #pragma omp critical
            cout << name << " >> Calculating: " << prefix << ".enc" << endl;

            for (int i = 0; i < numberChunks; i++)
            {
                // Load Database chunk
                Ctxt ctDbChunk = loadCtChunk(static_cast<uint32_t>(i), publicKey, database_path + prefix);

                // Load Query chunk
                Ctxt ctQueryChunk = loadCtChunk(static_cast<uint32_t>(i), publicKey, query_path + "query_" + currentBloom);

                // Perform Calculation under encryption
                ctDbChunk *= ctQueryChunk;

                // Aggregate Result per file
                ctResult += ctDbChunk;

            }
            totalSums(ea, ctResult);

            // Store result encrypted in directory
            storeCtResult(ctResult, publicKey, path_result + prefix);

            #pragma omp critical
            cout << name << " <<    Finished: " << prefix << "_result.enc" << endl;

        }
    }

    void removeFiles(string path, string filter)
    {
        const boost::regex my_filter(filter);

        fs::directory_iterator end_itr;
        for( boost::filesystem::directory_iterator i(path); i != end_itr; ++i )
        {
            // Skip if not a file
            if( !fs::is_regular_file( i->status() ) )
                continue;

            // skip if no match
            boost::smatch what;
            if( !boost::regex_match( i->path().filename().string(), what, my_filter ) )
                continue;

            // File matches, remove it
            fs::remove(i->path().string());
        }
    }
}
