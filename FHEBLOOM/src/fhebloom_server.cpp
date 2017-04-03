// File       fhebloom_server.cpp
// Authors    Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
//            David Hellmans (david.hellmanns (at) rwth-aachen.de)
//            Felix Schwinger (felix.schwinger (at) rwth-aachen.de)
// Brief      Server class file of FHEBLOOM approach.
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
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include "fhebloom_config.h"

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace bl = bloomLib;

int main(int argc, char* argv[])
{

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("run", "start server")

            ("dir_server", po::value<string>()->default_value("/tmp/"), "Path to the server processing directory [default: /tmp/]")
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    if (vm.count("help"))
    {
        cout << desc << "\n";
        return 1;
    }

    if(!(fs::exists(vm["dir_server"].as<string>() + string(bl::dir_server_pubKey) + "helib_context.key") || fs::exists(vm["dir_server"].as<string>() + string(bl::dir_server_pubKey) + "helib_public.key")))
    {
        cout << "[SRV] No key files found, please upload public key" << endl;
        return -1;
    }

    fstream contextFile(vm["dir_server"].as<string>() + string(bl::dir_server_pubKey) + "helib_context.key", fstream::in);
    // Read context from file
    unsigned long m1, p1, r1;
    vector<long> gens, ords;
    readContextBase(contextFile, m1, p1, r1, gens, ords);
    FHEcontext context(m1, p1, r1, gens, ords);
    contextFile >> context;

    fstream publicFile(vm["dir_server"].as<string>() + string(bl::dir_server_pubKey) + "helib_public.key", fstream::in);
    FHEPubKey publicKey(context);
    publicFile >> publicKey;

    ZZX G = context.alMod.getFactorsOverZZ()[0];
    EncryptedArray ea(context, G);

    int numberFiles = 0; // n
    int numberChunks = 0;

    if (!vm.count("run"))
    {
        cout << "[SRV] No launch specified" << endl;
        return 0;
    }

    /*
     * Step 1: Wait for new Database Upload or Query
     *
     */

    cout << "[SRV] Server up and running" << endl;

    cout << "[SRV] ## Starting Computation!" << endl;

    bl::execute("[SRV]", vm["dir_server"].as<string>() + bl::dir_server_db, vm["dir_server"].as<string>() + bl::dir_server_qry, vm["dir_server"].as<string>() + bl::dir_server_res, publicKey, ea);

    cout << "[SRV] ## Finished Computation!" << endl;

    return 0;
}

