#include "../headers/facialrecognision.h"

cv::CascadeClassifier faceCascade;
std::unordered_set<std::string> currentUsers;
std::unordered_map<int, std::string> nameMappings;
