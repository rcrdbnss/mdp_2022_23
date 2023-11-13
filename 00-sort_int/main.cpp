#include <cstdlib>
#include <fstream>
#include <vector>
#include <algorithm>

int main(int argc, char* argv[]) {
    {
        if (argc != 3) {
            return EXIT_FAILURE;
        }

        using mytype = int; // => typedef int mytype

        std::ifstream is{ argv[1], std::ios::binary };
        std::vector<mytype> v{ 
            std::istream_iterator<mytype>{is}, 
            std::istream_iterator<mytype>{}
        };

        std::sort(v.begin(), v.end());

        {
            std::ofstream os(argv[2]);
            if (!os) {
                return EXIT_FAILURE;
            }

            for (const auto& x : v) {
                os << x << '\n';
            }
        }
    }
    _CrtDumpMemoryLeaks();
    return EXIT_SUCCESS;
}
