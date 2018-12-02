#pragma once

#include "pushDisableWarnings.h"
#include <string>
#include "popDisableWarnings.h"

std::string urlEncode(const std::string& source);
std::string urlDecode(const std::string& source);
