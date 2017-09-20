#include <vector>
#include <input.h>
#include <iostream>

using namespace std;

int main(int argc, char * argv[]) {
    int n; cin >> n;
    vector<string> patterns;
    read(n, patterns, n);

    for (auto e : patterns) {
        cout << e << '\n';
    }


    // int n; cin >> n;
    // ios_base::sync_with_stdio (false);

    // int state_num = create_table(argv[1]);

    // // read input data
    // FILE * fpin = fopen(argv[2], "rb");
    // if (fpin == NULL) {
    //     perror("Open input file failed.");
    //     exit(1);
    // }
    // // obtain file size:
    // fseek (fpin , 0 , SEEK_END);
    // int input_size = ftell (fpin);
    // rewind (fpin);
    // // allocate memory to contain the whole file:
    // char * input_string = (char *) malloc (sizeof(char)*input_size);

    // int * match_result = (int *) malloc (sizeof(int) * input_size);

    // PFAC_CPU(input_string, match_result, input_size);
    
    // for (int i = 0; i < input_size; i++) {
    //     if (match_result[i] != 0) {
    //         cout << match_result[i] << "\n";
    //     }
    // }

    // return 0;
}
