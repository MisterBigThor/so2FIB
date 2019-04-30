SEMAPHORES

Test[0]: [testInvalidSemSignal] SEM_INIT(invalid semaphores id: negative & out of range id) :sem_init
Test[1]: [testInitInvalidSem] SEM_INIT(an already initialized semaphore) : sem_init,sem_destroy
Test[2]: [testDestroyNotInitSem] SEM_DESTROY(an uninitialized semaphore) : sem_destroy
Test[3]: [testDestroyNotOwner] SEM_DESTROY(not owner) : fork, sem_init, sem_destroy, sem_wait, sem_signal, nice, exit
Test[4]: [testDestroyExit] EXIT DESTROY OWNED SEMS : fork, exit, sem_init, sem_signal, sem_wait
Test[5]: [testDoNotDestroyExit] EXIT DO NOT DESTROY NOT OWNED SEMS : fork, exit, sem_init, sem_signal
Test[6]: [testUnitializedSemSignal] SEM_SIGNAL(not initialized semaphore) : sem_signal
Test[7]: [testReuseSem] TEST SEMAPHORE REUSE : sem_init,sem_destroy
Test[8]: [testUnitializedSemWait] SEM_WAIT(not initialized semaphore) : sem_wait
Test[9]: [testSignalDestroyed] SEM_SIGNAL(destroyed semaphore) : sem_signal,sem_init,sem_wait, sem_destroy, getpid, fork, exit
Test[10]: [testSemFunctionality2] SEM_WAIT RETURN VALUE ON SUCCESS : sem_signal,sem_init,sem_wait, sem_destroy, getpid, fork, exit
Test[11]: [testDestroyBusySem1] SEM_DESTROY(busy semaphore) : sem_wait, sem_signal, sem_init, sem_destroy,nice
Test[12]: [testDestroyBusySem2] SEM_DESTROY(busy semaphore):unblock sem_wait : fork, sem_wait, sem_signal, sem_init, sem_destroy,nice
Test[13]: [testBlockManyProcess] SYNCHRONIZING SEVERAL PROCESSES : sem_signal,sem_init,sem_wait, sem_destroy, fork, exit
Test[14]: [testReuseSem2] TEST SEMAPHORE REUSE: sem_signal,sem_init,sem_wait, sem_destroy, fork, exit
Test[15]: [testManySem] TEST SEMAPHORES LIMITS: sem_signal,sem_init,sem_wait, sem_destroy
Test[16]: [testDestroyBusySem3] SEM_DESTROY(busy semaphore):unblock sem_wait many inits: fork, sem_wait, sem_signal, sem_init, sem_destroy

CLONE
Test[0]: [testcloneInvalidFunction] clone(invalid function address)
Test[1]: [testcloneInvalidStack] clone(invalid stack address) : exit
Test[2]: [testcloneOk] clone(OK) : exit
Test[3]: [testcloneSharesMem] clone()shares memory : exit, sem_init, sem_wait, sem_signal, sem_destroy
Test[4]: [testcloneNoSharesMem] fork()does not share memory : fork, exit, sem_init, sem_wait, sem_signal, sem_destroy
Test[5]: [testcloneManyThreads] clone many threads (single) : exit, sem_init, sem_wait, sem_signal, sem_destroy
Test[6]: [testcloneMaxThreas] clone maximum threads  : exit, sem_init, sem_wait, sem_signal, sem_destroy
Test[7]: [testcloneManyThreads2] clone many threads (maximum)  : exit, sem_init, sem_wait, sem_signal, sem_destroy
Test[8]: [testcloneForkAfterClone] fork after clone  : exit, fork, sem_init, sem_wait, sem_signal, sem_destroy
Test[9]: [testcloneForkInClone]fork inside clone  : exit, fork, sem_init, sem_wait, sem_signal, sem_destroy

	
