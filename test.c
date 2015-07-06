#include <string.h>
#include <assert.h>
#include <ross.h>

#include "codes/lp-io.h"
#include "codes/codes.h"
#include "codes/codes_mapping.h"
#include "codes/configuration.h"
#include "codes/model-net.h"
#include "codes/lp-type-lookup.h"
#include "codes/local-storage-model.h"

// Prototypes for extern C functions:
extern double runtimeCalc(char *a, char *m, char * socket);
extern int getSockets(char *m, char*** buf);


int main(){
    int i;
    char **buf = NULL;
    int size = -1;
    char* app = "./models/fft/1D_FFT.aspen";
    char* machine = "./models/machine/llano.aspen";
    printf("%e\n", runtimeCalc(app, machine, "amd_a8_3850_cpu"));

    size = getSockets(machine,  &buf);

    if (size == -1){
        fprintf(stderr, "ERROR: No int was stored!\n");
    }
    else {
        printf("%d sockets should be returned.\n", size);
        if (!buf){
            fprintf(stderr, "ERROR: No sockets were returned!\n");
        }
        else {
            for (i = 0; i < size; i++){
                printf("%s\n", buf[i]);
            }
        }   
    }
    return 0;
}
