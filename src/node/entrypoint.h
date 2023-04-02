#pragma once

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <termcolor/termcolor.hpp>
#include <thread>
#include <vector>

#include "../Utility.cpp"
#include "./RPCWrapperCall.h"
#include "Consensus.h"
#include "Database.h"
#include "config.h"

using termcolor::reset, termcolor::yellow, termcolor::red, termcolor::blue, termcolor::cyan, termcolor::grey;
