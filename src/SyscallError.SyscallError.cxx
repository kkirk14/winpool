
/**
 * SyscallError.SyscallError.cxx
 */



#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * SyscallError constructor
 * 
 * error: Error code returned by GetLastError
 */
SyscallError::SyscallError(DWORD error) {
    this->error = error;
}
