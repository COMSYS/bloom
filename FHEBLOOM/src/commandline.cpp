// File       commandline.cpp
// Authors    Jan Pennekamp (jan.pennekamp (at) rwth-aachen.de)
//            David Hellmans (david.hellmanns (at) rwth-aachen.de)
//            Felix Schwinger (felix.schwinger (at) rwth-aachen.de)
// Brief      Commandline class file of FHEBLOOM approach.
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

#include <sstream>
#include <iostream>
#include "commandline.h"

int send_files_commandline(const char *host, const char *user, int port, const char *local_folder, const char *remote_directory)
{
    std::stringstream commandline_call;
    commandline_call << "rsync --delete -rze 'ssh -p " << port << "' " << local_folder << " " << user << "@" << host << ":" << remote_directory;
    system(commandline_call.str().c_str());
    return 0;
}

int recv_files_commandline(const char *host, const char *user, int port, const char *local_folder, const char *remote_directory)
{
    std::stringstream commandline_call;
    commandline_call << "rsync --delete -rze 'ssh -p " << port << "' " << user << "@" << host << ":" << remote_directory << " " << local_folder;
    system(commandline_call.str().c_str());
    return 0;
}
