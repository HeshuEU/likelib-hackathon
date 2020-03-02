#pragma once

#include "base/subprogram_router.hpp"

int testConnection(base::SubprogramRouter& router);
int getBalance(base::SubprogramRouter& router);
int transfer(base::SubprogramRouter& router);
int createContract(base::SubprogramRouter& router);
int messageCall(base::SubprogramRouter& router);
int compileCode(base::SubprogramRouter& router);
int generateKeys(base::SubprogramRouter& router);
