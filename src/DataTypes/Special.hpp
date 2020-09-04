#include <vector>
#include <string>

enum class HashFunction{
    INCLUDE,
    PUSH,
    PULL
};

struct Hash {
    std::vector<HashFunction> type;
    std::string value;
};

void Gather() {
  /* code */
}
