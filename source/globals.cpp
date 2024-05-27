#include "../headers/facialrecognision.h"

namespace fs = std::__fs::filesystem;
std::string directoryPath = fs::current_path();

cv::CascadeClassifier faceCascade;
std::unordered_set<std::string> currentUsers;
std::unordered_map<unsigned long, std::string> nameMappings;
std::vector<dlib::matrix<float,0,1>> faceDescriptors;
std::vector<unsigned long> labels;

