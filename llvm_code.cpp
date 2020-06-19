#include "llvm_code.h"


string freshVar(){
    return "%reg"+to_string(NUM++);
}
