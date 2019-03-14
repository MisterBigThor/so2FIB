
# HISTORIAL DE CAMBIOS 



3. ENTER THE SYSTEM

* [wrappers.S](wrappers.S) -> codigo en asm que nos proporciona un punto de entrada donde podemos guardar el contexto y hacer la operacion int 0x80 o sysenter.
* [sys.c](sys.c) -> sys_write, mecanismo de sistema al que llegaran desde la [sys_call_table.S](sys_call_table.S) y que implementa la operacion en si.
* [errno.h](include/errno.h) aqui definimos los codigos de error del sistema, que utilizamos en sys.c.
* [hardware.c](hardware.c) -> enable_int(void) hemos actualizado los bits para activar la interrupcion de reloj y de teclado
* [sys.c](sys.c) -> 

4. BASIC PROCESS MANAGEMENT
