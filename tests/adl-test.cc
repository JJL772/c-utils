#include "../adlparser.hxx"
#include <getopt.h>
#include <stdio.h>
#include <string>

int main(int argc, char** argv)
{
    std::string file;
    
    int opt;
    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
            file = optarg;
            break;
        default:
            printf("USAGE: %s -f FILE\n", argv[0]);
            return -1;
        }
    }
    
    if (file.empty()) {
        printf("NO FILE!\n");
        return -1;
    }
    
    FILE* fp = fopen(file.c_str(), "rb");
    if (!fp) {
        perror("fopen");
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    auto l = ftell(fp);
    std::string buf;
    buf.resize(l+1);
    fseek(fp, 0, SEEK_SET);
    fread(buf.data(), l, 1, fp);
    
    auto* root = adl_node::parse(buf.c_str());
    
    root->dump(stdout);

    return 0;
}